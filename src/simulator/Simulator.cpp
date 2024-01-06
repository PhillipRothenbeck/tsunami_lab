/**
 * @author Bohdan Babii, Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Simulator class that runs all simulations.
 **/

#include "Simulator.h"

#include <omp.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

#include "../io/Csv/Csv.h"
#include "../io/NetCDF/NetCDF.h"
#include "../patches/1d/WavePropagation1d.h"
#include "../patches/2d/WavePropagation2d.h"
#include "../setups/CheckPoint/CheckPoint.h"
#include "../timer.h"

template <typename Base, typename T>
inline bool instanceof (const T *ptr) {
    return dynamic_cast<const Base *>(ptr) != nullptr;
}

void tsunami_lab::simulator::runSimulation(tsunami_lab::setups::Setup *i_setup,
                                           tsunami_lab::t_real i_hStar,
                                           tsunami_lab::configs::SimConfig i_simConfig) {
    Timer *l_timer = new Timer();

    // define number of cells
    tsunami_lab::t_idx l_nx = i_simConfig.getXCells();
    tsunami_lab::t_idx l_ny = i_simConfig.getYCells();

    // define length of one cell
    tsunami_lab::t_real l_dx = i_simConfig.getXLength() / l_nx;
    tsunami_lab::t_real l_dy = i_simConfig.getYLength() / l_ny;

    // construct solver
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
    tsunami_lab::patches::WavePropagation *l_waveProp;

    if (i_simConfig.getDimension() == 1) {
        l_waveProp = new tsunami_lab::patches::WavePropagation1d(l_nx, i_simConfig.isRoeSolver());
    } else {
        l_waveProp = new tsunami_lab::patches::WavePropagation2d(l_nx, l_ny);
    }
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Create WaveProp Object");

    // maximum observed height in the setup
    tsunami_lab::t_real l_hMax = std::numeric_limits<tsunami_lab::t_real>::lowest();

    // set up solver
#pragma omp parallel for collapse(2) schedule(static, 32)
    for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++) {
        for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++) {
            tsunami_lab::t_real l_y = l_cy * l_dy;
            tsunami_lab::t_real l_x = l_cx * l_dx;

            // get initial values of the setup
            tsunami_lab::t_real l_h = i_setup->getHeight(l_x,
                                                         l_y);

            l_hMax = std::max(l_h, l_hMax);

            tsunami_lab::t_real l_hu = i_setup->getMomentumX(l_x,
                                                             l_y);
            tsunami_lab::t_real l_hv = i_setup->getMomentumY(l_x,
                                                             l_y);
            tsunami_lab::t_real l_b = i_setup->getBathymetry(l_x,
                                                             l_y);

            // set initial values in wave propagation solver
            l_waveProp->setHeight(l_cx,
                                  l_cy,
                                  l_h);

            l_waveProp->setMomentumX(l_cx,
                                     l_cy,
                                     l_hu);

            l_waveProp->setMomentumY(l_cx,
                                     l_cy,
                                     l_hv);

            l_waveProp->setBathymetry(l_cx,
                                      l_cy,
                                      l_b);
        }
    }
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Caculate hMax and Init WaveProp");

    // derive maximum wave speed in setup; the momentum is ignored
    tsunami_lab::t_real l_speedMax = std::sqrt(9.81 * l_hMax);

    // check if delta x is smaller than delta y
    bool l_isXStepSmaller = (l_dx <= l_dy);

    // choose l_dxy as l_dx if it is smaller or l_dy if it is smaller
    tsunami_lab::t_real l_dxy = l_dx * l_isXStepSmaller + l_dy * !l_isXStepSmaller;

    // derive constant time step; changes at simulation time are ignored
    tsunami_lab::t_real l_dt = 0.5 * l_dxy / l_speedMax;

    std::cout << l_hMax << " | " << l_speedMax << std::endl;

    std::cout << std::endl;
    std::cout << "runtime configuration" << std::endl;
    std::cout << "  number of cells in x-direction: " << l_nx << std::endl;
    std::cout << "  number of cells in y-direction: " << l_ny << std::endl;
    std::cout << "  cell size:                      " << l_dxy << std::endl;
    std::cout << "  time step:                      " << l_dt << std::endl;
    std::cout << std::endl;

    // derive scaling for a time step
    tsunami_lab::t_real l_scalingX = l_dt / l_dx;
    tsunami_lab::t_real l_scalingY = l_dt / l_dy;

    // set up time and print control
    tsunami_lab::t_idx l_frame = 0;
    tsunami_lab::t_real l_endTime = i_simConfig.getEndSimTime();
    tsunami_lab::t_real l_simTime = 0;
    if (i_simConfig.getFlagConfig().useCheckPoint()) {
        l_frame = i_simConfig.getCurrentFrame();
        l_simTime = i_simConfig.getStartSimTime();
    }
    if (i_simConfig.getDimension() == 1) {
        if (i_hStar == -1) {
            tsunami_lab::t_idx l_timeStep = 0;
            // iterate over time
            while (l_simTime < l_endTime) {
                if (l_timeStep % 25 == 0) {
                    std::cout << "  simulation time / #time steps: "
                              << l_simTime << " / " << l_timeStep << std::endl;

                    std::string l_path = "./out/solution_" + std::to_string(l_frame) + ".csv";
                    std::cout << "  writing wave field to " << l_path << std::endl;

                    std::ofstream l_file;
                    l_file.open(l_path);

                    tsunami_lab::io::Csv::write(l_dxy,
                                                l_nx,
                                                1,
                                                1,
                                                l_waveProp->getHeight(),
                                                l_waveProp->getMomentumX(),
                                                nullptr,
                                                l_waveProp->getBathymetry(),
                                                l_file);
                    l_file.close();
                    l_frame++;
                }

                l_waveProp->setGhostCells(i_simConfig.getBoundaryCondition());
                l_waveProp->timeStep(l_scalingX, 0);

                l_timeStep++;
                l_simTime += l_dt;
            }
        } else {
            tsunami_lab::t_idx l_number_of_time_steps = 100;
            bool l_is_correct_middle_state = false;
            for (tsunami_lab::t_idx l_timeStep = 0; l_timeStep < l_number_of_time_steps; l_timeStep++) {
                e_boundary l_boundary[4] = {OUTFLOW, OUTFLOW, OUTFLOW, OUTFLOW};
                l_waveProp->setGhostCells(l_boundary);
                l_waveProp->timeStep(l_scalingX, 0);

                tsunami_lab::t_real l_middle_state = l_waveProp->getHeight()[tsunami_lab::t_idx(5.0)];
                if (abs(l_middle_state - i_hStar) < 4.20) {
                    l_is_correct_middle_state = true;
                }
            }
            if (l_is_correct_middle_state) {
                std::cout << "middle state was calculated: true" << std::endl;
            } else {
                std::cout << "middle state was calculated: false" << std::endl;
            }
        }
    } else {
        std::string l_path = "./out/" + i_simConfig.getConfigName() + ".nc";
        std::cout << "  writing wave field to " << l_path << std::endl;
        t_idx l_timestepsPerFrame = 25;
        t_idx l_timeStep = l_timestepsPerFrame * l_frame;
        std::cout << l_timeStep << " > " << l_frame << std::endl;
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
        io::NetCDF *l_writer = new tsunami_lab::io::NetCDF(l_endTime,
                                                           l_dt,
                                                           l_timestepsPerFrame,
                                                           l_dxy,
                                                           l_nx,
                                                           l_ny,
                                                           l_waveProp->getStride(),
                                                           i_simConfig.getCoarseFactor(),
                                                           l_waveProp->getBathymetry(),
                                                           l_path);
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Create Writer Object");

        if (i_simConfig.getFlagConfig().useCheckPoint() && instanceof <tsunami_lab::setups::CheckPoint>(i_setup)) {
            tsunami_lab::setups::CheckPoint *l_checkpoint = (tsunami_lab::setups::CheckPoint *)i_setup;
            for (t_idx l_i = 0; l_i < l_frame; l_i++) {
                l_writer->store(l_checkpoint->getSimTimeData(l_i),
                                l_i,
                                l_checkpoint->getHeightData(l_i),
                                l_checkpoint->getMomentumXData(l_i),
                                l_checkpoint->getMomentumYData(l_i));
            }
            if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Load Checkpoint");
        }

        // iterate over time
        t_real l_checkPointTime = l_endTime / i_simConfig.getCheckPointCount();
        std::cout << l_checkPointTime << " | " << l_endTime << std::endl;
        t_idx l_checkPoints = l_simTime / l_checkPointTime;
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
        while (l_simTime < l_endTime) {
            if (l_timeStep % 25 == 0) {
                std::cout << "  simulation time / #time steps / #step: "
                          << l_simTime << " / " << l_timeStep << " / " << l_frame << std::endl;

                if (i_simConfig.getFlagConfig().useIO()) {
                    l_writer->store(l_simTime,
                                    l_frame,
                                    l_waveProp->getHeight(),
                                    l_waveProp->getMomentumX(),
                                    l_waveProp->getMomentumY());
                }

                if (i_simConfig.getFlagConfig().useCheckPoint() && l_simTime > l_checkPointTime * l_checkPoints) {
                    std::string l_checkpointPath = "./out/" + i_simConfig.getConfigName() + "_checkpoint.nc";
                    l_writer->write(l_frame, l_checkpointPath, l_simTime, l_endTime);
                    l_checkPoints++;
                }
                l_frame++;
            }
            l_waveProp->setGhostCells(i_simConfig.getBoundaryCondition());
            l_waveProp->timeStep(l_scalingX, l_scalingY);

            l_timeStep++;
            l_simTime += l_dt;
        }
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Simulation");
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
        if (i_simConfig.getFlagConfig().useIO())
            l_writer->write();
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Write NC File");
        // free memory
        std::cout << "finished time loop" << std::endl;
        std::cout << "freeing memory" << std::endl;
        delete l_writer;
    }
    delete l_waveProp;
    delete l_timer;
}

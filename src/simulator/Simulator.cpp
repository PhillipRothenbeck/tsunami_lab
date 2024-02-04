/**
 * @author Bohdan Babii
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 * @author Marek Sommerfeld (marek.sommerfeld AT uni-jena.de)
 * @author Moritz Rätz (moritz.raetz AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Simulator class that runs all simulations.
 **/

#include "Simulator.h"

#include <mpi.h>
#include <omp.h>

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

template <typename Base, typename T>
inline bool instanceof (const T *ptr) {
    return dynamic_cast<const Base *>(ptr) != nullptr;
}

void tsunami_lab::Simulator::sendData(setups::Setup *i_setup, Timer *i_timer) {
    i_timer->start();

    // define length of one cell
    t_real l_dx = m_simConfig.xLen / m_grid.globalNX;
    t_real l_dy = m_simConfig.yLen / m_grid.globalNY;

    // init sub-domain on rank 0 for holding the values of other processes
    alignas(4) t_real *l_tempHeight = new t_real[m_localSize];
    alignas(4) t_real *l_tempMomentumX = new t_real[m_localSize];
    alignas(4) t_real *l_tempMomentumY = new t_real[m_localSize];
    alignas(4) t_real *l_tempBathymetry = new t_real[m_localSize];

    // maximum observed height in the setup
    t_real l_hMax = std::numeric_limits<t_real>::lowest();

    for (int l_processID = 0; l_processID < m_parallelData.size; l_processID++) {
        t_idx l_processOffsetY = l_processID % m_parallelData.yDim * m_grid.localNY;
        t_idx l_processOffsetX = floor(l_processID / m_parallelData.yDim) * m_grid.localNX;
#pragma omp parallel for collapse(2) schedule(static, 8) reduction(max : l_hMax)
        for (t_idx l_cy = 0; l_cy < m_grid.localNY; l_cy++) {
            for (t_idx l_cx = 0; l_cx < m_grid.localNX; l_cx++) {
                t_real l_y = (l_cy + l_processOffsetY) * l_dy;
                t_real l_x = (l_cx + l_processOffsetX) * l_dx;

                // get initial values of the setup
                t_real l_h = i_setup->getHeight(l_x, l_y);
                t_real l_hu = i_setup->getMomentumX(l_x, l_y);
                t_real l_hv = i_setup->getMomentumY(l_x, l_y);
                t_real l_b = i_setup->getBathymetry(l_x, l_y);
                t_idx l_idx = (l_cy + 1) * (m_grid.localNX + 2) + (l_cx + 1);

                l_hMax = l_hMax < l_h ? l_h : l_hMax;

                if (l_processID == 0) {
                    // if rank 0 then set the values directly into the local sub-grid
                    m_height[l_idx] = l_h;
                    m_momentumX[l_idx] = l_hu;
                    m_momentumY[l_idx] = l_hv;
                    m_bathymetry[l_idx] = l_b;
                } else {
                    // else set the values into the temporary arrays
                    l_tempHeight[l_idx] = l_h;
                    l_tempMomentumX[l_idx] = l_hu;
                    l_tempMomentumY[l_idx] = l_hv;
                    l_tempBathymetry[l_idx] = l_b;
                }
            }
        }

        // send the sub-grid to the accoring process
        if (l_processID != 0) {
            int l_error = MPI_SUCCESS;

            l_error = MPI_Isend(l_tempHeight, 1, m_parallelData.subgrid, l_processID, 0, m_parallelData.communicator, &m_parallelData.subgridRequest[0]);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Isend(l_tempMomentumX, 1, m_parallelData.subgrid, l_processID, 1, m_parallelData.communicator, &m_parallelData.subgridRequest[1]);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Isend(l_tempMomentumY, 1, m_parallelData.subgrid, l_processID, 2, m_parallelData.communicator, &m_parallelData.subgridRequest[2]);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Isend(l_tempBathymetry, 1, m_parallelData.subgrid, l_processID, 3, m_parallelData.communicator, &m_parallelData.subgridRequest[3]);
            assert(l_error == MPI_SUCCESS);

            l_error = MPI_Waitall(4, m_parallelData.subgridRequest, MPI_STATUS_IGNORE);
            assert(l_error == MPI_SUCCESS);
            std::cout << "Rank 0: Successfully send the subgrid to process " << l_processID << std::endl;
        }
    }

    i_timer->printTime("domain decomposition", 0);

    // derive maximum wave speed in setup; the momentum is ignored
    t_real l_speedMax = std::sqrt(9.81 * l_hMax);

    // check if delta x is smaller than delta y
    bool l_isXStepSmaller = (l_dx <= l_dy);

    // choose l_dxy as l_dx if it is smaller or l_dy if it is smaller
    m_dxy = l_dx * l_isXStepSmaller + l_dy * !l_isXStepSmaller;

    // derive constant time step; changes at simulation time are ignored
    m_dt = 0.5 * m_dxy / l_speedMax;

    std::cout << std::endl;
    std::cout << "runtime configuration" << std::endl;
    std::cout << "  number of cells in x-direction:         " << m_grid.globalNX << std::endl;
    std::cout << "  number of cells in y-direction:         " << m_grid.globalNY << std::endl;
    std::cout << "  number of (local) cells in x-direction: " << m_grid.localNX << std::endl;
    std::cout << "  number of (local) cells in y-direction: " << m_grid.localNY << std::endl;
    std::cout << "  cell size:                              " << m_dxy << std::endl;
    std::cout << "  time step:                              " << m_dt << std::endl;
    std::cout << std::endl;

    // derive scaling for a time step
    m_scalingX = m_dt / l_dx;
    m_scalingY = m_dt / l_dy;

    m_endTime = m_simConfig.endSimTime;

    // send important values for the simulation to the other processes
    for (int l_processID = 1; l_processID < m_parallelData.size; l_processID++) {
        int l_error = MPI_SUCCESS;

        l_error = MPI_Send(&m_dxy, 1, MPI_FLOAT, l_processID, 4, m_parallelData.communicator);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Send(&m_dt, 1, MPI_FLOAT, l_processID, 5, m_parallelData.communicator);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Send(&m_scalingX, 1, MPI_FLOAT, l_processID, 6, m_parallelData.communicator);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Send(&m_scalingY, 1, MPI_FLOAT, l_processID, 7, m_parallelData.communicator);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Send(&m_endTime, 1, MPI_FLOAT, l_processID, 8, m_parallelData.communicator);
        assert(l_error == MPI_SUCCESS);
    }

    delete[] l_tempHeight;
    delete[] l_tempMomentumX;
    delete[] l_tempMomentumY;
    delete[] l_tempBathymetry;
}

void tsunami_lab::Simulator::recieveData(Timer *i_timer) {
    int l_error = MPI_SUCCESS;

    l_error = MPI_Irecv(m_height, m_localSize, MPI_FLOAT, 0, 0, m_parallelData.communicator, &m_parallelData.subgridRequest[0]);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Irecv(m_momentumX, m_localSize, MPI_FLOAT, 0, 1, m_parallelData.communicator, &m_parallelData.subgridRequest[1]);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Irecv(m_momentumY, m_localSize, MPI_FLOAT, 0, 2, m_parallelData.communicator, &m_parallelData.subgridRequest[2]);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Irecv(m_bathymetry, m_localSize, MPI_FLOAT, 0, 3, m_parallelData.communicator, &m_parallelData.subgridRequest[3]);
    assert(l_error == MPI_SUCCESS);

    l_error = MPI_Waitall(4, m_parallelData.subgridRequest, MPI_STATUS_IGNORE);
    assert(l_error == MPI_SUCCESS);
    i_timer->printTime("recieving data", m_parallelData.rank);

    l_error = MPI_Recv(&m_dxy, 1, MPI_FLOAT, 0, 4, m_parallelData.communicator, MPI_STATUS_IGNORE);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Recv(&m_dt, 1, MPI_FLOAT, 0, 5, m_parallelData.communicator, MPI_STATUS_IGNORE);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Recv(&m_scalingX, 1, MPI_FLOAT, 0, 6, m_parallelData.communicator, MPI_STATUS_IGNORE);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Recv(&m_scalingY, 1, MPI_FLOAT, 0, 7, m_parallelData.communicator, MPI_STATUS_IGNORE);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Recv(&m_endTime, 1, MPI_FLOAT, 0, 8, m_parallelData.communicator, MPI_STATUS_IGNORE);
    assert(l_error == MPI_SUCCESS);
}

void tsunami_lab::Simulator::runSimulation(setups::Setup *i_setup,
                                           configs::SimConfig i_simConfig,
                                           MPIKernel::ParallelData i_parallelData,
                                           tsunami_lab::MPIKernel::GridData i_grid) {
    m_simConfig = i_simConfig;
    m_parallelData = i_parallelData;
    m_grid = i_grid;

    // define basic variables
    m_nx = m_grid.globalNX;
    m_ny = m_grid.globalNY;
    m_localSize = (m_grid.localNX + 2) * (m_grid.localNY + 2);

    Timer *l_timer = new Timer(m_simConfig.flagConfig.useTiming());

    // initialize local subgrids for each MPI process
    m_height = new t_real[m_localSize];
    m_momentumX = new t_real[m_localSize];
    m_momentumY = new t_real[m_localSize];
    m_bathymetry = new t_real[m_localSize];

    if (m_parallelData.rank == 0) {
        sendData(i_setup, l_timer);
    } else {
        // wait until all data has been recieved
        recieveData(l_timer);
    }

    l_timer->start();

    patches::WavePropagation2d l_waveProp(m_grid.localNX,
                                          m_grid.localNY,
                                          m_parallelData,
                                          m_height,
                                          m_momentumX,
                                          m_momentumY,
                                          m_bathymetry);

    l_timer->printTime("initializing solver", m_parallelData.rank);

    // set up time and print control
    t_idx l_frame = 0;
    t_real l_simTime = 0;
    t_idx l_timestepsPerFrame = 25;
    t_idx l_timeStep = l_timestepsPerFrame * l_frame;

    std::string l_path = "./out/" + m_simConfig.configName + "_" + std::to_string(m_parallelData.rank) + ".nc";

    l_timer->start();

    int l_coordinates[2];
    MPI_Cart_coords(m_parallelData.communicator, m_parallelData.rank, 2, l_coordinates);

    io::NetCDF l_writer(m_endTime,
                        m_dt,
                        l_timestepsPerFrame,
                        l_coordinates,
                        m_dxy,
                        m_grid.localNX,
                        m_grid.localNY,
                        l_waveProp.getStride(),
                        m_simConfig.coarseFactor,
                        l_waveProp.getBathymetry(),
                        l_path);

    l_timer->printTime("creating writer object", m_parallelData.rank);

    // run simulation
    std::cout << "Rank " << m_parallelData.rank << ": Simulation started" << std::endl;
    while (l_simTime < m_endTime) {
        if (l_timeStep % 25 == 0) {
            if (m_parallelData.rank == 0) {
                std::cout << "  simulation time / #time steps / #step: "
                          << l_simTime << " / " << l_timeStep << " / " << l_frame << std::endl;
            }

            if (m_simConfig.flagConfig.useIO()) {
                l_writer.store(l_simTime,
                               l_frame,
                               l_waveProp.getHeight(),
                               l_waveProp.getMomentumX(),
                               l_waveProp.getMomentumY());
            }

            l_frame++;
        }
        l_waveProp.timeStep(m_scalingX, m_scalingY);

        l_timeStep++;
        l_simTime += m_dt;
    }

    l_timer->printTime("simulation", m_parallelData.rank);

    if (m_simConfig.flagConfig.useIO())
        l_writer.write(m_parallelData.rank);

    l_timer->printTime("writing output", m_parallelData.rank);

    std::cout << "Rank " << std::to_string(m_parallelData.rank) << ": finished time loop" << std::endl;

    // free memory
    delete l_timer;
}

/**
 * @author Bohdan Babii, Phillip Rothenbeck
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

#include "../allocator.h"

template <typename Base, typename T>
inline bool instanceof (const T *ptr) {
    return dynamic_cast<const Base *>(ptr) != nullptr;
}

void tsunami_lab::simulator::runSimulation(setups::Setup *i_setup,
                                           configs::SimConfig i_simConfig,
                                           MPIKernel::ParallelData i_parallelData,
                                           tsunami_lab::MPIKernel::Grid i_grid) {
    int l_error = MPI_SUCCESS;
    t_idx l_nx = i_grid.globalNX;
    t_idx l_ny = i_grid.globalNY;
    // important variable that need to be globally available
    t_real l_dxy, l_dt, l_scalingX, l_scalingY, l_endTime;
    // size of local domain
    long int l_localSize = (i_grid.localNX + 2) * (i_grid.localNY + 2);

    // initialize local subgrids for each MPI process
    auto *l_height = tsunami_lab::aligned_alloc_real(l_localSize);
    auto *l_momentumX = tsunami_lab::aligned_alloc_real(l_localSize);
    auto *l_momentumY = tsunami_lab::aligned_alloc_real(l_localSize);
    auto *l_bathymetry = tsunami_lab::aligned_alloc_real(l_localSize);

    // define number of cells
    if (i_parallelData.rank == 0) {
        // define length of one cell
        t_real l_dx = i_simConfig.getXLength() / l_nx;
        t_real l_dy = i_simConfig.getYLength() / l_ny;

        // init sub-domain on rank 0 for holding the values of other processes
        auto *l_tempHeight = tsunami_lab::aligned_alloc_real(l_localSize);
        auto *l_tempMomentumX = tsunami_lab::aligned_alloc_real(l_localSize);
        auto *l_tempMomentumY = tsunami_lab::aligned_alloc_real(l_localSize);
        auto *l_tempBathymetry = tsunami_lab::aligned_alloc_real(l_localSize);

        // maximum observed height in the setup
        t_real l_hMax = std::numeric_limits<t_real>::lowest();

        for (int l_processID = 0; l_processID < i_parallelData.size; l_processID++) {
            t_idx l_processOffsetY = l_processID % i_parallelData.yDim * i_grid.localNY;
            t_idx l_processOffsetX = floor(l_processID / i_parallelData.yDim) * i_grid.localNX;
            // #pragma omp parallel for collapse(2) schedule(static, 8) reduction(max : l_hMax)
            for (t_idx l_cy = 0; l_cy < i_grid.localNY; l_cy++) {
                for (t_idx l_cx = 0; l_cx < i_grid.localNX; l_cx++) {
                    t_real l_y = (l_cy + l_processOffsetY) * l_dy;
                    t_real l_x = (l_cx + l_processOffsetX) * l_dx;

                    // get initial values of the setup
                    t_real l_h = i_setup->getHeight(l_x,
                                                    l_y);

                    l_hMax = l_hMax < l_h ? l_h : l_hMax;

                    t_real l_hu = i_setup->getMomentumX(l_x,
                                                        l_y);
                    t_real l_hv = i_setup->getMomentumY(l_x,
                                                        l_y);
                    t_real l_b = i_setup->getBathymetry(l_x,
                                                        l_y);
                    t_idx l_idx = (l_cy + 1) * (i_grid.localNX + 2) + (l_cx + 1);

                    if (l_processID == 0) {  // if rank 0 then set the values directly into the local sub-grid
                        l_height[l_idx] = l_h;
                        l_momentumX[l_idx] = l_hu;
                        l_momentumY[l_idx] = l_hv;
                        l_bathymetry[l_idx] = l_b;
                    } else {  // else set the values into the temporary arrays
                        l_tempHeight[l_idx] = l_h;
                        l_tempMomentumX[l_idx] = l_hu;
                        l_tempMomentumY[l_idx] = l_hv;
                        l_tempBathymetry[l_idx] = l_b;
                    }
                }
            }

            // send the sub-grid to the accoring process
            if (l_processID != 0) {
                l_error = MPI_Send(l_tempHeight, l_localSize, MPI_FLOAT, l_processID, 0, i_parallelData.communicator);
                assert(l_error == MPI_SUCCESS);
                l_error = MPI_Send(l_tempMomentumX, l_localSize, MPI_FLOAT, l_processID, 1, i_parallelData.communicator);
                assert(l_error == MPI_SUCCESS);
                l_error = MPI_Send(l_tempMomentumY, l_localSize, MPI_FLOAT, l_processID, 2, i_parallelData.communicator);
                assert(l_error == MPI_SUCCESS);
                l_error = MPI_Send(l_tempBathymetry, l_localSize, MPI_FLOAT, l_processID, 3, i_parallelData.communicator);
                assert(l_error == MPI_SUCCESS);
                std::cout << "Successfully recieved the subgrid to " << l_processID << std::endl;
            }
        }

        // derive maximum wave speed in setup; the momentum is ignored
        t_real l_speedMax = std::sqrt(9.81 * l_hMax);

        // check if delta x is smaller than delta y
        bool l_isXStepSmaller = (l_dx <= l_dy);

        // choose l_dxy as l_dx if it is smaller or l_dy if it is smaller
        l_dxy = l_dx * l_isXStepSmaller + l_dy * !l_isXStepSmaller;

        // derive constant time step; changes at simulation time are ignored
        l_dt = 0.5 * l_dxy / l_speedMax;

        std::cout << std::endl;
        std::cout << "runtime configuration" << std::endl;
        std::cout << "  number of cells in x-direction:         " << l_nx << std::endl;
        std::cout << "  number of cells in y-direction:         " << l_ny << std::endl;
        std::cout << "  number of (local) cells in x-direction: " << i_grid.localNX << std::endl;
        std::cout << "  number of (local) cells in y-direction: " << i_grid.localNY << std::endl;
        std::cout << "  cell size:                              " << l_dxy << std::endl;
        std::cout << "  time step:                              " << l_dt << std::endl;
        std::cout << std::endl;

        // derive scaling for a time step
        l_scalingX = l_dt / l_dx;
        l_scalingY = l_dt / l_dy;

        l_endTime = i_simConfig.getEndSimTime();

        // send important values for the simulation to the other processes
        for (int l_processID = 1; l_processID < i_parallelData.size; l_processID++) {
            l_error = MPI_Send(&l_dxy, 1, MPI_FLOAT, l_processID, 4, i_parallelData.communicator);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Send(&l_dt, 1, MPI_FLOAT, l_processID, 5, i_parallelData.communicator);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Send(&l_scalingX, 1, MPI_FLOAT, l_processID, 6, i_parallelData.communicator);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Send(&l_scalingY, 1, MPI_FLOAT, l_processID, 7, i_parallelData.communicator);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Send(&l_endTime, 1, MPI_FLOAT, l_processID, 8, i_parallelData.communicator);
            assert(l_error == MPI_SUCCESS);
        }

        delete[] l_tempHeight;
        delete[] l_tempMomentumX;
        delete[] l_tempMomentumY;
        delete[] l_tempBathymetry;
    } else {
        // wait until all data has been recieved
        l_error = MPI_Recv(l_height, l_localSize, MPI_FLOAT, 0, 0, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(l_momentumX, l_localSize, MPI_FLOAT, 0, 1, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(l_momentumY, l_localSize, MPI_FLOAT, 0, 2, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(l_bathymetry, l_localSize, MPI_FLOAT, 0, 3, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(&l_dxy, 1, MPI_FLOAT, 0, 4, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(&l_dt, 1, MPI_FLOAT, 0, 5, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(&l_scalingX, 1, MPI_FLOAT, 0, 6, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(&l_scalingY, 1, MPI_FLOAT, 0, 7, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(&l_endTime, 1, MPI_FLOAT, 0, 8, i_parallelData.communicator, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
    }

    patches::WavePropagation2d l_waveProp(i_grid.localNX,
                                          i_grid.localNY,
                                          i_parallelData,
                                          l_height,
                                          l_momentumX,
                                          l_momentumY,
                                          l_bathymetry);

    // set up time and print control

    t_idx l_frame = 0;
    t_real l_simTime = 0;
    t_idx l_timestepsPerFrame = 25;
    t_idx l_timeStep = l_timestepsPerFrame * l_frame;

    //  if (i_simConfig.getFlagConfig().useCheckPoint()) {
    //      l_frame = i_simConfig.getCurrentFrame();
    //      l_simTime = i_simConfig.getStartSimTime();
    //  }
    std::string l_path = "./out/" + i_simConfig.getConfigName() + "_" + std::to_string(i_parallelData.rank) + ".nc";
    std::cout << "  writing wave field to " << l_path << std::endl;

    //  std::cout << l_timeStep << " > " << l_frame << std::endl;
    //  if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();

    int l_coordinates[2];
    MPI_Cart_coords(i_parallelData.communicator, i_parallelData.rank, 2, l_coordinates);

    io::NetCDF l_writer(l_endTime,
                        l_dt,
                        l_timestepsPerFrame,
                        l_coordinates,
                        l_dxy,
                        i_grid.localNX,
                        i_grid.localNY,
                        l_waveProp.getStride(),
                        i_simConfig.getCoarseFactor(),
                        l_waveProp.getBathymetry(),
                        l_path);

    //  if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Create Writer Object");

    //  if (i_simConfig.getFlagConfig().useCheckPoint() && instanceof <setups::CheckPoint>(i_setup)) {
    //      setups::CheckPoint *l_checkpoint = (setups::CheckPoint *)i_setup;
    //      for (t_idx l_i = 0; l_i < l_frame; l_i++) {
    //          l_writer->store(l_checkpoint->getSimTimeData(l_i),
    //                          l_i,
    //                          l_checkpoint->getHeightData(l_i),
    //                          l_checkpoint->getMomentumXData(l_i),
    //                          l_checkpoint->getMomentumYData(l_i));
    //      }
    //      if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Load Checkpoint");
    //  }

    //  // iterate over time
    //  t_real l_checkPointTime = l_endTime / i_simConfig.getCheckPointCount();
    //  std::cout << l_checkPointTime << " | " << l_endTime << std::endl;
    //  t_idx l_checkPoints = l_simTime / l_checkPointTime;
    //  if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();

    std::cout << i_parallelData.rank << " starts to simulate." << std::endl;
    while (l_simTime < l_endTime) {
        if (l_timeStep % 25 == 0) {
            if (i_parallelData.rank == 0) {
                std::cout << "  rank " << i_parallelData.rank << ": simulation time / #time steps / #step: "
                          << l_simTime << " / " << l_timeStep << " / " << l_frame << std::endl;
            }

            if (i_simConfig.getFlagConfig().useIO()) {
                l_writer.store(l_simTime,
                               l_frame,
                               l_waveProp.getHeight(),
                               l_waveProp.getMomentumX(),
                               l_waveProp.getMomentumY());
            }

            // if (i_simConfig.getFlagConfig().useCheckPoint() && l_simTime > l_checkPointTime * l_checkPoints) {
            //     std::string l_checkpointPath = "./out/" + i_simConfig.getConfigName() + "_checkpoint.nc";
            //     l_writer->write(l_frame, l_checkpointPath, l_simTime, l_endTime);
            //     l_checkPoints++;
            // }
            l_frame++;
        }
        // l_waveProp.setGhostCells(i_simConfig.getBoundaryCondition());
        l_waveProp.timeStep(l_scalingX, l_scalingY);

        l_timeStep++;
        l_simTime += l_dt;
    }

    //  if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Simulation");
    //  if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
    if (i_simConfig.getFlagConfig().useIO())
        l_writer.write();
    //  if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Write NC File");

    // free memory
    std::cout << "finished time loop" << std::endl;
    std::cout << "freeing memory" << std::endl;
    //  delete l_writer;
    // delete l_waveProp;
    // delete l_timer;
}

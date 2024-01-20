/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/
#include <mpi.h>
#include <cassert>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>

#include "MPIKernel/MPIKernel.h"
#include "configs/FlagConfig.h"
#include "configs/SimConfig.h"
#include "io/Json/ConfigLoader.h"
#include "simulator/Simulator.h"
#include "timer.h"

int main(int i_argc, char *i_argv[]) {
    // MPI_Init and error handling
    int l_error = MPI_Init(&i_argc, &i_argv);
    assert(l_error == MPI_SUCCESS);
    
    // get rank and communicator size and error handling
    int l_rank, l_worldSize;
    int l_nx, l_ny;
    l_error = MPI_Comm_size(MPI_COMM_WORLD, &l_worldSize);
    assert(l_error == MPI_SUCCESS);
    l_error =MPI_Comm_rank(MPI_COMM_WORLD, &l_rank);
    assert(l_error == MPI_SUCCESS);
    
    // timer and setup
    Timer *l_timer = new Timer();
    tsunami_lab::setups::Setup *l_setups = nullptr;

    // only rank 0 executes this part
    if (l_rank == 0) {
        std::cout << "####################################" << std::endl;
        std::cout << "### Tsunami Lab                  ###" << std::endl;
        std::cout << "###                              ###" << std::endl;
        std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
        std::cout << "####################################" << std::endl;

        if (i_argc < 2) {
            std::cerr << "invalid number of program parameter" << std::endl;
            std::cerr << "  ./build/tsunami_lab CONFIG_FILE_NAME.json" << std::endl;
            return EXIT_FAILURE;
        }

        std::string l_configName = std::string(i_argv[1]);

        tsunami_lab::configs::FlagConfig l_flagConfig = tsunami_lab::configs::FlagConfig();
        if (i_argc > 2) {
            for (int l_arguments = 1; l_arguments < i_argc; l_arguments++) {
                if (std::string(i_argv[l_arguments]).compare("-c") == 0) {
                    l_flagConfig.setUseCheckPoint(true);
                } else if (std::string(i_argv[l_arguments]).compare("-t") == 0) {
                    l_flagConfig.setUseTiming(true);
                } else if (std::string(i_argv[l_arguments]).compare("-nio") == 0) {
                    l_flagConfig.setUseIO(false);
                }
            }
        }

        tsunami_lab::configs::SimConfig l_simConfig = tsunami_lab::configs::SimConfig();

        if (l_flagConfig.useTiming()) l_timer->start();
        // load parameters from runtimeConfig.json
        tsunami_lab::t_idx err = tsunami_lab::io::ConfigLoader::loadConfig(l_configName,
                                                                           l_flagConfig,
                                                                           l_setups,
                                                                           l_simConfig);
        if (l_flagConfig.useTiming()) l_timer->printTime("Loading Config ");

        if (err != 0) {
            std::cout << "failed to read: " << l_configName << std::endl;
            delete l_setups;
            delete l_timer;
            return EXIT_FAILURE;
        }
        for (int i = 1; i < l_worldSize; i++) {
            l_nx = l_simConfig.getXCells(); // muss das jedes mal neu gefetched werden?
            l_ny = l_simConfig.getYCells(); // muss das jedes mal neu gefetched werden?
            
            // Send x / y to every process != 0
            l_error = MPI_Send(&l_nx, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            assert(l_error == MPI_SUCCESS);
            l_error = MPI_Send(&l_ny, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            assert(l_error == MPI_SUCCESS);
        }
    } else {
        // Receive x / y from process 0
        l_error = MPI_Recv(&l_nx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Recv(&l_ny, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
    }

    std::cout << "Process with rank " << l_rank << " has nx: " << l_nx << " and ny: " << l_ny << std::endl;

    // create ParralelData struct
    // tsunami_lab::MPIKernel::ParallelData *l_parallelData = new tsunami_lab::MPIKernel::ParallelData();
    tsunami_lab::MPIKernel::ParallelData l_parallelData;
    
    // init parallel data
    tsunami_lab::MPIKernel::initParallelData(l_nx, l_ny, &l_parallelData);
    std::cout << l_parallelData.rank << " has neighbors (up, down, left, right): " << l_parallelData.up << ", " << l_parallelData.down << ", " << l_parallelData.left << ", " << l_parallelData.right << std::endl;
    // start simulation from config
    // tsunami_lab::simulator::runSimulation(l_setups, l_simConfig);

    delete l_setups;
    delete l_timer;

    // free parallel datatypes
    tsunami_lab::MPIKernel::freeParallelData(&l_parallelData);
    // delete l_parallelData;
    std::cout << "finished, exiting" << std::endl;
    
    l_error = MPI_Finalize();
    assert(l_error == MPI_SUCCESS);
    return EXIT_SUCCESS;
}

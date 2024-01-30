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
    tsunami_lab::t_idx l_nx, l_ny;
    l_error = MPI_Comm_size(MPI_COMM_WORLD, &l_worldSize);
    assert(l_error == MPI_SUCCESS);
    l_error = MPI_Comm_rank(MPI_COMM_WORLD, &l_rank);
    assert(l_error == MPI_SUCCESS);

    // timer and setup
    Timer *l_timer = new Timer();
    tsunami_lab::setups::Setup *l_setup = nullptr;
    tsunami_lab::configs::SimConfig l_simConfig = tsunami_lab::configs::SimConfig();

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

    if (l_flagConfig.useTiming()) l_timer->start();
    // load parameters from runtimeConfig.json
    tsunami_lab::t_idx err = tsunami_lab::io::ConfigLoader::loadConfig(l_rank,
                                                                       l_configName,
                                                                       l_flagConfig,
                                                                       l_setup,
                                                                       l_simConfig);
    if (l_flagConfig.useTiming()) l_timer->printTime("Loading Config");

    if (err != 0) {
        std::cout << "failed to read: " << l_configName << std::endl;
        delete l_setup;
        delete l_timer;
        return EXIT_FAILURE;
    }

    l_nx = l_simConfig.getXCells();
    l_ny = l_simConfig.getYCells();

    // create ParralelData struct
    tsunami_lab::MPIKernel::ParallelData l_parallelData;
    tsunami_lab::MPIKernel::Grid l_grid;

    // init parallel data
    tsunami_lab::MPIKernel::init(l_nx, l_ny, &l_parallelData, &l_grid);

    // start simulation from config
    tsunami_lab::simulator::runSimulation(l_setup, l_simConfig, l_parallelData, l_grid);

    delete l_setup;
    delete l_timer;

    // free parallel datatypes
    tsunami_lab::MPIKernel::freeParallelData(&l_parallelData);
    std::cout << "finished, exiting" << std::endl;

    l_error = MPI_Finalize();
    assert(l_error == MPI_SUCCESS);
    return EXIT_SUCCESS;
}

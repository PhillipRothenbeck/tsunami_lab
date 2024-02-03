/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 * @author Marek Sommerfeld (marek.sommerfeld AT uni-jena.de)
 * @author Moritz Rätz (moritz.raetz AT uni-jena.de)
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

    Timer *l_timer = new Timer(l_flagConfig.useTiming());

    // load parameters from runtimeConfig.json
    tsunami_lab::t_idx err = tsunami_lab::io::ConfigLoader::loadConfig(l_rank,
                                                                       l_configName,
                                                                       l_flagConfig,
                                                                       l_setup,
                                                                       l_simConfig);
    l_timer->printTime("loading config", l_rank);

    if (err != 0) {
        std::cout << "failed to read: " << l_configName << std::endl;
        delete l_timer;
        return EXIT_FAILURE;
    }

    l_nx = l_simConfig.getXCells();
    l_ny = l_simConfig.getYCells();

    // init parallel data
    tsunami_lab::MPIKernel::ParallelData l_parallelData;
    tsunami_lab::MPIKernel::GridData l_grid;

    tsunami_lab::MPIKernel::init(l_nx, l_ny, &l_parallelData, &l_grid);

    // start simulation from config
    tsunami_lab::Simulator simulator = tsunami_lab::Simulator();
    simulator.runSimulation(l_setup, l_simConfig, l_parallelData, l_grid);

    // free memory
    delete l_timer;

    // free parallel datatypes
    tsunami_lab::MPIKernel::freeTypes(&l_parallelData);

    l_error = MPI_Finalize();
    assert(l_error == MPI_SUCCESS);

    std::cout << "Rank " << l_rank << ": finished and exiting" << std::endl;
    return EXIT_SUCCESS;
}

/**
 * @author Bohdan Babii, Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Simulator class that runs all simulations.
 **/
#ifndef TSUNAMI_LAB_SIMULATOR_SIMULATOR
#define TSUNAMI_LAB_SIMULATOR_SIMULATOR

#include <cstdlib>
#include <string>

#include "../MPIKernel/MPIKernel.h"
#include "../configs/SimConfig.h"
#include "../constants.h"
#include "../io/Csv/Csv.h"
#include "../io/NetCDF/NetCDF.h"
#include "../patches/2d/WavePropagation2d.h"
#include "../setups/CheckPoint/CheckPoint.h"
#include "../setups/Setup.h"
#include "../timer.h"

namespace tsunami_lab {
    class Simulator;
}

class tsunami_lab::Simulator {
   public:
    static void runSimulation(tsunami_lab::setups::Setup *i_setup,
                              tsunami_lab::configs::SimConfig i_simConfig,
                              tsunami_lab::MPIKernel::ParallelData i_parallelData,
                              tsunami_lab::MPIKernel::Grid i_grid);
};

#endif
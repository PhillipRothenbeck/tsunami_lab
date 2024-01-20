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

#include "../configs/SimConfig.h"
#include "../constants.h"
#include "../io/Csv/Csv.h"
#include "../io/NetCDF/NetCDF.h"
#include "../patches/2d/WavePropagation2d.h"
#include "../setups/CheckPoint/CheckPoint.h"
#include "../setups/Setup.h"
#include "../timer.h"

namespace tsunami_lab {
    class simulator;
}

struct ParallelData {
    tsunami_lab::t_idx size;
    tsunami_lab::t_idx rank;
    tsunami_lab::t_idx up, down, left, right;
    MPI_Comm communicator;
    MPI_Request requesst[8];
    MPI_Datatype row;
    MPI_Datatype column;
    MPI_Datatype text;
    MPI_Datatype file;
    MPI_Datatype restart;
};

struct Grid {
    tsunami_lab::t_idx localNX;
    tsunami_lab::t_idx localNY;
    tsunami_lab::t_idx globalNX;
    tsunami_lab::t_idx globalNY;
    tsunami_lab::t_real dX;
    tsunami_lab::t_real dY;
    tsunami_lab::t_real *height;
    tsunami_lab::t_real *momentumX;
    tsunami_lab::t_real *momentumY;
    tsunami_lab::t_real *bathymetry;
};

class tsunami_lab::simulator {
   private:
    // static void initParallelData(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData);

   public:
    static void runSimulation(tsunami_lab::setups::Setup *i_setup,
                              tsunami_lab::configs::SimConfig i_simConfig);
};

#endif
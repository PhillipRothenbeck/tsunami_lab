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
#include "../setups/Setup.h"

namespace tsunami_lab {
    class simulator;
}

class tsunami_lab::simulator {
   private:
   static void initParallelData (t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData);
   public:
   struct ParallelData {
    t_idx size;
    t_idx rank;
    t_idx up, down, left, right;
    MPI_Comm communicator;
    MPI_Request requesst[8];
    MPI_Datatype row;
    MPI_Datatype column;
    MPI_Datatype text;
    MPI_Datatype file;
    MPI_Datatype restart; 
    }
   struct Grid {
    t_idx localNX;
    t_idx localNY;
    t_idx globalNX;
    t_idx globalNY;
    t_real dX;
    t_real dY;
    t_real *height;
    t_real *momentumX;
    t_real *momentumY;
    t_real *bathymetry; 
    }
    static void runSimulation(tsunami_lab::setups::Setup *i_setup,
                              tsunami_lab::t_real i_hStar,
                              tsunami_lab::configs::SimConfig i_simConfig);
};


#endif
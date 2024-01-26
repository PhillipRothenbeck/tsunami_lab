/**
 * @author Phillip Rothenbeck, Moritz Rätz
 *
 * @section DESCRIPTION
 * routines and data structure
 **/
#ifndef TSUNAMI_LAB_MPI_KERNEL
#define TSUNAMI_LAB_MPI_KERNEL

#include <mpi.h>

#include "../constants.h"

namespace tsunami_lab {
    class MPIKernel;
}

class tsunami_lab::MPIKernel {
   public:
    struct ParallelData {
        int size;
        int rank;
        int up, down, left, right;
        int xDim, yDim;
        MPI_Comm communicator;
        MPI_Request request[8];
        MPI_Datatype row;
        MPI_Datatype column;
        MPI_Datatype text;
        MPI_Datatype file;
        MPI_Datatype restart;

        ParallelData() : size(1), rank(0), up(-2), down(-2), left(-2), right(-2){};
    };

    struct Grid {
        t_idx localNX;
        t_idx localNY;
        t_idx globalNX;
        t_idx globalNY;
        // float dX;
        // float dY;
        float *height;
        float *momentumX;
        float *momentumY;
        float *bathymetry;
    };

    static void init(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData, Grid *o_grid);
    static void initParallelData(t_idx i_globalNX, t_idx i_globalNY, t_idx i_localNX, t_idx i_localNY, int *i_dimension, ParallelData *o_parallelData);
    static void initGrid(t_idx i_localNX, t_idx i_localNY, t_idx i_globalNX, t_idx i_globalNY, Grid *o_grid);

    static void freeParallelData(ParallelData *o_parallelData);
};

#endif
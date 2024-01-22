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
        MPI_Comm communicator;
        MPI_Request request[8];
        MPI_Datatype row;
        MPI_Datatype column;
        MPI_Datatype text;
        MPI_Datatype file;
        MPI_Datatype restart;

        ParallelData() : size(0), rank(0), up(0), down(0), left(0), right(0){};
    };

    struct Grid {
        int localNX;
        int localNY;
        // int globalNX;
        // int globalNY;
        // float dX;
        // float dY;
        float *height;
        float *momentumX;
        float *momentumY;
        float *bathymetry;
    };

    static void init(int i_globalNX, int i_globalNY, ParallelData *o_parallelData, Grid *o_grid);
    static void initParallelData(int i_globalNX, int i_globalNY, int i_localNX, int i_localNY, int *i_dimension, ParallelData *o_parallelData);
    static void initGrid(int i_localNX, int i_localNY, Grid *o_grid);

    static void freeParallelData(ParallelData *o_parallelData);
};

#endif
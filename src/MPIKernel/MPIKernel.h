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
    };

    struct Grid {
        int localNX;
        int localNY;
        int globalNX;
        int globalNY;
        float dX;
        float dY;
        float *height;
        float *momentumX;
        float *momentumY;
        float *bathymetry;
    };

    static void initParallelData(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData);
};

#endif
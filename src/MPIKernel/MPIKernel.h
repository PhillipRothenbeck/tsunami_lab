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
        MPI_Request subgridRequest[4];
        MPI_Request firstRequest[8];
        MPI_Request secondRequest[8];
        MPI_Datatype horizontalBorder;
        MPI_Datatype verticalBorder;
        MPI_Datatype subgrid;

        ParallelData() : size(1), rank(0), up(-2), down(-2), left(-2), right(-2){};
    };

    struct GridData {
        t_idx localNX;
        t_idx localNY;
        t_idx globalNX;
        t_idx globalNY;
    };

    static void init(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData, GridData *o_grid);
    static void initParallelData(t_idx i_localNX, t_idx i_localNY, int *i_dimension, ParallelData *o_parallelData);
    static void initGrid(t_idx i_localNX, t_idx i_localNY, t_idx i_globalNX, t_idx i_globalNY, GridData *o_grid);

    static void freeParallelData(ParallelData *o_parallelData);
};

#endif
/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 * @author Moritz Rätz (moritz.raetz AT uni-jena.de)
 *
 * @section DESCRIPTION
 * routines and data structure
 **/

#include "MPIKernel.h"

#include <mpi.h>

#include <cassert>
#include <iostream>

void tsunami_lab::MPIKernel::init(t_idx i_nx, t_idx i_ny, ParallelData *o_parallelData, GridData *o_grid) {
    int l_localNX, l_localNY;
    int l_worldSize;
    int l_dimension[2] = {0, 0};

    // get number of processes in Communicator and rank of each process
    MPI_Comm_size(MPI_COMM_WORLD, &l_worldSize);

    // get number of subgrids (depending on number of total processes) in each dimension
    MPI_Dims_create(l_worldSize, 2, l_dimension);

    // calculate size of local domain in subgrid / process
    l_localNX = i_nx / l_dimension[0];
    l_localNY = i_ny / l_dimension[1];

    // can the global domain be divided evenly between the subgrids/processes?
    if ((t_idx)(l_localNX * l_dimension[0]) != i_nx) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNX << " x " << l_dimension[0] << " != " << i_nx << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    if ((t_idx)(l_localNY * l_dimension[1]) != i_ny) {
        std::cerr << "No even division between subgrids in y-direction possible. " << l_localNY << " x " << l_dimension[1] << " != " << i_ny << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }

    initParallelData(l_localNX, l_localNY, l_dimension, o_parallelData);
    initGridData(l_localNX, l_localNY, i_nx, i_ny, o_grid);
}

void tsunami_lab::MPIKernel::initParallelData(t_idx i_localNX, t_idx i_localNY, int *i_dimension, ParallelData *o_parallelData) {
    int l_period[2] = {0, 0};  // logical array for cart_create

    // create topolgy through cartesian communicator and cartesian shifts
    MPI_Cart_create(MPI_COMM_WORLD, 2, i_dimension, l_period, 1, &o_parallelData->communicator);
    MPI_Cart_shift(o_parallelData->communicator, 0, 1, &o_parallelData->left, &o_parallelData->right);
    MPI_Cart_shift(o_parallelData->communicator, 1, 1, &o_parallelData->up, &o_parallelData->down);

    MPI_Comm_size(o_parallelData->communicator, &o_parallelData->size);
    MPI_Comm_rank(o_parallelData->communicator, &o_parallelData->rank);

    o_parallelData->xDim = i_dimension[0];
    o_parallelData->yDim = i_dimension[1];

    // border Datatypes init (column: left/right, row: up/down)
    MPI_Type_vector(i_localNY, 1, i_localNX + 2, MPI_FLOAT, &o_parallelData->verticalBorder);
    MPI_Type_commit(&o_parallelData->verticalBorder);

    // MPI data type for tranfering the border cells of the up and down transfer
    MPI_Type_contiguous(i_localNX, MPI_FLOAT, &o_parallelData->horizontalBorder);
    MPI_Type_commit(&o_parallelData->horizontalBorder);

    // MPI data type for transfering subgrid
    MPI_Type_contiguous((i_localNX + 2) * (i_localNY + 2), MPI_FLOAT, &o_parallelData->subgrid);
    MPI_Type_commit(&o_parallelData->subgrid);
}

void tsunami_lab::MPIKernel::initGridData(t_idx i_localNX, t_idx i_localNY, t_idx i_globalNX, t_idx i_globalNY, GridData *o_grid) {
    o_grid->localNX = i_localNX;
    o_grid->localNY = i_localNY;
    o_grid->globalNX = i_globalNX;
    o_grid->globalNY = i_globalNY;
}

void tsunami_lab::MPIKernel::freeTypes(ParallelData *o_parallelData) {
    MPI_Type_free(&o_parallelData->horizontalBorder);
    MPI_Type_free(&o_parallelData->verticalBorder);
    MPI_Type_free(&o_parallelData->subgrid);
}

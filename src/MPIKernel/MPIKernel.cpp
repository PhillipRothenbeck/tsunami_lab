/**
 * @author Phillip Rothenbeck, Moritz Rätz
 *
 * @section DESCRIPTION
 * routines and data structure
 **/

#include "MPIKernel.h"

#include <mpi.h>

#include <cassert>
#include <iostream>

void tsunami_lab::MPIKernel::init(t_idx i_nx, t_idx i_ny, ParallelData *o_parallelData, Grid *o_grid) {
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

    initParallelData(i_nx, i_ny, l_localNX, l_localNY, l_dimension, o_parallelData);
    initGrid(l_localNX, l_localNY, i_nx, i_ny, o_grid);
}

void tsunami_lab::MPIKernel::initParallelData(t_idx i_globalNX, t_idx i_globalNY, t_idx i_localNX, t_idx i_localNY, int *i_dimension, ParallelData *o_parallelData) {
    int l_period[2] = {0, 0};  // logical array for cart_create
    int offset[2] = {1, 1};
    int coordinate[2];

    int size[2] = {(int)i_localNX + 2, (int)i_localNY + 2};
    int subsize[2] = {(int)i_localNX, (int)i_localNY};

    // create topolgy through cartesian communicator and cartesian shifts
    MPI_Cart_create(MPI_COMM_WORLD, 2, i_dimension, l_period, 1, &o_parallelData->communicator);
    // MPI_coords wenn man will
    MPI_Cart_shift(o_parallelData->communicator, 0, 1, &o_parallelData->left, &o_parallelData->right);
    MPI_Cart_shift(o_parallelData->communicator, 1, 1, &o_parallelData->up, &o_parallelData->down);

    MPI_Comm_size(o_parallelData->communicator, &o_parallelData->size);
    MPI_Comm_rank(o_parallelData->communicator, &o_parallelData->rank);

    // logging / informative cout
    if (o_parallelData->rank == 0) {
        std::cout << "Domain Decomposition: " << i_dimension[0] << " | " << i_dimension[1] << "\n Local Domain Size: " << i_localNX << " | " << i_localNY << std::endl;
    }

    o_parallelData->xDim = i_dimension[0];
    o_parallelData->yDim = i_dimension[1];

    // border Datatypes init (column: left/right, row: up/down)
    // hier bitte aufpassen welceher Datatype was ist (Column Major vs Row Major)
    MPI_Type_vector(i_localNY, 1, i_localNX + 2, MPI_FLOAT, &o_parallelData->column);
    MPI_Type_commit(&o_parallelData->column);

    // MPI data type for tranfering the border cells of the up and down transfer
    MPI_Type_contiguous(i_localNX, MPI_FLOAT, &o_parallelData->row);
    MPI_Type_commit(&o_parallelData->row);

    // text Datatype init (subarray only contains real cells, no ghostcells)
    if (o_parallelData->rank == 0) {
        size[0] = i_globalNX;
        size[1] = i_globalNY;
        offset[0] = 0;
        offset[1] = 0;
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_FLOAT, &o_parallelData->text);
    MPI_Type_commit(&o_parallelData->text);

    // get coordinates of cartesian communicator
    MPI_Cart_coords(o_parallelData->communicator, o_parallelData->rank, 2, coordinate);

    // restart Datatype init
    size[0] = i_localNX + 2;
    size[1] = i_localNY + 2;
    offset[0] = 1;
    offset[1] = 1;

    if (coordinate[0] == 0) {
        offset[0] = 0;
    }
    if (coordinate[1] == 0) {
        offset[1] = 0;
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_FLOAT, &o_parallelData->restart);
    MPI_Type_commit(&o_parallelData->restart);

    // file Datatype init
    size[0] = i_globalNX + 2;
    size[1] = i_globalNY + 2;
    offset[0] = 1 + coordinate[0] * i_localNX;  // stride???
    offset[1] = 1 + coordinate[1] * i_localNY;  // stride???

    for (int i = 0; i < 2; i++) {
        if (coordinate[i] == 0) {
            offset[i] -= 1;
            subsize[i] += 1;
        }
        if (coordinate[i] == i_dimension[i] - 1) {
            subsize[i] += 1;
        }
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_FLOAT, &o_parallelData->file);
    MPI_Type_commit(&o_parallelData->file);
}

void tsunami_lab::MPIKernel::initGrid(t_idx i_localNX, t_idx i_localNY, t_idx i_globalNX, t_idx i_globalNY, Grid *o_grid) {
    o_grid->localNX = i_localNX;
    o_grid->localNY = i_localNY;
    o_grid->globalNX = i_globalNX;
    o_grid->globalNY = i_globalNY;
}

void tsunami_lab::MPIKernel::freeParallelData(ParallelData *o_parallelData) {
    // Free MPI datatypes
    MPI_Type_free(&o_parallelData->row);
    MPI_Type_free(&o_parallelData->column);
    MPI_Type_free(&o_parallelData->text);
    MPI_Type_free(&o_parallelData->file);
    MPI_Type_free(&o_parallelData->restart);
}

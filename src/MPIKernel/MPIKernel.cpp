/**
 * @author Phillip Rothenbeck, Moritz Rätz
 *
 * @section DESCRIPTION
 * routines and data structure
 **/

#include "MPIKernel.h"

void tsunami_lab::MPIKernel::initParallelData(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData) {
    int l_localNX, l_localNY;
    int l_worldSize;
    int l_dimension[0, 0];
    int l_period[0, 0];  // logical array for cart_create
    int size[2] = {l_localNX + 2, l_localNY + 2};
    int subsize[2] = {l_localNX, l_localNY};
    int offset[2] = {1, 1};
    int coordinate[2];

    // get number of processes in Communicator and rank of each process
    MPI_Comm_size(MPI_COMM_WORLD, &l_worldSize);
    // MPI_Comm_rank(MPI_COMM_WORLD, &l_rank);

    // get number of subgrids (depending on number of total processes) in each dimension
    MPI_Dims_create(l_worldSize, 2, l_dimension);

    // calculate size of local domain in subgrid / process
    l_localNX = i_globalNX / l_dimension[0];
    l_localNY = i_globalNY / l_dimension[1];

    // can the global domain be divided evenly between the subgrids/processes?
    if (l_localNX * l_dimension[0] != i_globalNX) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNX << " x " << l_dimension[0] << " != " << i_globalNX << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    if (l_localNY * l_dimension[1] != i_globalNY) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNY << " x " << l_dimension[1] << " != " << i_globalNY << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }

    // create topolgy through cartesian communicator and cartesian shifts
    MPI_Cart_create(MPI_COMM_WORLD, 2, l_dimension, l_period, 1, &o_parallelData->communicator);
    // MPI_coords wenn man will
    MPI_Cart_shift(o_parallelData->communicator, 0, 1, &o_parallelData->up, &o_parallelData->down);
    MPI_Cart_shift(o_parallelData->communicator, 1, 1, &o_parallelData->left, &o_parallelData->right);

    MPI_Comm_size(o_parallelData->communicator, &o_parallelData->size);
    MPI_Comm_rank(o_parallelData->communicator, &o_parallelData->rank);

    // logging / informative cout
    if (o_parallelData->rank == 0) {
        std::cout << "Domain Decomposition: " << l_dimension[0] << " | " << l_dimension[1] << "\n Local Domain Size: " << l_localNX << " | " << l_localNY << std::endl;
    }

    // border Datatypes init (column: left/right, row: up/down)
    // hier bitte aufpassen welceher Datatype was ist (Column Major vs Row Major)
    MPI_Type_vector(l_localNY, 1, l_localNX + 2, MPI_DOUBLE, &o_parallelData->column);
    MPI_Type_commit(&o_parallelData->column);

    MPI_Type_contiguous(l_localNX, MPI_DOUBLE, &o_parallelData->row);
    MPI_Type_commit(&o_parallelData->row);

    // text Datatype init (subarray only contains real cells, no ghostcells)
    if (o_parallelData->rank == 0) {
        size[0] = i_globalNX;
        size[1] = i_globalNY;
        offset[0] = 0;
        offset[1] = 0;
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_DOUBLE, &o_parallelData->text);
    MPI_Type_commit(&o_parallelData->text);

    // get coordinates of cartesian communicator
    MPI_Cart_coords(o_parallelData->communicator, o_parallelData->rank, 2, coordinate);

    // restart Datatype init
    size[0] = l_localNX + 2;
    size[1] = l_localNY + 2;
    offset[0] = 1;
    offset[1] = 1;

    if (coordinate[0] == 0) {
        offset[0] = 0;
    }
    if (coordinate[1] == 0) {
        offset[1] = 0;
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_DOUBLE, &o_parallelData->restart);
    MPI_Type_commit(&o_parallelData->restart);

    // file Datatype init
    size[0] = i_globalNX + 2;
    size[1] = i_globalNY + 2;
    offset[0] = 1 + coordinate[0] * l_localNX;  // stride???
    offset[1] = 1 + coordinate[1] * l_localNY;  // stride???

    for (int i = 0; i < 2; i++) {
        if (coordinate[i] == 0) {
            offset[i] -= 1;
            subsize[i] += 1;
        }
        if (coordinate[i] == l_dimension[i] - 1) {
            subsize[i] += 1;
        }
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_DOUBLE, &o_parallelData->file);
    MPI_Type_commit(&o_parallelData->file);
}
/**
 * @author Bohdan Babii, Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Simulator class that runs all simulations.
 **/

#include "Simulator.h"

#include <mpi.h>
#include <omp.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

template <typename Base, typename T>
inline bool instanceof (const T *ptr) {
    return dynamic_cast<const Base *>(ptr) != nullptr;
}

void tsunami_lab::simulator::initParallelData(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData) {
    t_idx l_localNX, l_localNY;
    t_idx l_worldSize;
    t_idx l_dimension[0, 0];
    t_idx l_period[0, 0];  // logical array for cart_create
    t_idx size[2] = {l_localNX + 2, l_localNY + 2};
    t_idx subsize[2] = {l_localNX, l_localNY};
    t_idx offset[2] = {1, 1};
    t_idx coordinate[2];

    // get number of processes in Communicator and rank of each process
    MPI_Comm_size(MPI_COMM_WORLD, &l_worldSize);
    // MPI_Comm_rank(MPI_COMM_WORLD, &l_rank);

    // get number of subgrids (depending on number of total processes) in each dimension
    MPI_Dims_create(l_worldSize, 2, l_dimension);

    // calculate size of local domain in subgrid / process
    l_localNX = i_globalNX / dimension[0];
    l_localNY = i_globalNY / dimension[1];

    // can the global domain be divided evenly between the subgrids/processes?
    if (l_localNX * dimension[0] != i_globalNX) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNX << " x " << dimension[0] << " != " << i_globalNX << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    if (l_localNY * dimension[1] != i_globalNY) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNY << " x " << dimension[1] << " != " << i_globalNY << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }

    // create topolgy through cartesian communicator and cartesian shifts
    MPI_Cart_create(MPI_COMM_WORLD, 2, dimension, period, 1, &o_parallelData->communicator);
    // MPI_coords wenn man will
    MPI_Cart_shift(o_parallelData->communicator, 0, 1, &o_parallelData->up, &o_parallelData->down);
    MPI_Cart_shift(o_parallelData->communicator, 1, 1, &o_parallelData->left, &o_parallelData->right);

    MPI_Comm_size(o_parallelData->communicator, &o_parallelData->size);
    MPI_Comm_rank(o_parallelData->communicator, &o_parallelData->rank);

    // logging / informative cout
    if (o_parallelData->rank == 0) {
        std::cout << "Domain Decomposition: " << dims[0] << " | " << dims[1] << "\n Local Domain Size: " << l_localNX << " | " << l_localNY << std::endl;
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
        if (coordinate[i] == dimension[i] - 1) {
            subsize[i] += 1;
        }
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_DOUBLE, &o_parallelData->file);
    MPI_Type_commit(&o_parallelData->file);
}

void tsunami_lab::simulator::initParallelData(t_idx i_globalNX, t_idx i_globalNY, ParallelData *o_parallelData) {
    t_idx l_localNX, l_localNY;
    t_idx l_worldSize;
    t_idx l_dimension[0, 0];
    t_idx l_period[0, 0];  // logical array for cart_create
    t_idx size[2] = {l_localNX + 2, l_localNY + 2};
    t_idx subsize[2] = {l_localNX, l_localNY};
    t_idx offset[2] = {1, 1};
    t_idx coordinate[2];

    // get number of processes in Communicator and rank of each process
    MPI_Comm_size(MPI_COMM_WORLD, &l_worldSize);
    // MPI_Comm_rank(MPI_COMM_WORLD, &l_rank);

    // get number of subgrids (depending on number of total processes) in each dimension
    MPI_Dims_create(l_worldSize, 2, l_dimension);

    // calculate size of local domain in subgrid / process
    l_localNX = i_globalNX / dimension[0];
    l_localNY = i_globalNY / dimension[1];

    // can the global domain be divided evenly between the subgrids/processes?
    if (l_localNX * dimension[0] != i_globalNX) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNX << " x " << dimension[0] << " != " << i_globalNX << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }
    if (l_localNY * dimension[1] != i_globalNY) {
        std::cerr << "No even division between subgrids in x-direction possible. " << l_localNY << " x " << dimension[1] << " != " << i_globalNY << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -2);
    }

    // create topolgy through cartesian communicator and cartesian shifts
    MPI_Cart_create(MPI_COMM_WORLD, 2, dimension, period, 1, &o_parallelData->communicator);
    // MPI_coords wenn man will
    MPI_Cart_shift(o_parallelData->communicator, 0, 1, &o_parallelData->up, &o_parallelData->down);
    MPI_Cart_shift(o_parallelData->communicator, 1, 1, &o_parallelData->left, &o_parallelData->right);

    MPI_Comm_size(o_parallelData->communicator, &o_parallelData->size);
    MPI_Comm_rank(o_parallelData->communicator, &o_parallelData->rank);

    // logging / informative cout
    if (o_parallelData->rank == 0) {
        std::cout << "Domain Decomposition: " << dims[0] << " | " << dims[1] << "\n Local Domain Size: " << l_localNX << " | " << l_localNY << std::endl;
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
        if (coordinate[i] == dimension[i] - 1) {
            subsize[i] += 1;
        }
    }

    MPI_Type_create_subarray(2, size, subsize, offset, MPI_ORDER_C, MPI_DOUBLE, &o_parallelData->file);
    MPI_Type_commit(&o_parallelData->file);
}

void tsunami_lab::simulator::runSimulation(setups::Setup *i_setup,
                                           configs::SimConfig i_simConfig) {
    Timer *l_timer = new Timer();

    // define number of cells
    t_idx l_nx = i_simConfig.getXCells();
    t_idx l_ny = i_simConfig.getYCells();

    // define length of one cell
    t_real l_dx = i_simConfig.getXLength() / l_nx;
    t_real l_dy = i_simConfig.getYLength() / l_ny;

    // construct solver
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
    patches::WavePropagation *l_waveProp = new patches::WavePropagation2d(l_nx, l_ny);
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Create WaveProp Object");

    // maximum observed height in the setup
    t_real l_hMax = std::numeric_limits<t_real>::lowest();

    // set up solver
#pragma omp parallel for collapse(2) schedule(static, 8) reduction(max : l_hMax)
    for (t_idx l_cy = 0; l_cy < l_ny; l_cy++) {
        for (t_idx l_cx = 0; l_cx < l_nx; l_cx++) {
            t_real l_y = l_cy * l_dy;
            t_real l_x = l_cx * l_dx;

            // get initial values of the setup
            t_real l_h = i_setup->getHeight(l_x,
                                            l_y);

            l_hMax = l_hMax < l_h ? l_h : l_hMax;

            t_real l_hu = i_setup->getMomentumX(l_x,
                                                l_y);
            t_real l_hv = i_setup->getMomentumY(l_x,
                                                l_y);
            t_real l_b = i_setup->getBathymetry(l_x,
                                                l_y);

            // set initial values in wave propagation solver
            l_waveProp->setHeight(l_cx,
                                  l_cy,
                                  l_h);

            l_waveProp->setMomentumX(l_cx,
                                     l_cy,
                                     l_hu);

            l_waveProp->setMomentumY(l_cx,
                                     l_cy,
                                     l_hv);

            l_waveProp->setBathymetry(l_cx,
                                      l_cy,
                                      l_b);
        }
    }
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->prt_idxTime("Caculate hMax and Init WaveProp");

    // derive maximum wave speed in setup; the momentum is ignored
    t_real l_speedMax = std::sqrt(9.81 * l_hMax);

    // check if delta x is smaller than delta y
    bool l_isXStepSmaller = (l_dx <= l_dy);

    // choose l_dxy as l_dx if it is smaller or l_dy if it is smaller
    t_real l_dxy = l_dx * l_isXStepSmaller + l_dy * !l_isXStepSmaller;

    // derive constant time step; changes at simulation time are ignored
    t_real l_dt = 0.5 * l_dxy / l_speedMax;

    std::cout << l_hMax << " | " << l_speedMax << std::endl;

    std::cout << std::endl;
    std::cout << "runtime configuration" << std::endl;
    std::cout << "  number of cells in x-direction: " << l_nx << std::endl;
    std::cout << "  number of cells in y-direction: " << l_ny << std::endl;
    std::cout << "  cell size:                      " << l_dxy << std::endl;
    std::cout << "  time step:                      " << l_dt << std::endl;
    std::cout << std::endl;

    // derive scaling for a time step
    t_real l_scalingX = l_dt / l_dx;
    t_real l_scalingY = l_dt / l_dy;

    // set up time and print control
    t_idx l_frame = 0;
    t_real l_endTime = i_simConfig.getEndSimTime();
    t_real l_simTime = 0;
    if (i_simConfig.getFlagConfig().useCheckPoint()) {
        l_frame = i_simConfig.getCurrentFrame();
        l_simTime = i_simConfig.getStartSimTime();
    }
    std::string l_path = "./out/" + i_simConfig.getConfigName() + ".nc";
    std::cout << "  writing wave field to " << l_path << std::endl;
    t_idx l_timestepsPerFrame = 25;
    t_idx l_timeStep = l_timestepsPerFrame * l_frame;
    std::cout << l_timeStep << " > " << l_frame << std::endl;
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
    io::NetCDF *l_writer = new io::NetCDF(l_endTime,
                                          l_dt,
                                          l_timestepsPerFrame,
                                          l_dxy,
                                          l_nx,
                                          l_ny,
                                          l_waveProp->getStride(),
                                          i_simConfig.getCoarseFactor(),
                                          l_waveProp->getBathymetry(),
                                          l_path);
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Create Writer Object");

    if (i_simConfig.getFlagConfig().useCheckPoint() && instanceof <setups::CheckPoint>(i_setup)) {
        setups::CheckPoint *l_checkpoint = (setups::CheckPoint *)i_setup;
        for (t_idx l_i = 0; l_i < l_frame; l_i++) {
            l_writer->store(l_checkpoint->getSimTimeData(l_i),
                            l_i,
                            l_checkpoint->getHeightData(l_i),
                            l_checkpoint->getMomentumXData(l_i),
                            l_checkpoint->getMomentumYData(l_i));
        }
        if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Load Checkpoint");
    }

    // iterate over time
    t_real l_checkPointTime = l_endTime / i_simConfig.getCheckPointCount();
    std::cout << l_checkPointTime << " | " << l_endTime << std::endl;
    t_idx l_checkPoints = l_simTime / l_checkPointTime;
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
    while (l_simTime < l_endTime) {
        if (l_timeStep % 25 == 0) {
            std::cout << "  simulation time / #time steps / #step: "
                      << l_simTime << " / " << l_timeStep << " / " << l_frame << std::endl;

            if (i_simConfig.getFlagConfig().useIO()) {
                l_writer->store(l_simTime,
                                l_frame,
                                l_waveProp->getHeight(),
                                l_waveProp->getMomentumX(),
                                l_waveProp->getMomentumY());
            }

            if (i_simConfig.getFlagConfig().useCheckPoint() && l_simTime > l_checkPointTime * l_checkPoints) {
                std::string l_checkpointPath = "./out/" + i_simConfig.getConfigName() + "_checkpoint.nc";
                l_writer->write(l_frame, l_checkpointPath, l_simTime, l_endTime);
                l_checkPoints++;
            }
            l_frame++;
        }
        l_waveProp->setGhostCells(i_simConfig.getBoundaryCondition());
        l_waveProp->timeStep(l_scalingX, l_scalingY);

        l_timeStep++;
        l_simTime += l_dt;
    }
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Simulation");
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->start();
    if (i_simConfig.getFlagConfig().useIO())
        l_writer->write();
    if (i_simConfig.getFlagConfig().useTiming()) l_timer->printTime("Write NC File");
    // free memory
    std::cout << "finished time loop" << std::endl;
    std::cout << "freeing memory" << std::endl;
    delete l_writer;
    delete l_waveProp;
    delete l_timer;
}

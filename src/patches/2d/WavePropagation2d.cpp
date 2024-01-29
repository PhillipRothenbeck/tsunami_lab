/**
 * @author Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Two-dimensional wave propagation patch.
 **/

#include "WavePropagation2d.h"

#include <cassert>
#include <iostream>

tsunami_lab::patches::WavePropagation2d::WavePropagation2d(t_idx i_nCellsX,
                                                           t_idx i_mCellsY,
                                                           MPIKernel::ParallelData i_parallelData,
                                                           t_real *i_height,
                                                           t_real *i_momentumX,
                                                           t_real *i_momentumY,
                                                           t_real *i_bathymetry) {
    m_nCellsX = i_nCellsX;
    m_nCellsY = i_mCellsY;
    m_nCellsAll = (i_nCellsX + 2) * (i_mCellsY + 2);

    // allocate memory including a single ghost cell on each side
    m_h[0] = i_height;
    m_hu[0] = i_momentumX;
    m_hv[0] = i_momentumY;

    m_h[1] = new t_real[m_nCellsAll];
    m_hu[1] = new t_real[m_nCellsAll];
    m_hv[1] = new t_real[m_nCellsAll];
    m_b = i_bathymetry;

    // init to zero
    for (t_idx l_ce = 0; l_ce < m_nCellsAll; l_ce++) {
        // only set ghost cells to 0
        if ((l_ce <= i_nCellsX + 2) | (l_ce % (i_nCellsX + 2) == 0) | (l_ce % (i_nCellsX + 2) == i_nCellsX + 1) | (l_ce >= (i_mCellsY + 1) * (i_nCellsX + 2))) {
            m_h[0][l_ce] = 0;
            m_hu[0][l_ce] = 0;
            m_hv[0][l_ce] = 0;
            m_b[l_ce] = 0;
        }
        m_h[1][l_ce] = 0;
        m_hu[1][l_ce] = 0;
        m_hv[1][l_ce] = 0;
    }

    m_parallelData = i_parallelData;
}

tsunami_lab::patches::WavePropagation2d::~WavePropagation2d() {
    for (unsigned short l_st = 0; l_st < 2; l_st++) {
        delete[] m_h[l_st];
        delete[] m_hu[l_st];
        delete[] m_hv[l_st];
    }

    delete[] m_b;
}

tsunami_lab::t_idx tsunami_lab::patches::WavePropagation2d::getIndex(t_idx i_x, t_idx i_y) {
    return i_y * getStride() + i_x;
}

void tsunami_lab::patches::WavePropagation2d::timeStep(t_real i_scalingX,
                                                       t_real i_scalingY) {
    // pointers to old and new data
    t_real *l_hOld = m_h[m_step];
    t_real *l_huOld = m_hu[m_step];
    t_real *l_hvOld = m_hv[m_step];

    m_step = (m_step + 1) % 2;
    t_real *l_hNew = m_h[m_step];
    t_real *l_huNew = m_hu[m_step];
    t_real *l_hvNew = m_hv[m_step];

    // create arrays to save the data from the x-sweep
    t_real *l_hStar = new t_real[m_nCellsAll];
    t_real *l_huStar = new t_real[m_nCellsAll];
    t_real *l_hvStar = new t_real[m_nCellsAll];

    // init cell (Star) quantities
#pragma omp parallel for collapse(2) schedule(static, 32)
    for (t_idx l_ceY = 0; l_ceY < m_nCellsY + 2; l_ceY++) {
        for (t_idx l_ceX = 0; l_ceX < m_nCellsX + 2; l_ceX++) {
            t_idx l_idx = getIndex(l_ceX, l_ceY);
            l_hStar[l_idx] = l_hOld[l_idx];
            l_huStar[l_idx] = l_huOld[l_idx];
            l_hvStar[l_idx] = l_hvOld[l_idx];
        }
    }

    setSweepGhostCells(l_hOld, l_huOld, l_hvNew, m_b, 1);
    // std::cout << m_parallelData.rank << " gC x setting done" << std::endl;

    // iterate over edges in x-direction for every row and update with Riemann solutions (x-sweep)
#pragma omp parallel for collapse(2) shared(l_hStar, l_huStar)
    for (t_idx l_edY = 1; l_edY < m_nCellsY + 1; l_edY++) {
        for (t_idx l_edX = 0; l_edX < m_nCellsX + 1; l_edX++) {
            // determine left and right cell-id
            t_idx l_ceL = getIndex(l_edX, l_edY);
            t_idx l_ceR = getIndex(l_edX + 1, l_edY);

            // compute net-updates
            t_real l_netUpdates[2][2];

            solvers::FWave::netUpdates(l_hOld[l_ceL],
                                       l_hOld[l_ceR],
                                       l_huOld[l_ceL],
                                       l_huOld[l_ceR],
                                       m_b[l_ceL],
                                       m_b[l_ceR],
                                       l_netUpdates[0],
                                       l_netUpdates[1]);

            // update the cells' quantities
#pragma omp atomic update
            l_hStar[l_ceL] -= i_scalingX * l_netUpdates[0][0];
#pragma omp atomic update
            l_huStar[l_ceL] -= i_scalingX * l_netUpdates[0][1];

#pragma omp atomic update
            l_hStar[l_ceR] -= i_scalingX * l_netUpdates[1][0];
#pragma omp atomic update
            l_huStar[l_ceR] -= i_scalingX * l_netUpdates[1][1];
        }
    }

    // std::cout << m_parallelData.rank << " x sweep done" << std::endl;

    // init new cell quantities
#pragma omp parallel for collapse(2) schedule(static, 32)
    for (t_idx l_ceY = 1; l_ceY < m_nCellsY + 1; l_ceY++) {
        for (t_idx l_ceX = 1; l_ceX < m_nCellsX + 1; l_ceX++) {
            t_idx l_idx = getIndex(l_ceX, l_ceY);
            l_hNew[l_idx] = l_hStar[l_idx];
            l_huNew[l_idx] = l_huStar[l_idx];
            l_hvNew[l_idx] = l_hvStar[l_idx];
        }
    }

    setSweepGhostCells(l_hStar, l_huStar, l_hvStar, m_b, 2);
    // std::cout << m_parallelData.rank << " gC y setting done" << std::endl;

    // iterate over edges in y-direction for every column and update with Riemann solutions (y-sweep)
#pragma omp parallel for collapse(2) shared(l_hNew, l_hvNew)
    for (t_idx l_edX = 1; l_edX < m_nCellsX + 1; l_edX++) {
        for (t_idx l_edY = 0; l_edY < m_nCellsY + 1; l_edY++) {
            // determine upper and lower cell-id
            t_idx l_ceU = getIndex(l_edX, l_edY);
            t_idx l_ceD = getIndex(l_edX, l_edY + 1);

            // compute net-updates
            t_real l_netUpdates[2][2];

            solvers::FWave::netUpdates(l_hStar[l_ceU],
                                       l_hStar[l_ceD],
                                       l_hvStar[l_ceU],
                                       l_hvStar[l_ceD],
                                       m_b[l_ceU],
                                       m_b[l_ceD],
                                       l_netUpdates[0],
                                       l_netUpdates[1]);

            // update the cells' quantities
#pragma omp atomic update
            l_hNew[l_ceU] -= i_scalingY * l_netUpdates[0][0];
#pragma omp atomic update
            l_hvNew[l_ceU] -= i_scalingY * l_netUpdates[0][1];

#pragma omp atomic update
            l_hNew[l_ceD] -= i_scalingY * l_netUpdates[1][0];
#pragma omp atomic update
            l_hvNew[l_ceD] -= i_scalingY * l_netUpdates[1][1];
        }
    }

    delete[] l_hStar;
    delete[] l_huStar;
    delete[] l_hvStar;
}

void tsunami_lab::patches::WavePropagation2d::setSweepGhostCells(t_real *i_height,
                                                                 t_real *i_momentumX,
                                                                 t_real *i_momentumY,
                                                                 t_real *i_bathymetry,
                                                                 int i_mode) {
    int l_firstID, l_secondID, l_error;
    short axis[2][2];
    t_idx l_firstIdxSend, l_secondIdxSend, l_firstIdxRecv, l_secondIdxRecv;
    MPI_Datatype l_datatype;
    switch (i_mode) {
        case 1:
            // left / right
            l_firstID = m_parallelData.left;
            l_secondID = m_parallelData.right;
            l_datatype = m_parallelData.column;
            axis[0][0] = -1;
            axis[0][1] = 0;
            axis[1][0] = 1;
            axis[1][1] = 0;
            l_firstIdxSend = m_nCellsX + 3;
            l_secondIdxSend = m_nCellsX + 2 + m_nCellsX;
            l_firstIdxRecv = m_nCellsX + 2;
            l_secondIdxRecv = m_nCellsX + 2 + m_nCellsX + 1;
            break;
        case 2:
            // up / down
            l_firstID = m_parallelData.up;
            l_secondID = m_parallelData.down;
            l_datatype = m_parallelData.row;
            axis[0][0] = 0;
            axis[0][1] = -1;
            axis[1][0] = 0;
            axis[1][1] = 1;
            l_firstIdxSend = m_nCellsX + 3;
            l_secondIdxSend = (m_nCellsX + 2) * m_nCellsY + 1;
            l_firstIdxRecv = 1;
            l_secondIdxRecv = (m_nCellsX + 2) * (m_nCellsY + 1);
            break;
        default:
            std::cout << "wrong communication mode was entered " << i_mode << std::endl;
            return;
    }

    // send/recieve data from left or upper neighbor
    if (l_firstID != -2) {
        l_error = MPI_Isend(&i_height[l_firstIdxSend], 1, l_datatype, l_firstID, 0, m_parallelData.communicator, &m_parallelData.firstRequest[0]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Isend(&i_momentumX[l_firstIdxSend], 1, l_datatype, l_firstID, 1, m_parallelData.communicator, &m_parallelData.firstRequest[1]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Isend(&i_momentumY[l_firstIdxSend], 1, l_datatype, l_firstID, 2, m_parallelData.communicator, &m_parallelData.firstRequest[2]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Isend(&i_bathymetry[l_firstIdxSend], 1, l_datatype, l_firstID, 3, m_parallelData.communicator, &m_parallelData.firstRequest[3]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_height[l_secondIdxRecv], 1, l_datatype, l_firstID, 4, m_parallelData.communicator, &m_parallelData.firstRequest[4]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_momentumX[l_secondIdxRecv], 1, l_datatype, l_firstID, 5, m_parallelData.communicator, &m_parallelData.firstRequest[5]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_momentumY[l_secondIdxRecv], 1, l_datatype, l_firstID, 6, m_parallelData.communicator, &m_parallelData.firstRequest[6]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_bathymetry[l_secondIdxRecv], 1, l_datatype, l_firstID, 7, m_parallelData.communicator, &m_parallelData.firstRequest[7]);
        assert(l_error == MPI_SUCCESS);
    }

    // send/recieve data from right or lower neighbor
    if (l_secondID != -2) {
        // send/recv height to right or lower
        l_error = MPI_Isend(&i_height[l_secondIdxSend], 1, l_datatype, l_secondID, 4, m_parallelData.communicator, &m_parallelData.secondRequest[0]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_height[l_firstIdxRecv], 1, l_datatype, l_secondID, 0, m_parallelData.communicator, &m_parallelData.secondRequest[4]);
        assert(l_error == MPI_SUCCESS);

        // send/recv momentum x right or lower
        l_error = MPI_Isend(&i_momentumX[l_secondIdxSend], 1, l_datatype, l_secondID, 5, m_parallelData.communicator, &m_parallelData.secondRequest[1]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_momentumX[l_firstIdxRecv], 1, l_datatype, l_secondID, 1, m_parallelData.communicator, &m_parallelData.secondRequest[5]);
        assert(l_error == MPI_SUCCESS);

        // send/recv momentum y right or lower
        l_error = MPI_Isend(&i_momentumY[l_secondIdxSend], 1, l_datatype, l_secondID, 6, m_parallelData.communicator, &m_parallelData.secondRequest[2]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_momentumY[l_firstIdxRecv], 1, l_datatype, l_secondID, 2, m_parallelData.communicator, &m_parallelData.secondRequest[6]);
        assert(l_error == MPI_SUCCESS);

        // send/recv bathymetry right or lower
        l_error = MPI_Isend(&i_bathymetry[l_secondIdxSend], 1, l_datatype, l_secondID, 7, m_parallelData.communicator, &m_parallelData.secondRequest[3]);
        assert(l_error == MPI_SUCCESS);
        l_error = MPI_Irecv(&i_bathymetry[l_firstIdxRecv], 1, l_datatype, l_secondID, 3, m_parallelData.communicator, &m_parallelData.secondRequest[7]);
        assert(l_error == MPI_SUCCESS);
    }

    if (l_firstID != -2) {
        l_error = MPI_Waitall(8, m_parallelData.firstRequest, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
    }
    if (l_secondID != -2) {
        l_error = MPI_Waitall(8, m_parallelData.secondRequest, MPI_STATUS_IGNORE);
        assert(l_error == MPI_SUCCESS);
    }

    if (l_firstID == -2) {
        copyGhostCellsOutflow(axis[0], i_height);
        copyGhostCellsOutflow(axis[0], i_momentumX);
        copyGhostCellsOutflow(axis[0], i_momentumY);
        copyGhostCellsOutflow(axis[0], i_bathymetry);
    }

    if (l_secondID == -2) {
        copyGhostCellsOutflow(axis[1], i_height);
        copyGhostCellsOutflow(axis[1], i_momentumX);
        copyGhostCellsOutflow(axis[1], i_momentumY);
        copyGhostCellsOutflow(axis[1], i_bathymetry);
    }
}

void tsunami_lab::patches::WavePropagation2d::copyCornerCells(t_real *o_dataArray) {
    t_idx l_xMax = m_nCellsX + 1;
    t_idx l_yMax = m_nCellsY + 1;

    o_dataArray[getIndex(0, 0)] = o_dataArray[getIndex(1, 1)];
    o_dataArray[getIndex(l_xMax, 0)] = o_dataArray[getIndex(l_xMax - 1, 1)];
    o_dataArray[getIndex(0, l_yMax)] = o_dataArray[getIndex(1, l_yMax - 1)];
    o_dataArray[getIndex(l_xMax, l_yMax)] = o_dataArray[getIndex(l_xMax - 1, l_yMax - 1)];
}

void tsunami_lab::patches::WavePropagation2d::copyGhostCellsOutflow(short i_axis[2], t_real *o_dataArray) {
    t_idx l_xMax = m_nCellsX + 1;
    t_idx l_yMax = m_nCellsY + 1;

    if (i_axis[0] == 1) {
        for (t_idx l_iy = 1; l_iy < l_yMax; l_iy++) {
            o_dataArray[getIndex(l_xMax, l_iy)] = o_dataArray[getIndex(l_xMax - 1, l_iy)];
        }
    } else if (i_axis[0] == -1) {
        for (t_idx l_iy = 1; l_iy < l_yMax; l_iy++) {
            o_dataArray[getIndex(0, l_iy)] = o_dataArray[getIndex(1, l_iy)];
        }
    }

    if (i_axis[1] == 1) {
        for (t_idx l_ix = 1; l_ix < l_xMax; l_ix++) {
            o_dataArray[getIndex(l_ix, l_yMax)] = o_dataArray[getIndex(l_ix, l_yMax - 1)];
        }
    } else if (i_axis[1] == -1) {
        for (t_idx l_ix = 1; l_ix < l_xMax; l_ix++) {
            o_dataArray[getIndex(l_ix, 0)] = o_dataArray[getIndex(l_ix, 1)];
        }
    }

    copyCornerCells(o_dataArray);
}

void tsunami_lab::patches::WavePropagation2d::copyGhostCellsReflecting(short i_axis[2], t_real i_value, t_real *o_dataArray) {
    t_idx l_xMax = m_nCellsX + 1;
    t_idx l_yMax = m_nCellsY + 1;

    if (i_axis[0] == 1) {
        for (t_idx l_iy = 1; l_iy < l_yMax; l_iy++) {
            o_dataArray[getIndex(l_xMax, l_iy)] = i_value;
        }
    } else if (i_axis[0] == -1) {
        for (t_idx l_iy = 1; l_iy < l_yMax; l_iy++) {
            o_dataArray[getIndex(0, l_iy)] = i_value;
        }
    }

    if (i_axis[1] == 1) {
        for (t_idx l_ix = 1; l_ix < l_xMax; l_ix++) {
            o_dataArray[getIndex(l_ix, l_yMax)] = i_value;
        }
    } else if (i_axis[1] == -1) {
        for (t_idx l_ix = 1; l_ix < l_xMax; l_ix++) {
            o_dataArray[getIndex(l_ix, 0)] = i_value;
        }
    }

    copyCornerCells(o_dataArray);
}

void tsunami_lab::patches::WavePropagation2d::setGhostCells(e_boundary *i_boundary) {
    short l_axis[4][2] = {{1, 0},
                          {0, 1},
                          {-1, 0},
                          {0, -1}};

    for (t_idx l_i = 0; l_i < 4; l_i++) {
        if (i_boundary[l_i] == OUTFLOW) {
            copyGhostCellsOutflow(l_axis[l_i], m_h[m_step]);
            copyGhostCellsOutflow(l_axis[l_i], m_hu[m_step]);
            copyGhostCellsOutflow(l_axis[l_i], m_hv[m_step]);
            copyGhostCellsOutflow(l_axis[l_i], m_b);
        } else if (i_boundary[l_i] == REFLECTING) {
            copyGhostCellsReflecting(l_axis[l_i], 0, m_h[m_step]);
            copyGhostCellsReflecting(l_axis[l_i], 0, m_hu[m_step]);
            copyGhostCellsReflecting(l_axis[l_i], 0, m_hv[m_step]);
            copyGhostCellsReflecting(l_axis[l_i], 20, m_b);
        }
    }
}

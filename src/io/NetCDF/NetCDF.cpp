/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 * @author Marek Sommerfeld (marek.sommerfeld AT uni-jena.de)
 *
 * @section DESCRIPTION
 * IO-routines for reading and writing NetCDF files.
 **/

#include "NetCDF.h"

#include <omp.h>

#include <cmath>
#include <iostream>

tsunami_lab::io::NetCDF::NetCDF(t_real i_endTime,
                                t_real i_dt,
                                t_idx i_timeStepsPerFrame,
                                int i_coordinates[2],
                                t_real i_dxy,
                                t_idx i_nx,
                                t_idx i_ny,
                                t_idx i_stride,
                                t_idx i_coarseFactor,
                                t_real const *i_b,
                                std::string i_outFileName) {
    m_dxy = i_dxy;
    m_nx = i_nx;
    m_ny = i_ny;
    m_coarseFactor = i_coarseFactor;

    // set coarse dimensions
    m_nxCoarse = m_nx / m_coarseFactor;
    m_nyCoarse = m_ny / m_coarseFactor;

    m_nxy = m_nx * m_ny;
    m_nxyCoarse = m_nxCoarse * m_nyCoarse;
    m_stride = i_stride;
    m_outFileName = i_outFileName;

    m_frameCount = ceil((i_endTime / i_dt) / i_timeStepsPerFrame);
    m_dataSize = m_nxy * m_frameCount;

    m_time = new t_real[m_frameCount];
    m_height = new t_real[m_dataSize];
    m_momentumX = new t_real[m_dataSize];
    m_momentumY = new t_real[m_dataSize];

    m_dataX = new t_real[m_nx];
    m_dataY = new t_real[m_ny];
    m_dataB = new t_real[m_nxy];

    t_idx l_xStart = i_coordinates[0] * m_nx * m_dxy;
    t_idx l_yStart = i_coordinates[1] * m_ny * m_dxy;

    for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
        m_dataX[l_ix] = l_xStart + (l_ix + 0.5) * m_dxy;
    }
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        m_dataY[l_iy] = l_yStart + (l_iy + 0.5) * m_dxy;
    }
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
            m_dataB[l_ix + l_iy * m_nx] = i_b[(l_iy + 1) * m_stride + (l_ix + 1)];
        }
    }
}

int tsunami_lab::io::NetCDF::store(t_real i_simTime,
                                   t_idx i_frame,
                                   t_real const *i_h,
                                   t_real const *i_hu,
                                   t_real const *i_hv) {
    m_time[i_frame] = i_simTime;

    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
            t_real l_h = i_h[(l_iy + 1) * m_stride + (l_ix + 1)];
            // t_real l_b = m_dataB[l_iy * m_nx + l_ix];
            m_height[l_ix + l_iy * m_nx + m_nxy * i_frame] = l_h;
            m_momentumX[l_ix + l_iy * m_nx + m_nxy * i_frame] = i_hu[(l_iy + 1) * m_stride + (l_ix + 1)];
            m_momentumY[l_ix + l_iy * m_nx + m_nxy * i_frame] = i_hv[(l_iy + 1) * m_stride + (l_ix + 1)];
        }
    }

    return 0;
}

int tsunami_lab::io::NetCDF::write(int i_rank) {
    // create netCDF file
    std::cout << "Rank " << i_rank << ": Writing to "<< m_outFileName << std::endl;
    int l_nc_err = nc_create((m_outFileName).c_str(), NC_CLOBBER, &m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Create file." << std::endl;
        return 1;
    }

    // define dims and vars
    init();

    //* coarse output

    // x data coarse
    t_real *l_dataX = new tsunami_lab::t_real[m_nxCoarse];
#pragma omp parallel for schedule(static, 16)
    for (t_idx l_idx = 0; l_idx < m_nxCoarse; l_idx++) {
        t_idx l_ix = m_coarseFactor - 1 + (l_idx * m_coarseFactor);
        l_dataX[l_idx] = m_dataX[l_ix];
    }
    l_nc_err = nc_put_var_float(m_ncId, m_varXId, l_dataX);
    delete[] l_dataX;

    // y data coarse
    t_real *l_dataY = new tsunami_lab::t_real[m_nyCoarse];
#pragma omp parallel for schedule(static, 16)
    for (t_idx l_idy = 0; l_idy < m_nyCoarse; l_idy++) {
        t_idx l_iy = m_coarseFactor - 1 + (l_idy * m_coarseFactor);
        l_dataY[l_idy] = m_dataY[l_iy];
    }
    l_nc_err += nc_put_var_float(m_ncId, m_varYId, l_dataY);
    delete[] l_dataY;

    // bathymetry coarse
    t_real *l_dataB = new tsunami_lab::t_real[m_nxyCoarse];
#pragma omp parallel for schedule(static, 8)
    for (t_idx l_idx = 0; l_idx < m_nxyCoarse; l_idx++) {
        t_idx l_ix = m_coarseFactor * (l_idx % m_nxCoarse) + m_coarseFactor - 1;
        t_idx l_iy = m_coarseFactor * (t_idx)std::floor(l_idx / m_nxCoarse) + m_coarseFactor - 1;
        // average over neighbors
        l_dataB[l_idx] = m_dataB[l_iy * m_nx + l_ix];
        t_idx l_neighborCount = 1;
        if (m_coarseFactor != 1) {
            for (int l_offsetY = -(m_coarseFactor - 1); l_offsetY < (int)m_coarseFactor; l_offsetY++) {
                for (int l_offsetX = -(m_coarseFactor - 1); l_offsetX < (int)m_coarseFactor; l_offsetX++) {
                    int l_idxX = l_ix + l_offsetX;
                    int l_idxY = l_iy + l_offsetY;
                    if (tsunami_lab::io::NetCDF::isInBounds(l_idxX, l_idxY)) {
                        l_dataB[l_idx] += m_dataB[l_idxY * m_nx + l_idxX];
                        l_neighborCount++;
                    }
                }
            }
        }
        l_dataB[l_idx] /= l_neighborCount;
    }
    l_nc_err += nc_put_var_float(m_ncId, m_varBathymetryId, l_dataB);
    delete[] l_dataB;

	 // height, momentumX, momentumY coarse
    t_real *l_height = new tsunami_lab::t_real[m_nxyCoarse * m_frameCount];
    t_real *l_momentumX = new tsunami_lab::t_real[m_nxyCoarse * m_frameCount];
    t_real *l_momentumY = new tsunami_lab::t_real[m_nxyCoarse * m_frameCount];
#pragma omp parallel for schedule(static, 16)
    for (t_idx l_idx = 0; l_idx < m_nxyCoarse * m_frameCount; l_idx++) {
        t_idx l_frame = std::floor(l_idx / m_nxyCoarse);
        t_idx l_ix = m_coarseFactor * (l_idx % m_nxCoarse) + m_coarseFactor - 1;
        t_idx l_iy = m_coarseFactor * (t_idx)std::floor((l_idx % m_nxyCoarse) / m_nxCoarse) + m_coarseFactor - 1;
        t_idx l_framedIdx = (l_iy * m_nx + l_ix) + m_nxy * l_frame;
        // average over neighbors
        l_height[l_idx] = m_height[l_framedIdx];
        l_momentumX[l_idx] = m_momentumX[l_framedIdx];
        l_momentumY[l_idx] = m_momentumY[l_framedIdx];
        t_idx l_neighborCount = 1;
        if (m_coarseFactor != 1) {
            for (int l_offsetY = -(m_coarseFactor - 1); l_offsetY < (int)m_coarseFactor; l_offsetY++) {
                for (int l_offsetX = -(m_coarseFactor - 1); l_offsetX < (int)m_coarseFactor; l_offsetX++) {
                    int l_idxX = l_ix + l_offsetX;
                    int l_idxY = l_iy + l_offsetY;
                    if (tsunami_lab::io::NetCDF::isInBounds(l_idxX, l_idxY)) {
                        t_idx l_framedIdxOffset = (l_idxY * m_nx + l_idxX) + m_nxy * l_frame;
                        l_height[l_idx] += m_height[l_framedIdxOffset];
                        l_momentumX[l_idx] += m_momentumX[l_framedIdxOffset];
                        l_momentumY[l_idx] += m_momentumY[l_framedIdxOffset];
                        l_neighborCount++;
                    }
                }
            }
        }
        l_height[l_idx] /= l_neighborCount;
        l_momentumX[l_idx] /= l_neighborCount;
        l_momentumY[l_idx] /= l_neighborCount;
    }
    l_nc_err += nc_put_var_float(m_ncId, m_varHeightId, l_height);
    l_nc_err += nc_put_var_float(m_ncId, m_varMomentumXId, l_momentumX);
    l_nc_err += nc_put_var_float(m_ncId, m_varMomentumYId, l_momentumY);
    delete[] l_height;
    delete[] l_momentumX;
    delete[] l_momentumY;

	 // write time data
    l_nc_err += nc_put_var_float(m_ncId, m_varTimeId, m_time);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Put variables." << std::endl;
        return 1;
    }

	 // close file
    l_nc_err = nc_close(m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Close file." << std::endl;
        return 1;
    }

    return NC_NOERR;
}

int tsunami_lab::io::NetCDF::read(std::string i_nameBathymetry,
                                  std::string i_nameDisplacements,
                                  t_idx *o_bathymetryDimX,
                                  t_idx *o_bathymetryDimY,
                                  t_real *&o_bathymetryPosX,
                                  t_real *&o_bathymetryPosY,
                                  t_real *&o_bathymetry,
                                  t_idx *o_dispDimX,
                                  t_idx *o_dispDimY,
                                  t_real *&o_dispPosX,
                                  t_real *&o_dispPosY,
                                  t_real *&o_displacements) {
    // add res path
    i_nameBathymetry = "./res/" + i_nameBathymetry;
    i_nameDisplacements = "./res/" + i_nameDisplacements;
    int l_ncIDBathymetry, l_ncIDDisplacements;

    //* read bathymetry

    // open bathymetry file
    std::cout << "Rank 0: Loading bathymetry file: " << i_nameBathymetry << "..." << std::endl;
    int l_nc_err = nc_open(i_nameBathymetry.c_str(), 0, &l_ncIDBathymetry);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Could not open file " << i_nameBathymetry << std::endl;
        return 1;
    }

    // get dimensions
    std::size_t l_xDim, l_yDim;
    l_nc_err = nc_inq_dimlen(l_ncIDBathymetry, 0, &l_xDim);
    l_nc_err += nc_inq_dimlen(l_ncIDBathymetry, 1, &l_yDim);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Could not get dimensions. (bathymetry)" << std::endl;
        return 1;
    }

    *o_bathymetryDimX = (t_idx)l_xDim;
    *o_bathymetryDimY = (t_idx)l_yDim;
    o_bathymetry = new tsunami_lab::t_real[l_xDim * l_yDim];
    o_bathymetryPosX = new tsunami_lab::t_real[l_xDim];
    o_bathymetryPosY = new tsunami_lab::t_real[l_yDim];

    // get variable ids
    int l_varIDx, l_varIDy, l_varIDz;
    l_nc_err = nc_inq_varid(l_ncIDBathymetry, "x", &l_varIDx);
    l_nc_err += nc_inq_varid(l_ncIDBathymetry, "y", &l_varIDy);
    l_nc_err += nc_inq_varid(l_ncIDBathymetry, "z", &l_varIDz);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Could not get variable ids. (bathymetry)" << std::endl;
        return 1;
    }

    // load data
    l_nc_err = nc_get_var_float(l_ncIDBathymetry, l_varIDx, &o_bathymetryPosX[0]);
    l_nc_err += nc_get_var_float(l_ncIDBathymetry, l_varIDy, &o_bathymetryPosY[0]);
    l_nc_err += nc_get_var_float(l_ncIDBathymetry, l_varIDz, &o_bathymetry[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCERR: Could not load data. (bathymetry)" << std::endl;
        return 1;
    }

    //* read displacements

    // open displacement file
    std::cout << "Rank 0: Loading displacement file " << i_nameDisplacements << "..." << std::endl;
    l_nc_err = nc_open(i_nameDisplacements.c_str(), 0, &l_ncIDDisplacements);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Could not open file " << i_nameDisplacements << std::endl;
        return 1;
    }

    // get dimensions
    l_nc_err = nc_inq_dimlen(l_ncIDDisplacements, 0, &l_xDim);
    l_nc_err += nc_inq_dimlen(l_ncIDDisplacements, 1, &l_yDim);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Could not get dimensions. (displacement)" << std::endl;
        return 1;
    }

    *o_dispDimX = (t_idx)l_xDim;
    *o_dispDimY = (t_idx)l_yDim;
    o_displacements = new tsunami_lab::t_real[l_xDim * l_yDim];
    o_dispPosX = new tsunami_lab::t_real[l_xDim];
    o_dispPosY = new tsunami_lab::t_real[l_yDim];

    // get variable ids
    l_nc_err = nc_inq_varid(l_ncIDDisplacements, "x", &l_varIDx);
    l_nc_err += nc_inq_varid(l_ncIDDisplacements, "y", &l_varIDy);
    l_nc_err += nc_inq_varid(l_ncIDDisplacements, "z", &l_varIDz);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Could not get variable ids. (displacement)" << std::endl;
        return 1;
    }

    // load data
    l_nc_err = nc_get_var_float(l_ncIDDisplacements, l_varIDx, &o_dispPosX[0]);
    l_nc_err += nc_get_var_float(l_ncIDDisplacements, l_varIDy, &o_dispPosY[0]);
    l_nc_err += nc_get_var_float(l_ncIDDisplacements, l_varIDz, &o_displacements[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCERR: Could not load data. (displacement)" << std::endl;
        return 1;
    }
    return 0;
}

int tsunami_lab::io::NetCDF::init() {
    int l_nc_err;

    // define dimensions
    l_nc_err = nc_def_dim(m_ncId, "x", m_nxCoarse, &m_dimXId);
    l_nc_err += nc_def_dim(m_ncId, "y", m_nyCoarse, &m_dimYId);
    l_nc_err += nc_def_dim(m_ncId, "time", m_frameCount, &m_dimTimeId);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define dimensions." << std::endl;
        return 1;
    }

    // define variables
    l_nc_err = nc_def_var(m_ncId, "x", NC_FLOAT, 1, &m_dimXId, &m_varXId);
    l_nc_err += nc_put_att_text(m_ncId, m_varXId, "units", 6, "meters");
    l_nc_err += nc_put_att_text(m_ncId, m_varXId, "axis", 1, "X");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (x)" << std::endl;
        return 1;
    }

    l_nc_err = nc_def_var(m_ncId, "y", NC_FLOAT, 1, &m_dimYId, &m_varYId);
    l_nc_err += nc_put_att_text(m_ncId, m_varYId, "units", 6, "meters");
    l_nc_err += nc_put_att_text(m_ncId, m_varXId, "axis", 1, "Y");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (y)" << std::endl;
        return 1;
    }

    l_nc_err = nc_def_var(m_ncId, "time", NC_FLOAT, 1, &m_dimTimeId, &m_varTimeId);
    l_nc_err += nc_put_att_text(m_ncId, m_varTimeId, "units", 7, "seconds");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (time)" << std::endl;
        return 1;
    }

    int l_dimBathymetryIds[2] = {m_dimYId, m_dimXId};
    l_nc_err = nc_def_var(m_ncId, "bathymetry", NC_FLOAT, 2, l_dimBathymetryIds, &m_varBathymetryId);
    l_nc_err += nc_put_att_text(m_ncId, m_varBathymetryId, "units", 6, "meters");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (bathymetry)" << std::endl;
        return 1;
    }

    int l_dimHeightIds[3] = {m_dimTimeId, m_dimYId, m_dimXId};
    l_nc_err = nc_def_var(m_ncId, "height", NC_FLOAT, 3, l_dimHeightIds, &m_varHeightId);
    l_nc_err += nc_put_att_text(m_ncId, m_varHeightId, "units", 6, "meters");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (height)" << std::endl;
        return 1;
    }

    int l_dimMomentumXIds[3] = {m_dimTimeId, m_dimYId, m_dimXId};
    l_nc_err = nc_def_var(m_ncId, "momentum_x", NC_FLOAT, 3, l_dimMomentumXIds, &m_varMomentumXId);
    l_nc_err += nc_put_att_text(m_ncId, m_varMomentumXId, "units", 11, "meters*kg/s");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (momentumX)" << std::endl;
        return 1;
    }

    int l_dimMomentumYIds[3] = {m_dimTimeId, m_dimYId, m_dimXId};
    l_nc_err = nc_def_var(m_ncId, "momentum_y", NC_FLOAT, 3, l_dimMomentumYIds, &m_varMomentumYId);
    l_nc_err += nc_put_att_text(m_ncId, m_varMomentumYId, "units", 11, "meters*kg/s");
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: Define variables. (momentumY)" << std::endl;
        return 1;
    }

    l_nc_err = nc_enddef(m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "NCError: enddef" << std::endl;
        return 1;
    }

    return NC_NOERR;
}

bool tsunami_lab::io::NetCDF::isInBounds(int i_x, int i_y) {
    if (i_x < 0 || i_x >= (int)m_nx) {
        return false;
    }
    if (i_y < 0 || i_y >= (int)m_ny) {
        return false;
    }
    return true;
}

tsunami_lab::io::NetCDF::~NetCDF() {
    delete[] m_time;

    delete[] m_height;
    delete[] m_momentumX;
    delete[] m_momentumY;

    delete[] m_dataX;
    delete[] m_dataY;
    delete[] m_dataB;
}

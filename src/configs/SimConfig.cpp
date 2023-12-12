/**
 * @author Bohdan Babii, Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Configuration that holds all information for one simulation.
 **/

#include "SimConfig.h"

tsunami_lab::configs::SimConfig::SimConfig() {
}

tsunami_lab::configs::SimConfig::SimConfig(tsunami_lab::t_idx i_dimension,
                                           std::string i_configName,
                                           tsunami_lab::t_idx i_nx,
                                           tsunami_lab::t_idx i_ny,
                                           tsunami_lab::t_real i_xLen,
                                           tsunami_lab::t_real i_yLen,
                                           tsunami_lab::t_real i_endSimTime,
                                           tsunami_lab::t_real i_startSimTime,
                                           tsunami_lab::t_idx i_coarseFactor,
                                           e_boundary i_boundaryCondition[4],
                                           bool i_isRoeSolver) {
    m_dimension = i_dimension;
	 m_configName = i_configName;
    m_nx = i_nx;
    m_ny = i_ny;
    m_xLen = i_xLen;
    m_yLen = i_yLen;
    m_endSimTime = i_endSimTime;
    m_startSimTime = i_startSimTime;
    m_coarseFactor = i_coarseFactor;
    for (t_idx l_i = 0; l_i < 4; l_i++) {
        m_boundaryCondition[l_i] = i_boundaryCondition[l_i];
    }
    m_isRoeSolver = i_isRoeSolver;
}

tsunami_lab::configs::SimConfig::~SimConfig() {}
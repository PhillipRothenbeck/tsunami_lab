/**
 * @author Bohdan Babii (bohdan.babii AT uni-jena.de)
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Configuration that holds all information for one simulation.
 **/
#ifndef TSUNAMI_LAB_CONFIGS_SIM_CONFIG
#define TSUNAMI_LAB_CONFIGS_SIM_CONFIG

#include <cstdlib>
#include <string>

#include "../constants.h"
#include "FlagConfig.h"

namespace tsunami_lab {
    namespace configs {
        struct SimConfig;
    }
}  // namespace tsunami_lab

struct tsunami_lab::configs::SimConfig {
    //! config file name
    std::string configName;
    //! flag config
    tsunami_lab::configs::FlagConfig flagConfig;
    //! number of cells in x-direction
    tsunami_lab::t_idx nx = 50;
    //! number of cells in y-direction
    tsunami_lab::t_idx ny = 50;
    //! length of the simulation in x-direction
    tsunami_lab::t_real xLen = tsunami_lab::t_real(10.0);
    //! length of the simulation in y-direction
    tsunami_lab::t_real yLen = tsunami_lab::t_real(10.0);
    //! time of the simulation.
    tsunami_lab::t_real endSimTime = tsunami_lab::t_real(1.25);
    //! start time of the simulation.
    tsunami_lab::t_real startSimTime = tsunami_lab::t_real(0);
    //! last frame simulated.
    t_idx currentFrame = t_idx(0);
    //! coarse output - factor
    tsunami_lab::t_real coarseFactor = tsunami_lab::t_real(1.0);
};

#endif
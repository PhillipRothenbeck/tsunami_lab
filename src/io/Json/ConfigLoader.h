/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Class loading the config.json file and initializing the simulation.
 **/

#ifndef TSUNAMI_LAB_IO_CONFIG_LOADER
#define TSUNAMI_LAB_IO_CONFIG_LOADER

#include <string>

#include "../../configs/FlagConfig.h"
#include "../../configs/SimConfig.h"
#include "../../constants.h"
#include "../../io/NetCDF/NetCDF.h"
#include "../../setups/Setup.h"
#include "../../setups/TsunamiEvent2d/TsunamiEvent2d.h"

namespace tsunami_lab {
    namespace io {
        class ConfigLoader;
    }
}  // namespace tsunami_lab

/**
 * @brief Class to load Setup and Configuration based on a .json file.
 *
 */
class tsunami_lab::io::ConfigLoader {
   public:
    /**
     * @brief Load the given .json file and extract all information from it. Also load all .nc file that are mentioned in the .json file.
     *
     * @param i_rank identifier of the respective MPI process.
     * @param i_path path of the .json file to be read.
     * @param i_flagConfig the program flags.
     * @param o_setup return the setup, that is supposed to be used for the simulation.
     * @param o_simConfig return the configuration holding all information of the given .json file.
     * @return tsunami_lab::t_idx
     */
    static tsunami_lab::t_idx loadConfig(int i_rank,
                                         std::string i_path,
                                         tsunami_lab::configs::FlagConfig i_flagConfig,
                                         tsunami_lab::setups::Setup *&o_setup,
                                         tsunami_lab::configs::SimConfig &o_simConfig);
};

#endif

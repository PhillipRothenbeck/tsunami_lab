/**
 * @author Phillip Rothenbeck
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
#include "../../io/Csv/Csv.h"
#include "../../io/NetCDF/NetCDF.h"
#include "../../setups/CheckPoint/CheckPoint.h"
#include "../../setups/Setup.h"
#include "../../setups/TsunamiEvent2d/TsunamiEvent2d.h"

namespace tsunami_lab {
    namespace io {
        class ConfigLoader;
    }
}  // namespace tsunami_lab

class tsunami_lab::io::ConfigLoader {
   public:
    static tsunami_lab::t_idx loadConfig(std::string i_path,
                                         tsunami_lab::configs::FlagConfig i_flagConfig,
                                         tsunami_lab::setups::Setup *&i_setup,
                                         tsunami_lab::t_real &i_hStar,
                                         tsunami_lab::configs::SimConfig &i_simConfig);
};

#endif

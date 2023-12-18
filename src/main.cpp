/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>

#include "configs/FlagConfig.h"
#include "configs/SimConfig.h"
#include "io/Json/ConfigLoader.h"
#include "simulator/Simulator.h"
#include "timer.h"

int main(int i_argc, char *i_argv[]) {
    std::cout << "####################################" << std::endl;
    std::cout << "### Tsunami Lab                  ###" << std::endl;
    std::cout << "###                              ###" << std::endl;
    std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
    std::cout << "####################################" << std::endl;

    if (i_argc < 2) {
        std::cerr << "invalid number of program parameter" << std::endl;
        std::cerr << "  ./build/tsunami_lab CONFIG_FILE_NAME.json" << std::endl;
        return EXIT_FAILURE;
    }

    Timer *l_timer = new Timer();

    std::string l_configName = std::string(i_argv[1]);

    tsunami_lab::configs::FlagConfig l_flagConfig = tsunami_lab::configs::FlagConfig();
    if (i_argc > 2) {
        for (int l_arguments = 1; l_arguments < i_argc; l_arguments++) {
            if (std::string(i_argv[l_arguments]).compare("-c") == 0) {
                l_flagConfig.setUseCheckPoint(true);
            } else if (std::string(i_argv[l_arguments]).compare("-t") == 0) {
                l_flagConfig.setUseTiming(true);
            } else if (std::string(i_argv[l_arguments]).compare("-nio") == 0) {
					l_flagConfig.setUseIO(false);
				}
        }
    }

    tsunami_lab::setups::Setup *l_setups = nullptr;
    tsunami_lab::t_real l_hStar = -1;
    tsunami_lab::configs::SimConfig l_simConfig = tsunami_lab::configs::SimConfig();

    if (l_flagConfig.useTiming()) l_timer->start();
    // load parameters from runtimeConfig.json
    tsunami_lab::t_idx err = tsunami_lab::io::ConfigLoader::loadConfig(l_configName,
                                                                       l_flagConfig,
                                                                       l_setups,
                                                                       l_hStar,
                                                                       l_simConfig);
	 if (l_flagConfig.useTiming()) l_timer->printTime("Loading Config ");

    if (err != 0) {
        std::cout << "failed to read: " << l_configName << std::endl;
        delete l_setups;
        delete l_timer;
        return EXIT_FAILURE;
    }

    // start simulation from config
    tsunami_lab::simulator::runSimulation(l_setups, l_hStar, l_simConfig);

    delete l_setups;
    delete l_timer;
    std::cout << "finished, exiting" << std::endl;
    return EXIT_SUCCESS;
}

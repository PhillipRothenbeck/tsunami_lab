/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Test loading the config files.
 **/
#include <catch2/catch.hpp>

#define private public
#include "ConfigLoader.h"
#undef public

TEST_CASE("Test reading a config JSON file.", "[ConfigLoader]") {
    std::string l_path = "demo_config.json";
    tsunami_lab::setups::Setup *l_setups = nullptr;
    tsunami_lab::configs::SimConfig l_simConfig = tsunami_lab::configs::SimConfig();
    tsunami_lab::configs::FlagConfig l_flagConfig = tsunami_lab::configs::FlagConfig();

    tsunami_lab::t_idx err;
    err = tsunami_lab::io::ConfigLoader::loadConfig(0,
                                                    l_path,
                                                    l_flagConfig,
                                                    l_setups,
                                                    l_simConfig);

    REQUIRE(err == 0);
    REQUIRE(l_simConfig.nx == 500);
    REQUIRE(l_simConfig.ny == 500);
    REQUIRE(l_simConfig.xLen == 100.0);
    REQUIRE(l_simConfig.yLen == 500.0);
    REQUIRE(l_simConfig.endSimTime == 5.0);
    REQUIRE(l_simConfig.startSimTime == 0);
    REQUIRE(l_simConfig.coarseFactor == 1.0);
    delete l_setups;
}
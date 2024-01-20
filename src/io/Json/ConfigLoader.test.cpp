/**
 * @author Phillip Rothenbeck
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
    err = tsunami_lab::io::ConfigLoader::loadConfig(l_path,
                                                    l_flagConfig,
                                                    l_setups,
                                                    l_simConfig);

    REQUIRE(err == 0);
    REQUIRE(l_simConfig.getXCells() == 500);
    REQUIRE(l_simConfig.getYCells() == 500);
    REQUIRE(l_simConfig.getXLength() == 100.0);
    REQUIRE(l_simConfig.getYLength() == 500.0);
    REQUIRE(l_simConfig.getEndSimTime() == 5.0);
    REQUIRE(l_simConfig.getStartSimTime() == 0);
    REQUIRE(l_simConfig.getCoarseFactor() == 1.0);
    REQUIRE(l_simConfig.getBoundaryCondition()[0] == tsunami_lab::OUTFLOW);
    REQUIRE(l_simConfig.getBoundaryCondition()[1] == tsunami_lab::REFLECTING);
    REQUIRE(l_simConfig.isRoeSolver());
    REQUIRE(l_simConfig.getCheckPointCount() == 2);
    delete l_setups;
}
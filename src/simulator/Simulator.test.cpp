/**
 * @author Bohdan Babii
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Unit-tests for Simulator.
 **/
#include <catch2/catch.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../constants.h"
#define private public
#include "Simulator.h"
#undef public

TEST_CASE("Test the simulation running method.", "[Simulator]") {
    tsunami_lab::configs::FlagConfig l_flagConfig = tsunami_lab::configs::FlagConfig();
    tsunami_lab::configs::SimConfig l_config;
    l_config.flagConfig = l_flagConfig;
}
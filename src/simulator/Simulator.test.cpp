/**
 * @author Bohdan Babii, Phillip Rothenbeck
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
    tsunami_lab::e_boundary l_boundary[4] = {tsunami_lab::OUTFLOW, tsunami_lab::OUTFLOW, tsunami_lab::OUTFLOW, tsunami_lab::OUTFLOW};
    tsunami_lab::configs::FlagConfig l_flagConfig = tsunami_lab::configs::FlagConfig();
    tsunami_lab::configs::SimConfig l_config = tsunami_lab::configs::SimConfig("simulator",
                                                                               l_flagConfig,
                                                                               0,
                                                                               50,
                                                                               1,
                                                                               10,
                                                                               1,
                                                                               1.25,
                                                                               0,
                                                                               0,
                                                                               1.0,
                                                                               l_boundary,
                                                                               false);
}
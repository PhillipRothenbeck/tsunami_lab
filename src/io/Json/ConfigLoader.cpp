/**
 * @author Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Class loading the config.json file and initializing the simulation.
 **/

#include "ConfigLoader.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <string>

tsunami_lab::t_idx tsunami_lab::io::ConfigLoader::loadConfig(int i_rank,
                                                             std::string i_configName,
                                                             tsunami_lab::configs::FlagConfig i_flagConfig,
                                                             tsunami_lab::setups::Setup *&o_setup,
                                                             tsunami_lab::configs::SimConfig &o_simConfig) {
    std::string l_path = "./res/configs/" + i_configName;

    std::ifstream l_file(l_path);
    if (l_file.fail()) {
        std::cerr << "failed to read " << l_path << std::endl;
        return EXIT_FAILURE;
    }

    // read JSON config file
    nlohmann::json l_configFile = nlohmann::json::parse(l_file);

    // set number of cells in x-direction
    if (l_configFile.contains("nx")) {
        o_simConfig.nx = l_configFile.at("nx");
    }

    // set number of cells in y-direction
    if (l_configFile.contains("ny")) {
        o_simConfig.ny = l_configFile.at("ny");
    }

    // set length of simulation x-direction
    o_simConfig.xLen = 10;
    if (l_configFile.contains("xLen")) {
        o_simConfig.xLen = l_configFile.at("xLen");
    }

    // set length of simulation y-direction
    o_simConfig.yLen = 10;
    if (l_configFile.contains("yLen")) {
        o_simConfig.yLen = l_configFile.at("yLen");
    }

    // offset of the epicenter to the 0-point in x-direction
    tsunami_lab::t_real l_epicenterOffsetX = -(o_simConfig.xLen / 2);
    if (l_configFile.contains("epicenterOffsetX")) {
        l_epicenterOffsetX = l_configFile.at("epicenterOffsetX");
    }

    // offset of the epicenter to the 0-point in x-direction
    tsunami_lab::t_real l_epicenterOffsetY = -(o_simConfig.yLen / 2);
    if (l_configFile.contains("epicenterOffsetY")) {
        l_epicenterOffsetY = l_configFile.at("epicenterOffsetY");
    }

    // set time of simulation
    if (l_configFile.contains("simTime")) {
        o_simConfig.endSimTime = l_configFile.at("simTime");
    }

    // factor for coarse output
    if (l_configFile.contains("coarseFactor")) {
        o_simConfig.coarseFactor = l_configFile.at("coarseFactor");

        if (o_simConfig.coarseFactor < 1) {
            std::cout << "factor for coarse input can't be smaller than 1" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // set bathymetry and displacements file names
    std::string l_bathymetryFileName = "dummy_bathymetry.nc", l_displacementsFileName = "dummy_disp.nc";
    if (l_configFile.contains("bathymetryFileName")) {
        l_bathymetryFileName = l_configFile.at("bathymetryFileName");
    }

    if (l_configFile.contains("displacementsFileName")) {
        l_displacementsFileName = l_configFile.at("displacementsFileName");
    }

    o_simConfig.configName = i_configName.substr(0, i_configName.find_last_of("."));
    if (i_rank == 0) {
        tsunami_lab::t_idx l_bathymetryDimX, l_bathymetryDimY, l_dispDimX, l_dispDimY;
        tsunami_lab::t_real *l_bathymetry;
        tsunami_lab::t_real *l_bathymetryPosX;
        tsunami_lab::t_real *l_bathymetryPosY;
        tsunami_lab::t_real *l_displacements;
        tsunami_lab::t_real *l_dispPosX;
        tsunami_lab::t_real *l_dispPosY;

        int l_err = tsunami_lab::io::NetCDF::read(l_bathymetryFileName,
                                                  l_displacementsFileName,
                                                  &l_bathymetryDimX,
                                                  &l_bathymetryDimY,
                                                  l_bathymetryPosX,
                                                  l_bathymetryPosY,
                                                  l_bathymetry,
                                                  &l_dispDimX,
                                                  &l_dispDimY,
                                                  l_dispPosX,
                                                  l_dispPosY,
                                                  l_displacements);

        if (l_err != 0) {
            std::cout << "Failed to read the betCDF files" << std::endl;
            return EXIT_FAILURE;
        }

        o_setup = new tsunami_lab::setups::TsunamiEvent2d(o_simConfig.xLen,
                                                          o_simConfig.xLen,
                                                          l_bathymetryDimX,
                                                          l_bathymetryDimY,
                                                          l_bathymetryPosX,
                                                          l_bathymetryPosY,
                                                          l_bathymetry,
                                                          l_dispDimX,
                                                          l_dispDimY,
                                                          l_dispPosX,
                                                          l_dispPosY,
                                                          l_displacements,
                                                          l_epicenterOffsetX,
                                                          l_epicenterOffsetY);
    }

    o_simConfig.flagConfig = i_flagConfig;

    return 0;
}
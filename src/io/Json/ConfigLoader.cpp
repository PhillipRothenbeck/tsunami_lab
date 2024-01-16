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

tsunami_lab::t_idx tsunami_lab::io::ConfigLoader::loadConfig(std::string i_configName,
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
    tsunami_lab::t_idx l_nx = 50;
    if (l_configFile.contains("nx")) {
        l_nx = l_configFile.at("nx");
    }

    // set number of cells in y-direction
    tsunami_lab::t_idx l_ny = 50;
    if (l_configFile.contains("ny")) {
        l_ny = l_configFile.at("ny");
    }

    // set length of simulation x-direction
    tsunami_lab::t_real l_xLen = 10;
    if (l_configFile.contains("xLen")) {
        l_xLen = l_configFile.at("xLen");
    }

    // set length of simulation y-direction
    tsunami_lab::t_real l_yLen = 10;
    if (l_configFile.contains("yLen")) {
        l_yLen = l_configFile.at("yLen");
    }

    // offset of the epicenter to the 0-point in x-direction
    tsunami_lab::t_real l_epicenterOffsetX = -(l_xLen / 2);
    if (l_configFile.contains("epicenterOffsetX")) {
        l_epicenterOffsetX = l_configFile.at("epicenterOffsetX");
    }

    // offset of the epicenter to the 0-point in x-direction
    tsunami_lab::t_real l_epicenterOffsetY = -(l_yLen / 2);
    if (l_configFile.contains("epicenterOffsetY")) {
        l_epicenterOffsetY = l_configFile.at("epicenterOffsetY");
    }

    // set time of simulation
    tsunami_lab::t_real l_endSimTime = 1.25;
    if (l_configFile.contains("simTime")) {
        l_endSimTime = l_configFile.at("simTime");
    }

    // set start time of simulation
    tsunami_lab::t_real l_startSimTime = 0;

    // set boundary condition
    e_boundary l_boundaryCond[4] = {OUTFLOW, OUTFLOW, OUTFLOW, OUTFLOW};
    if (l_configFile.contains("boundaryCond")) {
        std::string l_boundary = l_configFile.at("boundaryCond");
        for (t_idx l_i = 0; l_i < l_boundary.length(); l_i++) {
            if (l_boundary[l_i] == 'R') {
                l_boundaryCond[l_i] = REFLECTING;
            }
        }
    }

    // set solver
    bool l_useRoeSolver = false;
    if (l_configFile.contains("solver")) {
        std::string solver = l_configFile.at("solver");
        if (solver.compare("Roe") == 0) {
            l_useRoeSolver = true;
        }
    }

    // factor for coarse output
    tsunami_lab::t_idx l_checkPointCount = 5;
    if (l_configFile.contains("checkPoints")) {
        l_checkPointCount = l_configFile.at("checkPoints");
    }

    // factor for coarse output
    tsunami_lab::t_idx l_coarseFactor = 1;
    if (l_configFile.contains("coarseFactor")) {
        l_coarseFactor = l_configFile.at("coarseFactor");

        if (l_coarseFactor < 1) {
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

    // set setup configuration
    std::string l_setupName = "TsunamiEvent";
    if (l_configFile.contains("setup")) {
        l_setupName = l_configFile.at("setup");
    }
    std::cout << "simulation setup was set to: " << l_setupName << std::endl;

    // check if checkpoint exists
    std::string l_configName = i_configName.substr(0, i_configName.find_last_of("."));
    std::string l_checkPointPath = "out/" + l_configName + "_checkpoint.nc";
    std::ifstream f(l_checkPointPath.c_str());
    t_idx l_startFrame = 0;
    if (i_flagConfig.useCheckPoint() && f.good()) {
        std::cout << "Reading out/" + l_configName + "_checkpoint.nc" << std::endl;
        t_real *l_height;
        t_real *l_momentumX;
        t_real *l_momentumY;
        t_real *l_bathymetry;
        t_real *l_time;
        t_real l_endSimTime;
        tsunami_lab::io::NetCDF::readCheckpoint(l_checkPointPath,
                                                l_height,
                                                l_momentumX,
                                                l_momentumY,
                                                l_bathymetry,
                                                l_time,
                                                &l_startFrame,
                                                &l_endSimTime,
                                                &l_startSimTime);

        o_setup = new tsunami_lab::setups::CheckPoint(l_xLen,
                                                      l_yLen,
                                                      l_nx,
                                                      l_ny,
                                                      l_startFrame,
                                                      l_height,
                                                      l_momentumX,
                                                      l_momentumY,
                                                      l_bathymetry,
                                                      l_time);
    } else if (l_setupName.compare("TsunamiEvent") == 0) {
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

        o_setup = new tsunami_lab::setups::TsunamiEvent2d(l_xLen,
                                                          l_yLen,
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
    } else {
        std::cout << "unknown setup was entered" << std::endl;
        return EXIT_FAILURE;
    }

    o_simConfig = tsunami_lab::configs::SimConfig(l_configName,
                                                  i_flagConfig,
                                                  l_checkPointCount,
                                                  l_nx,
                                                  l_ny,
                                                  l_xLen,
                                                  l_yLen,
                                                  l_endSimTime,
                                                  l_startSimTime,
                                                  l_startFrame,
                                                  l_coarseFactor,
                                                  l_boundaryCond,
                                                  l_useRoeSolver);

    return 0;
}
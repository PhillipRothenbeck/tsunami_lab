/**
 * @author Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * CheckPoint Loader Setup unit test.
 **/

#include "CheckPoint.h"

#include <catch2/catch.hpp>

TEST_CASE("Test the two-dimensional checkpoint setup data.", "[Checkpoint]") {
    tsunami_lab::t_idx l_nx = 3;
    tsunami_lab::t_idx l_ny = 2;
    tsunami_lab::t_idx l_dimX = 3.0;
    tsunami_lab::t_idx l_dimY = 2.0;
    tsunami_lab::t_idx l_frame = 2;
    tsunami_lab::t_real *l_height_ptr = new tsunami_lab::t_real[12];
    tsunami_lab::t_real *l_momentumX_ptr = new tsunami_lab::t_real[12];
    tsunami_lab::t_real *l_momentumY_ptr = new tsunami_lab::t_real[12];
    tsunami_lab::t_real *l_bathymetry_ptr = new tsunami_lab::t_real[12];
    tsunami_lab::t_real l_bathymetry[12] = {1.5, 2.5,
                                            3.5, 4.5,
                                            5.5, 6.5,
                                            1.5, 2.5,
                                            3.5, 4.5,
                                            5.5, 6.5};
    tsunami_lab::t_real l_height[12] = {0, 0,
                                        2, 1,
                                        0, 0,
                                        0, 0,
                                        0, 0,
                                        0, 0};
    tsunami_lab::t_real l_momentumX[12] = {0, 1,
                                           2, 3,
                                           4, 5,
                                           6, 7,
                                           8, 9,
                                           10, 11};
    tsunami_lab::t_real l_momentumY[12] = {0.1, 0.2,
                                           0.3, 0.4,
                                           0.5, 0.6,
                                           0.2, 0.4,
                                           0.8, 1.6,
                                           3.2, 6.4};

    tsunami_lab::t_real *l_time = new tsunami_lab::t_real[2];
    for (tsunami_lab::t_idx l_i = 0; l_i < 12; l_i++) {
        l_bathymetry_ptr[l_i] = l_bathymetry[l_i];
        l_height_ptr[l_i] = l_height[l_i];
        l_momentumX_ptr[l_i] = l_momentumX[l_i];
        l_momentumY_ptr[l_i] = l_momentumY[l_i];
    }
    for (tsunami_lab::t_idx l_i = 0; l_i < 2; l_i++) {
        l_time[l_i] = tsunami_lab::t_real(l_i);
    }
    tsunami_lab::setups::CheckPoint l_setup = tsunami_lab::setups::CheckPoint(l_dimX,
                                                                              l_dimY,
                                                                              l_nx,
                                                                              l_ny,
                                                                              l_frame,
                                                                              l_height_ptr,
                                                                              l_momentumX_ptr,
                                                                              l_momentumY_ptr,
                                                                              l_bathymetry_ptr,
                                                                              l_time);
    tsunami_lab::t_real l_momentumYTest = 0.1;
    tsunami_lab::t_real l_momentumXTest = 5;
    tsunami_lab::t_real l_bathymetryTest = 0.5;
    for (tsunami_lab::t_real l_ceY = 0; l_ceY < 2; l_ceY++) {
        for (tsunami_lab::t_real l_ceX = 0; l_ceX < 3; l_ceX++) {
            l_momentumYTest *= 2;
            l_momentumXTest += 1;
            l_bathymetryTest += 1;
            REQUIRE(l_setup.getMomentumX(l_ceX, l_ceY) == l_momentumXTest);
            REQUIRE(l_setup.getMomentumY(l_ceX, l_ceY) == l_momentumYTest);
            REQUIRE(l_setup.getHeight(l_ceX, l_ceY) == 0);
            REQUIRE(l_setup.getBathymetry(l_ceX, l_ceY) == l_bathymetryTest);
        }
    }
}
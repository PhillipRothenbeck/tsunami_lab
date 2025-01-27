/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Unit tests for the one-dimensional wave propagation patch.
 **/
#include "WavePropagation1d.h"

#include <catch2/catch.hpp>

TEST_CASE("Test the 1d wave propagation solver (Roe).", "[WaveProp1d]") {
    /*
     * Test case:
     *
     *   Single dam break problem between cell 49 and 50.
     *     left | right
     *       10 | 8
     *        0 | 0
     *
     *   Elsewhere steady state.
     *
     * The net-updates at the respective edge are given as
     * (see derivation in Roe solver):
     *    left          | right
     *      9.394671362 | -9.394671362
     *    -88.25985     | -88.25985
     */

    // construct solver (roe) and setup a dambreak problem
    tsunami_lab::patches::WavePropagation1d m_waveProp(100, 1);

    for (std::size_t l_ce = 0; l_ce < 50; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             10);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                0);
    }
    for (std::size_t l_ce = 50; l_ce < 100; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             8);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                0);
    }

    // set outflow boundary condition
    tsunami_lab::e_boundary l_boundary[2] = {tsunami_lab::OUTFLOW, tsunami_lab::OUTFLOW};
    m_waveProp.setGhostCells(l_boundary);

    // perform a time step
    m_waveProp.timeStep(0.1, 0);

    // steady state
    for (std::size_t l_ce = 0; l_ce < 49; l_ce++) {
        REQUIRE(m_waveProp.getHeight()[l_ce] == Approx(10));
        REQUIRE(m_waveProp.getMomentumX()[l_ce] == Approx(0));
    }

    // dam-break
    REQUIRE(m_waveProp.getHeight()[49] == Approx(10 - 0.1 * 9.394671362));
    REQUIRE(m_waveProp.getMomentumX()[49] == Approx(0 + 0.1 * 88.25985));

    REQUIRE(m_waveProp.getHeight()[50] == Approx(8 + 0.1 * 9.394671362));
    REQUIRE(m_waveProp.getMomentumX()[50] == Approx(0 + 0.1 * 88.25985));

    // steady state
    for (std::size_t l_ce = 51; l_ce < 100; l_ce++) {
        REQUIRE(m_waveProp.getHeight()[l_ce] == Approx(8));
        REQUIRE(m_waveProp.getMomentumX()[l_ce] == Approx(0));
    }
}

TEST_CASE("Test the 1d wave propagation solver (FWave).", "[WaveProp1d]") {
    /*
     * Test case:
     *
     *   Single dam break problem between cell 49 and 50.
     *     left | right
     *       10 | 8
     *        0 | 0
     *
     *   Elsewhere steady state.
     *
     * The net-updates at the respective edge are given as
     * (see derivation in f-wave solver):
     *    left             | right
     *     9.39475         | -9.39475
     *    -88.25991835     | -88.25991835
     */

    // construct solver (F_Wave) and setup a dambreak problem
    tsunami_lab::patches::WavePropagation1d m_waveProp(100, 0);

    for (std::size_t l_ce = 0; l_ce < 50; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             10);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                0);
        m_waveProp.setBathymetry(l_ce,
                                 0,
                                 5);
    }
    for (std::size_t l_ce = 50; l_ce < 100; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             8);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                0);
        m_waveProp.setBathymetry(l_ce,
                                 0,
                                 0);
    }

    // set outflow boundary condition
    tsunami_lab::e_boundary l_boundary[2] = {tsunami_lab::OUTFLOW, tsunami_lab::OUTFLOW};
    m_waveProp.setGhostCells(l_boundary);

    // perform a time step
    m_waveProp.timeStep(0.1, 0);

    // steady state
    for (std::size_t l_ce = 0; l_ce < 49; l_ce++) {
        REQUIRE(m_waveProp.getHeight()[l_ce] == Approx(10));
        REQUIRE(m_waveProp.getMomentumX()[l_ce] == Approx(0));
        REQUIRE(m_waveProp.getBathymetry()[l_ce] == Approx(5));
    }

    // dam-break
    REQUIRE(m_waveProp.getHeight()[49] == Approx(10 - 0.1 * 32.8816));
    REQUIRE(m_waveProp.getMomentumX()[49] == Approx(0 + 0.1 * 308.90947936));
    REQUIRE(m_waveProp.getBathymetry()[49] == Approx(5));

    REQUIRE(m_waveProp.getHeight()[50] == Approx(8 + 0.1 * 32.8816));
    REQUIRE(m_waveProp.getMomentumX()[50] == Approx(0 + 0.1 * 308.90947936));
    REQUIRE(m_waveProp.getBathymetry()[50] == Approx(0));

    // steady state
    for (std::size_t l_ce = 51; l_ce < 100; l_ce++) {
        REQUIRE(m_waveProp.getHeight()[l_ce] == Approx(8));
        REQUIRE(m_waveProp.getMomentumX()[l_ce] == Approx(0));
        REQUIRE(m_waveProp.getBathymetry()[l_ce] == Approx(0));
    }
}

TEST_CASE("Test Reflecting Boundary Condition.", "[ReflectingBoundaryConditions]") {
    /*
     * Test case:
     *
     *   Single dam break problem between cell 49 and 50.
     *     left | right
     *       10 | 8
     *        3 | 4
     */

    // construct solver (F_Wave) and setup a dambreak problem
    tsunami_lab::patches::WavePropagation1d m_waveProp(100, 0);

    for (std::size_t l_ce = 0; l_ce < 50; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             10);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                3);
    }
    for (std::size_t l_ce = 50; l_ce < 100; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             8);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                4);
    }

    // set reflecting boundary condition
    tsunami_lab::e_boundary l_boundary[2] = {tsunami_lab::REFLECTING, tsunami_lab::REFLECTING};
    m_waveProp.setGhostCells(l_boundary);

    // Ghost Reflecting Boundary condition
    REQUIRE(m_waveProp.getHeight()[-1] == Approx(10));
    REQUIRE(m_waveProp.getMomentumX()[-1] == Approx(-3));

    REQUIRE(m_waveProp.getHeight()[100] == Approx(8));
    REQUIRE(m_waveProp.getMomentumX()[100] == Approx(-4));
}

TEST_CASE("Test Left Reflecting Boundary Condition.", "[LeftReflectingBoundaryConditions]") {
    /*
     * Test case:
     *
     *   Single dam break problem between cell 49 and 50.
     *     left | right
     *       10 | 8
     *        3 | 4
     */

    // construct solver (F_Wave) and setup a dambreak problem
    tsunami_lab::patches::WavePropagation1d m_waveProp(100, 0);

    for (std::size_t l_ce = 0; l_ce < 50; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             10);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                3);
    }
    for (std::size_t l_ce = 50; l_ce < 100; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             8);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                4);
    }

    // set left reflecting and right outflow boundary conditions
    tsunami_lab::e_boundary l_boundary[2] = {tsunami_lab::REFLECTING, tsunami_lab::OUTFLOW};
    m_waveProp.setGhostCells(l_boundary);

    // Ghost Reflecting Boundary condition
    REQUIRE(m_waveProp.getHeight()[-1] == Approx(10));
    REQUIRE(m_waveProp.getMomentumX()[-1] == Approx(-3));

    REQUIRE(m_waveProp.getHeight()[100] == Approx(8));
    REQUIRE(m_waveProp.getMomentumX()[100] == Approx(4));
}

TEST_CASE("Test Right Reflecting Boundary Condition.", "[RightReflectingBoundaryConditions]") {
    /*
     * Test case:
     *
     *   Single dam break problem between cell 49 and 50.
     *     left | right
     *       10 | 8
     *        3 | 4
     */

    // construct solver (F_Wave) and setup a dambreak problem
    tsunami_lab::patches::WavePropagation1d m_waveProp(100, 0);

    for (std::size_t l_ce = 0; l_ce < 50; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             10);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                3);
    }
    for (std::size_t l_ce = 50; l_ce < 100; l_ce++) {
        m_waveProp.setHeight(l_ce,
                             0,
                             8);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                4);
    }

    // set right reflecting and left outflow boundary conditions
    tsunami_lab::e_boundary l_boundary[2] = {tsunami_lab::OUTFLOW, tsunami_lab::REFLECTING};
    m_waveProp.setGhostCells(l_boundary);

    // Ghost Reflecting Boundary condition
    REQUIRE(m_waveProp.getHeight()[-1] == Approx(10));
    REQUIRE(m_waveProp.getMomentumX()[-1] == Approx(3));

    REQUIRE(m_waveProp.getHeight()[100] == Approx(8));
    REQUIRE(m_waveProp.getMomentumX()[100] == Approx(-4));
}
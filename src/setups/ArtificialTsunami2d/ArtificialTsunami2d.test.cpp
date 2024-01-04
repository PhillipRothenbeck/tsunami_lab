/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 * @author Moritz Rätz (moritz.raetz AT uni-jena.de)
 * @author Marek Sommerfeld (marek.sommerfeld AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Artificial two-dimensional tsunami event test.
 **/
#include "ArtificialTsunami2d.h"

#include <catch2/catch.hpp>

TEST_CASE("Test the two-dimensional artificial tsunami setup.", "[ArtificialTsunami2d]") {
    tsunami_lab::setups::ArtificialTsunami2d l_artificialTsunami(10000, 10000);

    for (int l_x = 0; l_x < 10; l_x++) {
        for (int l_y = 0; l_y < 10; l_y++) {
            REQUIRE(l_artificialTsunami.getHeight(l_x, l_y) == 100);
            REQUIRE(l_artificialTsunami.getMomentumX(l_x, l_y) == 0);
            REQUIRE(l_artificialTsunami.getMomentumY(l_x, l_y) == 0);
            REQUIRE(l_artificialTsunami.getBathymetry(l_x, l_y) == -100);
        }
    }
}
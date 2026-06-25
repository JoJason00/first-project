#include <catch2/catch_test_macros.hpp>

#include "add.h"

TEST_CASE("division function works") {
    compute c;
    REQUIRE(c.division(6, 2) == 3);
    REQUIRE(c.division(6, 0) == 0);
}
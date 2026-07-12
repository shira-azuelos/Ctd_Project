#include "../../doctest.h"
#include "../../include/model/position.h"

TEST_CASE("Position struct verification") {
    model::Position pos1(3, 4);
    CHECK(pos1.row == 3);
    CHECK(pos1.col == 4);

    model::Position pos2(3, 4);
    CHECK(pos1.row == pos2.row);
    CHECK(pos1.col == pos2.col);
}
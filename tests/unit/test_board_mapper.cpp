#include "../../doctest.h"
#include "input/board_mapper.h"

TEST_CASE("BoardMapper: Converts pixels to cells correctly") {
    int width = 8;
    int height = 8;
                
    auto pos1 = input::BoardMapper::pixel_to_cell(50, 50, width, height);
    REQUIRE(pos1.has_value());
    CHECK(pos1->row == 0);
    CHECK(pos1->col == 0);

    auto pos2 = input::BoardMapper::pixel_to_cell(250, 150, width, height);
    REQUIRE(pos2.has_value());
    CHECK(pos2->row == 1);
    CHECK(pos2->col == 2);
}

TEST_CASE("BoardMapper: Rejects clicks outside the board") {
    int width = 8;
    int height = 8;

    auto pos1 = input::BoardMapper::pixel_to_cell(-10, 50, width, height);
    CHECK_FALSE(pos1.has_value());

    auto pos2 = input::BoardMapper::pixel_to_cell(850, 50, width, height);
    CHECK_FALSE(pos2.has_value());
}
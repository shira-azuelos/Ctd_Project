#include "../../doctest.h"
#include "io/board_parser.h"
#include <vector>
#include <string>
#include <stdexcept>

TEST_CASE("BoardParser: Accepts a valid rectangular board") {
    std::vector<std::string> lines = {
        "wK .",
        ". bR"
    };
    
    auto board = io::BoardParser::parse(lines);
    
    CHECK(board->get_height() == 2);
    CHECK(board->get_width() == 2);
    
    auto piece1 = board->get_piece_at(model::Position(0, 0));
    REQUIRE(piece1.get() != nullptr); 
    CHECK(piece1->color == model::PieceColor::WHITE);
    CHECK(piece1->kind == model::PieceKind::KING);
    
    auto empty_cell = board->get_piece_at(model::Position(0, 1));
    CHECK(empty_cell.get() == nullptr);     

    auto piece2 = board->get_piece_at(model::Position(1, 1));
    REQUIRE(piece2.get() != nullptr); 
    CHECK(piece2->color == model::PieceColor::BLACK);
    CHECK(piece2->kind == model::PieceKind::ROOK);
}

TEST_CASE("BoardParser: Rejects inconsistent row length") {
    std::vector<std::string> lines = {
        "wK .",
        "."
    };
    
    CHECK_THROWS_AS(io::BoardParser::parse(lines), std::invalid_argument);
}

TEST_CASE("BoardParser: Rejects invalid piece token") {
    std::vector<std::string> lines = {
        "wX ."
    };
    CHECK_THROWS_AS(io::BoardParser::parse(lines), std::invalid_argument);
}
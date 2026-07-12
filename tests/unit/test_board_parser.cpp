#include "../../doctest.h"
#include "io/board_parser.h"
#include <vector>
#include <string>
#include <stdexcept>

TEST_CASE("BoardParser: Accepts a valid rectangular board") {
    // הגדרת לוח חוקי של 2x2
    std::vector<std::string> lines = {
        "wK .",
        ". bR"
    };
    
    auto board = io::BoardParser::parse(lines);
    
    // מוודא שממדי הלוח נגזרו נכון
    CHECK(board->get_height() == 2);
    CHECK(board->get_width() == 2);
    
    // מוודא שהמלך הלבן נמצא בתא (0,0)
    auto piece1 = board->get_piece_at(model::Position(0, 0));
    REQUIRE(piece1.get() != nullptr); // התיקון כאן: שימוש ב-.get()
    CHECK(piece1->color == model::PieceColor::WHITE);
    CHECK(piece1->kind == model::PieceKind::KING);
    
    // מוודא שהתא (0,1) ריק
    auto empty_cell = board->get_piece_at(model::Position(0, 1));
    CHECK(empty_cell.get() == nullptr); // התיקון כאן
    
    // מוודא שהצריח השחור נמצא בתא (1,1)
    auto piece2 = board->get_piece_at(model::Position(1, 1));
    REQUIRE(piece2.get() != nullptr); // התיקון כאן
    CHECK(piece2->color == model::PieceColor::BLACK);
    CHECK(piece2->kind == model::PieceKind::ROOK);
}

TEST_CASE("BoardParser: Rejects inconsistent row length") {
    // הגדרת לוח עם שורה ראשונה של 2 תאים ושורה שנייה של תא 1
    std::vector<std::string> lines = {
        "wK .",
        "."
    };
    
    // מוודא שהפונקציה זורקת חריגה
    CHECK_THROWS_AS(io::BoardParser::parse(lines), std::invalid_argument);
}

TEST_CASE("BoardParser: Rejects invalid piece token") {
    // הגדרת לוח עם טוקן שגוי
    std::vector<std::string> lines = {
        "wX ."
    };
    
    // מוודא שהפונקציה זורקת חריגה
    CHECK_THROWS_AS(io::BoardParser::parse(lines), std::invalid_argument);
}
#include "../../doctest.h"
#include "rules/piece_rules.h"
#include "model/board.h"
#include "model/position.h"
#include "model/piece.h"

using namespace rules;
using namespace model;

TEST_CASE("PieceRules: King movement") {
    Board board(8, 8);
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::KING, PieceColor::WHITE, src, Position(4, 4), false));
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::KING, PieceColor::WHITE, src, Position(3, 4), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::KING, PieceColor::WHITE, src, Position(5, 3), false));
}

TEST_CASE("PieceRules: Rook movement") {
    Board board(8, 8);
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::ROOK, PieceColor::WHITE, src, Position(3, 7), false));
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::ROOK, PieceColor::WHITE, src, Position(0, 3), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::ROOK, PieceColor::WHITE, src, Position(4, 4), false));
}

TEST_CASE("PieceRules: Bishop movement") {
    Board board(8, 8);
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::BISHOP, PieceColor::WHITE, src, Position(5, 5), false));
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::BISHOP, PieceColor::WHITE, src, Position(1, 1), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::BISHOP, PieceColor::WHITE, src, Position(3, 4), false));
}

TEST_CASE("PieceRules: Queen movement") {
    Board board(8, 8);
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::QUEEN, PieceColor::WHITE, src, Position(5, 5), false));
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::QUEEN, PieceColor::WHITE, src, Position(3, 7), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::QUEEN, PieceColor::WHITE, src, Position(5, 4), false));
}

TEST_CASE("PieceRules: Knight movement") {
    Board board(8, 8);
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::KNIGHT, PieceColor::WHITE, src, Position(5, 4), false));
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::KNIGHT, PieceColor::WHITE, src, Position(2, 5), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::KNIGHT, PieceColor::WHITE, src, Position(4, 4), false));
}

TEST_CASE("PieceRules: Pawn movement") {
    Board board(8, 8);

    // חייל לבן (מתחיל בשורה 6, נע למעלה - שורה קטנה יותר)
    // צעד אחד תקין
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::WHITE, Position(6, 1), Position(5, 1), false));
    // צעד כפול מעמדת התחלה תקין
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::WHITE, Position(6, 1), Position(4, 1), false));
    // צעד כפול לא מעמדת התחלה לא תקין
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::WHITE, Position(5, 1), Position(3, 1), false));
    // תנועה לאחור לא תקינה
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::WHITE, Position(6, 1), Position(7, 1), false));
    // תפיסה באלכסון תקינה
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::WHITE, Position(6, 1), Position(5, 2), true));
    // לא יכול לאכול ישר
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::WHITE, Position(6, 1), Position(5, 1), true));

    // חייל שחור (מתחיל בשורה 1, נע למטה - שורה גדולה יותר)
    // צעד אחד תקין
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::BLACK, Position(1, 1), Position(2, 1), false));
    // צעד כפול מעמדת התחלה תקין
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::BLACK, Position(1, 1), Position(3, 1), false));
    // צעד כפול לא מעמדת התחלה לא תקין
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::BLACK, Position(2, 1), Position(4, 1), false));
    // תנועה לאחור לא תקינה
    CHECK_FALSE(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::BLACK, Position(1, 1), Position(0, 1), false));
    // תפיסה באלכסון תקינה
    CHECK(PieceRules::is_valid_geometry(board, PieceKind::PAWN, PieceColor::BLACK, Position(1, 1), Position(2, 2), true));
}
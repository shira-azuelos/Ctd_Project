#include "../../doctest.h"
#include "rules/piece_rules.h"
#include "model/position.h"
#include "model/piece.h"

using namespace rules;
using namespace model;

TEST_CASE("PieceRules: King movement") {
    Position src(3, 3);
    // הוספנו צבע (WHITE) ופרמטר תפיסה (false) לכל הבדיקות
    CHECK(PieceRules::is_valid_geometry(PieceKind::KING, PieceColor::WHITE, src, Position(4, 4), false));
    CHECK(PieceRules::is_valid_geometry(PieceKind::KING, PieceColor::WHITE, src, Position(3, 4), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::KING, PieceColor::WHITE, src, Position(5, 3), false));
}

TEST_CASE("PieceRules: Rook movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::ROOK, PieceColor::WHITE, src, Position(3, 7), false));
    CHECK(PieceRules::is_valid_geometry(PieceKind::ROOK, PieceColor::WHITE, src, Position(0, 3), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::ROOK, PieceColor::WHITE, src, Position(4, 4), false));
}

TEST_CASE("PieceRules: Bishop movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::BISHOP, PieceColor::WHITE, src, Position(5, 5), false));
    CHECK(PieceRules::is_valid_geometry(PieceKind::BISHOP, PieceColor::WHITE, src, Position(1, 1), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::BISHOP, PieceColor::WHITE, src, Position(3, 4), false));
}

TEST_CASE("PieceRules: Queen movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::QUEEN, PieceColor::WHITE, src, Position(5, 5), false));
    CHECK(PieceRules::is_valid_geometry(PieceKind::QUEEN, PieceColor::WHITE, src, Position(3, 7), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::QUEEN, PieceColor::WHITE, src, Position(5, 4), false));
}

TEST_CASE("PieceRules: Knight movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::KNIGHT, PieceColor::WHITE, src, Position(5, 4), false));
    CHECK(PieceRules::is_valid_geometry(PieceKind::KNIGHT, PieceColor::WHITE, src, Position(2, 5), false));
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::KNIGHT, PieceColor::WHITE, src, Position(4, 4), false));
}

TEST_CASE("PieceRules: Pawn movement") {
    // חייל לבן ב-(1,1) נע ל-(2,1)
    CHECK(PieceRules::is_valid_geometry(PieceKind::PAWN, PieceColor::WHITE, Position(1, 1), Position(2, 1), false));
    // חייל לבן לא יכול לרדת למטה
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::PAWN, PieceColor::WHITE, Position(1, 1), Position(0, 1), false));
    
    // חייל שחור ב-(6,1) נע ל-(5,1)
    CHECK(PieceRules::is_valid_geometry(PieceKind::PAWN, PieceColor::BLACK, Position(6, 1), Position(5, 1), false));
    
    // תפיסה באלכסון
    CHECK(PieceRules::is_valid_geometry(PieceKind::PAWN, PieceColor::WHITE, Position(1, 1), Position(2, 2), true));
    // לא יכול לתפוס ישר
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::PAWN, PieceColor::WHITE, Position(1, 1), Position(2, 1), true));
}
#include "../../doctest.h"
#include "rules/piece_rules.h"
#include "model/position.h"

using namespace rules;
using namespace model;

TEST_CASE("PieceRules: King movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::KING, src, Position(4, 4))); // אלכסון אחד
    CHECK(PieceRules::is_valid_geometry(PieceKind::KING, src, Position(3, 4))); // ישר אחד
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::KING, src, Position(5, 3))); // רחוק מדי
}

TEST_CASE("PieceRules: Rook movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::ROOK, src, Position(3, 7))); // אופקי
    CHECK(PieceRules::is_valid_geometry(PieceKind::ROOK, src, Position(0, 3))); // אנכי
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::ROOK, src, Position(4, 4))); // אלכסון נדחה
}

TEST_CASE("PieceRules: Bishop movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::BISHOP, src, Position(5, 5))); // אלכסון
    CHECK(PieceRules::is_valid_geometry(PieceKind::BISHOP, src, Position(1, 1))); // אלכסון
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::BISHOP, src, Position(3, 4))); // ישר נדחה
}

TEST_CASE("PieceRules: Queen movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::QUEEN, src, Position(5, 5))); // אלכסון
    CHECK(PieceRules::is_valid_geometry(PieceKind::QUEEN, src, Position(3, 7))); // ישר
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::QUEEN, src, Position(5, 4))); // פרש נדחה
}

TEST_CASE("PieceRules: Knight movement") {
    Position src(3, 3);
    CHECK(PieceRules::is_valid_geometry(PieceKind::KNIGHT, src, Position(5, 4))); // צורת L
    CHECK(PieceRules::is_valid_geometry(PieceKind::KNIGHT, src, Position(2, 5))); // צורת L
    CHECK_FALSE(PieceRules::is_valid_geometry(PieceKind::KNIGHT, src, Position(4, 4))); // אלכסון נדחה
}
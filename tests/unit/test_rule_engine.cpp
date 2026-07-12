#include "../../doctest.h"
#include "model/board.h"
#include "rules/piece_rules.h"

TEST_CASE("Capture and Blockers Logic") {
    auto board = std::make_shared<model::Board>(8, 8);
    
    // הצבת צריח לבן ב-(0,0) וחייל לבן ב-(0,1) - חסימה
    board->add_piece(std::make_shared<model::Piece>("wR", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0)));
    board->add_piece(std::make_shared<model::Piece>("wP", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(0, 1)));
    
    // הצבת חייל שחור ב-(0,2) - אויב
    board->add_piece(std::make_shared<model::Piece>("bP", model::PieceColor::BLACK, model::PieceKind::PAWN, model::Position(0, 2)));

    // בדיקה: צריח לא יכול לעבור דרך החייל הלבן
    CHECK_FALSE(rules::PieceRules::is_path_clear(*board, model::Position(0, 0), model::Position(0, 2)));

    // בדיקה: לא ניתן לאכול את החייל הלבן (אותו צבע)
    // (כאן נכנסת הלוגיקה של GameEngine שבודקת צבעים)
}
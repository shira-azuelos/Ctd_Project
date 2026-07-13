#include "../../doctest.h"
#include "model/board.h"
#include "rules/piece_rules.h"

TEST_CASE("Capture and Blockers Logic") {
    auto board = std::make_shared<model::Board>(8, 8);
    
    board->add_piece(std::make_shared<model::Piece>("wR", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0)));
    board->add_piece(std::make_shared<model::Piece>("wP", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(0, 1)));
    
    board->add_piece(std::make_shared<model::Piece>("bP", model::PieceColor::BLACK, model::PieceKind::PAWN, model::Position(0, 2)));

    CHECK_FALSE(rules::PieceRules::is_path_clear(*board, model::Position(0, 0), model::Position(0, 2)));

}
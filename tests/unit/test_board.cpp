#include "../../doctest.h"
#include "../../include/model/board.h"
#include "../../include/model/piece.h"

TEST_CASE("Board boundary and tracking testing") {
    model::Board board(8, 8);
    CHECK(board.get_width() == 8);
    CHECK(board.get_height() == 8);

    model::Position valid_pos(5, 5);
    model::Position invalid_pos(9, 2);
    CHECK(board.is_in_bounds(valid_pos));
    CHECK_FALSE(board.is_in_bounds(invalid_pos));

    auto piece = std::make_shared<model::Piece>("wP", model::PieceColor::WHITE, model::PieceKind::PAWN, valid_pos);
    board.add_piece(piece);
    CHECK(board.get_piece_at(valid_pos).get() == piece.get());

    board.remove_piece(valid_pos);
    CHECK(board.get_piece_at(valid_pos).get() == nullptr);
}
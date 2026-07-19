#include "model/board_factory.h"
#include "model/piece.h"
#include <vector>
#include <string>

namespace model {

std::shared_ptr<Board> BoardFactory::create_default_board() {
    auto board = std::make_shared<Board>(8, 8);

    std::vector<PieceKind> kinds = {
        PieceKind::ROOK, PieceKind::KNIGHT, PieceKind::BISHOP, PieceKind::QUEEN,
        PieceKind::KING, PieceKind::BISHOP, PieceKind::KNIGHT, PieceKind::ROOK
    };
    std::vector<std::string> ids = {"R1", "N1", "B1", "Q", "K", "B2", "N2", "R2"};

    for (int col = 0; col < 8; ++col) {
        board->add_piece(std::make_shared<Piece>("b" + ids[col], PieceColor::BLACK, kinds[col], Position(0, col)));
        board->add_piece(std::make_shared<Piece>("w" + ids[col], PieceColor::WHITE, kinds[col], Position(7, col)));
        
        board->add_piece(std::make_shared<Piece>("bP" + std::to_string(col), PieceColor::BLACK, PieceKind::PAWN, Position(1, col)));
        board->add_piece(std::make_shared<Piece>("wP" + std::to_string(col), PieceColor::WHITE, PieceKind::PAWN, Position(6, col)));
    }

    return board;
}

}

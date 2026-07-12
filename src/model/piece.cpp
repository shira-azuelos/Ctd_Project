#include "model/piece.h"
#include <utility>

namespace model {

Piece::Piece(std::string id, PieceColor color, PieceKind kind, Position cell)
    : id(std::move(id)), color(color), kind(kind), cell(cell), state(PieceState::IDLE) {}

} 
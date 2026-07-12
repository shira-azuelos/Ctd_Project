#pragma once
#include "../model/piece.h"
#include "../model/position.h"
#include "../model/board.h"

namespace rules {

class PieceRules {
public:
    static bool is_valid_geometry(model::PieceKind kind, model::PieceColor color, const model::Position& src, const model::Position& dest, bool is_capturing);
    
    static bool is_path_clear(const model::Board& board, const model::Position& src, const model::Position& dest);
};

} 
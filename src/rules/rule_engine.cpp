#include "../include/rules/rule_engine.h"
#include "../include/rules/piece_rules.h"

namespace rules {

bool RuleEngine::validate_move(const model::Board& board, const model::Position& src, const model::Position& dest) {
    auto piece = board.get_piece_at(src);
    if (!piece) return false;

    auto target = board.get_piece_at(dest);
    if (target && target->color == piece->color) return false;

    bool is_capturing = (target != nullptr);
    if (!PieceRules::is_valid_geometry(piece->kind, piece->color, src, dest, is_capturing)) {
        return false;
    }

    if (!PieceRules::is_path_clear(board, src, dest)) {
        return false;
    }

    return true;
}

} 
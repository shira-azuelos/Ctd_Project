#include "engine/game_engine.h"
#include "rules/piece_rules.h"
#include <cmath>
#include <algorithm>

namespace engine {

GameEngine::GameEngine(std::shared_ptr<model::Board> b) : board(b) {}

void GameEngine::request_move(const model::Position& source, const model::Position& dest) {
    auto piece = board->get_piece_at(source);
    if (!piece) return;

    auto target = board->get_piece_at(dest);
    bool is_capturing = (target != nullptr);

    if (!rules::PieceRules::is_valid_geometry(piece->kind, piece->color, source, dest, is_capturing)) {
        return; 
    }

    if (!rules::PieceRules::is_path_clear(*board, source, dest)) {
        return;
    }

    if (is_capturing) {
        if (target->color == piece->color) {
            return;
        }
    }

    int dist = std::max(std::abs(dest.row - source.row), std::abs(dest.col - source.col));
    arbiter.start_motion(piece, source, dest, dist * 1000);
}

void engine::GameEngine::wait(int ms) {
    arbiter.advance_time(ms, board);
}

} 
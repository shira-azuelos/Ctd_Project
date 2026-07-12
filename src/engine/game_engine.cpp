#include "engine/game_engine.h"
#include "rules/piece_rules.h"
#include <cmath>
#include <algorithm>

namespace engine {

GameEngine::GameEngine(std::shared_ptr<model::Board> b) : board(b) {}

void GameEngine::request_move(const model::Position& source, const model::Position& dest) {
    auto piece = board->get_piece_at(source);
    if (!piece) return;

    if (!rules::PieceRules::is_valid_geometry(piece->kind, source, dest)) return;

    if (!rules::PieceRules::is_path_clear(*board, source, dest)) return;

    auto target = board->get_piece_at(dest);
    if (target) {
        if (target->color == piece->color) return; 
    }

    int dist = std::max(std::abs(dest.row - source.row), std::abs(dest.col - source.col));
    arbiter.start_motion(piece, source, dest, dist * 1000);
}

void GameEngine::wait(int ms) {
    arbiter.advance_time(ms, board);
}

} 
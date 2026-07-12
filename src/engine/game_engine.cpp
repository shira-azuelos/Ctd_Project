#include "engine/game_engine.h"
#include <cmath>
#include <algorithm>

namespace engine {

GameEngine::GameEngine(std::shared_ptr<model::Board> b) : board(b) {}

void GameEngine::request_move(const model::Position& source, const model::Position& dest) {
    auto piece = board->get_piece_at(source);
    if (!piece) return;

    int dist_row = std::abs(dest.row - source.row);
    int dist_col = std::abs(dest.col - source.col);
    int distance = std::max(dist_row, dist_col);
    int duration = distance * 1000;

    arbiter.start_motion(piece, source, dest, duration);
}

void GameEngine::wait(int ms) {
    arbiter.advance_time(ms, board);
}

} 
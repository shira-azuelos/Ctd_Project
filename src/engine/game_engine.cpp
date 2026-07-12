#include "../include/engine/game_engine.h"
#include "../include/rules/rule_engine.h"
#include <cmath>
#include <algorithm>

namespace engine {

void GameEngine::request_move(const model::Position& src, const model::Position& dest) {
    if (state->is_game_over()) return;
    if (arbiter.is_moving()) return;

    auto board = state->get_board();
    if (!rules::RuleEngine::validate_move(*board, src, dest)) {
        return;
    }

    auto piece = board->get_piece_at(src);
    int dist = std::max(std::abs(dest.row - src.row), std::abs(dest.col - src.col));
    arbiter.start_motion(piece, src, dest, dist * BASE_MOVE_TIME_MS);
}

void GameEngine::request_jump(const model::Position& pos) {
    if (state->is_game_over()) return;

    auto piece = state->get_board()->get_piece_at(pos);
    if (!piece || arbiter.is_piece_moving(piece)) return;

    arbiter.start_jump(piece, pos, 1000);
}

void GameEngine::wait(int ms) {
    arbiter.advance_time(ms, state->get_board());
    state->check_game_status();
}

} 
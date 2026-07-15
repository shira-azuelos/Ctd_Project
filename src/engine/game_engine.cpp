#include "../include/engine/game_engine.h"
#include "../include/rules/rule_engine.h"
#include <cmath>
#include <algorithm>

namespace engine {

GameEngine::GameEngine(std::shared_ptr<model::Board> b) {
    state = std::make_shared<model::GameState>(b);
}

std::shared_ptr<model::GameState> GameEngine::get_state() const {
    return state;
}

bool GameEngine::is_moving() const {
    return arbiter.is_moving();
}

void GameEngine::request_move(const model::Position& src, const model::Position& dest) {
    if (state->is_game_over()) return;
    if (arbiter.is_moving()) return;

    auto board = state->get_board();
    auto piece = board->get_piece_at(src);
    if (!piece || arbiter.is_piece_cooling_down(piece)) {
        return;
    }

    if (!rules::RuleEngine::validate_move(*board, src, dest)) {
        return;
    }

    int dist = std::max(std::abs(dest.row - src.row), std::abs(dest.col - src.col));
    arbiter.start_motion(piece, src, dest, dist * BASE_MOVE_TIME_MS);
}

void GameEngine::request_jump(const model::Position& pos) {
    if (state->is_game_over()) return;

    auto piece = state->get_board()->get_piece_at(pos);
    if (!piece || arbiter.is_piece_moving(piece) || arbiter.is_piece_cooling_down(piece)) return;

    arbiter.start_jump(piece, pos, 1000);
}


void GameEngine::wait(int ms) {
    arbiter.advance_time(ms, state->get_board());
    state->check_game_status();
}

std::optional<realtime::Motion> GameEngine::get_active_motion() const {
    return arbiter.get_active_motion();
}

bool GameEngine::is_piece_cooling_down(std::shared_ptr<model::Piece> piece) const {
    return arbiter.is_piece_cooling_down(piece);
}

int GameEngine::get_piece_cooldown_remaining_ms(std::shared_ptr<model::Piece> piece) const {
    return arbiter.get_piece_cooldown_remaining_ms(piece);
}

int GameEngine::get_piece_cooldown_total_ms(std::shared_ptr<model::Piece> piece) const {
    return arbiter.get_piece_cooldown_total_ms(piece);
}

const realtime::RealTimeArbiter& GameEngine::get_arbiter() const {
    return arbiter;
}

} 
#include "../include/model/game_state.h"

namespace model {

GameState::GameState(std::shared_ptr<Board> b) : board(b) {
    int w = board->get_width();
    int h = board->get_height();
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            auto p = board->get_piece_at(Position(r, c));
            if (p && p->kind == PieceKind::KING) {
                if (p->color == PieceColor::WHITE) initial_w_kings++;
                else initial_b_kings++;
            }
        }
    }
}

std::shared_ptr<Board> GameState::get_board() const {
    return board;
}

bool GameState::is_game_over() const {
    return game_over_flag;
}

void GameState::set_game_over(bool state) {
    game_over_flag = state;
}

void GameState::check_game_status() {
    if (game_over_flag) return;
    int w_kings = 0, b_kings = 0;
    int w = board->get_width(), h = board->get_height();
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            auto piece = board->get_piece_at(Position(r, c));
            if (piece && piece->kind == PieceKind::KING) {
                if (piece->color == PieceColor::WHITE) w_kings++;
                else b_kings++;
            }
        }
    }
    if ((initial_w_kings > 0 && w_kings == 0) || (initial_b_kings > 0 && b_kings == 0)) {
        game_over_flag = true;
    }
}

int GameState::get_white_score() const {
    return white_score;
}

int GameState::get_black_score() const {
    return black_score;
}

void GameState::add_to_white_score(int pts) {
    white_score += pts;
}

void GameState::add_to_black_score(int pts) {
    black_score += pts;
}

} 
#include "../include/model/game_state.h"

namespace model {

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

} 
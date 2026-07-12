#pragma once
#include <memory>
#include "../model/board.h"

namespace model {

class GameState {
private:
    std::shared_ptr<Board> board;
    bool game_over_flag = false;
    int initial_w_kings = 0;
    int initial_b_kings = 0;

public:
    GameState(std::shared_ptr<Board> b) : board(b) {
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
    
    std::shared_ptr<Board> get_board() const { return board; }
    bool is_game_over() const { return game_over_flag; }
    void set_game_over(bool state) { game_over_flag = state; }
    
    void check_game_status();
};

} 
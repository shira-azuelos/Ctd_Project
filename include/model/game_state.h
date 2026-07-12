#pragma once
#include <memory>
#include "../include/model/board.h"

namespace model {

class GameState {
private:
    std::shared_ptr<Board> board;
    bool game_over_flag = false;

public:
    GameState(std::shared_ptr<Board> b) : board(b) {}
    
    std::shared_ptr<Board> get_board() const { return board; }
    bool is_game_over() const { return game_over_flag; }
    void set_game_over(bool state) { game_over_flag = state; }
    
    void check_game_status();
};

}
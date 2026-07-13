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
    GameState(std::shared_ptr<Board> b);
    
    std::shared_ptr<Board> get_board() const;

    bool is_game_over() const;
    
    void set_game_over(bool state);
    
    void check_game_status();
};

} 
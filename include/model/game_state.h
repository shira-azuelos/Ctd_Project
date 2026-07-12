#pragma once
#include <memory>
#include "board.h"

namespace model {

class GameState {
public:
    std::shared_ptr<Board> board;
    bool game_over;

    explicit GameState(std::shared_ptr<Board> b);
};

} 
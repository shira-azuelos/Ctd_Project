#include "model/game_state.h"
#include <utility>

namespace model {

GameState::GameState(std::shared_ptr<Board> b) 
    : board(std::move(b)), game_over(false) {}

} 
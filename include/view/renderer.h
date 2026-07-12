#pragma once
#include <memory>
#include <vector>
#include <string>
#include "../include/model/game_state.h"

namespace view {
class Renderer {
public:
    static std::vector<std::string> render_board(std::shared_ptr<model::GameState> state);
};
}
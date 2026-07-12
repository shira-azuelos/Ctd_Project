#pragma once
#include <string>
#include <vector>
#include <memory>
#include "model/game_state.h"

namespace io {

class BoardPrinter {
public:
    static std::vector<std::string> print(const std::shared_ptr<model::GameState>& state);
};

} 
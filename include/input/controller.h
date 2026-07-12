#pragma once
#include <memory>
#include <optional>
#include "model/position.h"
#include "model/board.h"
#include "engine/game_engine.h"

namespace input {

class Controller {
private:
    std::shared_ptr<engine::GameEngine> engine;
    std::shared_ptr<model::Board> board;
    std::optional<model::Position> selected_cell;

public:
    Controller(std::shared_ptr<engine::GameEngine> eng, std::shared_ptr<model::Board> b);
    void click(int x, int y);
};

} 
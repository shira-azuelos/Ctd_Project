#pragma once
#include <memory>
#include <optional>
#include <opencv2/opencv.hpp>
#include "model/board.h"
#include "model/position.h"
#include "engine/game_engine.h"

namespace input {

struct GuiState {
    std::shared_ptr<engine::GameEngine> game_engine;
    std::shared_ptr<model::Board> board;
    std::optional<model::Position> selected_cell;
    std::shared_ptr<model::Piece> dragged_piece = nullptr;
    int drag_x = 0;
    int drag_y = 0;
};

class GuiController {
public:
    static void on_mouse(int event, int x, int y, int flags, void* userdata);
};

}

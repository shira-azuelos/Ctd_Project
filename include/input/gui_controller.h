#pragma once
#include <memory>
#include <optional>
#include <opencv2/opencv.hpp>
#include "model/board.h"
#include "model/position.h"
#include "engine/game_engine.h"

namespace network {
class SocketClient;
}

namespace input {

struct GuiState {
    std::shared_ptr<engine::GameEngine> game_engine = nullptr;
    std::shared_ptr<network::SocketClient> socket_client = nullptr;
    std::shared_ptr<model::Board> board;
    std::optional<model::Position> selected_cell;
    std::shared_ptr<model::Piece> dragged_piece = nullptr;
    int drag_x = 0;
    int drag_y = 0;
    bool in_opening_screen = true;
};

class GuiController {
private:
    static bool handle_opening_click(GuiState* g_state, int event, int x, int y);
    static void handle_left_click_down(GuiState* g_state, const model::Position& cell, int x, int y);
    static void handle_left_click_up(GuiState* g_state, const model::Position& cell);
    static void handle_right_click_down(GuiState* g_state, const model::Position& cell);

public:
    static void on_mouse(int event, int x, int y, int flags, void* userdata);
};

}

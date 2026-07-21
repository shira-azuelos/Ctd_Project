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

constexpr int KEY_BACKSPACE = 8;
constexpr int KEY_ENTER_CR = 13;
constexpr int KEY_ENTER_LF = 10;
constexpr int ASCII_PRINTABLE_MIN = 32;
constexpr int ASCII_PRINTABLE_MAX = 126;
constexpr size_t MAX_ROOM_INPUT_LENGTH = 20;

struct GuiState {
    std::shared_ptr<engine::GameEngine> game_engine = nullptr;
    std::shared_ptr<network::SocketClient> socket_client = nullptr;
    std::shared_ptr<model::Board> board;
    std::optional<model::Position> selected_cell;
    std::shared_ptr<model::Piece> dragged_piece = nullptr;
    int drag_x = 0;
    int drag_y = 0;
    bool in_opening_screen = true;

    bool show_room_dialog = false;
    std::string room_input_text = "";
};

class GuiController {
private:
    static bool handle_opening_click(GuiState* g_state, int event, int x, int y);
    static void handle_left_click_down(GuiState* g_state, const model::Position& cell, int x, int y);
    static void handle_left_click_up(GuiState* g_state, const model::Position& cell);
    static void handle_right_click_down(GuiState* g_state, const model::Position& cell);

public:
    static void on_mouse(int event, int x, int y, int flags, void* userdata);
    static bool on_key(int key, GuiState* g_state);
};

}

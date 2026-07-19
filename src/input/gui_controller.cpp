#include "input/gui_controller.h"
#include "input/board_mapper.h"
#include "pubsub/message_bus.h"
#include <iostream>

namespace input {

void GuiController::on_mouse(int event, int x, int y, int flags, void* userdata) {
    auto* g_state = static_cast<GuiState*>(userdata);
    if (g_state->game_engine->get_state()->is_game_over()) {
        g_state->dragged_piece = nullptr;
        return;
    }

    int board_x = x - 100;

    if (event == cv::EVENT_MOUSEMOVE) {
        if (g_state->dragged_piece) {
            g_state->drag_x = x;
            g_state->drag_y = y;
        }
    }
    
    auto cell_opt = BoardMapper::pixel_to_cell(board_x, y, g_state->board->get_width(), g_state->board->get_height());
    if (!cell_opt) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
        return;
    }

    model::Position cell = *cell_opt;

    if (event == cv::EVENT_LBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        auto selected_piece = g_state->selected_cell ? g_state->board->get_piece_at(*g_state->selected_cell) : nullptr;

        if (clicked_piece && (!selected_piece || clicked_piece->color == selected_piece->color)) {
            if (!g_state->game_engine->is_piece_cooling_down(clicked_piece)) {
                g_state->selected_cell = cell;
                g_state->dragged_piece = clicked_piece;
                g_state->drag_x = x;
                g_state->drag_y = y;
            }
        } else if (g_state->selected_cell) {
            g_state->game_engine->request_move(*g_state->selected_cell, cell);
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        if (g_state->dragged_piece) {
            auto source_cell = g_state->dragged_piece->cell;
            if (source_cell != cell) {
                g_state->game_engine->request_move(source_cell, cell);
                g_state->selected_cell.reset();
            }
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        if (clicked_piece) {
            g_state->game_engine->request_jump(cell);
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
    }
}

}

#include "input/controller.h"
#include "input/board_mapper.h"

namespace input {

Controller::Controller(std::shared_ptr<engine::GameEngine> eng, std::shared_ptr<model::Board> b)
    : engine(eng), board(b) {}

void Controller::click(int x, int y) {
    auto cell_opt = BoardMapper::pixel_to_cell(x, y, board->get_width(), board->get_height());
    
    if (!cell_opt) {
        if (selected_cell) {
            selected_cell.reset();
        }
        return;
    }

    model::Position cell = *cell_opt;
    auto clicked_piece = board->get_piece_at(cell);

    if (!selected_cell) {
        if (clicked_piece) {
            selected_cell = cell;
        }
    } else {
        auto selected_piece = board->get_piece_at(*selected_cell);
        if (clicked_piece && clicked_piece->color == selected_piece->color) {
            selected_cell = cell;
        } else {
            engine->request_move(*selected_cell, cell);
            selected_cell.reset();
        }
    }
}

} 
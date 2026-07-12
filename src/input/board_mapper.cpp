#include "input/board_mapper.h"

namespace input {

std::optional<model::Position> BoardMapper::pixel_to_cell(int x, int y, int board_width, int board_height) {
    if (x < 0 || y < 0) {
        return std::nullopt;
    }

    int col = x / CELL_SIZE;
    int row = y / CELL_SIZE;
    
    if (row >= 0 && row < board_height && col >= 0 && col < board_width) {
        return model::Position(row, col);
    }
    return std::nullopt; 
}

} 
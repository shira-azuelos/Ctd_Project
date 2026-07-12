#pragma once
#include <optional>
#include "model/position.h"

namespace input {

class BoardMapper {
public:
    static const int CELL_SIZE = 100;

    static std::optional<model::Position> pixel_to_cell(int x, int y, int board_width, int board_height);
};

}
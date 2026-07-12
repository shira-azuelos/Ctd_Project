#include "model/position.h"

namespace model {

Position::Position(int r, int c) : row(r), col(c) {}

bool Position::operator==(const Position& other) const {
    return row == other.row && col == other.col;
}

bool Position::operator!=(const Position& other) const {
    return !(*this == other);
}

std::string Position::to_string() const {
    return "(" + std::to_string(row) + ", " + std::to_string(col) + ")";
}

} 
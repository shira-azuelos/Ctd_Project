#include "model/board.h"
#include <stdexcept>

namespace model {

Board::Board(int w, int h) : width(w), height(h) {
    grid.resize(width * height, nullptr);
}


int Board::get_width() const { return width; }
int Board::get_height() const { return height; }

bool Board::is_in_bounds(const Position& pos) const {
    return pos.row >= 0 && pos.row < height && pos.col >= 0 && pos.col < width;
}

std::shared_ptr<Piece> Board::get_piece_at(const Position& pos) const {
    if (!is_in_bounds(pos)) return nullptr;
    return grid[pos.row * width + pos.col];
}

bool Board::add_piece(std::shared_ptr<Piece> piece) {
    if (!piece || !is_in_bounds(piece->cell)) return false;
    
    if (grid[piece->cell.row * width + piece->cell.col] != nullptr) {
        return false;
    }
    
    grid[piece->cell.row * width + piece->cell.col] = piece;
    return true;
}

void Board::remove_piece(const Position& pos) {
    if (is_in_bounds(pos)) {
        if (grid[pos.row * width + pos.col]) {
            grid[pos.row * width + pos.col]->state = PieceState::CAPTURED;
        }
        grid[pos.row * width + pos.col] = nullptr;
    }
}

void Board::move_piece(const Position& source, const Position& dest) {
    if (!is_in_bounds(source) || !is_in_bounds(dest)) {
        throw std::invalid_argument("Source or destination out of bounds");
    }

    std::shared_ptr<Piece> moving_piece = grid[source.row * width + source.col];
    if (!moving_piece) {
        throw std::invalid_argument("No piece at source position");
    }

    grid[source.row * width + source.col] = nullptr;
    moving_piece->cell = dest;
    grid[dest.row * width + dest.col] = moving_piece;
}

} 
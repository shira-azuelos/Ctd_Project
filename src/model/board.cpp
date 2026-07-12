#include "model/board.h"
#include <stdexcept>

namespace model {

Board::Board(int w, int h) : width(w), height(h) {
    grid.resize(height, std::vector<std::shared_ptr<Piece>>(width, nullptr));
}

int Board::get_width() const { return width; }
int Board::get_height() const { return height; }

bool Board::is_in_bounds(const Position& pos) const {
    return pos.row >= 0 && pos.row < height && pos.col >= 0 && pos.col < width;
}

std::shared_ptr<Piece> Board::get_piece_at(const Position& pos) const {
    if (!is_in_bounds(pos)) return nullptr;
    return grid[pos.row][pos.col];
}

bool Board::add_piece(std::shared_ptr<Piece> piece) {
    if (!piece || !is_in_bounds(piece->cell)) return false;
    
    if (grid[piece->cell.row][piece->cell.col] != nullptr) {
        return false;
    }
    
    grid[piece->cell.row][piece->cell.col] = piece;
    return true;
}

void Board::remove_piece(const Position& pos) {
    if (is_in_bounds(pos)) {
        if (grid[pos.row][pos.col]) {
            grid[pos.row][pos.col]->state = PieceState::CAPTURED;
        }
        grid[pos.row][pos.col] = nullptr;
    }
}

void Board::move_piece(const Position& source, const Position& dest) {
    if (!is_in_bounds(source) || !is_in_bounds(dest)) {
        throw std::invalid_argument("Source or destination out of bounds");
    }

    std::shared_ptr<Piece> moving_piece = grid[source.row][source.col];
    if (!moving_piece) {
        throw std::invalid_argument("No piece at source position");
    }

    grid[source.row][source.col] = nullptr;
    moving_piece->cell = dest;
    grid[dest.row][dest.col] = moving_piece;
}

} 
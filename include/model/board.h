#pragma once
#include <vector>
#include <memory>
#include "piece.h"
#include "position.h"

namespace model {
class Board {
private:
    int width;
    int height;
    std::vector<std::vector<std::shared_ptr<Piece>>> grid;

public:
    Board(int w, int h);

    int get_width() const;
    int get_height() const;

    bool is_in_bounds(const Position& pos) const;

    std::shared_ptr<Piece> get_piece_at(const Position& pos) const;

    bool add_piece(std::shared_ptr<Piece> piece);

    void remove_piece(const Position& pos);

    void move_piece(const Position& source, const Position& dest);
};

} 
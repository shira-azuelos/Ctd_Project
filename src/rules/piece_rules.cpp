#include "rules/piece_rules.h"
#include <cmath>

namespace rules {

bool PieceRules::is_valid_geometry(model::PieceKind kind, model::PieceColor color, const model::Position& src, const model::Position& dest, bool is_capturing) {
    int d_row = dest.row - src.row;
    int d_col = std::abs(dest.col - src.col);

    if (kind == model::PieceKind::PAWN) {
        int direction = (color == model::PieceColor::WHITE) ? 1 : -1;

        if (is_capturing) {
            return (d_row == direction) && (d_col == 1);
        } else {
            return (d_row == direction) && (d_col == 0);
        }
    }

    switch (kind) {
        case model::PieceKind::KING:
            return std::abs(d_row) <= 1 && d_col <= 1;
        case model::PieceKind::ROOK:
            return (d_row == 0) || (d_col == 0);
        case model::PieceKind::BISHOP:
            return std::abs(d_row) == d_col;
        case model::PieceKind::QUEEN:
            return (d_row == 0 || d_col == 0) || (std::abs(d_row) == d_col);
        case model::PieceKind::KNIGHT:
            return (std::abs(d_row) == 2 && d_col == 1) || (std::abs(d_row) == 1 && d_col == 2);
        default:
            return false;
    }
}

bool PieceRules::is_path_clear(const model::Board& board, const model::Position& src, const model::Position& dest) {
    auto piece = board.get_piece_at(src);
    if (piece && piece->kind == model::PieceKind::KNIGHT) return true;

    int row_step = (dest.row > src.row) ? 1 : (dest.row < src.row) ? -1 : 0;
    int col_step = (dest.col > src.col) ? 1 : (dest.col < src.col) ? -1 : 0;

    int curr_row = src.row + row_step;
    int curr_col = src.col + col_step;

    while (curr_row != dest.row || curr_col != dest.col) {
        if (board.get_piece_at(model::Position(curr_row, curr_col))) {
            return false;
        }
        curr_row += row_step;
        curr_col += col_step;
    }
    return true;
}

}
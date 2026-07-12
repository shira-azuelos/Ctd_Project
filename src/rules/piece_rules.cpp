#include "rules/piece_rules.h"
#include <cmath>

namespace rules {

bool PieceRules::is_valid_geometry(model::PieceKind kind, const model::Position& src, const model::Position& dest) {
    if (src.row == dest.row && src.col == dest.col) {
        return false;
    }

    int d_row = std::abs(dest.row - src.row);
    int d_col = std::abs(dest.col - src.col);

    switch (kind) {
        case model::PieceKind::KING:
            return d_row <= 1 && d_col <= 1;
        case model::PieceKind::ROOK:
            return d_row == 0 || d_col == 0;
        case model::PieceKind::BISHOP:
            return d_row == d_col;
        case model::PieceKind::QUEEN:
            return (d_row == 0 || d_col == 0) || (d_row == d_col);
        case model::PieceKind::KNIGHT:
            return (d_row == 2 && d_col == 1) || (d_row == 1 && d_col == 2);
        case model::PieceKind::PAWN:
            return (dest.row == src.row + 1) && (dest.col == src.col);
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
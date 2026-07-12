#include "../include/model/game_state.h"

namespace model {

void GameState::check_game_status() {
    if (game_over_flag) return;

    bool white_king = false;
    bool black_king = false;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            auto piece = board->get_piece_at(Position(r, c));
            if (piece && piece->kind == PieceKind::KING) {
                if (piece->color == PieceColor::WHITE) white_piece_king = true; 
                if (piece->color == PieceColor::BLACK) black_piece_king = true;
                white_king = true; 
                black_king = true;
            }
        }
    }

    if (!white_king || !black_king) {
        game_over_flag = true;
        return;
    }

    for (int c = 0; c < 8; ++c) {
        auto white_pawn = board->get_piece_at(Position(7, c));
        if (white_pawn && white_pawn->kind == PieceKind::PAWN && white_pawn->color == PieceColor::WHITE) {
            white_pawn->kind = PieceKind::QUEEN;
        }

        auto black_pawn = board->get_piece_at(Position(0, c));
        if (black_pawn && black_pawn->kind == PieceKind::PAWN && black_pawn->color == PieceColor::BLACK) {
            black_pawn->kind = PieceKind::QUEEN;
        }
    }
}

} 
#include "engine/game_engine.h"
#include "rules/piece_rules.h"
#include <cmath>
#include <algorithm>

namespace engine {

void GameEngine::update_game_state() {
    if (game_over_flag) return;

    bool white_king_alive = false;
    bool black_king_alive = false;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            auto piece = board->get_piece_at(model::Position(r, c));
            if (piece && piece->kind == model::PieceKind::KING) {
                if (piece->color == model::PieceColor::WHITE) white_king_alive = true;
                if (piece->color == model::PieceColor::BLACK) black_king_alive = true;
            }
        }
    }

    if (!white_king_alive || !black_king_alive) {
        game_over_flag = true;
        return;
    }

    for (int c = 0; c < 8; ++c) {
        auto top_piece = board->get_piece_at(model::Position(7, c));
        if (top_piece && top_piece->color == model::PieceColor::WHITE && top_piece->kind == model::PieceKind::PAWN) {
            top_piece->kind = model::PieceKind::QUEEN;
        }

        auto bottom_piece = board->get_piece_at(model::Position(0, c));
        if (bottom_piece && bottom_piece->color == model::PieceColor::BLACK && bottom_piece->kind == model::PieceKind::PAWN) {
            bottom_piece->kind = model::PieceKind::QUEEN;
        }
    }
}

void GameEngine::request_move(const model::Position& source, const model::Position& dest) {
    if (game_over_flag) {
        return;
    }

    if (arbiter.is_moving()) {
        return;
    }

    auto piece = board->get_piece_at(source);
    if (!piece) {
        return;
    }

    auto target = board->get_piece_at(dest);
    if (target && target->color == piece->color) {
        return;
    }

    bool is_capturing = (target != nullptr);
    if (!rules::PieceRules::is_valid_geometry(piece->kind, piece->color, source, dest, is_capturing)) {
        return; 
    }

    if (!rules::PieceRules::is_path_clear(*board, source, dest)) {
        return;
    }

    int dist = std::max(std::abs(dest.row - source.row), std::abs(dest.col - source.col));
    arbiter.start_motion(piece, source, dest, dist * BASE_MOVE_TIME_MS);
}

void GameEngine::wait(int ms) {
    arbiter.advance_time(ms, board); 
    update_game_state();
}

} 
#include "../include/realtime/real_time_arbiter.h"

namespace realtime {

void RealTimeArbiter::start_motion(std::shared_ptr<model::Piece> piece, model::Position src, model::Position dst, int total_ms) {
    active_motion = Motion{piece, src, dst, total_ms, total_ms};
}

void RealTimeArbiter::start_jump(std::shared_ptr<model::Piece> piece, model::Position pos, int total_ms) {
    active_jump = Jump{piece, pos, total_ms};
}

void RealTimeArbiter::advance_time(int ms, std::shared_ptr<model::Board> board) {
    if (active_jump.has_value()) {
        active_jump->remaining_ms -= ms;
    }

    if (active_motion.has_value()) {
        active_motion->remaining_ms -= ms;
        
        if (active_motion->remaining_ms <= 0) {
            auto dest = active_motion->dest;
            auto src = active_motion->source;
            auto moving_piece = active_motion->piece;
            bool captured_in_air = false;
            
            if (active_jump.has_value() && active_jump->pos == dest && active_jump->remaining_ms >= 0) {
                if (active_jump->piece->color != moving_piece->color) {
                    board->remove_piece(src); 
                    captured_in_air = true;
                }
            }
            
            if (!captured_in_air) {
                board->move_piece(src, dest);
                if (moving_piece->kind == model::PieceKind::PAWN) {
                    if (moving_piece->color == model::PieceColor::WHITE && dest.row == 0) {
                        moving_piece->kind = model::PieceKind::QUEEN;
                    } else if (moving_piece->color == model::PieceColor::BLACK && dest.row == board->get_height() - 1) {
                        moving_piece->kind = model::PieceKind::QUEEN;
                    }
                }
            }
            active_motion.reset();
        }
    }
    
    if (active_jump.has_value() && active_jump->remaining_ms <= 0) {
        active_jump.reset();
    }
}

} 
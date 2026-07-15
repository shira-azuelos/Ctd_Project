#include "../include/realtime/real_time_arbiter.h"

namespace realtime {

void RealTimeArbiter::start_motion(std::shared_ptr<model::Piece> piece, model::Position src, model::Position dst, int total_ms) {
    if (piece) piece->state = model::PieceState::MOVING;
    active_motion = Motion{piece, src, dst, total_ms, total_ms};
}

void RealTimeArbiter::start_jump(std::shared_ptr<model::Piece> piece, model::Position pos, int total_ms) {
    if (piece) piece->state = model::PieceState::MOVING;
    active_jump = Jump{piece, pos, total_ms, total_ms};
}

void RealTimeArbiter::advance_time(int ms, std::shared_ptr<model::Board> board) {
    for (auto it = active_cooldowns.begin(); it != active_cooldowns.end(); ) {
        it->remaining_ms -= ms;
        if (it->remaining_ms <= 0) {
            it = active_cooldowns.erase(it);
        } else {
            ++it;
        }
    }

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
                if (moving_piece) {
                    moving_piece->state = model::PieceState::IDLE;
                    active_cooldowns.push_back(Cooldown{moving_piece, 3000, 3000, false});
                }
                if (moving_piece && moving_piece->kind == model::PieceKind::PAWN) {
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
        if (active_jump->piece) {
            active_jump->piece->state = model::PieceState::IDLE;
            active_cooldowns.push_back(Cooldown{active_jump->piece, 3000, 3000, true});
        }
        active_jump.reset();
    }
}

bool RealTimeArbiter::is_moving() const {
    return active_motion.has_value();
}

bool RealTimeArbiter::is_piece_moving(std::shared_ptr<model::Piece> piece) const {
    return active_motion.has_value() && active_motion->piece == piece;
}

bool RealTimeArbiter::is_piece_cooling_down(std::shared_ptr<model::Piece> piece) const {
    for (const auto& cd : active_cooldowns) {
        if (cd.piece == piece) return true;
    }
    return false;
}

bool RealTimeArbiter::is_piece_on_long_rest(std::shared_ptr<model::Piece> piece) const {
    for (const auto& cd : active_cooldowns) {
        if (cd.piece == piece) return cd.is_long_rest;
    }
    return false;
}

int RealTimeArbiter::get_piece_cooldown_remaining_ms(std::shared_ptr<model::Piece> piece) const {
    for (const auto& cd : active_cooldowns) {
        if (cd.piece == piece) return cd.remaining_ms;
    }
    return 0;
}

int RealTimeArbiter::get_piece_cooldown_total_ms(std::shared_ptr<model::Piece> piece) const {
    for (const auto& cd : active_cooldowns) {
        if (cd.piece == piece) return cd.total_ms;
    }
    return 0;
}

void RealTimeArbiter::reset() {
    active_motion.reset();
    active_jump.reset();
    active_cooldowns.clear();
}

std::optional<Motion> RealTimeArbiter::get_active_motion() const {
    return active_motion;
}

std::optional<Jump> RealTimeArbiter::get_active_jump() const {
    return active_jump;
}

} 
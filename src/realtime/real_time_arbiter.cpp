#include "realtime/real_time_arbiter.h"

namespace realtime {

void RealTimeArbiter::start_motion(std::shared_ptr<model::Piece> piece, const model::Position& src, const model::Position& dest, int ms) {
    active_motion = Motion{piece, src, dest, ms};
}

void RealTimeArbiter::advance_time(int ms, std::shared_ptr<model::Board> board) {
    if (active_motion) {
        active_motion->remaining_ms -= ms;
        if (active_motion->remaining_ms <= 0) {
            board->move_piece(active_motion->source, active_motion->dest);
            active_motion.reset();
        }
    }
}

} 
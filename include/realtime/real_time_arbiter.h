#pragma once
#include <memory>
#include <optional>
#include "model/position.h"
#include "model/piece.h"
#include "model/board.h"

namespace realtime {

struct Motion {
    std::shared_ptr<model::Piece> piece;
    model::Position source;
    model::Position dest;
    int remaining_ms;
};

class RealTimeArbiter {
private:
    std::optional<Motion> active_motion;

public:
    void start_motion(std::shared_ptr<model::Piece> piece, const model::Position& src, const model::Position& dest, int ms);
    void advance_time(int ms, std::shared_ptr<model::Board> board);
};

} 
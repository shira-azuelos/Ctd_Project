#pragma once
#include <memory>
#include "../include/model/position.h"
#include "../include/model/piece.h"

namespace realtime {

struct Motion {
    std::shared_ptr<model::Piece> piece;
    model::Position source;
    model::Position dest;
    int remaining_ms;
    int total_ms;
};

struct Jump {
    std::shared_ptr<model::Piece> piece;
    model::Position pos;
    int remaining_ms;
    int total_ms;
};

class MotionState {
public:
    static model::Position calculate_current_pos(const model::Position& src, const model::Position& dest, int total_ms, int remaining_ms);
};

} 
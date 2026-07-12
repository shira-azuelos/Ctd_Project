#pragma once
#include <memory>
#include "model/board.h"
#include "model/position.h"
#include "realtime/real_time_arbiter.h"

namespace engine {

class GameEngine {
private:
    std::shared_ptr<model::Board> board;
    realtime::RealTimeArbiter arbiter;

public:
    explicit GameEngine(std::shared_ptr<model::Board> b);
    void request_move(const model::Position& source, const model::Position& dest);
    void wait(int ms);
};

} 
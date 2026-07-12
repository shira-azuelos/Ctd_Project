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
    bool game_over_flag = false; 

    void update_game_state(); 

public:
    static constexpr int BASE_MOVE_TIME_MS = 1000;

    GameEngine(std::shared_ptr<model::Board> b) : board(b) {}

    bool is_moving() const { return arbiter.is_moving(); }
    bool is_game_over() const { return game_over_flag; } 

    void request_move(const model::Position& source, const model::Position& dest);
    void wait(int ms);
};

} 
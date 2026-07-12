#pragma once
#include <memory>
#include "../include/model/game_state.h"
#include "../include/realtime/real_time_arbiter.h"

namespace engine {

class GameEngine {
private:
    std::shared_ptr<model::GameState> state;
    realtime::RealTimeArbiter arbiter;

public:
    static constexpr int BASE_MOVE_TIME_MS = 1000;

    GameEngine(std::shared_ptr<model::Board> b) {
        state = std::make_shared<model::GameState>(b);
    }

    std::shared_ptr<model::GameState> get_state() const { return state; }
    bool is_moving() const { return arbiter.is_moving(); }
    
    void request_move(const model::Position& src, const model::Position& dest);
    void request_jump(const model::Position& pos);
    void wait(int ms);
};

}
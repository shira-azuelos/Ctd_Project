#pragma once
#include <memory>
#include <optional>
#include "../include/model/game_state.h"
#include "../include/realtime/real_time_arbiter.h"

namespace engine {

class GameEngine {
private:
    std::shared_ptr<model::GameState> state;
    realtime::RealTimeArbiter arbiter;

public:
    static constexpr int BASE_MOVE_TIME_MS = 1000;

    GameEngine(std::shared_ptr<model::Board> b);

    std::shared_ptr<model::GameState> get_state() const;

    bool is_moving() const;
    
    std::optional<realtime::Motion> get_active_motion() const;

    void request_move(const model::Position& src, const model::Position& dest);

    void request_jump(const model::Position& pos);
    
    void wait(int ms);
};

}
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

    std::optional<realtime::Jump> get_active_jump() const;

    std::vector<realtime::Motion> get_active_motions() const;

    std::vector<realtime::Jump> get_active_jumps() const;

    const realtime::RealTimeArbiter& get_arbiter() const;

    bool is_piece_cooling_down(std::shared_ptr<model::Piece> piece) const;
    
    bool is_piece_on_long_rest(std::shared_ptr<model::Piece> piece) const;

    int get_piece_cooldown_remaining_ms(std::shared_ptr<model::Piece> piece) const;
    
    int get_piece_cooldown_total_ms(std::shared_ptr<model::Piece> piece) const;

    void request_move(const model::Position& src, const model::Position& dest);

    void request_jump(const model::Position& pos);
    
    void wait(int ms);
};

}
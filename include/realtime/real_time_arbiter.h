#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../include/model/board.h"
#include "../include/realtime/motion.h"

namespace model {
class GameState;
}

namespace realtime {

struct Cooldown {
    std::shared_ptr<model::Piece> piece;
    int remaining_ms;
    int total_ms;
    bool is_long_rest = false;
};

class RealTimeArbiter {
private:
    std::vector<Motion> active_motions;
    std::vector<Jump> active_jumps;
    std::vector<Cooldown> active_cooldowns;

public:
    void start_motion(std::shared_ptr<model::Piece> piece, model::Position src, model::Position dst, int total_ms);
    
    void start_jump(std::shared_ptr<model::Piece> piece, model::Position pos, int total_ms);
    
    void advance_time(int ms, std::shared_ptr<model::Board> board, std::shared_ptr<model::GameState> state = nullptr);
    
    bool is_moving() const;
   
    bool is_piece_moving(std::shared_ptr<model::Piece> piece) const;
   
    std::optional<Motion> get_active_motion() const;

    std::optional<Jump> get_active_jump() const;

    std::vector<Motion> get_active_motions() const;

    std::vector<Jump> get_active_jumps() const;

    std::vector<Cooldown> get_active_cooldowns() const;

    bool is_piece_cooling_down(std::shared_ptr<model::Piece> piece) const;
    
    bool is_piece_on_long_rest(std::shared_ptr<model::Piece> piece) const;

    int get_piece_cooldown_remaining_ms(std::shared_ptr<model::Piece> piece) const;
    
    int get_piece_cooldown_total_ms(std::shared_ptr<model::Piece> piece) const;

    void reset();
};

} 
#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../include/model/board.h"
#include "../include/realtime/motion.h"

namespace realtime {

struct Cooldown {
    std::shared_ptr<model::Piece> piece;
    int remaining_ms;
    int total_ms;
};

class RealTimeArbiter {
private:
    std::optional<Motion> active_motion;
    std::optional<Jump> active_jump;
    std::vector<Cooldown> active_cooldowns;

public:
    void start_motion(std::shared_ptr<model::Piece> piece, model::Position src, model::Position dst, int total_ms);
    
    void start_jump(std::shared_ptr<model::Piece> piece, model::Position pos, int total_ms);
    
    void advance_time(int ms, std::shared_ptr<model::Board> board);
    
    bool is_moving() const;
   
    bool is_piece_moving(std::shared_ptr<model::Piece> piece) const;
   
    std::optional<Motion> get_active_motion() const;

    bool is_piece_cooling_down(std::shared_ptr<model::Piece> piece) const;

    int get_piece_cooldown_remaining_ms(std::shared_ptr<model::Piece> piece) const;
    
    int get_piece_cooldown_total_ms(std::shared_ptr<model::Piece> piece) const;

    void reset();
};

} 
#include "../include/realtime/real_time_arbiter.h"
#include "../include/model/game_state.h"
#include "pubsub/message_bus.h"
#include <algorithm>
#include <iostream>

namespace realtime {

void RealTimeArbiter::start_motion(std::shared_ptr<model::Piece> piece, model::Position src, model::Position dst, int total_ms, std::shared_ptr<model::Board> board) {
    if (piece) piece->state = model::PieceState::MOVING;
    
    std::shared_ptr<model::Piece> victim = nullptr;
    if (board) {
        victim = board->get_piece_at(dst);
        if (victim && piece && victim->color == piece->color) {
            victim = nullptr;
        }
        board->clear_cell(src);
    }
    
    active_motions.push_back(Motion{piece, src, dst, total_ms, total_ms, victim});
    
    pubsub::MessageBus::get_instance().publish(pubsub::Event{
        pubsub::EventType::PLAY_SOUND,
        pubsub::SoundPayload{"move"}
    });
}

void RealTimeArbiter::start_jump(std::shared_ptr<model::Piece> piece, model::Position pos, int total_ms) {
    if (piece) piece->state = model::PieceState::MOVING;
    active_jumps.push_back(Jump{piece, pos, total_ms, total_ms});
    
    pubsub::MessageBus::get_instance().publish(pubsub::Event{
        pubsub::EventType::PLAY_SOUND,
        pubsub::SoundPayload{"jump"}
    });
}

static int get_piece_value(model::PieceKind kind) {
    switch (kind) {
        case model::PieceKind::PAWN: return 1;
        case model::PieceKind::KNIGHT: return 3;
        case model::PieceKind::BISHOP: return 3;
        case model::PieceKind::ROOK: return 5;
        case model::PieceKind::QUEEN: return 9;
        case model::PieceKind::KING: return 10;
        default: return 0;
    }
}

void RealTimeArbiter::advance_time(int ms, std::shared_ptr<model::Board> board, std::shared_ptr<model::GameState> state) {
    for (auto it = active_cooldowns.begin(); it != active_cooldowns.end(); ) {
        it->remaining_ms -= ms;
        if (it->remaining_ms <= 0) {
            it = active_cooldowns.erase(it);
        } else {
            ++it;
        }
    }

    struct Event {
        enum class Type { MOTION, JUMP };
        Type type;
        size_t index_in_vec;
        int arrival_time; 
    };

    std::vector<Event> events;
    for (size_t i = 0; i < active_motions.size(); ++i) {
        if (active_motions[i].remaining_ms <= ms) {
            events.push_back(Event{Event::Type::MOTION, i, active_motions[i].remaining_ms});
        }
    }
    for (size_t i = 0; i < active_jumps.size(); ++i) {
        if (active_jumps[i].remaining_ms <= ms) {
            events.push_back(Event{Event::Type::JUMP, i, active_jumps[i].remaining_ms});
        }
    }

    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::vector<bool> handled_motions(active_motions.size(), false);
    std::vector<bool> handled_jumps(active_jumps.size(), false);

    for (const auto& ev : events) {
        if (ev.type == Event::Type::JUMP) {
            handled_jumps[ev.index_in_vec] = true;
            auto& jump = active_jumps[ev.index_in_vec];
            if (jump.piece) {
                auto current_piece = board->get_piece_at(jump.pos);
                if (current_piece == jump.piece) {
                    jump.piece->state = model::PieceState::IDLE;
                    active_cooldowns.push_back(Cooldown{jump.piece, 3000, 3000, true});
                    
                    std::string log_msg = jump.piece->id + " landed at (" + std::to_string(jump.pos.row) + "," + std::to_string(jump.pos.col) + ")";
                    pubsub::MessageBus::get_instance().publish(pubsub::Event{
                        pubsub::EventType::MOVE_LOGGED,
                        log_msg
                    });
                }
            }
        } 
        else if (ev.type == Event::Type::MOTION) {
            handled_motions[ev.index_in_vec] = true;
            auto& motion = active_motions[ev.index_in_vec];
            auto dest = motion.dest;
            auto src = motion.source;
            auto moving_piece = motion.piece;

            if (moving_piece && moving_piece->state != model::PieceState::CAPTURED) {
                auto piece_at_dest = board->get_piece_at(dest);
                if (piece_at_dest) {
                    bool is_escaping = false;
                    for (size_t i = 0; i < active_motions.size(); ++i) {
                        if (!handled_motions[i] && active_motions[i].piece && active_motions[i].piece->id == piece_at_dest->id) {
                            is_escaping = true;
                            break;
                        }
                    }
                    if (is_escaping) {
                        piece_at_dest = nullptr;
                    }
                }

                std::shared_ptr<model::Piece> jumping_piece_at_dest = nullptr;
                for (size_t j = 0; j < active_jumps.size(); ++j) {
                    if (active_jumps[j].pos == dest) {
                        bool j_completed_earlier = false;
                        if (active_jumps[j].remaining_ms <= ms) {
                            if (active_jumps[j].remaining_ms < ev.arrival_time) {
                                j_completed_earlier = true;
                            }
                        }
                        if (!j_completed_earlier) {
                            jumping_piece_at_dest = active_jumps[j].piece;
                            break;
                        }
                    }
                }

                if (jumping_piece_at_dest) {
                    if (jumping_piece_at_dest->color == moving_piece->color) {
                        moving_piece->cell = src;
                        board->add_piece(moving_piece);
                        moving_piece->state = model::PieceState::IDLE;
                        active_cooldowns.push_back(Cooldown{moving_piece, 3000, 3000, false});
                        std::cout << "[Arbiter] Friendly block mid-air. " << moving_piece->id << " stuck at (" << src.row << ", " << src.col << ")" << std::endl;
                        
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::PLAY_SOUND,
                            pubsub::SoundPayload{"block"}
                        });
                        std::string log_msg = "Friendly block mid-air. " + moving_piece->id + " stuck at (" + std::to_string(src.row) + "," + std::to_string(src.col) + ")";
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::MOVE_LOGGED,
                            log_msg
                        });
                    } else {
                        moving_piece->state = model::PieceState::CAPTURED;
                        std::cout << "[Arbiter] Mid-air capture. Jumping " << jumping_piece_at_dest->id << " captured " << moving_piece->id << " at (" << dest.row << ", " << dest.col << ")" << std::endl;
                        
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::PLAY_SOUND,
                            pubsub::SoundPayload{"capture"}
                        });
                        std::string log_msg = "Mid-air capture. Jumping " + jumping_piece_at_dest->id + " captured " + moving_piece->id + " at (" + std::to_string(dest.row) + "," + std::to_string(dest.col) + ")";
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::MOVE_LOGGED,
                            log_msg
                        });
                        
                        if (state) {
                            int pts = get_piece_value(moving_piece->kind);
                            if (jumping_piece_at_dest->color == model::PieceColor::WHITE) {
                                state->add_to_white_score(pts);
                            } else {
                                state->add_to_black_score(pts);
                            }
                        }
                    }
                }
                else if (piece_at_dest) {
                    if (piece_at_dest->color == moving_piece->color) {
                        moving_piece->cell = src;
                        board->add_piece(moving_piece);
                        moving_piece->state = model::PieceState::IDLE;
                        active_cooldowns.push_back(Cooldown{moving_piece, 3000, 3000, false});
                        std::cout << "[Arbiter] Friendly block. " << moving_piece->id << " stuck at (" << src.row << ", " << src.col << ")" << std::endl;
                        
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::PLAY_SOUND,
                            pubsub::SoundPayload{"block"}
                        });
                        std::string log_msg = "Friendly block. " + moving_piece->id + " stuck at (" + std::to_string(src.row) + "," + std::to_string(src.col) + ")";
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::MOVE_LOGGED,
                            log_msg
                        });
                    } else {
                        board->remove_piece(dest);
                        moving_piece->cell = dest;
                        board->add_piece(moving_piece);

                        moving_piece->state = model::PieceState::IDLE;
                        active_cooldowns.push_back(Cooldown{moving_piece, 3000, 3000, false});
                        std::cout << "[Arbiter] Late capture on board. " << moving_piece->id << " captured " << piece_at_dest->id << " at (" << dest.row << ", " << dest.col << ")" << std::endl;
                        
                        std::string log_msg = moving_piece->id + " captured " + piece_at_dest->id + " at (" + std::to_string(dest.row) + "," + std::to_string(dest.col) + ")";
                        pubsub::MessageBus::get_instance().publish(pubsub::Event{
                            pubsub::EventType::MOVE_LOGGED,
                            log_msg
                        });
                        
                        if (state) {
                            int pts = get_piece_value(piece_at_dest->kind);
                            if (moving_piece->color == model::PieceColor::WHITE) {
                                state->add_to_white_score(pts);
                            } else {
                                state->add_to_black_score(pts);
                            }
                        }
                        
                        if (moving_piece->kind == model::PieceKind::PAWN) {
                            if ((moving_piece->color == model::PieceColor::WHITE && dest.row == 0) ||
                                (moving_piece->color == model::PieceColor::BLACK && dest.row == board->get_height() - 1)) {
                                moving_piece->kind = model::PieceKind::QUEEN;
                                pubsub::MessageBus::get_instance().publish(pubsub::Event{
                                    pubsub::EventType::PLAY_SOUND,
                                    pubsub::SoundPayload{"coronation"}
                                });
                            }
                        }
                    }
                } 
                else {
                    moving_piece->cell = dest;
                    board->add_piece(moving_piece);

                    moving_piece->state = model::PieceState::IDLE;
                    active_cooldowns.push_back(Cooldown{moving_piece, 3000, 3000, false});
                    
                    std::string log_msg = moving_piece->id + " moved to (" + std::to_string(dest.row) + "," + std::to_string(dest.col) + ")";
                    pubsub::MessageBus::get_instance().publish(pubsub::Event{
                        pubsub::EventType::MOVE_LOGGED,
                        log_msg
                    });

                    if (moving_piece->kind == model::PieceKind::PAWN) {
                        if ((moving_piece->color == model::PieceColor::WHITE && dest.row == 0) ||
                            (moving_piece->color == model::PieceColor::BLACK && dest.row == board->get_height() - 1)) {
                            moving_piece->kind = model::PieceKind::QUEEN;
                        }
                    }
                }
            }
        }
    }

    std::vector<Motion> remaining_motions;
    for (size_t i = 0; i < active_motions.size(); ++i) {
        if (!handled_motions[i]) {
            active_motions[i].remaining_ms -= ms;
            remaining_motions.push_back(active_motions[i]);
        }
    }
    active_motions = remaining_motions;

    std::vector<Jump> remaining_jumps;
    for (size_t i = 0; i < active_jumps.size(); ++i) {
        if (!handled_jumps[i]) {
            active_jumps[i].remaining_ms -= ms;
            remaining_jumps.push_back(active_jumps[i]);
        }
    }
    active_jumps = remaining_jumps;
}

bool RealTimeArbiter::is_moving() const {
    return !active_motions.empty();
}

bool RealTimeArbiter::is_piece_moving(std::shared_ptr<model::Piece> piece) const {
    if (!piece) return false;
    for (const auto& m : active_motions) {
        if (m.piece && m.piece->id == piece->id) return true;
    }
    return false;
}

bool RealTimeArbiter::is_piece_cooling_down(std::shared_ptr<model::Piece> piece) const {
    if (!piece) return false;
    for (const auto& cd : active_cooldowns) {
        if (cd.piece && cd.piece->id == piece->id) return true;
    }
    return false;
}

bool RealTimeArbiter::is_piece_on_long_rest(std::shared_ptr<model::Piece> piece) const {
    if (!piece) return false;
    for (const auto& cd : active_cooldowns) {
        if (cd.piece && cd.piece->id == piece->id) return cd.is_long_rest;
    }
    return false;
}

int RealTimeArbiter::get_piece_cooldown_remaining_ms(std::shared_ptr<model::Piece> piece) const {
    if (!piece) return 0;
    for (const auto& cd : active_cooldowns) {
        if (cd.piece && cd.piece->id == piece->id) return cd.remaining_ms;
    }
    return 0;
}

int RealTimeArbiter::get_piece_cooldown_total_ms(std::shared_ptr<model::Piece> piece) const {
    if (!piece) return 0;
    for (const auto& cd : active_cooldowns) {
        if (cd.piece && cd.piece->id == piece->id) return cd.total_ms;
    }
    return 0;
}

void RealTimeArbiter::reset() {
    active_motions.clear();
    active_jumps.clear();
    active_cooldowns.clear();
}

std::vector<Motion> RealTimeArbiter::get_active_motions() const {
    return active_motions;
}

std::vector<Jump> RealTimeArbiter::get_active_jumps() const {
    return active_jumps;
}

std::vector<Cooldown> RealTimeArbiter::get_active_cooldowns() const {
    return active_cooldowns;
}

void RealTimeArbiter::set_active_cooldowns(const std::vector<Cooldown>& cds) {
    active_cooldowns = cds;
}

void RealTimeArbiter::set_active_motions(const std::vector<Motion>& motions) {
    active_motions = motions;
}

void RealTimeArbiter::set_active_jumps(const std::vector<Jump>& jumps) {
    active_jumps = jumps;
}

} 
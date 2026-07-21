#include "../include/model/game_state.h"
#include "pubsub/message_bus.h"
#include "realtime/real_time_arbiter.h"


namespace model {

GameState::GameState(std::shared_ptr<Board> b) : board(b) {
    int w = board->get_width();
    int h = board->get_height();
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            auto p = board->get_piece_at(Position(r, c));
            if (p && p->kind == PieceKind::KING) {
                if (p->color == PieceColor::WHITE) initial_w_kings++;
                else initial_b_kings++;
            }
        }
    }
}

std::shared_ptr<Board> GameState::get_board() const {
    return board;
}

bool GameState::is_game_over() const {
    return game_over_flag;
}

void GameState::set_game_over(bool state) {
    game_over_flag = state;
}

void GameState::check_game_status(const realtime::RealTimeArbiter& arbiter) {
    if (game_over_flag) return;
    int w_kings = 0, b_kings = 0;
    int w = board->get_width(), h = board->get_height();

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            auto piece = board->get_piece_at(Position(r, c));
            if (piece && piece->kind == PieceKind::KING) {
                if (piece->color == PieceColor::WHITE) w_kings++;
                else b_kings++;
            }
        }
    }

    for (const auto& m : arbiter.get_active_motions()) {
        if (m.piece && m.piece->kind == PieceKind::KING && m.piece->state != PieceState::CAPTURED) {
            if (m.piece->color == PieceColor::WHITE) w_kings++;
            else b_kings++;
        }
    }
    for (const auto& j : arbiter.get_active_jumps()) {
        if (j.piece && j.piece->kind == PieceKind::KING && j.piece->state != PieceState::CAPTURED) {
            if (j.piece->color == PieceColor::WHITE) w_kings++;
            else b_kings++;
        }
    }

    bool white_lost = (initial_w_kings > 0 && w_kings == 0);
    bool black_lost = (initial_b_kings > 0 && b_kings == 0);

    if (white_lost || black_lost) {
        game_over_flag = true;

        pubsub::MessageBus::get_instance().publish(pubsub::Event{
            pubsub::EventType::GAME_STATUS,
            std::string("game_over")
        });
        pubsub::MessageBus::get_instance().publish(pubsub::Event{
            pubsub::EventType::PLAY_SOUND,
            pubsub::SoundPayload{"game_win"}
        });
    }
}

int GameState::get_white_score() const {
    return white_score;
}

int GameState::get_black_score() const {
    return black_score;
}

void GameState::add_to_white_score(int pts) {
    white_score += pts;
    pubsub::MessageBus::get_instance().publish(pubsub::Event{
        pubsub::EventType::SCORE_CHANGED,
        pubsub::ScorePayload{"WHITE", white_score}
    });
}

void GameState::add_to_black_score(int pts) {
    black_score += pts;
    pubsub::MessageBus::get_instance().publish(pubsub::Event{
        pubsub::EventType::SCORE_CHANGED,
        pubsub::ScorePayload{"BLACK", black_score}
    });
}

void GameState::set_white_score(int score) {
    white_score = score;
    pubsub::MessageBus::get_instance().publish(pubsub::Event{
        pubsub::EventType::SCORE_CHANGED,
        pubsub::ScorePayload{"WHITE", white_score}
    });
}

void GameState::set_black_score(int score) {
    black_score = score;
    pubsub::MessageBus::get_instance().publish(pubsub::Event{
        pubsub::EventType::SCORE_CHANGED,
        pubsub::ScorePayload{"BLACK", black_score}
    });
}

} 
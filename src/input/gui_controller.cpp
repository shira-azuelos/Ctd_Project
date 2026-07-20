#include "input/gui_controller.h"
#include "input/board_mapper.h"
#include "pubsub/message_bus.h"
#include "network/socket_client.h"
#include "rules/rule_engine.h"
#include <iostream>

namespace input {

void GuiController::on_mouse(int event, int x, int y, int flags, void* userdata) {
    auto* g_state = static_cast<GuiState*>(userdata);
    
    bool game_over = false;
    if (g_state->socket_client) {
        game_over = g_state->socket_client->get_game_state()->is_game_over();
    } else if (g_state->game_engine) {
        game_over = g_state->game_engine->get_state()->is_game_over();
    }
    
    if (game_over) {
        g_state->dragged_piece = nullptr;
        return;
    }

    int board_x = x - 100;

    if (board_x < 0 || board_x >= 800 || y < 0 || y >= 800) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
        return;
    }
    
    auto cell_opt = BoardMapper::pixel_to_cell(board_x, y, g_state->board->get_width(), g_state->board->get_height());
    if (!cell_opt) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
        return;
    }

    model::Position cell = *cell_opt;

    if (event == cv::EVENT_LBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        auto selected_piece = g_state->selected_cell ? g_state->board->get_piece_at(*g_state->selected_cell) : nullptr;

        if (clicked_piece && (!selected_piece || clicked_piece->color == selected_piece->color)) {
            bool allow_interaction = true;
            bool on_cooldown = false;
            
            if (g_state->socket_client) {
                std::string my_color = g_state->socket_client->get_assigned_color();
                std::string piece_color = (clicked_piece->color == model::PieceColor::WHITE) ? "WHITE" : "BLACK";
                if (my_color != piece_color) {
                    allow_interaction = false;
                }
                on_cooldown = g_state->socket_client->get_arbiter()->is_piece_cooling_down(clicked_piece);
            } else if (g_state->game_engine) {
                on_cooldown = g_state->game_engine->is_piece_cooling_down(clicked_piece);
            }

            if (allow_interaction && !on_cooldown) {
                g_state->selected_cell = cell;
                g_state->dragged_piece = clicked_piece;
                g_state->drag_x = x;
                g_state->drag_y = y;

                bool threatens_enemy = false;
                for (int r = 0; r < g_state->board->get_height(); ++r) {
                    for (int c = 0; c < g_state->board->get_width(); ++c) {
                        model::Position target_pos(r, c);
                        if (target_pos != cell && rules::RuleEngine::validate_move(*g_state->board, cell, target_pos)) {
                            auto piece_at_target = g_state->board->get_piece_at(target_pos);
                            if (piece_at_target && piece_at_target->color != clicked_piece->color) {
                                threatens_enemy = true;
                                break;
                            }
                        }
                    }
                    if (threatens_enemy) break;
                }

                if (threatens_enemy) {
                    pubsub::MessageBus::get_instance().publish(pubsub::Event{
                        pubsub::EventType::PLAY_SOUND,
                        pubsub::SoundPayload{"capture"}
                    });
                }
            }
        } else if (g_state->selected_cell) {
            if (g_state->socket_client) {
                g_state->socket_client->send_move(g_state->selected_cell->row, g_state->selected_cell->col, cell.row, cell.col);
            } else if (g_state->game_engine) {
                g_state->game_engine->request_move(*g_state->selected_cell, cell);
            }
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        if (g_state->dragged_piece) {
            auto source_cell = g_state->dragged_piece->cell;
            if (source_cell != cell) {
                if (g_state->socket_client) {
                    g_state->socket_client->send_move(source_cell.row, source_cell.col, cell.row, cell.col);
                } else if (g_state->game_engine) {
                    g_state->game_engine->request_move(source_cell, cell);
                }
                g_state->selected_cell.reset();
            }
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        if (clicked_piece) {
            bool allow_interaction = true;
            if (g_state->socket_client) {
                std::string my_color = g_state->socket_client->get_assigned_color();
                std::string piece_color = (clicked_piece->color == model::PieceColor::WHITE) ? "WHITE" : "BLACK";
                if (my_color != piece_color) {
                    allow_interaction = false;
                }
            }

            if (allow_interaction) {
                if (g_state->socket_client) {
                    g_state->socket_client->send_jump(cell.row, cell.col);
                } else if (g_state->game_engine) {
                    g_state->game_engine->request_jump(cell);
                }
                g_state->selected_cell.reset();
                g_state->dragged_piece = nullptr;
            }
        }
    }
}

}

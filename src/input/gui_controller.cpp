#include "input/gui_controller.h"
#include "input/board_mapper.h"
#include "pubsub/message_bus.h"
#include "network/socket_client.h"
#include "rules/rule_engine.h"
#include <iostream>

namespace input {

bool GuiController::handle_opening_click(GuiState* g_state, int event, int x, int y) {
    if (!g_state->in_opening_screen) return false;

    if (event == cv::EVENT_LBUTTONDOWN) {
        if (g_state->show_room_dialog) {
            if (x >= 260 && x <= 400 && y >= 460 && y <= 505) {
                if (g_state->socket_client) {
                    std::string room_name = g_state->room_input_text.empty() ? "Battle Room" : g_state->room_input_text;
                    g_state->socket_client->send_create_room(room_name);
                }
                g_state->show_room_dialog = false;
                return true;
            }
            if (x >= 430 && x <= 570 && y >= 460 && y <= 505) {
                if (g_state->socket_client && !g_state->room_input_text.empty()) {
                    g_state->socket_client->send_join_room(g_state->room_input_text);
                }
                g_state->show_room_dialog = false;
                return true;
            }
            if (x >= 600 && x <= 740 && y >= 460 && y <= 505) {
                g_state->show_room_dialog = false;
                return true;
            }
            return true;
        }

        if (g_state->socket_client) {
            if (x >= 220 && x <= 780 && y >= 555 && y <= 615) {
                if (g_state->socket_client->show_popup()) {
                    g_state->socket_client->dismiss_popup();
                }
                g_state->show_room_dialog = true;
                g_state->room_input_text = "";
                return true;
            }

            if (g_state->socket_client->show_popup()) {
                g_state->socket_client->dismiss_popup();
                return true;
            }

            if (x >= 220 && x <= 780 && y >= 470 && y <= 545) {
                auto match_st = g_state->socket_client->get_match_state();
                if (match_st == network::MatchState::IDLE || match_st == network::MatchState::TIMEOUT) {
                    g_state->socket_client->send_find_match();
                } else if (match_st == network::MatchState::SEARCHING) {
                    g_state->socket_client->send_cancel_match();
                }
                return true;
            }
        } else {
            if (x >= 220 && x <= 780 && y >= 470 && y <= 545) {
                g_state->in_opening_screen = false;
            }
        }
    }
    return true;
}

bool GuiController::on_key(int key, GuiState* g_state) {
    if (!g_state || !g_state->show_room_dialog) return false;

    if (key == KEY_BACKSPACE) {
        if (!g_state->room_input_text.empty()) {
            g_state->room_input_text.pop_back();
        }
        return true;
    }
    else if (key == KEY_ENTER_CR || key == KEY_ENTER_LF) {
        if (g_state->socket_client && !g_state->room_input_text.empty()) {
            g_state->socket_client->send_join_room(g_state->room_input_text);
        }
        g_state->show_room_dialog = false;
        return true;
    }
    else if (key >= ASCII_PRINTABLE_MIN && key <= ASCII_PRINTABLE_MAX) {
        if (g_state->room_input_text.length() < MAX_ROOM_INPUT_LENGTH) {
            g_state->room_input_text += static_cast<char>(key);
        }
        return true;
    }
    return false;
}

void GuiController::handle_left_click_down(GuiState* g_state, const model::Position& cell, int x, int y) {
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

void GuiController::handle_left_click_up(GuiState* g_state, const model::Position& cell) {
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

void GuiController::handle_right_click_down(GuiState* g_state, const model::Position& cell) {
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

void GuiController::on_mouse(int event, int x, int y, int flags, void* userdata) {
    auto* g_state = static_cast<GuiState*>(userdata);

    if (handle_opening_click(g_state, event, x, y)) {
        return;
    }
    
    if (event == cv::EVENT_MOUSEMOVE) {
        if (g_state->dragged_piece) {
            g_state->drag_x = x;
            g_state->drag_y = y;
        }
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
        handle_left_click_down(g_state, cell, x, y);
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        handle_left_click_up(g_state, cell);
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        handle_right_click_down(g_state, cell);
    }
}

}


#include "network/socket_client.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>

namespace network {

static std::string extract_string(const std::string& str, const std::string& key, size_t start_pos = 0) {
    size_t pos = str.find("\"" + key + "\"", start_pos);
    if (pos == std::string::npos) return "";
    size_t colon = str.find(":", pos);
    if (colon == std::string::npos) return "";
    size_t quote1 = str.find("\"", colon);
    if (quote1 == std::string::npos) return "";
    size_t quote2 = str.find("\"", quote1 + 1);
    if (quote2 == std::string::npos) return "";
    return str.substr(quote1 + 1, quote2 - quote1 - 1);
}

static int extract_int(const std::string& str, const std::string& key, size_t start_pos = 0) {
    size_t pos = str.find("\"" + key + "\"", start_pos);
    if (pos == std::string::npos) return 0;
    size_t colon = str.find(":", pos);
    if (colon == std::string::npos) return 0;
    size_t comma_or_brace = str.find_first_of(",}", colon);
    if (comma_or_brace == std::string::npos) return 0;
    return std::stoi(str.substr(colon + 1, comma_or_brace - colon - 1));
}

static bool extract_bool(const std::string& str, const std::string& key, size_t start_pos = 0) {
    size_t pos = str.find("\"" + key + "\"", start_pos);
    if (pos == std::string::npos) return false;
    size_t colon = str.find(":", pos);
    if (colon == std::string::npos) return false;
    size_t comma_or_brace = str.find_first_of(",}", colon);
    if (comma_or_brace == std::string::npos) return false;
    std::string val = str.substr(colon + 1, comma_or_brace - colon - 1);
    return val.find("true") != std::string::npos;
}

SocketClient::SocketClient(const std::string& ip, int port) 
    : m_server_ip(ip), m_server_port(port) {
    board = std::make_shared<model::Board>(8, 8);
    game_state = std::make_shared<model::GameState>(board);
}

SocketClient::~SocketClient() {
    disconnect();
}

bool SocketClient::connect_to_server() {
    try {
        m_client.init_asio();
        m_client.set_message_handler(std::bind(&SocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
        
        m_client.clear_access_channels(websocketpp::log::alevel::all);
        m_client.clear_error_channels(websocketpp::log::elevel::all);

        std::string uri = "ws://" + m_server_ip + ":" + std::to_string(m_server_port);
        websocketpp::lib::error_code ec;
        ws_client_t::connection_ptr con = m_client.get_connection(uri, ec);
        if (ec) {
            std::cerr << "[Client] Connection failed: " << ec.message() << std::endl;
            return false;
        }

        m_hdl = con->get_handle();
        m_client.connect(con);

        m_running = true;
        m_client_thread = std::thread([this]() {
            m_client.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Client] Exception: " << e.what() << std::endl;
        return false;
    }
}

void SocketClient::disconnect() {
    if (!m_running) return;
    m_running = false;

    websocketpp::lib::error_code ec;
    m_client.close(m_hdl, websocketpp::close::status::normal, "Client disconnect", ec);

    m_client.stop();

    if (m_client_thread.joinable()) {
        m_client_thread.join();
    }
}

void SocketClient::on_message(websocketpp::connection_hdl hdl, ws_client_t::message_ptr msg) {
    std::string payload = msg->get_payload();
    if (payload.rfind("COLOR ", 0) == 0) {
        std::lock_guard<std::mutex> lock(state_mutex);
        assigned_color = payload.substr(6);
        std::cout << "[Client] My assigned color is: " << assigned_color << std::endl;
    } else {
        parse_and_update_state(payload);
    }
}

void SocketClient::send_move(int sr, int sc, int dr, int dc) {
    std::stringstream ss;
    ss << "MOVE " << sr << " " << sc << " " << dr << " " << dc;
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, ss.str(), websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "[Client] Send move failed: " << ec.message() << std::endl;
    }
}

void SocketClient::send_jump(int r, int c) {
    std::stringstream ss;
    ss << "JUMP " << r << " " << c;
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, ss.str(), websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "[Client] Send jump failed: " << ec.message() << std::endl;
    }
}

void SocketClient::parse_and_update_state(const std::string& json_str) {
    bool game_over = extract_bool(json_str, "game_over");
    int white_score = extract_int(json_str, "white_score");
    int black_score = extract_int(json_str, "black_score");

    auto new_board = std::make_shared<model::Board>(8, 8);
    std::map<std::string, std::shared_ptr<model::Piece>> parsed_pieces;
    std::vector<realtime::Cooldown> cds;

    size_t pieces_start = json_str.find("\"pieces\":[");
    if (pieces_start != std::string::npos) {
        size_t pieces_end = json_str.find("],\"motions\":", pieces_start);
        if (pieces_end != std::string::npos) {
            std::string pieces_array = json_str.substr(pieces_start + 10, pieces_end - (pieces_start + 10));
            
            size_t obj_pos = 0;
            while (true) {
                size_t start_obj = pieces_array.find("{", obj_pos);
                if (start_obj == std::string::npos) break;
                size_t end_obj = pieces_array.find("}", start_obj);
                if (end_obj == std::string::npos) break;
                
                std::string obj_str = pieces_array.substr(start_obj, end_obj - start_obj + 1);
                
                std::string id = extract_string(obj_str, "id");
                int kind = extract_int(obj_str, "kind");
                int color = extract_int(obj_str, "color");
                int row = extract_int(obj_str, "row");
                int col = extract_int(obj_str, "col");
                int state = extract_int(obj_str, "state");
                
                auto piece = std::make_shared<model::Piece>(
                    id, 
                    static_cast<model::PieceColor>(color), 
                    static_cast<model::PieceKind>(kind), 
                    model::Position(row, col)
                );
                piece->state = static_cast<model::PieceState>(state);
                parsed_pieces[id] = piece;
                
                bool on_cooldown = extract_bool(obj_str, "on_cooldown");
                if (on_cooldown) {
                    int remaining = extract_int(obj_str, "cooldown_remaining");
                    int total = extract_int(obj_str, "cooldown_total");
                    bool is_long_rest = extract_bool(obj_str, "is_long_rest");
                    cds.push_back(realtime::Cooldown{piece, remaining, total, is_long_rest});
                }

                obj_pos = end_obj + 1;
            }
        }
    }

    std::vector<realtime::Motion> parsed_motions;
    size_t motions_start = json_str.find("\"motions\":[");
    if (motions_start != std::string::npos) {
        size_t motions_end = json_str.find("],\"jumps\":", motions_start);
        if (motions_end != std::string::npos) {
            std::string motions_array = json_str.substr(motions_start + 11, motions_end - (motions_start + 11));
            
            size_t obj_pos = 0;
            while (true) {
                size_t start_obj = motions_array.find("{", obj_pos);
                if (start_obj == std::string::npos) break;
                size_t end_obj = motions_array.find("}", start_obj);
                if (end_obj == std::string::npos) break;
                
                std::string obj_str = motions_array.substr(start_obj, end_obj - start_obj + 1);
                std::string piece_id = extract_string(obj_str, "piece_id");
                int src_row = extract_int(obj_str, "src_row");
                int src_col = extract_int(obj_str, "src_col");
                int dest_row = extract_int(obj_str, "dest_row");
                int dest_col = extract_int(obj_str, "dest_col");
                int total_ms = extract_int(obj_str, "total_ms");
                int remaining_ms = extract_int(obj_str, "remaining_ms");
                
                std::string victim_id = extract_string(obj_str, "victim_id");
                std::shared_ptr<model::Piece> attacker = nullptr;
                if (parsed_pieces.count(piece_id)) {
                    attacker = parsed_pieces[piece_id];
                }
                
                std::shared_ptr<model::Piece> victim = nullptr;
                if (!victim_id.empty() && parsed_pieces.count(victim_id)) {
                    victim = parsed_pieces[victim_id];
                }
                
                if (attacker) {
                    parsed_motions.push_back(realtime::Motion{
                        attacker,
                        model::Position(src_row, src_col),
                        model::Position(dest_row, dest_col),
                        remaining_ms,
                        total_ms,
                        victim
                    });
                }
                obj_pos = end_obj + 1;
            }
        }
    }

    std::vector<realtime::Jump> parsed_jumps;
    size_t jumps_start = json_str.find("\"jumps\":[");
    if (jumps_start != std::string::npos) {
        size_t jumps_end = json_str.find("]", jumps_start);
        if (jumps_end != std::string::npos) {
            std::string jumps_array = json_str.substr(jumps_start + 9, jumps_end - (jumps_start + 9));
            
            size_t obj_pos = 0;
            while (true) {
                size_t start_obj = jumps_array.find("{", obj_pos);
                if (start_obj == std::string::npos) break;
                size_t end_obj = jumps_array.find("}", start_obj);
                if (end_obj == std::string::npos) break;
                
                std::string obj_str = jumps_array.substr(start_obj, end_obj - start_obj + 1);
                std::string piece_id = extract_string(obj_str, "piece_id");
                int row = extract_int(obj_str, "row");
                int col = extract_int(obj_str, "col");
                int total_ms = extract_int(obj_str, "total_ms");
                int remaining_ms = extract_int(obj_str, "remaining_ms");
                
                std::shared_ptr<model::Piece> piece = nullptr;
                if (parsed_pieces.count(piece_id)) {
                    piece = parsed_pieces[piece_id];
                }
                
                if (piece) {
                    parsed_jumps.push_back(realtime::Jump{
                        piece,
                        model::Position(row, col),
                        total_ms,
                        remaining_ms
                    });
                }
                obj_pos = end_obj + 1;
            }
        }
    }

    for (const auto& pair : parsed_pieces) {
        auto piece = pair.second;
        bool is_in_flight = false;
        for (const auto& m : parsed_motions) {
            if (m.piece && m.piece->id == piece->id) {
                is_in_flight = true;
                break;
            }
        }
        if (!is_in_flight) {
            for (const auto& j : parsed_jumps) {
                if (j.piece && j.piece->id == piece->id) {
                    is_in_flight = true;
                    break;
                }
            }
        }
        if (!is_in_flight) {
            new_board->add_piece(piece);
        }
    }

    auto new_game_state = std::make_shared<model::GameState>(new_board);
    new_game_state->set_white_score(white_score);
    new_game_state->set_black_score(black_score);
    new_game_state->set_game_over(game_over);

    {
        std::lock_guard<std::mutex> lock(state_mutex);
        board = new_board;
        game_state = new_game_state;

        std::vector<realtime::Motion> final_motions;
        for (const auto& pm : parsed_motions) {
            bool found = false;
            for (const auto& am : arbiter.get_active_motions()) {
                if (am.piece->id == pm.piece->id) {
                    final_motions.push_back(am);
                    found = true;
                    break;
                }
            }
            if (!found) {
                final_motions.push_back(pm);
            }
        }
        for (const auto& am : arbiter.get_active_motions()) {
            if (am.remaining_ms > 0) {
                bool in_server = false;
                for (const auto& pm : parsed_motions) {
                    if (pm.piece->id == am.piece->id) {
                        in_server = true;
                        break;
                    }
                }
                if (!in_server) {
                    bool in_final = false;
                    for (const auto& fm : final_motions) {
                        if (fm.piece->id == am.piece->id) {
                            in_final = true;
                            break;
                        }
                    }
                    if (!in_final) {
                        final_motions.push_back(am);
                    }
                }
            }
        }
        arbiter.set_active_motions(final_motions);

        std::vector<realtime::Jump> final_jumps;
        for (const auto& pj : parsed_jumps) {
            bool found = false;
            for (const auto& aj : arbiter.get_active_jumps()) {
                if (aj.piece->id == pj.piece->id) {
                    final_jumps.push_back(aj);
                    found = true;
                    break;
                }
            }
            if (!found) {
                final_jumps.push_back(pj);
            }
        }
        for (const auto& aj : arbiter.get_active_jumps()) {
            if (aj.remaining_ms > 0) {
                bool in_server = false;
                for (const auto& pj : parsed_jumps) {
                    if (pj.piece->id == aj.piece->id) {
                        in_server = true;
                        break;
                    }
                }
                if (!in_server) {
                    bool in_final = false;
                    for (const auto& fj : final_jumps) {
                        if (fj.piece->id == aj.piece->id) {
                            in_final = true;
                            break;
                        }
                    }
                    if (!in_final) {
                        final_jumps.push_back(aj);
                    }
                }
            }
        }
        arbiter.set_active_jumps(final_jumps);

        std::vector<realtime::Cooldown> final_cooldowns;
        for (const auto& acd : arbiter.get_active_cooldowns()) {
            if (acd.piece) {
                bool exists = false;
                if (new_board) {
                    for (int r = 0; r < new_board->get_height(); ++r) {
                        for (int c = 0; c < new_board->get_width(); ++c) {
                            auto p = new_board->get_piece_at(model::Position(r, c));
                            if (p && p->id == acd.piece->id) {
                                exists = true;
                                break;
                            }
                        }
                        if (exists) break;
                    }
                }
                if (exists && acd.remaining_ms > 0) {
                    final_cooldowns.push_back(acd);
                }
            }
        }

        for (auto pcd : cds) {
            bool is_moving_locally = false;
            for (const auto& am : arbiter.get_active_motions()) {
                if (am.piece && am.piece->id == pcd.piece->id) {
                    is_moving_locally = true;
                    break;
                }
            }

            bool found = false;
            for (auto& acd : final_cooldowns) {
                if (acd.piece && acd.piece->id == pcd.piece->id) {
                    if (is_moving_locally) {
                        acd.remaining_ms = acd.total_ms;
                    }
                    found = true;
                    break;
                }
            }
            if (!found) {
                pcd.remaining_ms = pcd.total_ms;
                final_cooldowns.push_back(pcd);
            }
        }
        arbiter.set_active_cooldowns(final_cooldowns);
    }
}

void SocketClient::advance_animations(int ms) {
    std::lock_guard<std::mutex> lock(state_mutex);
    
    std::vector<realtime::Motion> current_motions = arbiter.get_active_motions();
    std::vector<realtime::Motion> next_motions;
    for (auto& m : current_motions) {
        m.remaining_ms -= ms;
        if (m.remaining_ms < 0) {
            m.remaining_ms = 0;
        }
        next_motions.push_back(m);
    }
    arbiter.set_active_motions(next_motions);
    
    std::vector<realtime::Jump> current_jumps = arbiter.get_active_jumps();
    std::vector<realtime::Jump> next_jumps;
    for (auto& j : current_jumps) {
        j.remaining_ms -= ms;
        if (j.remaining_ms < 0) {
            j.remaining_ms = 0;
        }
        next_jumps.push_back(j);
    }
    arbiter.set_active_jumps(next_jumps);

    std::vector<realtime::Cooldown> current_cooldowns = arbiter.get_active_cooldowns();
    std::vector<realtime::Cooldown> next_cooldowns;
    for (auto& cd : current_cooldowns) {
        bool is_moving_locally = false;
        for (const auto& m : next_motions) {
            if (m.piece && m.piece->id == cd.piece->id) {
                is_moving_locally = true;
                break;
            }
        }
        if (!is_moving_locally) {
            cd.remaining_ms -= ms;
        }
        if (cd.remaining_ms > 0) {
            next_cooldowns.push_back(cd);
        }
    }
    arbiter.set_active_cooldowns(next_cooldowns);
}

std::shared_ptr<model::GameState> SocketClient::get_game_state() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return game_state;
}

std::shared_ptr<model::Board> SocketClient::get_board() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return board;
}

std::vector<realtime::Motion> SocketClient::get_active_motions() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return arbiter.get_active_motions();
}

std::vector<realtime::Jump> SocketClient::get_active_jumps() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return arbiter.get_active_jumps();
}

realtime::RealTimeArbiter* SocketClient::get_arbiter() {
    return &arbiter;
}

std::string SocketClient::get_assigned_color() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return assigned_color;
}

}

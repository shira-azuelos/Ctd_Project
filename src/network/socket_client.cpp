#include "network/socket_client.h"
#include "pubsub/message_bus.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <fstream>
#include <iomanip>

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
    : m_server_ip(ip), m_server_port(port), m_server_game_over(false) {
    board = std::make_shared<model::Board>(8, 8);
    game_state = std::make_shared<model::GameState>(board);
}

SocketClient::~SocketClient() {
    disconnect();
}

bool SocketClient::connect_to_server() {
    try {
        m_client.init_asio();
        m_client.set_open_handler([this](websocketpp::connection_hdl hdl) {
            std::lock_guard<std::mutex> lock(state_mutex);
            m_connected = true;
        });
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

        int wait_count = 0;
        while (!m_connected && wait_count < 30) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            wait_count++;
        }
        return m_connected;
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
void SocketClient::log_client_activity(const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << " [CLIENT:" << m_username << "] " << msg;
    std::string line = ss.str();

    std::cout << line << std::endl;

    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ofstream ofs("client_activity.log", std::ios::app);
    if (ofs.is_open()) {
        ofs << line << std::endl;
    }
}

void SocketClient::on_message(websocketpp::connection_hdl hdl, ws_client_t::message_ptr msg) {
    std::string payload = msg->get_payload();
    if (payload.rfind("AUTH_OK ", 0) == 0) {
        std::stringstream ss(payload.substr(8));
        std::string color, uname;
        int elo;
        ss >> color >> uname >> elo;
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            assigned_color = color;
            m_username = uname;
            m_elo = elo;
            m_logged_in = true;
        }
        log_client_activity("Login successful! Color: " + color + " | User: " + uname + " | ELO: " + std::to_string(elo));
    } else if (payload.rfind("AUTH_FAIL", 0) == 0) {
        log_client_activity("Login failed! " + payload);
    } else if (payload.rfind("COLOR ", 0) == 0) {
        std::lock_guard<std::mutex> lock(state_mutex);
        assigned_color = payload.substr(6);
        log_client_activity("My assigned color is: " + assigned_color);
    } else if (payload == "SEARCHING") {
        std::lock_guard<std::mutex> lock(state_mutex);
        m_match_state = MatchState::SEARCHING;
        log_client_activity("Searching for opponent...");
    } else if (payload == "SEARCH_CANCELLED") {
        std::lock_guard<std::mutex> lock(state_mutex);
        m_match_state = MatchState::IDLE;
        log_client_activity("Search cancelled.");
    } else if (payload.rfind("MATCH_FOUND ", 0) == 0) {
        std::stringstream ss(payload.substr(12));
        std::string col, w_usr, b_usr;
        int w_e, b_e;
        ss >> col >> w_usr >> w_e >> b_usr >> b_e;
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            assigned_color = col;
            m_white_user = w_usr;
            m_white_elo = w_e;
            m_black_user = b_usr;
            m_black_elo = b_e;
            m_match_state = MatchState::MATCHED;
        }
        log_client_activity("MATCH FOUND! Playing as " + col + " (" + w_usr + " vs " + b_usr + ")");
    } else if (payload.rfind("ROOM_CREATED ", 0) == 0) {
        std::stringstream ss(payload.substr(13));
        std::string r_id, r_name, r_color;
        ss >> r_id >> r_name >> r_color;
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            m_room_id = r_id;
            m_room_name = r_name;
            assigned_color = r_color;
            m_is_viewer = false;
            m_match_state = MatchState::MATCHED;
            m_show_popup = false;
        }
        log_client_activity("ROOM CREATED successfully! ID: " + r_id + " Name: " + r_name + " Color: " + r_color);
    } else if (payload.rfind("ROOM_JOINED ", 0) == 0) {
        std::stringstream ss(payload.substr(12));
        std::string r_id, r_name, r_color;
        ss >> r_id >> r_name >> r_color;
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            m_room_id = r_id;
            m_room_name = r_name;
            assigned_color = r_color;
            m_is_viewer = (r_color == "VIEWER");
            m_match_state = MatchState::MATCHED;
            m_show_popup = false;
        }
        log_client_activity("ROOM JOINED! ID: " + r_id + " Name: " + r_name + " Role: " + r_color);
    } else if (payload.rfind("ROOM_ERROR ", 0) == 0) {
        std::lock_guard<std::mutex> lock(state_mutex);
        m_show_popup = true;
        m_popup_msg = "Room Error: " + payload.substr(11);
        log_client_activity("Room Error: " + payload.substr(11));
    } else if (payload == "MATCH_TIMEOUT") {
        std::lock_guard<std::mutex> lock(state_mutex);
        m_match_state = MatchState::TIMEOUT;
        m_popup_msg = "No opponent found within +-100 ELO range.\nPlease try again later.";
        m_show_popup = true;
        log_client_activity("Matchmaking timed out after 60 seconds.");
    } else {
        parse_and_update_state(payload);
    }
}

void SocketClient::send_login(const std::string& username, const std::string& password) {
    m_username = username;
    std::stringstream ss;
    ss << "LOGIN " << username << " " << password;
    websocketpp::lib::error_code ec;
    
    int retries = 0;
    while (retries < 10) {
        ec.clear();
        m_client.send(m_hdl, ss.str(), websocketpp::frame::opcode::text, ec);
        if (!ec) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        retries++;
    }
}

std::string SocketClient::get_username() const {
    return m_username;
}

int SocketClient::get_elo() const {
    return m_elo;
}

bool SocketClient::is_logged_in() const {
    return m_logged_in;
}

std::string SocketClient::get_white_username() const {
    return m_white_user;
}

int SocketClient::get_white_elo() const {
    return m_white_elo;
}

std::string SocketClient::get_black_username() const {
    return m_black_user;
}

int SocketClient::get_black_elo() const {
    return m_black_elo;
}

void SocketClient::send_move(int sr, int sc, int dr, int dc) {
    std::stringstream ss;
    ss << "MOVE " << ++m_seq_counter << " " << sr << " " << sc << " " << dr << " " << dc;
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, ss.str(), websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "[Client] Send move failed: " << ec.message() << std::endl;
    }
}

void SocketClient::send_jump(int r, int c) {
    std::stringstream ss;
    ss << "JUMP " << ++m_seq_counter << " " << r << " " << c;
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, ss.str(), websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "[Client] Send jump failed: " << ec.message() << std::endl;
    }
}

void SocketClient::parse_sounds(const std::string& json_str) {
    size_t sounds_start = json_str.find("\"sounds\":[");
    if (sounds_start == std::string::npos) return;
    
    size_t sounds_end = json_str.find("],\"pieces\":", sounds_start);
    if (sounds_end == std::string::npos) return;

    std::string sounds_array = json_str.substr(sounds_start + 10, sounds_end - (sounds_start + 10));
    size_t pos = 0;
    while (true) {
        size_t q1 = sounds_array.find("\"", pos);
        if (q1 == std::string::npos) break;
        size_t q2 = sounds_array.find("\"", q1 + 1);
        if (q2 == std::string::npos) break;
        std::string s_name = sounds_array.substr(q1 + 1, q2 - q1 - 1);
        if (!s_name.empty()) {
            bool has_active_king_capture = false;
            for (const auto& m : arbiter.get_active_motions()) {
                if (m.captured_piece && m.captured_piece->kind == model::PieceKind::KING && m.remaining_ms > 0) {
                    has_active_king_capture = true;
                    break;
                }
            }
            if ((s_name == "game_win" || s_name == "game_over") && has_active_king_capture) {
                m_delayed_sounds.push_back(s_name);
            } else {
                pubsub::MessageBus::get_instance().publish(pubsub::Event{
                    pubsub::EventType::PLAY_SOUND,
                    pubsub::SoundPayload{s_name}
                });
            }
        }
        pos = q2 + 1;
    }
}

void SocketClient::parse_pieces(const std::string& json_str, 
                                std::map<std::string, std::shared_ptr<model::Piece>>& parsed_pieces, 
                                std::vector<realtime::Cooldown>& cds) {
    size_t pieces_start = json_str.find("\"pieces\":[");
    if (pieces_start == std::string::npos) return;

    size_t pieces_end = json_str.find("],\"motions\":", pieces_start);
    if (pieces_end == std::string::npos) return;

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

void SocketClient::parse_motions(const std::string& json_str, 
                                 const std::map<std::string, std::shared_ptr<model::Piece>>& parsed_pieces, 
                                 std::vector<realtime::Motion>& parsed_motions) {
    size_t motions_start = json_str.find("\"motions\":[");
    if (motions_start == std::string::npos) return;

    size_t motions_end = json_str.find("],\"jumps\":", motions_start);
    if (motions_end == std::string::npos) return;

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
            attacker = parsed_pieces.at(piece_id);
        }
        
        std::shared_ptr<model::Piece> victim = nullptr;
        if (!victim_id.empty() && parsed_pieces.count(victim_id)) {
            victim = parsed_pieces.at(victim_id);
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

void SocketClient::parse_jumps(const std::string& json_str, 
                               const std::map<std::string, std::shared_ptr<model::Piece>>& parsed_pieces, 
                               std::vector<realtime::Jump>& parsed_jumps) {
    size_t jumps_start = json_str.find("\"jumps\":[");
    if (jumps_start == std::string::npos) return;

    size_t jumps_end = json_str.find("]", jumps_start);
    if (jumps_end == std::string::npos) return;

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
            piece = parsed_pieces.at(piece_id);
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

void SocketClient::parse_and_update_state(const std::string& json_str) {
    bool game_over = extract_bool(json_str, "game_over");
    int white_score = extract_int(json_str, "white_score");
    int black_score = extract_int(json_str, "black_score");

    std::string disc_user = extract_string(json_str, "disconnect_user");
    int disc_cd = extract_int(json_str, "disconnect_countdown");

    std::string w_user = extract_string(json_str, "white_user");
    int w_elo = extract_int(json_str, "white_elo");
    std::string b_user = extract_string(json_str, "black_user");
    int b_elo = extract_int(json_str, "black_elo");

    std::string r_id = extract_string(json_str, "room_id");
    std::string r_name = extract_string(json_str, "room_name");

    if (!w_user.empty()) m_white_user = w_user;
    if (w_elo > 0) m_white_elo = w_elo;
    if (!b_user.empty()) m_black_user = b_user;
    if (b_elo > 0) m_black_elo = b_elo;
    if (!r_id.empty()) m_room_id = r_id;
    if (!r_name.empty()) m_room_name = r_name;

    parse_sounds(json_str);

    auto new_board = std::make_shared<model::Board>(8, 8);
    std::map<std::string, std::shared_ptr<model::Piece>> parsed_pieces;
    std::vector<realtime::Cooldown> cds;

    parse_pieces(json_str, parsed_pieces, cds);

    std::vector<realtime::Motion> parsed_motions;
    parse_motions(json_str, parsed_pieces, parsed_motions);

    std::vector<realtime::Jump> parsed_jumps;
    parse_jumps(json_str, parsed_pieces, parsed_jumps);

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

    m_server_game_over = game_over;

    bool has_active_king_capture = false;
    for (const auto& m : arbiter.get_active_motions()) {
        if (m.captured_piece && m.captured_piece->kind == model::PieceKind::KING && m.remaining_ms > 0) {
            has_active_king_capture = true;
            break;
        }
    }

    bool local_game_over = game_over;
    if (has_active_king_capture) {
        local_game_over = false;
    }

    auto new_game_state = std::make_shared<model::GameState>(new_board);
    new_game_state->set_white_score(white_score);
    new_game_state->set_black_score(black_score);
    new_game_state->set_game_over(local_game_over);

    {
        std::lock_guard<std::mutex> lock(state_mutex);
        board = new_board;
        game_state = new_game_state;
        m_disconnect_user = disc_user;
        m_disconnect_countdown = disc_cd;

        std::vector<realtime::Motion> final_motions;
        for (const auto& pm : parsed_motions) {
            bool found = false;
            for (const auto& am : arbiter.get_active_motions()) {
                if (am.piece && pm.piece && am.piece->id == pm.piece->id) {
                    realtime::Motion merged = am;
                    merged.piece = pm.piece;
                    merged.dest = pm.dest;
                    merged.source = pm.source;
                    merged.total_ms = pm.total_ms;
                    final_motions.push_back(merged);
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
                if (aj.piece && pj.piece && aj.piece->id == pj.piece->id) {
                    realtime::Jump merged = aj;
                    merged.piece = pj.piece;
                    merged.pos = pj.pos;
                    merged.total_ms = pj.total_ms;
                    final_jumps.push_back(merged);
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

    if (m_server_game_over && game_state && !game_state->is_game_over()) {
        bool still_has_active_king_capture = false;
        for (const auto& m : next_motions) {
            if (m.captured_piece && m.captured_piece->kind == model::PieceKind::KING && m.remaining_ms > 0) {
                still_has_active_king_capture = true;
                break;
            }
        }
        if (!still_has_active_king_capture) {
            game_state->set_game_over(true);
            for (const auto& sound : m_delayed_sounds) {
                pubsub::MessageBus::get_instance().publish(pubsub::Event{
                    pubsub::EventType::PLAY_SOUND,
                    pubsub::SoundPayload{sound}
                });
            }
            m_delayed_sounds.clear();
        }
    }
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

void SocketClient::send_find_match() {
    std::lock_guard<std::mutex> lock(state_mutex);
    m_match_state = MatchState::SEARCHING;
    m_show_popup = false;
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, "FIND_MATCH", websocketpp::frame::opcode::text, ec);
}

void SocketClient::send_cancel_match() {
    std::lock_guard<std::mutex> lock(state_mutex);
    m_match_state = MatchState::IDLE;
    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, "CANCEL_MATCH", websocketpp::frame::opcode::text, ec);
}

MatchState SocketClient::get_match_state() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_match_state;
}

bool SocketClient::show_popup() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_show_popup;
}

std::string SocketClient::get_popup_msg() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_popup_msg;
}

void SocketClient::dismiss_popup() {
    std::lock_guard<std::mutex> lock(state_mutex);
    m_show_popup = false;
    m_match_state = MatchState::IDLE;
}

void SocketClient::send_create_room(const std::string& room_name) {
    std::lock_guard<std::mutex> lock(state_mutex);
    m_show_popup = false;
    websocketpp::lib::error_code ec;
    std::string msg = "CREATE_ROOM " + room_name;
    m_client.send(m_hdl, msg, websocketpp::frame::opcode::text, ec);
    log_client_activity("Sent CREATE_ROOM: " + room_name);
}

void SocketClient::send_join_room(const std::string& room_id) {
    std::lock_guard<std::mutex> lock(state_mutex);
    m_show_popup = false;
    websocketpp::lib::error_code ec;
    std::string msg = "JOIN_ROOM " + room_id;
    m_client.send(m_hdl, msg, websocketpp::frame::opcode::text, ec);
    log_client_activity("Sent JOIN_ROOM: " + room_id);
}

std::string SocketClient::get_room_id() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_room_id;
}

std::string SocketClient::get_room_name() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_room_name;
}

bool SocketClient::is_viewer() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_is_viewer;
}

std::string SocketClient::get_disconnect_user() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_disconnect_user;
}

int SocketClient::get_disconnect_countdown() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return m_disconnect_countdown;
}

}



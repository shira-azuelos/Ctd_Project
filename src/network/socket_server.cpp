#include "network/socket_server.h"
#include "model/board_factory.h"
#include "pubsub/message_bus.h"
#include "io/user_manager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace network {

static thread_local std::string tls_current_room_id;

void SocketServer::log_server_activity(const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << " [SERVER] " << msg;
    std::string line = ss.str();

    std::cout << line << std::endl;

    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ofstream ofs("server_activity.log", std::ios::app);
    if (ofs.is_open()) {
        ofs << line << std::endl;
    }
}

std::string SocketServer::generate_room_id() {
    static int counter = 1000;
    counter++;
    return "RM-" + std::to_string(counter);
}

bool SocketServer::is_same_connection(websocketpp::connection_hdl h1, websocketpp::connection_hdl h2) {
    return !h1.owner_before(h2) && !h2.owner_before(h1);
}

bool SocketServer::start(int port) {
    m_board = model::BoardFactory::create_default_board();
    m_game_engine = std::make_shared<engine::GameEngine>(m_board);

    pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::PLAY_SOUND, [this](const pubsub::Event& ev) {
        auto payload = std::any_cast<pubsub::SoundPayload>(ev.payload);
        std::lock_guard<std::mutex> lock(m_sounds_mutex);
        if (!tls_current_room_id.empty()) {
            std::lock_guard<std::mutex> r_lock(m_rooms_mutex);
            for (auto& r : m_rooms) {
                if (r->id == tls_current_room_id) {
                    r->pending_sounds.push_back(payload.sound_name);
                    break;
                }
            }
        } else {
            m_pending_sounds.push_back(payload.sound_name);
        }
    });

    try {
        m_server.init_asio();
        m_server.set_open_handler(std::bind(&SocketServer::on_open, this, std::placeholders::_1));
        m_server.set_close_handler(std::bind(&SocketServer::on_close, this, std::placeholders::_1));
        m_server.set_message_handler(std::bind(&SocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
        
        m_server.clear_access_channels(websocketpp::log::alevel::all);
        m_server.clear_error_channels(websocketpp::log::elevel::all);

        m_server.listen(port);
        m_server.start_accept();

        m_running = true;
        m_server_thread = std::thread([this]() {
            m_server.run();
        });
        
        m_game_thread = std::thread(&SocketServer::game_loop, this);
        
        std::cout << "[Server] WebSocket++ server running on port " << port << "..." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Server] Start failed: " << e.what() << std::endl;
        return false;
    }
}

void SocketServer::stop() {
    if (!m_running) return;
    m_running = false;

    m_server.stop_listening();

    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (const auto& client : m_clients) {
            websocketpp::lib::error_code ec;
            m_server.close(client->hdl, websocketpp::close::status::normal, "Server shutting down", ec);
        }
        m_clients.clear();
    }

    m_server.stop();

    if (m_server_thread.joinable()) m_server_thread.join();
    if (m_game_thread.joinable()) m_game_thread.join();

    std::cout << "[Server] Stopped." << std::endl;
}

SocketServer::~SocketServer() {
    stop();
}

void SocketServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_clients_mutex);
    
    std::string color = "VIEWER";

    auto client = std::make_shared<ClientInfo>();
    client->hdl = hdl;
    client->color = color;
    m_clients.push_back(client);

    std::string init_msg = "COLOR " + color;
    websocketpp::lib::error_code ec;
    m_server.send(hdl, init_msg, websocketpp::frame::opcode::text, ec);

    std::cout << "[Server] Client connected." << std::endl;
}

void SocketServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_clients_mutex);
    
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (is_same_connection((*it)->hdl, hdl)) {
            std::cout << "[Server] Client disconnected: " << (*it)->username << " (" << (*it)->color << ")" << std::endl;
            m_clients.erase(it);
            break;
        }
    }
}

void SocketServer::on_message(websocketpp::connection_hdl hdl, ws_server_t::message_ptr msg) {
    std::string payload = msg->get_payload();
    std::stringstream ss(payload);
    std::string cmd;
    ss >> cmd;

    std::string sender_color = "VIEWER";
    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (const auto& client : m_clients) {
            if (is_same_connection(client->hdl, hdl)) {
                sender_color = client->color;
                break;
            }
        }
    }

    if (cmd == "LOGIN") {
        std::string user_name, pass_word;
        ss >> user_name >> pass_word;
        io::User authenticated_user;
        bool ok = io::UserManager::get_instance().authenticate_or_register(user_name, pass_word, authenticated_user);
        
        std::string reply;
        if (ok) {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            for (auto& client : m_clients) {
                if (is_same_connection(client->hdl, hdl)) {
                    client->username = authenticated_user.username;
                    client->elo = authenticated_user.elo;
                    reply = "AUTH_OK " + client->color + " " + client->username + " " + std::to_string(client->elo);
                    break;
                }
            }
        } else {
            reply = "AUTH_FAIL Incorrect_Password";
        }
        websocketpp::lib::error_code ec;
        m_server.send(hdl, reply, websocketpp::frame::opcode::text, ec);
    }
    else if (cmd == "FIND_MATCH") {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (auto& client : m_clients) {
            if (is_same_connection(client->hdl, hdl)) {
                client->is_searching = true;
                client->search_start_time = std::chrono::steady_clock::now();
                client->in_game = false;
                std::cout << "[Server] Player " << client->username << " (ELO: " << client->elo << ") is searching for a match..." << std::endl;
                websocketpp::lib::error_code ec;
                m_server.send(hdl, "SEARCHING", websocketpp::frame::opcode::text, ec);
                break;
            }
        }
    }
    else if (cmd == "CANCEL_MATCH") {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (auto& client : m_clients) {
            if (is_same_connection(client->hdl, hdl)) {
                client->is_searching = false;
                std::cout << "[Server] Player " << client->username << " cancelled match search." << std::endl;
                websocketpp::lib::error_code ec;
                m_server.send(hdl, "SEARCH_CANCELLED", websocketpp::frame::opcode::text, ec);
                break;
            }
        }
    }
    else if (cmd == "CREATE_ROOM") {
        std::string room_name;
        std::getline(ss >> std::ws, room_name);
        if (room_name.empty()) room_name = "Battle Room";

        std::shared_ptr<ClientInfo> client_sender = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            for (auto& c : m_clients) {
                if (is_same_connection(c->hdl, hdl)) {
                    client_sender = c;
                    break;
                }
            }
        }

        if (client_sender) {
            auto room = std::make_shared<Room>();
            room->id = generate_room_id();
            room->name = room_name;
            room->white_player = client_sender;
            room->board = model::BoardFactory::create_default_board();
            room->game_engine = std::make_shared<engine::GameEngine>(room->board);

            client_sender->room_id = room->id;
            client_sender->color = "WHITE";
            client_sender->in_game = false;
            client_sender->is_viewer = false;
            client_sender->is_searching = false;

            {
                std::lock_guard<std::mutex> lock(m_rooms_mutex);
                m_rooms.push_back(room);
            }

            log_server_activity("Player " + client_sender->username + " created room " + room->id + " (\"" + room->name + "\") as WHITE.");

            websocketpp::lib::error_code ec;
            std::string reply = "ROOM_CREATED " + room->id + " " + room->name + " WHITE";
            m_server.send(hdl, reply, websocketpp::frame::opcode::text, ec);
        }
    }
    else if (cmd == "JOIN_ROOM") {
        std::string req_room_id;
        ss >> req_room_id;

        std::shared_ptr<ClientInfo> client_sender = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            for (auto& c : m_clients) {
                if (is_same_connection(c->hdl, hdl)) {
                    client_sender = c;
                    break;
                }
            }
        }

        if (client_sender) {
            std::shared_ptr<Room> target_room = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_rooms_mutex);
                for (auto& r : m_rooms) {
                    if (r->id == req_room_id || r->name == req_room_id) {
                        target_room = r;
                        break;
                    }
                }
            }

            if (!target_room) {
                log_server_activity("Player " + client_sender->username + " failed to join room " + req_room_id + " (Not Found).");
                websocketpp::lib::error_code ec;
                m_server.send(hdl, "ROOM_ERROR Room_Not_Found", websocketpp::frame::opcode::text, ec);
            } else {
                websocketpp::lib::error_code ec;
                if (!target_room->black_player) {
                    target_room->black_player = client_sender;
                    client_sender->room_id = target_room->id;
                    client_sender->color = "BLACK";
                    client_sender->in_game = true;
                    if (target_room->white_player) target_room->white_player->in_game = true;
                    client_sender->is_viewer = false;
                    client_sender->is_searching = false;

                    log_server_activity("Player " + client_sender->username + " joined room " + target_room->id + " as BLACK. Game started!");

                    std::string msg_black = "ROOM_JOINED " + target_room->id + " " + target_room->name + " BLACK";
                    m_server.send(hdl, msg_black, websocketpp::frame::opcode::text, ec);

                    if (target_room->white_player) {
                        std::string msg_white = "MATCH_FOUND WHITE " + target_room->white_player->username + " " + std::to_string(target_room->white_player->elo) + " " + client_sender->username + " " + std::to_string(client_sender->elo);
                        m_server.send(target_room->white_player->hdl, msg_white, websocketpp::frame::opcode::text, ec);
                    }

                    std::string match_msg_black = "MATCH_FOUND BLACK " + (target_room->white_player ? target_room->white_player->username : "WHITE") + " " + std::to_string(target_room->white_player ? target_room->white_player->elo : 1200) + " " + client_sender->username + " " + std::to_string(client_sender->elo);
                    m_server.send(hdl, match_msg_black, websocketpp::frame::opcode::text, ec);
                } else {
                    client_sender->room_id = target_room->id;
                    client_sender->color = "VIEWER";
                    client_sender->is_viewer = true;
                    client_sender->in_game = true;
                    target_room->viewers.push_back(client_sender);

                    log_server_activity("Player " + client_sender->username + " joined room " + target_room->id + " as VIEWER (Spectator Mode).");

                    std::string msg_viewer = "ROOM_JOINED " + target_room->id + " " + target_room->name + " VIEWER";
                    m_server.send(hdl, msg_viewer, websocketpp::frame::opcode::text, ec);
                }
            }
        }
    }
    else if (cmd == "MOVE") {
        int seq, sr, sc, dr, dc;
        ss >> seq >> sr >> sc >> dr >> dc;
        
        std::shared_ptr<ClientInfo> client_sender = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            for (auto& c : m_clients) {
                if (is_same_connection(c->hdl, hdl)) {
                    client_sender = c;
                    break;
                }
            }
        }

        if (client_sender) {
            if (seq <= client_sender->last_seq) {
                return;
            }
            client_sender->last_seq = seq;

            tls_current_room_id = client_sender->room_id;

            std::shared_ptr<engine::GameEngine> target_engine = m_game_engine;
            std::shared_ptr<model::Board> target_board = m_board;

            if (!client_sender->room_id.empty()) {
                std::lock_guard<std::mutex> lock(m_rooms_mutex);
                for (auto& r : m_rooms) {
                    if (r->id == client_sender->room_id) {
                        target_engine = r->game_engine;
                        target_board = r->board;
                        break;
                    }
                }
            }

            if (target_board && target_engine && !client_sender->is_viewer) {
                auto piece = target_board->get_piece_at(model::Position(sr, sc));
                if (piece) {
                    std::string expected_color = (piece->color == model::PieceColor::WHITE) ? "WHITE" : "BLACK";
                    if (client_sender->color == expected_color) {
                        target_engine->request_move(model::Position(sr, sc), model::Position(dr, dc));
                        log_server_activity("Move requested by " + client_sender->username + " (" + client_sender->color + ") in room " + client_sender->room_id + " from (" + std::to_string(sr) + "," + std::to_string(sc) + ") to (" + std::to_string(dr) + "," + std::to_string(dc) + ").");
                    }
                }
            }
            tls_current_room_id = "";
        }
    } 
    else if (cmd == "JUMP") {
        int seq, r, c;
        ss >> seq >> r >> c;

        std::shared_ptr<ClientInfo> client_sender = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_clients_mutex);
            for (auto& cl : m_clients) {
                if (is_same_connection(cl->hdl, hdl)) {
                    client_sender = cl;
                    break;
                }
            }
        }

        if (client_sender) {
            if (seq <= client_sender->last_seq) {
                return;
            }
            client_sender->last_seq = seq;

            tls_current_room_id = client_sender->room_id;

            std::shared_ptr<engine::GameEngine> target_engine = m_game_engine;
            std::shared_ptr<model::Board> target_board = m_board;

            if (!client_sender->room_id.empty()) {
                std::lock_guard<std::mutex> lock(m_rooms_mutex);
                for (auto& rm : m_rooms) {
                    if (rm->id == client_sender->room_id) {
                        target_engine = rm->game_engine;
                        target_board = rm->board;
                        break;
                    }
                }
            }

            if (target_board && target_engine && !client_sender->is_viewer) {
                auto piece = target_board->get_piece_at(model::Position(r, c));
                if (piece) {
                    std::string expected_color = (piece->color == model::PieceColor::WHITE) ? "WHITE" : "BLACK";
                    if (client_sender->color == expected_color) {
                        target_engine->request_jump(model::Position(r, c));
                        log_server_activity("Jump requested by " + client_sender->username + " (" + client_sender->color + ") in room " + client_sender->room_id + " at (" + std::to_string(r) + "," + std::to_string(c) + ").");
                    }
                }
            }
            tls_current_room_id = "";
        }
    }
}

void SocketServer::process_matchmaking() {
    std::lock_guard<std::mutex> lock(m_clients_mutex);
    auto now = std::chrono::steady_clock::now();

    std::vector<std::shared_ptr<ClientInfo>> searching;
    for (auto& c : m_clients) {
        if (c->is_searching) {
            searching.push_back(c);
        }
    }

    for (size_t i = 0; i < searching.size(); ++i) {
        if (!searching[i]->is_searching) continue;
        for (size_t j = i + 1; j < searching.size(); ++j) {
            if (!searching[j]->is_searching) continue;

            int elo_diff = std::abs(searching[i]->elo - searching[j]->elo);
            if (elo_diff <= 100) {
                auto c1 = searching[i];
                auto c2 = searching[j];

                c1->is_searching = false;
                c2->is_searching = false;
                c1->in_game = true;
                c2->in_game = true;
                c1->color = "WHITE";
                c2->color = "BLACK";

                m_board = model::BoardFactory::create_default_board();
                m_game_engine = std::make_shared<engine::GameEngine>(m_board);
                m_elo_updated_for_game = false;

                std::cout << "[Server] Match found! White: " << c1->username << " (" << c1->elo 
                          << ") vs Black: " << c2->username << " (" << c2->elo << ") [Diff: " << elo_diff << "]" << std::endl;

                websocketpp::lib::error_code ec;
                std::string msg1 = "MATCH_FOUND WHITE " + c1->username + " " + std::to_string(c1->elo) + " " + c2->username + " " + std::to_string(c2->elo);
                std::string msg2 = "MATCH_FOUND BLACK " + c1->username + " " + std::to_string(c1->elo) + " " + c2->username + " " + std::to_string(c2->elo);
                m_server.send(c1->hdl, msg1, websocketpp::frame::opcode::text, ec);
                m_server.send(c2->hdl, msg2, websocketpp::frame::opcode::text, ec);
                break;
            }
        }
    }

    for (auto& c : searching) {
        if (c->is_searching) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - c->search_start_time).count();
            if (elapsed >= 60) {
                c->is_searching = false;
                std::cout << "[Server] Matchmaking timeout (60s) for player " << c->username << " (no player in +-100 ELO range)." << std::endl;
                websocketpp::lib::error_code ec;
                m_server.send(c->hdl, "MATCH_TIMEOUT", websocketpp::frame::opcode::text, ec);
            }
        }
    }
}

void SocketServer::game_loop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        process_matchmaking();
        
        if (m_game_engine) {
            m_game_engine->update(30);
            auto state = m_game_engine->get_state();
            if (state && state->is_game_over() && !m_elo_updated_for_game) {
                m_elo_updated_for_game = true;
                std::shared_ptr<ClientInfo> white_c, black_c;
                {
                    std::lock_guard<std::mutex> lock(m_clients_mutex);
                    for (const auto& c : m_clients) {
                        if (c->color == "WHITE" && c->room_id.empty()) white_c = c;
                        if (c->color == "BLACK" && c->room_id.empty()) black_c = c;
                    }
                }
                if (white_c && black_c) {
                    bool white_won = (state->get_white_score() >= state->get_black_score());
                    if (white_won) {
                        io::UserManager::get_instance().update_elo_after_game(white_c->username, black_c->username, false);
                    } else {
                        io::UserManager::get_instance().update_elo_after_game(black_c->username, white_c->username, false);
                    }
                    white_c->elo = io::UserManager::get_instance().get_user(white_c->username).elo;
                    black_c->elo = io::UserManager::get_instance().get_user(black_c->username).elo;
                    log_server_activity("Match game over! Winner: " + std::string(white_won ? white_c->username : black_c->username));
                }
            }
            broadcast_state();
        }

        std::vector<std::shared_ptr<Room>> rooms_copy;
        {
            std::lock_guard<std::mutex> lock(m_rooms_mutex);
            rooms_copy = m_rooms;
        }

        for (auto& room : rooms_copy) {
            if (room->game_engine) {
                tls_current_room_id = room->id;
                room->game_engine->update(30);
                tls_current_room_id = "";
                auto state = room->game_engine->get_state();
                if (state && state->is_game_over() && !room->elo_updated) {
                    room->elo_updated = true;
                    if (room->white_player && room->black_player) {
                        bool white_won = (state->get_white_score() >= state->get_black_score());
                        if (white_won) {
                            io::UserManager::get_instance().update_elo_after_game(room->white_player->username, room->black_player->username, false);
                        } else {
                            io::UserManager::get_instance().update_elo_after_game(room->black_player->username, room->white_player->username, false);
                        }
                        room->white_player->elo = io::UserManager::get_instance().get_user(room->white_player->username).elo;
                        room->black_player->elo = io::UserManager::get_instance().get_user(room->black_player->username).elo;
                        log_server_activity("Room " + room->id + " game over! Winner: " + std::string(white_won ? room->white_player->username : room->black_player->username));
                    }
                }
                broadcast_room_state(room);
            }
        }
    }
}

void SocketServer::broadcast_room_state(std::shared_ptr<Room> room) {
    if (!room) return;

    bool is_over = room->game_engine && room->game_engine->get_state() && room->game_engine->get_state()->is_game_over();
    bool white_won = is_over ? (room->game_engine->get_state()->get_white_score() >= room->game_engine->get_state()->get_black_score()) : true;

    std::vector<std::shared_ptr<ClientInfo>> room_recipients;
    if (room->white_player) room_recipients.push_back(room->white_player);
    if (room->black_player) room_recipients.push_back(room->black_player);
    for (const auto& v : room->viewers) {
        if (v) room_recipients.push_back(v);
    }

    for (const auto& client : room_recipients) {
        websocketpp::lib::error_code ec;
        std::vector<std::string> client_sounds = room->pending_sounds;
        if (is_over) {
            bool is_winner = (client->color == "WHITE" && white_won) || (client->color == "BLACK" && !white_won);
            std::string end_sound = is_winner ? "game_win" : "game_over";
            bool has_end_sound = false;
            for (auto& s : client_sounds) {
                if (s == "game_win" || s == "game_over") {
                    s = end_sound;
                    has_end_sound = true;
                }
            }
            if (!has_end_sound) {
                client_sounds.push_back(end_sound);
            }
        }

        std::stringstream sounds_ss;
        sounds_ss << "[";
        for (size_t i = 0; i < client_sounds.size(); ++i) {
            if (i > 0) sounds_ss << ",";
            sounds_ss << "\"" << client_sounds[i] << "\"";
        }
        sounds_ss << "]";

        std::string client_json = serialize_room_state(room);
        size_t sounds_pos = client_json.find("\"sounds\":[");
        if (sounds_pos != std::string::npos) {
            size_t sounds_end = client_json.find("],", sounds_pos);
            if (sounds_end != std::string::npos) {
                client_json.replace(sounds_pos + 9, sounds_end - sounds_pos - 9 + 1, sounds_ss.str() + ",");
            }
        }

        m_server.send(client->hdl, client_json, websocketpp::frame::opcode::text, ec);
    }

    room->pending_sounds.clear();
}

std::string SocketServer::serialize_room_state(std::shared_ptr<Room> room) {
    if (!room || !room->game_engine) return "{}";
    std::stringstream ss;
    auto state = room->game_engine->get_state();
    ss << "{";
    ss << "\"game_over\":" << (state->is_game_over() ? "true" : "false") << ",";
    ss << "\"white_score\":" << state->get_white_score() << ",";
    ss << "\"black_score\":" << state->get_black_score() << ",";

    std::string w_user = room->white_player ? room->white_player->username : "White";
    int w_elo = room->white_player ? room->white_player->elo : 1200;
    std::string b_user = room->black_player ? room->black_player->username : "Black";
    int b_elo = room->black_player ? room->black_player->elo : 1200;

    ss << "\"white_user\":\"" << w_user << "\",";
    ss << "\"white_elo\":" << w_elo << ",";
    ss << "\"black_user\":\"" << b_user << "\",";
    ss << "\"black_elo\":" << b_elo << ",";
    ss << "\"room_id\":\"" << room->id << "\",";
    ss << "\"room_name\":\"" << room->name << "\",";

    ss << "\"sounds\":[";
    for (size_t i = 0; i < room->pending_sounds.size(); ++i) {
        ss << "\"" << room->pending_sounds[i] << "\"";
        if (i + 1 < room->pending_sounds.size()) ss << ",";
    }
    ss << "],";

    auto board = room->board;

    struct SerializedPiece {
        std::shared_ptr<model::Piece> piece;
        int row;
        int col;
    };
    std::vector<SerializedPiece> pieces_to_serialize;

    for (int r = 0; r < board->get_height(); ++r) {
        for (int c = 0; c < board->get_width(); ++c) {
            auto piece = board->get_piece_at(model::Position(r, c));
            if (piece) {
                pieces_to_serialize.push_back({piece, r, c});
            }
        }
    }

    for (const auto& m : room->game_engine->get_active_motions()) {
        if (m.piece) {
            bool exists = false;
            for (const auto& sp : pieces_to_serialize) {
                if (sp.piece == m.piece) { exists = true; break; }
            }
            if (!exists) {
                pieces_to_serialize.push_back({m.piece, m.source.row, m.source.col});
            }
        }
    }

    for (const auto& j : room->game_engine->get_active_jumps()) {
        if (j.piece) {
            bool exists = false;
            for (const auto& sp : pieces_to_serialize) {
                if (sp.piece == j.piece) { exists = true; break; }
            }
            if (!exists) {
                pieces_to_serialize.push_back({j.piece, j.pos.row, j.pos.col});
            }
        }
    }

    ss << "\"pieces\":[";
    bool first_piece = true;
    for (const auto& sp : pieces_to_serialize) {
        if (!first_piece) ss << ",";
        first_piece = false;

        bool on_cd = room->game_engine->get_arbiter().is_piece_cooling_down(sp.piece);
        int rem_cd = room->game_engine->get_arbiter().get_piece_cooldown_remaining_ms(sp.piece);
        int tot_cd = room->game_engine->get_arbiter().get_piece_cooldown_total_ms(sp.piece);
        bool long_rest = room->game_engine->get_arbiter().is_piece_on_long_rest(sp.piece);

        ss << "{";
        ss << "\"id\":\"" << sp.piece->id << "\",";
        ss << "\"kind\":" << static_cast<int>(sp.piece->kind) << ",";
        ss << "\"color\":" << static_cast<int>(sp.piece->color) << ",";
        ss << "\"row\":" << sp.row << ",";
        ss << "\"col\":" << sp.col << ",";
        ss << "\"state\":" << static_cast<int>(sp.piece->state) << ",";
        ss << "\"on_cooldown\":" << (on_cd ? "true" : "false") << ",";
        ss << "\"cooldown_remaining\":" << rem_cd << ",";
        ss << "\"cooldown_total\":" << tot_cd << ",";
        ss << "\"is_long_rest\":" << (long_rest ? "true" : "false");
        ss << "}";
    }
    ss << "],";

    ss << "\"motions\":[";
    auto motions = room->game_engine->get_active_motions();
    for (size_t i = 0; i < motions.size(); ++i) {
        const auto& m = motions[i];
        if (i > 0) ss << ",";
        ss << "{";
        ss << "\"piece_id\":\"" << m.piece->id << "\",";
        ss << "\"src_row\":" << m.source.row << ",";
        ss << "\"src_col\":" << m.source.col << ",";
        ss << "\"dest_row\":" << m.dest.row << ",";
        ss << "\"dest_col\":" << m.dest.col << ",";
        ss << "\"total_ms\":" << m.total_ms << ",";
        ss << "\"remaining_ms\":" << m.remaining_ms << ",";
        ss << "\"victim_id\":\"" << (m.captured_piece ? m.captured_piece->id : "") << "\"";
        ss << "}";
    }
    ss << "],";

    ss << "\"jumps\":[";
    auto jumps = room->game_engine->get_active_jumps();
    for (size_t i = 0; i < jumps.size(); ++i) {
        const auto& j = jumps[i];
        if (i > 0) ss << ",";
        ss << "{";
        ss << "\"piece_id\":\"" << j.piece->id << "\",";
        ss << "\"row\":" << j.pos.row << ",";
        ss << "\"col\":" << j.pos.col << ",";
        ss << "\"total_ms\":" << j.total_ms << ",";
        ss << "\"remaining_ms\":" << j.remaining_ms;
        ss << "}";
    }
    ss << "]";

    ss << "}";
    return ss.str();
}

void SocketServer::broadcast_state() {
    std::string state_json = serialize_game_state();

    bool is_over = m_game_engine && m_game_engine->get_state() && m_game_engine->get_state()->is_game_over();
    bool white_won = is_over ? (m_game_engine->get_state()->get_white_score() >= m_game_engine->get_state()->get_black_score()) : true;

    std::lock_guard<std::mutex> lock(m_clients_mutex);
    for (const auto& client : m_clients) {
        if (!client->room_id.empty()) continue;

        websocketpp::lib::error_code ec;
        std::string client_json = state_json;

        if (is_over) {
            bool is_winner = (client->color == "WHITE" && white_won) || (client->color == "BLACK" && !white_won);
            std::string target_sound = is_winner ? "game_win" : "game_over";
            
            size_t pos = client_json.find("\"game_win\"");
            if (pos != std::string::npos && !is_winner) {
                client_json.replace(pos, 10, "\"" + target_sound + "\"");
            }
        }

        m_server.send(client->hdl, client_json, websocketpp::frame::opcode::text, ec);
    }
}

std::string SocketServer::serialize_game_state() {
    std::stringstream ss;
    auto state = m_game_engine->get_state();
    ss << "{";
    ss << "\"game_over\":" << (state->is_game_over() ? "true" : "false") << ",";
    ss << "\"white_score\":" << state->get_white_score() << ",";
    ss << "\"black_score\":" << state->get_black_score() << ",";

    std::string w_user = "White", b_user = "Black";
    int w_elo = 1200, b_elo = 1200;
    {
        std::lock_guard<std::mutex> lock(m_clients_mutex);
        for (const auto& c : m_clients) {
            if (c->color == "WHITE") { w_user = c->username; w_elo = c->elo; }
            if (c->color == "BLACK") { b_user = c->username; b_elo = c->elo; }
        }
    }
    ss << "\"white_user\":\"" << w_user << "\",";
    ss << "\"white_elo\":" << w_elo << ",";
    ss << "\"black_user\":\"" << b_user << "\",";
    ss << "\"black_elo\":" << b_elo << ",";

    std::vector<std::string> sounds_to_send;
    {
        std::lock_guard<std::mutex> lock(m_sounds_mutex);
        sounds_to_send = m_pending_sounds;
        m_pending_sounds.clear();
    }
    ss << "\"sounds\":[";
    for (size_t i = 0; i < sounds_to_send.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "\"" << sounds_to_send[i] << "\"";
    }
    ss << "],";

    struct SerializedPiece {
        std::shared_ptr<model::Piece> piece;
        int row;
        int col;
    };
    std::vector<SerializedPiece> pieces_to_serialize;

    for (int r = 0; r < m_board->get_height(); ++r) {
        for (int c = 0; c < m_board->get_width(); ++c) {
            auto piece = m_board->get_piece_at(model::Position(r, c));
            if (piece) {
                pieces_to_serialize.push_back({piece, r, c});
            }
        }
    }

    for (const auto& m : m_game_engine->get_active_motions()) {
        if (m.piece) {
            bool exists = false;
            for (const auto& sp : pieces_to_serialize) {
                if (sp.piece == m.piece) { exists = true; break; }
            }
            if (!exists) {
                pieces_to_serialize.push_back({m.piece, m.source.row, m.source.col});
            }
        }
    }

    for (const auto& j : m_game_engine->get_active_jumps()) {
        if (j.piece) {
            bool exists = false;
            for (const auto& sp : pieces_to_serialize) {
                if (sp.piece == j.piece) { exists = true; break; }
            }
            if (!exists) {
                pieces_to_serialize.push_back({j.piece, j.pos.row, j.pos.col});
            }
        }
    }

    ss << "\"pieces\":[";
    bool first_piece = true;
    for (const auto& sp : pieces_to_serialize) {
        if (!first_piece) ss << ",";
        first_piece = false;
        ss << "{";
        ss << "\"id\":\"" << sp.piece->id << "\",";
        ss << "\"kind\":" << static_cast<int>(sp.piece->kind) << ",";
        ss << "\"color\":" << static_cast<int>(sp.piece->color) << ",";
        ss << "\"row\":" << sp.row << ",";
        ss << "\"col\":" << sp.col << ",";
        ss << "\"state\":" << static_cast<int>(sp.piece->state) << ",";
        
        bool on_cooldown = m_game_engine->is_piece_cooling_down(sp.piece);
        ss << "\"on_cooldown\":" << (on_cooldown ? "true" : "false") << ",";
        ss << "\"cooldown_remaining\":" << m_game_engine->get_piece_cooldown_remaining_ms(sp.piece) << ",";
        ss << "\"cooldown_total\":" << m_game_engine->get_piece_cooldown_total_ms(sp.piece) << ",";
        ss << "\"is_long_rest\":" << (m_game_engine->get_arbiter().is_piece_on_long_rest(sp.piece) ? "true" : "false");
        ss << "}";
    }
    ss << "],";

    ss << "\"motions\":[";
    bool first_motion = true;
    for (const auto& m : m_game_engine->get_active_motions()) {
        if (!m.piece) continue;
        if (!first_motion) ss << ",";
        first_motion = false;
        ss << "{";
        ss << "\"piece_id\":\"" << m.piece->id << "\",";
        ss << "\"src_row\":" << m.source.row << ",";
        ss << "\"src_col\":" << m.source.col << ",";
        ss << "\"dest_row\":" << m.dest.row << ",";
        ss << "\"dest_col\":" << m.dest.col << ",";
        ss << "\"total_ms\":" << m.total_ms << ",";
        ss << "\"remaining_ms\":" << m.remaining_ms << ",";
        ss << "\"victim_id\":\"" << (m.captured_piece ? m.captured_piece->id : "") << "\"";
        ss << "}";
    }
    ss << "],";

    ss << "\"jumps\":[";
    bool first_jump = true;
    for (const auto& j : m_game_engine->get_active_jumps()) {
        if (!j.piece) continue;
        if (!first_jump) ss << ",";
        first_jump = false;
        ss << "{";
        ss << "\"piece_id\":\"" << j.piece->id << "\",";
        ss << "\"row\":" << j.pos.row << ",";
        ss << "\"col\":" << j.pos.col << ",";
        ss << "\"total_ms\":" << j.total_ms << ",";
        ss << "\"remaining_ms\":" << j.remaining_ms;
        ss << "}";
    }
    ss << "]";

    ss << "}";
    return ss.str();
}

}

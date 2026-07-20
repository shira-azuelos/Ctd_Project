#include "network/socket_server.h"
#include "model/board_factory.h"
#include "pubsub/message_bus.h"
#include "io/user_manager.h"
#include <iostream>
#include <sstream>
#include <chrono>

namespace network {

static bool g_elo_updated_for_game = false;

bool SocketServer::is_same_connection(websocketpp::connection_hdl h1, websocketpp::connection_hdl h2) {
    return !h1.owner_before(h2) && !h2.owner_before(h1);
}

bool SocketServer::start(int port) {
    m_board = model::BoardFactory::create_default_board();
    m_game_engine = std::make_shared<engine::GameEngine>(m_board);

    pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::PLAY_SOUND, [this](const pubsub::Event& ev) {
        auto payload = std::any_cast<pubsub::SoundPayload>(ev.payload);
        std::lock_guard<std::mutex> lock(m_sounds_mutex);
        m_pending_sounds.push_back(payload.sound_name);
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
    if (m_clients.size() == 0) {
        color = "WHITE";
    } else if (m_clients.size() == 1) {
        color = "BLACK";
    }

    auto client = std::make_shared<ClientInfo>();
    client->hdl = hdl;
    client->color = color;
    m_clients.push_back(client);

    std::string init_msg = "COLOR " + color;
    websocketpp::lib::error_code ec;
    m_server.send(hdl, init_msg, websocketpp::frame::opcode::text, ec);

    std::cout << "[Server] Client connected as " << color << std::endl;
}

void SocketServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_clients_mutex);
    
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (is_same_connection((*it)->hdl, hdl)) {
            std::cout << "[Server] Client disconnected: " << (*it)->color << std::endl;
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
    else if (cmd == "MOVE") {
        int sr, sc, dr, dc;
        ss >> sr >> sc >> dr >> dc;
        
        auto piece = m_board->get_piece_at(model::Position(sr, sc));
        if (piece) {
            std::string expected_color = (piece->color == model::PieceColor::WHITE) ? "WHITE" : "BLACK";
            if (sender_color == expected_color) {
                m_game_engine->request_move(model::Position(sr, sc), model::Position(dr, dc));
            }
        }
    } 
    else if (cmd == "JUMP") {
        int r, c;
        ss >> r >> c;
        
        auto piece = m_board->get_piece_at(model::Position(r, c));
        if (piece) {
            std::string expected_color = (piece->color == model::PieceColor::WHITE) ? "WHITE" : "BLACK";
            if (sender_color == expected_color) {
                m_game_engine->request_jump(model::Position(r, c));
            }
        }
    }
}

void SocketServer::game_loop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (m_game_engine) {
            m_game_engine->wait(30);
            auto state = m_game_engine->get_state();
            if (state && state->is_game_over() && !g_elo_updated_for_game) {
                g_elo_updated_for_game = true;
                std::shared_ptr<ClientInfo> white_c, black_c;
                {
                    std::lock_guard<std::mutex> lock(m_clients_mutex);
                    for (const auto& c : m_clients) {
                        if (c->color == "WHITE") white_c = c;
                        if (c->color == "BLACK") black_c = c;
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
                }
            }
            broadcast_state();
        }
    }
}

void SocketServer::broadcast_state() {
    std::string state_json = serialize_game_state();

    std::lock_guard<std::mutex> lock(m_clients_mutex);
    for (const auto& client : m_clients) {
        websocketpp::lib::error_code ec;
        m_server.send(client->hdl, state_json, websocketpp::frame::opcode::text, ec);
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

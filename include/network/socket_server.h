#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include "engine/game_engine.h"

namespace network {

typedef websocketpp::server<websocketpp::config::asio> ws_server_t;

struct ClientInfo {
    websocketpp::connection_hdl hdl;
    std::string color = "VIEWER";
    std::string username = "Guest";
    int elo = 1200;
    bool is_searching = false;
    std::chrono::steady_clock::time_point search_start_time;
    bool in_game = false;
};

class SocketServer {
private:
    ws_server_t m_server;
    std::thread m_server_thread;
    std::thread m_game_thread;
    bool m_running = false;

    std::mutex m_clients_mutex;
    std::vector<std::shared_ptr<ClientInfo>> m_clients;

    std::shared_ptr<model::Board> m_board;
    std::shared_ptr<engine::GameEngine> m_game_engine;
    bool m_elo_updated_for_game = false;

    std::mutex m_sounds_mutex;
    std::vector<std::string> m_pending_sounds;

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, ws_server_t::message_ptr msg);
    
    void game_loop();
    void process_matchmaking();
    void broadcast_state();
    std::string serialize_game_state();

    bool is_same_connection(websocketpp::connection_hdl h1, websocketpp::connection_hdl h2);

public:
    SocketServer() = default;
    ~SocketServer();

    bool start(int port = 8080);
    void stop();
};

}


#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include "model/board.h"
#include "model/game_state.h"
#include "realtime/real_time_arbiter.h"

namespace network {

typedef websocketpp::client<websocketpp::config::asio_client> ws_client_t;

class SocketClient {
private:
    ws_client_t m_client;
    websocketpp::connection_hdl m_hdl;
    std::thread m_client_thread;
    bool m_running = false;

    std::string m_server_ip;
    int m_server_port;

    std::shared_ptr<model::Board> board;
    std::shared_ptr<model::GameState> game_state;
    
    realtime::RealTimeArbiter arbiter;

    std::string assigned_color = "VIEWER";
    std::mutex state_mutex;

    void on_message(websocketpp::connection_hdl hdl, ws_client_t::message_ptr msg);
    void parse_and_update_state(const std::string& json_str);

public:
    SocketClient(const std::string& ip = "127.0.0.1", int port = 8080);
    ~SocketClient();

    bool connect_to_server();
    void disconnect();

    void send_move(int sr, int sc, int dr, int dc);
    void send_jump(int r, int c);
    
    void advance_animations(int ms);

    std::shared_ptr<model::GameState> get_game_state();
    std::shared_ptr<model::Board> get_board();
    std::vector<realtime::Motion> get_active_motions();
    std::vector<realtime::Jump> get_active_jumps();
    realtime::RealTimeArbiter* get_arbiter();
    std::string get_assigned_color();
};

}

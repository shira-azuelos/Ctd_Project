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

enum class MatchState {
    IDLE,
    SEARCHING,
    MATCHED,
    TIMEOUT
};

class SocketClient {
private:
    ws_client_t m_client;
    websocketpp::connection_hdl m_hdl;
    std::thread m_client_thread;
    bool m_running = false;

    std::string m_server_ip;
    int m_server_port;
    bool m_connected = false;

    std::shared_ptr<model::Board> board;
    std::shared_ptr<model::GameState> game_state;
    
    realtime::RealTimeArbiter arbiter;

    std::string assigned_color = "VIEWER";
    std::string m_username = "Guest";
    int m_elo = 1200;
    bool m_logged_in = false;
    
    std::string m_white_user = "WHITE";
    int m_white_elo = 1200;
    std::string m_black_user = "BLACK";
    int m_black_elo = 1200;

    MatchState m_match_state = MatchState::IDLE;
    std::string m_popup_msg = "";
    bool m_show_popup = false;

    std::string m_room_id = "";
    std::string m_room_name = "";
    bool m_is_viewer = false;
    uint32_t m_seq_counter{0}; 
    bool m_server_game_over = false;
    std::vector<std::string> m_delayed_sounds;
    
    mutable std::mutex state_mutex;

    void on_message(websocketpp::connection_hdl hdl, ws_client_t::message_ptr msg);
    void parse_and_update_state(const std::string& json_str);
    void parse_sounds(const std::string& json_str);
    void parse_pieces(const std::string& json_str, std::map<std::string, std::shared_ptr<model::Piece>>& parsed_pieces, std::vector<realtime::Cooldown>& cds);
    void parse_motions(const std::string& json_str, const std::map<std::string, std::shared_ptr<model::Piece>>& parsed_pieces, std::vector<realtime::Motion>& parsed_motions);
    void parse_jumps(const std::string& json_str, const std::map<std::string, std::shared_ptr<model::Piece>>& parsed_pieces, std::vector<realtime::Jump>& parsed_jumps);
    void log_client_activity(const std::string& msg);

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
    std::string get_username() const;
    int get_elo() const;
    bool is_logged_in() const;

    std::string get_white_username() const;
    int get_white_elo() const;
    std::string get_black_username() const;
    int get_black_elo() const;
    
    void send_login(const std::string& username, const std::string& password);

    void send_find_match();
    void send_cancel_match();
    MatchState get_match_state() const;
    bool show_popup() const;
    std::string get_popup_msg() const;
    void dismiss_popup();

    void send_create_room(const std::string& room_name);
    void send_join_room(const std::string& room_id);
    std::string get_room_id() const;
    std::string get_room_name() const;
    bool is_viewer() const;
};

}


#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

namespace io {

struct User {
    std::string username;
    std::string password;
    int elo = 1200;
    int wins = 0;
    int losses = 0;
};

class UserManager {
private:
    std::unordered_map<std::string, User> m_users;
    std::mutex m_mutex;
    std::string m_filepath = "users.json";

    UserManager();

public:
    static UserManager& get_instance();

    bool load(const std::string& filepath = "users.json");
    bool save(const std::string& filepath = "users.json");

    bool authenticate_or_register(const std::string& username, const std::string& password, User& out_user);
    User get_user(const std::string& username);
    
    void update_elo_after_game(const std::string& winner_user, const std::string& loser_user, bool is_draw = false);
};

}

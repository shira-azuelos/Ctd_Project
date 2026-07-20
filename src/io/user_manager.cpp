#include "io/user_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace io {

UserManager::UserManager() {
    load();
}

UserManager& UserManager::get_instance() {
    static UserManager instance;
    return instance;
}

bool UserManager::load(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_filepath = filepath;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string json_str = ss.str();
    file.close();

    m_users.clear();

    size_t pos = 0;
    while (true) {
        size_t u_pos = json_str.find("\"username\"", pos);
        if (u_pos == std::string::npos) break;

        size_t end_obj = json_str.find("}", u_pos);
        if (end_obj == std::string::npos) break;

        std::string obj_str = json_str.substr(u_pos, end_obj - u_pos);

        auto extract_val = [&](const std::string& key) -> std::string {
            size_t k_pos = obj_str.find("\"" + key + "\"");
            if (k_pos == std::string::npos) return "";
            size_t colon = obj_str.find(":", k_pos);
            if (colon == std::string::npos) return "";
            size_t val_start = colon + 1;
            while (val_start < obj_str.length() && (obj_str[val_start] == ' ' || obj_str[val_start] == '"')) val_start++;
            size_t val_end = val_start;
            while (val_end < obj_str.length() && obj_str[val_end] != '"' && obj_str[val_end] != ',' && obj_str[val_end] != '\n' && obj_str[val_end] != '\r' && obj_str[val_end] != '}') val_end++;
            return obj_str.substr(val_start, val_end - val_start);
        };

        User user;
        user.username = extract_val("username");
        user.password = extract_val("password");
        
        std::string elo_str = extract_val("elo");
        user.elo = elo_str.empty() ? 1200 : std::stoi(elo_str);

        std::string wins_str = extract_val("wins");
        user.wins = wins_str.empty() ? 0 : std::stoi(wins_str);

        std::string losses_str = extract_val("losses");
        user.losses = losses_str.empty() ? 0 : std::stoi(losses_str);

        if (!user.username.empty()) {
            m_users[user.username] = user;
        }

        pos = end_obj + 1;
    }

    std::cout << "[UserManager] Loaded " << m_users.size() << " users from " << filepath << std::endl;
    return true;
}

bool UserManager::save(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string path = filepath.empty() ? m_filepath : filepath;
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[UserManager] Failed to open " << path << " for writing." << std::endl;
        return false;
    }

    file << "[\n";
    bool first = true;
    for (const auto& pair : m_users) {
        if (!first) file << ",\n";
        first = false;
        const auto& u = pair.second;
        file << "  {\n";
        file << "    \"username\": \"" << u.username << "\",\n";
        file << "    \"password\": \"" << u.password << "\",\n";
        file << "    \"elo\": " << u.elo << ",\n";
        file << "    \"wins\": " << u.wins << ",\n";
        file << "    \"losses\": " << u.losses << "\n";
        file << "  }";
    }
    file << "\n]\n";
    file.close();

    std::cout << "[UserManager] Saved " << m_users.size() << " users to " << path << std::endl;
    return true;
}

bool UserManager::authenticate_or_register(const std::string& username, const std::string& password, User& out_user) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_users.find(username);
    if (it != m_users.end()) {
        if (it->second.password == password) {
            out_user = it->second;
            std::cout << "[UserManager] User '" << username << "' authenticated successfully. ELO: " << out_user.elo << std::endl;
            return true;
        } else {
            std::cout << "[UserManager] Authentication failed for user '" << username << "': Incorrect password." << std::endl;
            return false;
        }
    } else {
        User new_user{username, password, 1200, 0, 0};
        m_users[username] = new_user;
        out_user = new_user;
        std::cout << "[UserManager] Registered new user '" << username << "' with default ELO 1200." << std::endl;
        
        std::ofstream file(m_filepath);
        if (file.is_open()) {
            file << "[\n";
            bool first = true;
            for (const auto& pair : m_users) {
                if (!first) file << ",\n";
                first = false;
                const auto& u = pair.second;
                file << "  {\n";
                file << "    \"username\": \"" << u.username << "\",\n";
                file << "    \"password\": \"" << u.password << "\",\n";
                file << "    \"elo\": " << u.elo << ",\n";
                file << "    \"wins\": " << u.wins << ",\n";
                file << "    \"losses\": " << u.losses << "\n";
                file << "  }";
            }
            file << "\n]\n";
            file.close();
        }
        return true;
    }
}

User UserManager::get_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_users.find(username);
    if (it != m_users.end()) return it->second;
    return User{username, "", 1200, 0, 0};
}

void UserManager::update_elo_after_game(const std::string& winner_user, const std::string& loser_user, bool is_draw) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_users.find(winner_user) == m_users.end() || m_users.find(loser_user) == m_users.end()) {
        std::cerr << "[UserManager] Cannot update ELO: One of the users not found." << std::endl;
        return;
    }

    User& u_win = m_users[winner_user];
    User& u_lose = m_users[loser_user];

    double ra = u_win.elo;
    double rb = u_lose.elo;

    double ea = 1.0 / (1.0 + std::pow(10.0, (rb - ra) / 400.0));
    double eb = 1.0 / (1.0 + std::pow(10.0, (ra - rb) / 400.0));

    double k = 32.0;
    double sa = is_draw ? 0.5 : 1.0;
    double sb = is_draw ? 0.5 : 0.0;

    int new_elo_a = std::round(ra + k * (sa - ea));
    int new_elo_b = std::round(rb + k * (sb - eb));

    std::cout << "[ELO Update] " << u_win.username << ": " << u_win.elo << " -> " << new_elo_a;
    std::cout << " | " << u_lose.username << ": " << u_lose.elo << " -> " << new_elo_b << std::endl;

    u_win.elo = new_elo_a;
    u_lose.elo = new_elo_b;

    if (!is_draw) {
        u_win.wins++;
        u_lose.losses++;
    }

    std::ofstream file(m_filepath);
    if (file.is_open()) {
        file << "[\n";
        bool first = true;
        for (const auto& pair : m_users) {
            if (!first) file << ",\n";
            first = false;
            const auto& u = pair.second;
            file << "  {\n";
            file << "    \"username\": \"" << u.username << "\",\n";
            file << "    \"password\": \"" << u.password << "\",\n";
            file << "    \"elo\": " << u.elo << ",\n";
            file << "    \"wins\": " << u.wins << ",\n";
            file << "    \"losses\": " << u.losses << "\n";
            file << "  }";
        }
        file << "\n]\n";
        file.close();
    }
}

}

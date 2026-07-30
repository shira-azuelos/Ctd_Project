#include "io/game_history_logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

namespace io {
void GameHistoryLogger::save(const std::string& room_id,
                              const std::string& white_user,
                              const std::string& black_user,
                              const std::string& winner) {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::stringstream ts;
    ts << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");

    std::ofstream log("game_history.log", std::ios::app);
    if (log.is_open()) {
        log << "INSERT INTO games (room_id, white_user, black_user, winner, created_at) VALUES ("
            << "'" << room_id    << "',"
            << "'" << white_user << "',"
            << "'" << black_user << "',"
            << "'" << winner     << "',"
            << "'" << ts.str()  << "');\n";
        log.close();
    }

    std::cout << "[GameHistory] Saved: room=" << room_id
              << " white=" << white_user
              << " black=" << black_user
              << " winner=" << winner << std::endl;
}

}

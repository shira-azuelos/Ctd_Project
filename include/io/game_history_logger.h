#pragma once
#include <string>
#include <chrono>

namespace io {

class GameHistoryLogger {
public:
    static void save(const std::string& room_id,
                     const std::string& white_user,
                     const std::string& black_user,
                     const std::string& winner);
};

} 

#pragma once
#include <string>

namespace io {

class RedisSessionStore {
public:
    static void set_session(const std::string& username,
                            const std::string& room_id,
                            int ttl_seconds = 20);
};

}

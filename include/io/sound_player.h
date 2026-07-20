#pragma once
#include <string>

namespace io {

class SoundPlayer {
public:
    static void play(const std::string& sound_name);
};

}

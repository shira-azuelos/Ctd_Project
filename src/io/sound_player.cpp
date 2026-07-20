#include "io/sound_player.h"
#include <unordered_map>
#include <windows.h>
#include <mmeapi.h>
#include <iostream>

namespace io {

void SoundPlayer::play(const std::string& sound_name) {
    static std::unordered_map<std::string, std::string> sound_map = {
        {"move", "assets/sounds/move.wav"},
        {"jump", "assets/sounds/jump.mp3"},
        {"capture", "assets/sounds/capture.wav"},
        {"illegal_move", "assets/sounds/illegal_move.wav"},
        {"illegal", "assets/sounds/illegal_move.wav"},
        {"block", "assets/sounds/illegal_move.wav"},
        {"coronation", "assets/sounds/coronation.mp3"},
        {"promotion", "assets/sounds/coronation.mp3"},
        {"game_over", "assets/sounds/game_over.mp3"}
    };

    auto it = sound_map.find(sound_name);
    if (it == sound_map.end()) return;

    std::string path = it->second;
    
    char abs_path_buf[MAX_PATH];
    GetFullPathNameA(path.c_str(), MAX_PATH, abs_path_buf, NULL);
    std::string abs_path(abs_path_buf);

    if (abs_path.length() >= 4 && abs_path.substr(abs_path.length() - 4) == ".wav") {
        PlaySoundA(abs_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);
    } else {
        std::string alias = "snd_" + sound_name;
        std::string cmd_close = "close " + alias;
        mciSendStringA(cmd_close.c_str(), NULL, 0, NULL);
        
        std::string cmd_open = "open \"" + abs_path + "\" alias " + alias;
        mciSendStringA(cmd_open.c_str(), NULL, 0, NULL);
        
        std::string cmd_play = "play " + alias + " from 0";
        mciSendStringA(cmd_play.c_str(), NULL, 0, NULL);
    }
}

}

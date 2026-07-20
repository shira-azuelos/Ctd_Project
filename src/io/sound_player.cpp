#include "io/sound_player.h"
#include <unordered_map>
#include <windows.h>
#include <mmeapi.h>
#include <iostream>
#include <thread>
#include <atomic>

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
        {"game_over", "assets/sounds/game_over.mp3"},
        {"game_win", "assets/sounds/game_win.mp3"},
        {"win", "assets/sounds/game_win.mp3"}
    };

    std::string path_mp3 = "assets/sounds/" + sound_name + ".mp3";
    std::string path_wav = "assets/sounds/" + sound_name + ".wav";
    
    std::string path = "";
    if (GetFileAttributesA(path_mp3.c_str()) != INVALID_FILE_ATTRIBUTES) {
        path = path_mp3;
    } else if (GetFileAttributesA(path_wav.c_str()) != INVALID_FILE_ATTRIBUTES) {
        path = path_wav;
    } else {
        auto it = sound_map.find(sound_name);
        if (it != sound_map.end()) path = it->second;
        else return;
    }
    
    char abs_path_buf[MAX_PATH];
    GetFullPathNameA(path.c_str(), MAX_PATH, abs_path_buf, NULL);
    std::string abs_path(abs_path_buf);

    std::thread([sound_name, abs_path]() {
        if (abs_path.length() >= 4 && abs_path.substr(abs_path.length() - 4) == ".wav") {
            PlaySoundA(abs_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        } else {
            static std::atomic<int> counter{0};
            int id = ++counter;
            std::string alias = "snd_" + sound_name + "_" + std::to_string(id);
            
            std::string cmd_open = "open \"" + abs_path + "\" alias " + alias;
            mciSendStringA(cmd_open.c_str(), NULL, 0, NULL);
            
            std::string cmd_play = "play " + alias + " wait";
            mciSendStringA(cmd_play.c_str(), NULL, 0, NULL);

            std::string cmd_close = "close " + alias;
            mciSendStringA(cmd_close.c_str(), NULL, 0, NULL);
        }
    }).detach();
}

}

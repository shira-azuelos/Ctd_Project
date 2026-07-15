#include "view/animation_manager.h"
#include <chrono>
#include <algorithm>

namespace view {

int AnimationManager::get_idle_frame() {
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return (now_ms / 250) % 5 + 1;
}

int AnimationManager::get_cooldown_frame(double cooldown_progress) {
    int frame = static_cast<int>((1.0 - cooldown_progress) * 5) + 1;
    return std::min(5, std::max(1, frame));
}

int AnimationManager::get_progress_frame(double progress) {
    int frame = static_cast<int>(progress * 5) + 1;
    return std::min(5, std::max(1, frame));
}

}

#pragma once

namespace view {

class AnimationManager {
public:
    static int get_idle_frame();

    static int get_cooldown_frame(double cooldown_progress);
    
    static int get_progress_frame(double progress);
};

}

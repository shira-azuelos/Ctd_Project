#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include <memory>

namespace view {

class OverlayRenderer {
public:
    void draw_overlays(Img& canvas, const std::shared_ptr<model::GameState>& state,
                       const std::string& disconnect_user = "", int disconnect_countdown = 0);
};

}

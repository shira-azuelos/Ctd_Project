#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include <memory>

namespace view {

class ScoreRenderer {
public:
    void draw(Img& canvas, const std::shared_ptr<model::GameState>& state);
};

}

#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include <memory>

namespace view {

class ScoreRenderer {
private:
    void draw_panel(Img& canvas, const std::string& name, int score, int x_offset);

public:
    void draw(Img& canvas, const std::shared_ptr<model::GameState>& state);
};

}

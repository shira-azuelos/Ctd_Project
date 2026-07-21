#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include <memory>
#include <string>

namespace view {

class ScoreRenderer {
private:
    void draw_panel(Img& canvas, const std::string& name, int score, int x_offset, const std::string& username = "", int elo = 1200);

public:
    void draw(Img& canvas, const std::shared_ptr<model::GameState>& state, 
              const std::string& white_user = "WHITE", int white_elo = 1200,
              const std::string& black_user = "BLACK", int black_elo = 1200,
              const std::string& room_id = "", const std::string& room_name = "", bool is_viewer = false);
};

}

#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace view {

class Renderer {
private:
    Img board_img;
    std::map<std::string, Img> piece_images;

public:
    Renderer();
    static std::vector<std::string> render_board(std::shared_ptr<model::GameState> state);
    void draw(Img& canvas, const std::shared_ptr<model::GameState>& state);
};

}
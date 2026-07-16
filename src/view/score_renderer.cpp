#include "view/score_renderer.h"
#include <string>

namespace view {

void ScoreRenderer::draw_panel(Img& canvas, const std::string& name, int score, int x_offset) {
    canvas.draw_rect(x_offset, 0, 100, 800, cv::Scalar(15, 15, 15), -1, 1.0);
    
    int border_x = (x_offset == 0) ? 99 : 900;
    canvas.draw_rect(border_x, 0, 1, 800, cv::Scalar(40, 40, 40), -1, 1.0);
    
    int title_offset = (name == "WHITE") ? 918 : 18;
    canvas.put_text(name, title_offset, 380, 0.55, cv::Scalar(150, 150, 150), 2);
    
    std::string score_str = std::to_string(score);
    int score_x = x_offset + (score_str.length() > 1 ? 28 : 38);
    canvas.put_text(score_str, score_x, 430, 1.0, cv::Scalar(255, 255, 255), 3);
    canvas.draw_rect(x_offset + 10, 460, 80, 1, cv::Scalar(40, 40, 40), -1, 1.0);
}

void ScoreRenderer::draw(Img& canvas, const std::shared_ptr<model::GameState>& state) {
    draw_panel(canvas, "BLACK", state->get_black_score(), 0);
    draw_panel(canvas, "WHITE", state->get_white_score(), 900);
}

}

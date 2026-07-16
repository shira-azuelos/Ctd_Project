#include "view/score_renderer.h"

namespace view {

void ScoreRenderer::draw(Img& canvas, const std::shared_ptr<model::GameState>& state) {
    // Left panel (Black score)
    canvas.draw_rect(0, 0, 100, 800, cv::Scalar(15, 15, 15), -1, 1.0);
    canvas.draw_rect(99, 0, 1, 800, cv::Scalar(40, 40, 40), -1, 1.0);
    canvas.put_text("BLACK", 18, 380, 0.55, cv::Scalar(150, 150, 150), 2);
    
    std::string black_score_str = std::to_string(state->get_black_score());
    // Center the number roughly inside the 100px width
    int b_offset = (black_score_str.length() > 1) ? 28 : 38;
    canvas.put_text(black_score_str, b_offset, 430, 1.0, cv::Scalar(255, 255, 255), 3);

    // Right panel (White score)
    canvas.draw_rect(900, 0, 100, 800, cv::Scalar(15, 15, 15), -1, 1.0);
    canvas.draw_rect(900, 0, 1, 800, cv::Scalar(40, 40, 40), -1, 1.0);
    canvas.put_text("WHITE", 918, 380, 0.55, cv::Scalar(150, 150, 150), 2);

    std::string white_score_str = std::to_string(state->get_white_score());
    int w_offset = (white_score_str.length() > 1) ? 928 : 938;
    canvas.put_text(white_score_str, w_offset, 430, 1.0, cv::Scalar(255, 255, 255), 3);
}

}

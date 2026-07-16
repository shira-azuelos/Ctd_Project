#include "view/overlay_renderer.h"

namespace view {

void OverlayRenderer::draw_overlays(Img& canvas, const std::shared_ptr<model::GameState>& state) {
    if (state->is_game_over()) {
        canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(0, 0, 0), -1, 0.7);
        canvas.put_text("GAME OVER", 280, 420, 2.5, cv::Scalar(0, 0, 255), 6);
        canvas.put_text("Press ESC to exit", 340, 480, 1.0, cv::Scalar(255, 255, 255), 2);
    }
}

}

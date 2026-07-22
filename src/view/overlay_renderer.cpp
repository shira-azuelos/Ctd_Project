#include "view/overlay_renderer.h"

namespace view {

void OverlayRenderer::draw_overlays(Img& canvas, const std::shared_ptr<model::GameState>& state,
                                   const std::string& disconnect_user, int disconnect_countdown) {
    if (state->is_game_over()) {
        canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(0, 0, 0), -1, 0.7);
        canvas.put_text("GAME OVER", 280, 420, 2.5, cv::Scalar(0, 0, 255), 6);
        canvas.put_text("Press ESC to exit", 340, 480, 1.0, cv::Scalar(255, 255, 255), 2);
    } else if (!disconnect_user.empty() && disconnect_countdown > 0) {
        canvas.draw_rect(200, 20, 600, 80, cv::Scalar(20, 20, 20), -1, 0.85);
        canvas.draw_rect(200, 20, 600, 80, cv::Scalar(0, 0, 220), 3);

        std::string title = "PLAYER DISCONNECTED: " + disconnect_user;
        std::string countdown_text = "Reconnecting... " + std::to_string(disconnect_countdown) + "s remaining until forfeit";

        canvas.put_text(title, 230, 50, 0.8, cv::Scalar(0, 215, 255), 2);
        canvas.put_text(countdown_text, 230, 80, 0.6, cv::Scalar(255, 255, 255), 2);
    }
}

}

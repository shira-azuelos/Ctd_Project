#include "view/score_renderer.h"
#include <string>

namespace view {

void ScoreRenderer::draw_panel(Img& canvas, const std::string& name, int score, int x_offset, const std::string& username, int elo) {
    canvas.draw_rect(x_offset, 0, 100, 800, cv::Scalar(15, 15, 15), -1, 1.0);
    
    int border_x = (x_offset == 0) ? 99 : 900;
    canvas.draw_rect(border_x, 0, 1, 800, cv::Scalar(40, 40, 40), -1, 1.0);
    
    int title_offset = (name == "WHITE") ? 918 : 18;
    canvas.put_text(name, title_offset, 340, 0.55, cv::Scalar(150, 150, 150), 2);

    if (!username.empty()) {
        int u_x = x_offset + 10;
        canvas.put_text(username.substr(0, 8), u_x, 370, 0.45, cv::Scalar(200, 200, 200), 1);
        canvas.put_text(std::to_string(elo), u_x, 395, 0.45, cv::Scalar(220, 180, 80), 1);
    }
    
    std::string score_str = std::to_string(score);
    int score_x = x_offset + (score_str.length() > 1 ? 28 : 38);
    canvas.put_text(score_str, score_x, 440, 1.0, cv::Scalar(255, 255, 255), 3);
    canvas.draw_rect(x_offset + 10, 470, 80, 1, cv::Scalar(40, 40, 40), -1, 1.0);
}

void ScoreRenderer::draw(Img& canvas, const std::shared_ptr<model::GameState>& state,
                         const std::string& white_user, int white_elo,
                         const std::string& black_user, int black_elo,
                         const std::string& room_id, const std::string& room_name, bool is_viewer) {
    draw_panel(canvas, "BLACK", state->get_black_score(), 0, black_user, black_elo);
    draw_panel(canvas, "WHITE", state->get_white_score(), 900, white_user, white_elo);

    if (!room_id.empty()) {
        canvas.draw_rect(200, 6, 600, 30, cv::Scalar(25, 20, 35), -1, 0.9);
        canvas.draw_rect(200, 6, 600, 30, cv::Scalar(180, 100, 255), 1, 0.95);
        std::string room_txt = "ROOM: " + room_id + " (\"" + room_name + "\")";
        if (is_viewer) room_txt += "  * SPECTATOR (VIEWER MODE)";
        canvas.put_text(room_txt, 215, 27, 0.5, is_viewer ? cv::Scalar(100, 230, 255) : cv::Scalar(220, 180, 255), 2);
    }
}

}

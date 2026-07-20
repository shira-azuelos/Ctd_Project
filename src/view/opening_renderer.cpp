#include "view/opening_renderer.h"
#include <iostream>
#include <chrono>

namespace view {

OpeningRenderer::OpeningRenderer() {
    try {
        bg_img.read("assets/Home_screen.jpg", {1000, 800}, false);
        loaded = true;
    } catch (const std::exception& e) {
        std::cerr << "[OpeningRenderer] Failed to load Home_screen.jpg: " << e.what() << std::endl;
        loaded = false;
    }
}

void OpeningRenderer::draw(Img& canvas, const std::string& username, int elo, bool waiting_for_opponent) {
    if (loaded) {
        bg_img.draw_on(canvas, 0, 0);
    } else {
        canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(20, 20, 25), -1, 1.0);
    }

    canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(10, 10, 15), -1, 0.45);
    
    canvas.draw_rect(150, 60, 700, 130, cv::Scalar(15, 15, 20), -1, 0.75);
    canvas.draw_rect(150, 60, 700, 130, cv::Scalar(215, 170, 60), 3, 0.9);

    canvas.put_text("KUNGFU CHESS", 260, 125, 1.7, cv::Scalar(0, 0, 0), 6);
    canvas.put_text("KUNGFU CHESS", 260, 125, 1.7, cv::Scalar(50, 215, 255), 3);
    
    canvas.put_text("REAL-TIME CHESS BATTLE", 360, 165, 0.65, cv::Scalar(220, 220, 220), 2);

    canvas.draw_rect(250, 280, 500, 110, cv::Scalar(20, 20, 28), -1, 0.85);
    canvas.draw_rect(250, 280, 500, 110, cv::Scalar(70, 70, 90), 2, 0.8);
    
    std::string user_display = "WELCOME, " + (username.empty() ? "PLAYER" : username);
    canvas.put_text(user_display, 280, 325, 0.75, cv::Scalar(255, 255, 255), 2);

    std::string elo_display = "RATING ELO: " + std::to_string(elo);
    canvas.put_text(elo_display, 280, 365, 0.7, cv::Scalar(220, 180, 60), 2);

    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(now - start_time).count();
    bool pulse = (static_cast<int>(elapsed_sec * 2.5) % 2 == 0);

    canvas.draw_rect(220, 470, 560, 75, cv::Scalar(20, 30, 20), -1, 0.85);
    canvas.draw_rect(220, 470, 560, 75, cv::Scalar(60, 180, 100), 2, 0.9);

    if (waiting_for_opponent) {
        canvas.put_text("WAITING FOR OPPONENT TO CONNECT...", 250, 518, 0.7, cv::Scalar(200, 220, 255), 2);
    } else {
        cv::Scalar prompt_color = pulse ? cv::Scalar(100, 255, 140) : cv::Scalar(220, 255, 220);
        canvas.put_text("PRESS SPACE OR CLICK TO START GAME", 240, 518, 0.7, prompt_color, 2);
    }

    canvas.draw_rect(100, 630, 800, 110, cv::Scalar(15, 15, 22), -1, 0.85);
    canvas.draw_rect(100, 630, 800, 110, cv::Scalar(60, 60, 80), 1, 0.8);

    canvas.put_text("CONTROLS & RULES:", 120, 660, 0.55, cv::Scalar(215, 170, 60), 2);
    canvas.put_text("* Click/Drag piece to move in real-time (1000ms flight)", 120, 688, 0.5, cv::Scalar(200, 200, 200), 1);
    canvas.put_text("* Red box & sound warns when enemy piece is under threat", 120, 712, 0.5, cv::Scalar(200, 200, 200), 1);
    canvas.put_text("* Right-Click idle piece after long rest to Jump (escape)", 120, 734, 0.5, cv::Scalar(200, 200, 200), 1);
}

}

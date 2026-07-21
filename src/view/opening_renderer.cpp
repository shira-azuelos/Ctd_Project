#include "view/opening_renderer.h"
#include <iostream>
#include <chrono>
#include <sstream>

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

void OpeningRenderer::draw_header(Img& canvas) {
    canvas.draw_rect(150, 60, 700, 130, cv::Scalar(15, 15, 20), -1, 0.75);
    canvas.draw_rect(150, 60, 700, 130, cv::Scalar(215, 170, 60), 3, 0.9);

    canvas.put_text("KUNGFU CHESS", 260, 125, 1.7, cv::Scalar(0, 0, 0), 6);
    canvas.put_text("KUNGFU CHESS", 260, 125, 1.7, cv::Scalar(50, 215, 255), 3);
    
    canvas.put_text("REAL-TIME CHESS BATTLE", 360, 165, 0.65, cv::Scalar(220, 220, 220), 2);
}

void OpeningRenderer::draw_user_profile(Img& canvas, const std::string& username, int elo) {
    canvas.draw_rect(250, 280, 500, 110, cv::Scalar(20, 20, 28), -1, 0.85);
    canvas.draw_rect(250, 280, 500, 110, cv::Scalar(70, 70, 90), 2, 0.8);
    
    std::string user_display = "WELCOME, " + (username.empty() ? "PLAYER" : username);
    canvas.put_text(user_display, 280, 325, 0.75, cv::Scalar(255, 255, 255), 2);

    std::string elo_display = "RATING ELO: " + std::to_string(elo);
    canvas.put_text(elo_display, 280, 365, 0.7, cv::Scalar(220, 180, 60), 2);
}

void OpeningRenderer::draw_play_button(Img& canvas, bool searching, int elapsed_sec) {
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double time_sec = std::chrono::duration<double>(now - start_time).count();
    bool pulse = (static_cast<int>(time_sec * 3.0) % 2 == 0);

    if (searching) {
        canvas.draw_rect(220, 470, 560, 75, cv::Scalar(40, 20, 20), -1, 0.9);
        canvas.draw_rect(220, 470, 560, 75, cv::Scalar(50, 180, 220), 3, 0.9);

        std::string search_txt = "SEARCHING FOR OPPONENT (+-100 ELO)... (" + std::to_string(elapsed_sec) + "s)";
        canvas.put_text(search_txt, 235, 518, 0.58, cv::Scalar(200, 230, 255), 2);
    } else {
        cv::Scalar fill_color = pulse ? cv::Scalar(15, 45, 15) : cv::Scalar(25, 60, 25);
        cv::Scalar border_color = pulse ? cv::Scalar(100, 255, 140) : cv::Scalar(60, 200, 100);

        canvas.draw_rect(220, 470, 560, 75, fill_color, -1, 0.9);
        canvas.draw_rect(220, 470, 560, 75, border_color, 3, 0.95);

        canvas.put_text("PLAY GAME (MATCHMAKING)", 290, 518, 0.8, border_color, 2);
    }
}

void OpeningRenderer::draw_rules(Img& canvas) {
    canvas.draw_rect(100, 630, 800, 110, cv::Scalar(15, 15, 22), -1, 0.85);
    canvas.draw_rect(100, 630, 800, 110, cv::Scalar(60, 60, 80), 1, 0.8);

    canvas.put_text("STAGE 4 MATCHMAKING RULES:", 120, 660, 0.55, cv::Scalar(215, 170, 60), 2);
    canvas.put_text("* Click PLAY to search for an opponent with ELO rating +-100 points.", 120, 688, 0.5, cv::Scalar(200, 200, 200), 1);
    canvas.put_text("* Search times out after 1 minute (60s) if no suitable player is found.", 120, 712, 0.5, cv::Scalar(200, 200, 200), 1);
    canvas.put_text("* Click/Drag pieces in real-time once paired with your opponent.", 120, 734, 0.5, cv::Scalar(200, 200, 200), 1);
}

void OpeningRenderer::draw_popup_modal(Img& canvas, const std::string& popup_msg) {
    canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(0, 0, 0), -1, 0.7);

    canvas.draw_rect(200, 240, 600, 320, cv::Scalar(20, 20, 30), -1, 0.95);
    canvas.draw_rect(200, 240, 600, 320, cv::Scalar(50, 180, 255), 3, 0.95);

    canvas.put_text("MATCHMAKING NOTICE", 325, 290, 0.85, cv::Scalar(50, 220, 255), 2);
    canvas.draw_rect(220, 310, 560, 2, cv::Scalar(50, 180, 255), -1, 0.8);

    canvas.put_text("No opponent found within +-100 ELO rating.", 240, 360, 0.6, cv::Scalar(240, 240, 240), 2);
    canvas.put_text("Search timed out after 1 minute (60s). Try again!", 225, 400, 0.6, cv::Scalar(220, 220, 150), 2);

    canvas.draw_rect(400, 460, 200, 55, cv::Scalar(30, 80, 40), -1, 0.9);
    canvas.draw_rect(400, 460, 200, 55, cv::Scalar(60, 220, 100), 2, 0.95);
    canvas.put_text("OK", 480, 498, 0.85, cv::Scalar(255, 255, 255), 2);
}

void OpeningRenderer::draw(Img& canvas, const std::string& username, int elo, 
                      bool searching, int elapsed_sec, 
                      bool show_popup, const std::string& popup_msg) {
    if (loaded) {
        bg_img.draw_on(canvas, 0, 0);
    } else {
        canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(20, 20, 25), -1, 1.0);
    }

    canvas.draw_rect(0, 0, 1000, 800, cv::Scalar(10, 10, 15), -1, 0.45);
    
    draw_header(canvas);
    draw_user_profile(canvas, username, elo);
    draw_play_button(canvas, searching, elapsed_sec);
    draw_rules(canvas);

    if (show_popup) {
        draw_popup_modal(canvas, popup_msg);
    }
}

}



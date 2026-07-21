#pragma once
#include "view/img.h"
#include <string>

namespace view {

class OpeningRenderer {
private:
    Img bg_img;
    bool loaded = false;

    void draw_header(Img& canvas);
    void draw_user_profile(Img& canvas, const std::string& username, int elo);
    void draw_play_button(Img& canvas, bool searching, int elapsed_sec);
    void draw_rules(Img& canvas);
    void draw_popup_modal(Img& canvas, const std::string& popup_msg);

public:
    OpeningRenderer();
    void draw(Img& canvas, const std::string& username = "Player", int elo = 1200, 
              bool searching = false, int elapsed_sec = 0, 
              bool show_popup = false, const std::string& popup_msg = "");
};

}



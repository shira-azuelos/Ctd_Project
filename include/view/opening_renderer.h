#pragma once
#include "view/img.h"
#include <string>

namespace view {

class OpeningRenderer {
private:
    Img bg_img;
    bool loaded = false;

public:
    OpeningRenderer();
    void draw(Img& canvas, const std::string& username = "Player", int elo = 1200, 
              bool searching = false, int elapsed_sec = 0, 
              bool show_popup = false, const std::string& popup_msg = "");
};

}


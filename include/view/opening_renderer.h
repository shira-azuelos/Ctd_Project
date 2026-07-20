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
    void draw(Img& canvas, const std::string& username = "Player", int elo = 1200, bool waiting_for_opponent = false);
};

}

#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include <memory>
#include <optional>

namespace realtime {
class RealTimeArbiter;
}

namespace view {

class BoardRenderer {
private:
    Img board_img;

public:
    BoardRenderer();
    void draw_background(Img& canvas);
    void draw_highlights(Img& canvas,
                         const std::shared_ptr<model::GameState>& state,
                         const std::optional<model::Position>& selected_cell,
                         const std::optional<model::Position>& hovered_cell = std::nullopt,
                         const realtime::RealTimeArbiter* arbiter = nullptr);
};

}

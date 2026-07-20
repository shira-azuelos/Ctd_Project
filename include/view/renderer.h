#pragma once
#include "view/img.h"
#include "view/board_renderer.h"
#include "view/piece_renderer.h"
#include "view/overlay_renderer.h"
#include "model/game_state.h"
#include "realtime/motion.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace realtime {
class RealTimeArbiter;
struct Motion;
struct Jump;
}

#include "view/score_renderer.h"
#include "view/process_renderer.h"
#include "view/opening_renderer.h"

namespace view {

class Renderer {
private:
    BoardRenderer board_renderer;
    PieceRenderer piece_renderer;
    OverlayRenderer overlay_renderer;
    ScoreRenderer score_renderer;
    ProcessRenderer process_renderer;
    OpeningRenderer opening_renderer;

public:
    Renderer();
    static std::vector<std::string> render_board(std::shared_ptr<model::GameState> state);
    void draw(Img& canvas, const std::shared_ptr<model::GameState>& state,
              const std::optional<model::Position>& selected_cell = std::nullopt,
              const std::vector<realtime::Motion>& active_motions = {},
              const std::vector<realtime::Jump>& active_jumps = {},
              const realtime::RealTimeArbiter* arbiter = nullptr,
              const DragInfo& drag_info = {},
              const std::string& white_user = "WHITE", int white_elo = 1200,
              const std::string& black_user = "BLACK", int black_elo = 1200);

    void draw_opening(Img& canvas, const std::string& username = "", int elo = 1200, bool waiting = false);
};

}
#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include "realtime/motion.h"
#include <memory>
#include <map>
#include <string>
#include <optional>

namespace realtime {
class RealTimeArbiter;
struct Motion;
struct Jump;
}

namespace view {

class PieceRenderer {
private:
    std::map<std::string, Img> piece_images;

public:
    void draw_pieces(Img& canvas,
                     const std::shared_ptr<model::GameState>& state,
                     const std::optional<realtime::Motion>& active_motion,
                     const std::optional<realtime::Jump>& active_jump,
                     const realtime::RealTimeArbiter* arbiter);
};

}

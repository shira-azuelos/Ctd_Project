#pragma once
#include "view/img.h"
#include "model/game_state.h"
#include "realtime/motion.h"
#include <memory>
#include <map>
#include <string>
#include <vector>

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
                     const std::vector<realtime::Motion>& active_motions,
                     const std::vector<realtime::Jump>& active_jumps,
                     const realtime::RealTimeArbiter* arbiter);
};

}

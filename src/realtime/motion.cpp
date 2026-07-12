#include "../include/realtime/motion.h"
#include <cmath>
#include <algorithm>

namespace realtime {

model::Position MotionState::calculate_current_pos(const model::Position& src, const model::Position& dest, int total_ms, int remaining_ms) {
    if (total_ms <= 0) return dest;
    float progress = 1.0f - (static_cast<float>(remaining_ms) / total_ms);
    progress = std::max(0.0f, std::min(1.0f, progress));
    
    int row = src.row + static_cast<int>(std::round(progress * (dest.row - src.row)));
    int col = src.col + static_cast<int>(std::round(progress * (dest.col - src.col)));
    
    return model::Position(row, col);
}

} 
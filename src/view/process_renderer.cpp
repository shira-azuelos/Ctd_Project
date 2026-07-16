#include "view/process_renderer.h"
#include "realtime/real_time_arbiter.h"
#include <vector>
#include <string>
#include <cstdio>

namespace view {

struct ProcessItem {
    std::string text;
    cv::Scalar color;
};

void ProcessRenderer::draw(Img& canvas, const realtime::RealTimeArbiter* arbiter) {
    if (!arbiter) return;

    std::vector<ProcessItem> black_processes;
    std::vector<ProcessItem> white_processes;

    for (const auto& motion : arbiter->get_active_motions()) {
        if (!motion.piece) continue;
        std::string name = motion.piece->id.substr(1); 
        ProcessItem item{ name + ": MOVE", cv::Scalar(130, 180, 80) }; 
        if (motion.piece->color == model::PieceColor::BLACK) {
            black_processes.push_back(item);
        } else {
            white_processes.push_back(item);
        }
    }

    for (const auto& jump : arbiter->get_active_jumps()) {
        if (!jump.piece) continue;
        std::string name = jump.piece->id.substr(1);
        ProcessItem item{ name + ": JUMP", cv::Scalar(220, 120, 80) };
        if (jump.piece->color == model::PieceColor::BLACK) {
            black_processes.push_back(item);
        } else {
            white_processes.push_back(item);
        }
    }

    for (const auto& cooldown : arbiter->get_active_cooldowns()) {
        if (!cooldown.piece) continue;
        std::string name = cooldown.piece->id.substr(1);
        
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1fs", (double)cooldown.remaining_ms / 1000.0);
        
        ProcessItem item{ name + ": " + buf, cv::Scalar(80, 175, 205) }; 
        if (cooldown.piece->color == model::PieceColor::BLACK) {
            black_processes.push_back(item);
        } else {
            white_processes.push_back(item);
        }
    }

    int b_y = 490;
    if (!black_processes.empty()) {
        canvas.put_text("ACTIVE", 22, b_y, 0.4, cv::Scalar(120, 120, 120), 1);
        b_y += 25;
        for (const auto& proc : black_processes) {
            if (b_y > 780) break; 
            canvas.put_text(proc.text, 15, b_y, 0.45, proc.color, 1);
            b_y += 25;
        }
    }

    int w_y = 490;
    if (!white_processes.empty()) {
        canvas.put_text("ACTIVE", 922, w_y, 0.4, cv::Scalar(120, 120, 120), 1);
        w_y += 25;
        for (const auto& proc : white_processes) {
            if (w_y > 780) break;
            canvas.put_text(proc.text, 915, w_y, 0.45, proc.color, 1);
            w_y += 25;
        }
    }
}

}

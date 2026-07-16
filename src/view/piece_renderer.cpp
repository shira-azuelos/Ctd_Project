#include "view/piece_renderer.h"
#include "view/animation_manager.h"
#include "realtime/real_time_arbiter.h"

namespace view {

void PieceRenderer::draw_pieces(Img& canvas,
                                 const std::shared_ptr<model::GameState>& state,
                                 const std::vector<realtime::Motion>& active_motions,
                                 const std::vector<realtime::Jump>& active_jumps,
                                 const realtime::RealTimeArbiter* arbiter,
                                 const DragInfo& drag_info) {
    auto board = state->get_board();
    if (!board) return;

    for (int row = 0; row < board->get_height(); ++row) {
        for (int col = 0; col < board->get_width(); ++col) {
            model::Position cell_pos(row, col);
            auto piece = board->get_piece_at(cell_pos);
            if (piece) {
                if (drag_info.piece == piece) {
                    continue;
                }

                bool is_piece_moving = false;
                for (const auto& m : active_motions) {
                    if (m.piece == piece) {
                        is_piece_moving = true;
                        break;
                    }
                }
                if (is_piece_moving) continue;

                bool is_piece_jumping = false;
                for (const auto& j : active_jumps) {
                    if (j.piece == piece) {
                        is_piece_jumping = true;
                        break;
                    }
                }
                if (is_piece_jumping) continue;

                char k_char = model::KIND_TO_CHAR.at(piece->kind);
                char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
                std::string folder = std::string(1, k_char) + std::string(1, c_char);
                
                bool on_cooldown = arbiter && arbiter->is_piece_cooling_down(piece);
                
                if (on_cooldown) {
                    int remaining = arbiter->get_piece_cooldown_remaining_ms(piece);
                    int total = arbiter->get_piece_cooldown_total_ms(piece);
                    double cooldown_progress = (total > 0) ? (double)remaining / total : 0.0;
                    if (cooldown_progress < 0.0) cooldown_progress = 0.0;
                    if (cooldown_progress > 1.0) cooldown_progress = 1.0;

                    int frame = AnimationManager::get_cooldown_frame(cooldown_progress);

                    std::string rest_state = (arbiter->is_piece_on_long_rest(piece)) ? "long_rest" : "short_rest";
                    std::string key = folder + "_" + rest_state + "_" + std::to_string(frame);
                    
                    if (piece_images.find(key) == piece_images.end()) {
                        std::string path = "assets/pieces/" + folder + "/states/" + rest_state + "/sprites/" + std::to_string(frame) + ".png";
                        piece_images[key].read(path, {100, 100}, false);
                    }
                    
                    piece_images[key].draw_on(canvas, col * 100, row * 100);

                    int cd_height = static_cast<int>(cooldown_progress * 100);
                    if (cd_height > 0) {
                        canvas.draw_rect(col * 100, row * 100 + 100 - cd_height, 100, cd_height, cv::Scalar(80, 175, 205), -1, 0.4);
                    }
                } else {
                    int idle_frame = AnimationManager::get_idle_frame();
                    
                    std::string key = folder + "_idle_" + std::to_string(idle_frame);
                    if (piece_images.find(key) == piece_images.end()) {
                        std::string path = "assets/pieces/" + folder + "/states/idle/sprites/" + std::to_string(idle_frame) + ".png";
                        piece_images[key].read(path, {100, 100}, false);
                    }
                    
                    piece_images[key].draw_on(canvas, col * 100, row * 100);
                }
            }
        }
    }

    for (const auto& motion : active_motions) {
        if (!motion.piece) continue;
        auto piece = motion.piece;
        char k_char = model::KIND_TO_CHAR.at(piece->kind);
        char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
        std::string folder = std::string(1, k_char) + std::string(1, c_char);
        
        double progress = (double)(motion.total_ms - motion.remaining_ms) / motion.total_ms;
        if (progress < 0.0) progress = 0.0;
        if (progress > 1.0) progress = 1.0;
        
        int frame = AnimationManager::get_progress_frame(progress);
        
        std::string key = folder + "_move_" + std::to_string(frame);
        if (piece_images.find(key) == piece_images.end()) {
            std::string path = "assets/pieces/" + folder + "/states/move/sprites/" + std::to_string(frame) + ".png";
            piece_images[key].read(path, {100, 100}, false);
        }
        
        int start_x = motion.source.col * 100;
        int start_y = motion.source.row * 100;
        int end_x = motion.dest.col * 100;
        int end_y = motion.dest.row * 100;
        
        int current_x = start_x + static_cast<int>(progress * (end_x - start_x));
        int current_y = start_y + static_cast<int>(progress * (end_y - start_y));
        
        piece_images[key].draw_on(canvas, current_x, current_y);
    }

    for (const auto& jump : active_jumps) {
        if (!jump.piece) continue;
        auto piece = jump.piece;
        char k_char = model::KIND_TO_CHAR.at(piece->kind);
        char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
        std::string folder = std::string(1, k_char) + std::string(1, c_char);
        
        int total = jump.total_ms;
        int remaining = jump.remaining_ms;
        double progress = (total > 0) ? (double)(total - remaining) / total : 0.0;
        if (progress < 0.0) progress = 0.0;
        if (progress > 1.0) progress = 1.0;
        
        int frame = AnimationManager::get_progress_frame(progress);
        
        std::string key = folder + "_jump_" + std::to_string(frame);
        if (piece_images.find(key) == piece_images.end()) {
            std::string path = "assets/pieces/" + folder + "/states/jump/sprites/" + std::to_string(frame) + ".png";
            piece_images[key].read(path, {100, 100}, false);
        }
        
        piece_images[key].draw_on(canvas, jump.pos.col * 100, jump.pos.row * 100);
    }

    if (drag_info.piece) {
        auto piece = drag_info.piece;
        char k_char = model::KIND_TO_CHAR.at(piece->kind);
        char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
        std::string folder = std::string(1, k_char) + std::string(1, c_char);

        int idle_frame = AnimationManager::get_idle_frame();
        std::string key = folder + "_idle_" + std::to_string(idle_frame);
        
        if (piece_images.find(key) == piece_images.end()) {
            std::string path = "assets/pieces/" + folder + "/states/idle/sprites/" + std::to_string(idle_frame) + ".png";
            piece_images[key].read(path, {100, 100}, false);
        }
        
        piece_images[key].draw_on(canvas, drag_info.x - 50, drag_info.y - 50);
    }
}

}

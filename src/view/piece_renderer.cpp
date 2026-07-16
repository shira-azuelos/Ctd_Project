#include "view/piece_renderer.h"
#include "view/animation_manager.h"
#include "realtime/real_time_arbiter.h"

namespace view {

Img& PieceRenderer::get_piece_image(std::shared_ptr<model::Piece> piece, const std::string& state, int frame) {
    char k_char = model::KIND_TO_CHAR.at(piece->kind);
    char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
    std::string folder = std::string(1, k_char) + std::string(1, c_char);

    std::string key = folder + "_" + state + "_" + std::to_string(frame);
    if (piece_images.find(key) == piece_images.end()) {
        std::string path = "assets/pieces/" + folder + "/states/" + state + "/sprites/" + std::to_string(frame) + ".png";
        piece_images[key].read(path, {100, 100}, false);
    }
    return piece_images[key];
}

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
                if (drag_info.piece == piece) continue;

                bool is_piece_moving = false;
                for (const auto& m : active_motions) {
                    if (m.piece == piece) { is_piece_moving = true; break; }
                }
                if (is_piece_moving) continue;

                bool is_piece_jumping = false;
                for (const auto& j : active_jumps) {
                    if (j.piece == piece) { is_piece_jumping = true; break; }
                }
                if (is_piece_jumping) continue;
                
                bool on_cooldown = arbiter && arbiter->is_piece_cooling_down(piece);
                int draw_x = col * 100 + 100;
                
                if (on_cooldown) {
                    int remaining = arbiter->get_piece_cooldown_remaining_ms(piece);
                    int total = arbiter->get_piece_cooldown_total_ms(piece);
                    double cooldown_progress = (total > 0) ? (double)remaining / total : 0.0;
                    cooldown_progress = std::max(0.0, std::min(1.0, cooldown_progress));

                    int frame = AnimationManager::get_cooldown_frame(cooldown_progress);
                    std::string rest_state = (arbiter->is_piece_on_long_rest(piece)) ? "long_rest" : "short_rest";
                    
                    get_piece_image(piece, rest_state, frame).draw_on(canvas, draw_x, row * 100);

                    int cd_height = static_cast<int>(cooldown_progress * 100);
                    if (cd_height > 0) {
                        canvas.draw_rect(draw_x, row * 100 + 100 - cd_height, 100, cd_height, cv::Scalar(80, 175, 205), -1, 0.4);
                    }
                } else {
                    int idle_frame = AnimationManager::get_idle_frame();
                    get_piece_image(piece, "idle", idle_frame).draw_on(canvas, draw_x, row * 100);
                }
            }
        }
    }

    for (const auto& motion : active_motions) {
        if (!motion.piece) continue;
        double progress = (double)(motion.total_ms - motion.remaining_ms) / motion.total_ms;
        progress = std::max(0.0, std::min(1.0, progress));
        
        int frame = AnimationManager::get_progress_frame(progress);
        
        int start_x = motion.source.col * 100 + 100;
        int start_y = motion.source.row * 100;
        int end_x = motion.dest.col * 100 + 100;
        int end_y = motion.dest.row * 100;
        
        int current_x = start_x + static_cast<int>(progress * (end_x - start_x));
        int current_y = start_y + static_cast<int>(progress * (end_y - start_y));
        
        get_piece_image(motion.piece, "move", frame).draw_on(canvas, current_x, current_y);
    }

    for (const auto& jump : active_jumps) {
        if (!jump.piece) continue;
        int total = jump.total_ms;
        int remaining = jump.remaining_ms;
        double progress = (total > 0) ? (double)(total - remaining) / total : 0.0;
        progress = std::max(0.0, std::min(1.0, progress));
        
        int frame = AnimationManager::get_progress_frame(progress);
        get_piece_image(jump.piece, "jump", frame).draw_on(canvas, jump.pos.col * 100 + 100, jump.pos.row * 100);
    }

    if (drag_info.piece) {
        int idle_frame = AnimationManager::get_idle_frame();
        get_piece_image(drag_info.piece, "idle", idle_frame).draw_on(canvas, drag_info.x - 50, drag_info.y - 50);
    }
}

}

#include "view/renderer.h"
#include "io/board_printer.h"
#include "rules/rule_engine.h"
#include "realtime/real_time_arbiter.h"
#include <optional>

namespace view {

Renderer::Renderer() {
    board_img.read("assets/board.png", {800, 800}, false);
}

std::vector<std::string> Renderer::render_board(std::shared_ptr<model::GameState> state) {
    return io::BoardPrinter::print(state);
}

void view::Renderer::draw(Img& canvas, const std::shared_ptr<model::GameState>& state,
                          const std::optional<model::Position>& selected_cell,
                          const std::optional<realtime::Motion>& active_motion,
                          const realtime::RealTimeArbiter* arbiter) {
    if (!state) return;
    
    board_img.draw_on(canvas, 0, 0);

    auto board = state->get_board();
    if (!board) return;

    bool game_over = state->is_game_over();

    for (int row = 0; row < board->get_height(); ++row) {
        for (int col = 0; col < board->get_width(); ++col) {
            model::Position cell_pos(row, col);
            
            bool is_selected = !game_over && selected_cell && (*selected_cell == cell_pos);
            bool is_valid_move = false;
            bool is_valid_attack = false;
            
            if (!game_over && selected_cell && !is_selected) {
                if (rules::RuleEngine::validate_move(*board, *selected_cell, cell_pos)) {
                    auto piece_at_dest = board->get_piece_at(cell_pos);
                    if (piece_at_dest) {
                        is_valid_attack = true;
                    } else {
                        is_valid_move = true;
                    }
                }
            }

            auto piece = board->get_piece_at(cell_pos);
            if (piece) {
                char k_char = model::KIND_TO_CHAR.at(piece->kind);
                char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
                std::string folder = std::string(1, k_char) + std::string(1, c_char);
                
                std::string suffix = ((row + col) % 2 == 0) ? "_light" : "_dark";
                std::string state_suffix = "";
                if (is_selected) {
                    state_suffix = "_selected";
                } else if (is_valid_attack) {
                    state_suffix = "_attack";
                }
                
                std::string key = folder + suffix + state_suffix;
                
                if (piece_images.find(key) == piece_images.end()) {
                    std::string path = "assets/pieces/" + folder + "/states/idle/sprites/1" + suffix + state_suffix + ".png";
                    piece_images[key].read(path, {100, 100}, false);
                }
                
                piece_images[key].draw_on(canvas, col * 100, row * 100);

                if (active_motion && active_motion->piece == piece) {
                    double progress = (double)(active_motion->total_ms - active_motion->remaining_ms) / active_motion->total_ms;
                    if (progress < 0.0) progress = 0.0;
                    if (progress > 1.0) progress = 1.0;
                    
                    int fill_height = static_cast<int>(progress * 100);
                    if (fill_height > 0) {
                        canvas.draw_rect(col * 100, row * 100 + 100 - fill_height, 100, fill_height, cv::Scalar(0, 255, 255), -1, 0.45);
                    }
                }

                if (arbiter && arbiter->is_piece_cooling_down(piece)) {
                    int remaining = arbiter->get_piece_cooldown_remaining_ms(piece);
                    int total = arbiter->get_piece_cooldown_total_ms(piece);
                    if (total > 0 && remaining > 0) {
                        double cooldown_progress = (double)remaining / total;
                        if (cooldown_progress < 0.0) cooldown_progress = 0.0;
                        if (cooldown_progress > 1.0) cooldown_progress = 1.0;
                        
                        int cd_height = static_cast<int>(cooldown_progress * 100);
                        if (cd_height > 0) {
                            canvas.draw_rect(col * 100, row * 100 + 100 - cd_height, 100, cd_height, cv::Scalar(255, 0, 0), -1, 0.4);
                        }
                    }
                }
            } else if (is_valid_move) {
                if ((row + col) % 2 == 0) { 
                    canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(229, 82, 227), -1, 0.45);
                } else { 
                    canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(220, 200, 255), -1, 0.45);
                }
            }
        }
    }

    if (game_over) {
        canvas.draw_rect(0, 0, 800, 800, cv::Scalar(0, 0, 0), -1, 0.7);
        canvas.put_text("GAME OVER", 180, 420, 2.5, cv::Scalar(0, 0, 255), 6);
        canvas.put_text("Press ESC to exit", 240, 480, 1.0, cv::Scalar(255, 255, 255), 2);
    }
}

}
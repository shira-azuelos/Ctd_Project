#include "view/board_renderer.h"
#include "rules/rule_engine.h"
#include "realtime/real_time_arbiter.h"

namespace view {

BoardRenderer::BoardRenderer() {
    board_img.read("assets/board.png", {800, 800}, false);
}

void BoardRenderer::draw_background(Img& canvas) {
    board_img.draw_on(canvas, 0, 0);
}

void BoardRenderer::draw_highlights(Img& canvas,
                                     const std::shared_ptr<model::GameState>& state,
                                     const std::optional<model::Position>& selected_cell,
                                     const std::optional<model::Position>& hovered_cell,
                                     const realtime::RealTimeArbiter* arbiter) {
    auto board = state->get_board();
    if (!board) return;
    bool game_over = state->is_game_over();

    for (int row = 0; row < board->get_height(); ++row) {
        for (int col = 0; col < board->get_width(); ++col) {
            model::Position cell_pos(row, col);
            
            bool is_selected = !game_over && selected_cell && (*selected_cell == cell_pos);
            bool is_hovered = !game_over && hovered_cell && (*hovered_cell == cell_pos);
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

            if (is_selected) {
                canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(64, 140, 164), 4);
            }
            else if (is_valid_attack) {
                canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(0, 0, 255), 4);
            }
            else if (is_valid_move) {
                if ((row + col) % 2 == 0) { 
                    canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(80, 180, 130), -1, 0.45);
                } else { 
                    canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(120, 210, 160), -1, 0.45);
                }
            }

            if (is_hovered) {
                canvas.draw_rect(col * 100, row * 100, 100, 100, cv::Scalar(255, 255, 255), -1, 0.15);
            }
        }
    }
}

}

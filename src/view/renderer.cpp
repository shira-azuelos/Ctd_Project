#include "view/renderer.h"
#include "io/board_printer.h"

namespace view {

Renderer::Renderer() : board_renderer(), piece_renderer(), overlay_renderer(), score_renderer(), process_renderer() {}

std::vector<std::string> Renderer::render_board(std::shared_ptr<model::GameState> state) {
    return io::BoardPrinter::print(state);
}

void Renderer::draw(Img& canvas, const std::shared_ptr<model::GameState>& state,
                    const std::optional<model::Position>& selected_cell,
                    const std::vector<realtime::Motion>& active_motions,
                    const std::vector<realtime::Jump>& active_jumps,
                    const realtime::RealTimeArbiter* arbiter,
                    const DragInfo& drag_info,
                    const std::string& white_user, int white_elo,
                    const std::string& black_user, int black_elo,
                    const std::string& room_id, const std::string& room_name, bool is_viewer) {
    if (!state) return;
    
    board_renderer.draw_background(canvas);
    board_renderer.draw_highlights(canvas, state, selected_cell, arbiter);
    piece_renderer.draw_pieces(canvas, state, active_motions, active_jumps, arbiter, drag_info);
    overlay_renderer.draw_overlays(canvas, state);
    score_renderer.draw(canvas, state, white_user, white_elo, black_user, black_elo, room_id, room_name, is_viewer);
    process_renderer.draw(canvas, arbiter);
}

void Renderer::draw_opening(Img& canvas, const std::string& username, int elo, 
                          bool searching, int elapsed_sec, 
                          bool show_popup, const std::string& popup_msg,
                          bool show_room_dialog, const std::string& room_input) {
    opening_renderer.draw(canvas, username, elo, searching, elapsed_sec, show_popup, popup_msg, show_room_dialog, room_input);
}

}
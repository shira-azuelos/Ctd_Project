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
                    const DragInfo& drag_info) {
    if (!state) return;
    
    board_renderer.draw_background(canvas);
    board_renderer.draw_highlights(canvas, state, selected_cell, arbiter);
    piece_renderer.draw_pieces(canvas, state, active_motions, active_jumps, arbiter, drag_info);
    overlay_renderer.draw_overlays(canvas, state);
    score_renderer.draw(canvas, state);
    process_renderer.draw(canvas, arbiter);
}

}
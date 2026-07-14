#include "view/renderer.h"
#include "io/board_printer.h"

namespace view {

Renderer::Renderer() {
    board_img.read("assets/board.png", {800, 800}, false);
}

std::vector<std::string> Renderer::render_board(std::shared_ptr<model::GameState> state) {
    return io::BoardPrinter::print(state);
}

void Renderer::draw(Img& canvas, const std::shared_ptr<model::GameState>& state) {
    if (!state) return;
    
    board_img.draw_on(canvas, 0, 0);

    auto board = state->get_board();
    if (!board) return;

    for (int row = 0; row < board->get_height(); ++row) {
        for (int col = 0; col < board->get_width(); ++col) {
            auto piece = board->get_piece_at(model::Position(row, col));
            if (piece) {
                char k_char = model::KIND_TO_CHAR.at(piece->kind);
                char c_char = (piece->color == model::PieceColor::WHITE) ? 'W' : 'B';
                std::string folder = std::string(1, k_char) + std::string(1, c_char);
                
                std::string suffix = ((row + col) % 2 == 0) ? "_light" : "_dark";
                std::string key = folder + suffix;
                
                if (piece_images.find(key) == piece_images.end()) {
                    std::string path = "assets/pieces/" + folder + "/states/idle/sprites/1" + suffix + ".png";
                    piece_images[key].read(path, {100, 100}, false);
                }
                
                piece_images[key].draw_on(canvas, col * 100, row * 100);
            }
        }
    }
}

}
#include "../include/view/renderer.h"
#include "../include/io/board_printer.h"

namespace view {
std::vector<std::string> Renderer::render_board(std::shared_ptr<model::GameState> state) {
    return io::BoardPrinter::print(state);
}
}
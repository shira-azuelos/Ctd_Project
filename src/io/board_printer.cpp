#include "io/board_printer.h"

namespace io {

std::vector<std::string> BoardPrinter::print(const std::shared_ptr<model::GameState>& state) {
    if (!state || !state->get_board()) {
        return {};
    }

    auto board = state->get_board();
    std::vector<std::string> output;

    for (int row = 0; row < board->get_height(); ++row) {
        std::string line = "";
        for (int col = 0; col < board->get_width(); ++col) {
            auto piece = board->get_piece_at(model::Position(row, col));
            
            if (!piece) {
                line += ".";
            } else {
                if (piece->color == model::PieceColor::WHITE) line += "w";
                else line += "b";

                line += model::KIND_TO_CHAR.at(piece->kind);
            }
            
            if (col < board->get_width() - 1) {
                line += " ";
            }
        }
        output.push_back(line);
    }

    return output;
}

} 
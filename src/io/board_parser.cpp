#include "../include/io/board_parser.h"
#include <sstream>
#include <stdexcept>

namespace io {

std::shared_ptr<model::Board> BoardParser::parse(const std::vector<std::string>& lines) {
    if (lines.empty()) {
        throw std::invalid_argument("Cannot parse an empty board");
    }

    int height = lines.size();
    int width = -1;
    std::vector<std::vector<std::string>> grid_tokens;

    for (const auto& line : lines) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> row_tokens;
        while (ss >> token) {
            row_tokens.push_back(token);
        }

        if (width == -1) {
            width = row_tokens.size();
        } else if (width != (int)row_tokens.size()) {
            throw std::invalid_argument("ERROR ROW_WIDTH_MISMATCH"); 
        }
        grid_tokens.push_back(row_tokens);
    }

    auto board = std::make_shared<model::Board>(width, height);

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            std::string t = grid_tokens[r][c];
            if (t == ".") continue;

            if (t.size() != 2) throw std::invalid_argument("ERROR UNKNOWN_TOKEN"); 

            model::PieceColor color;
            if (t[0] == 'w') color = model::PieceColor::WHITE;
            else if (t[0] == 'b') color = model::PieceColor::BLACK;
            else throw std::invalid_argument("ERROR UNKNOWN_TOKEN"); 

            auto it = model::CHAR_TO_KIND.find(t[1]);
            if (it == model::CHAR_TO_KIND.end()) {
                throw std::invalid_argument("ERROR UNKNOWN_TOKEN");
            }
            model::PieceKind kind = it->second; 

            auto piece = std::make_shared<model::Piece>(t, color, kind, model::Position(r, c));
            board->add_piece(piece);
        }
    }
    return board;
}

} 
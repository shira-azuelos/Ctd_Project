#include "io/board_parser.h"
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace io {

std::shared_ptr<model::Board> BoardParser::parse(const std::vector<std::string>& lines) {
    if (lines.empty()) {
        throw std::invalid_argument("Cannot parse an empty board");
    }

    int height = lines.size();
    int width = -1;
    std::vector<std::vector<std::string>> parsed_tokens;

    // פירוק השורות למילים (tokens) ובדיקת אורך עקבי
    for (const auto& line : lines) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (width == -1) {
            width = tokens.size();
        } else if (width != tokens.size()) {
            throw std::invalid_argument("Inconsistent row length detected"); // דחיית אורך לא עקבי
        }
        parsed_tokens.push_back(tokens);
    }

    auto board = std::make_shared<model::Board>(width, height);
    int piece_counter = 1;

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            std::string token = parsed_tokens[row][col];
            
            if (token == ".") { // תא ריק
                continue;
            }

            if (token.length() != 2) {
                throw std::invalid_argument("Invalid piece token: " + token); // דחיית טוקן לא חוקי
            }

            model::PieceColor color;
            if (token[0] == 'w') color = model::PieceColor::WHITE;
            else if (token[0] == 'b') color = model::PieceColor::BLACK;
            else throw std::invalid_argument("Invalid piece color in token: " + token);

            model::PieceKind kind;
            char k = token[1];
            if (k == 'K') kind = model::PieceKind::KING;
            else if (k == 'Q') kind = model::PieceKind::QUEEN;
            else if (k == 'R') kind = model::PieceKind::ROOK;
            else if (k == 'B') kind = model::PieceKind::BISHOP;
            else if (k == 'N') kind = model::PieceKind::KNIGHT;
            else if (k == 'P') kind = model::PieceKind::PAWN;
            else throw std::invalid_argument("Invalid piece kind in token: " + token);

            std::string id = "piece_" + std::to_string(piece_counter++);
            auto piece = std::make_shared<model::Piece>(id, color, kind, model::Position(row, col));
            board->add_piece(piece);
        }
    }

    return board;
}

} // namespace io
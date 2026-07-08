#include "Board.h"
#include <sstream>

void Board::addRow(const std::string& line) {
    std::istringstream rowStream(line);
    std::vector<std::unique_ptr<Piece>> row;
    std::string token;
    
    std::string emptyCellStr(1, EMPTY_CELL_CHAR);

    while (rowStream >> token) {
        if (token == emptyCellStr) row.push_back(nullptr);
        else {
            char c = token[0], t = token[1];
            if (t == TYPE_KING) row.push_back(std::make_unique<King>(c));
            else if (t == TYPE_QUEEN) row.push_back(std::make_unique<Queen>(c));
            else if (t == TYPE_ROOK) row.push_back(std::make_unique<Rook>(c));
            else if (t == TYPE_BISHOP) row.push_back(std::make_unique<Bishop>(c));
            else if (t == TYPE_KNIGHT) row.push_back(std::make_unique<Knight>(c));
            else if (t == TYPE_PAWN) row.push_back(std::make_unique<Pawn>(c));
        }
    }
    state_.push_back(std::move(row));
    rows_ = state_.size();
    cols_ = state_[0].size();
}

const Piece* Board::getPiece(int r, int c) const {
    if (r < 0 || r >= (int)state_.size() || c < 0 || c >= (int)state_[0].size()) return nullptr;
    return state_[r][c].get();
}

void Board::click(int x, int y) {
    int r = y / 100, c = x / 100;
    
    if (selectedRow_ == -1) {
        if (getPiece(r, c)) {
            selectedRow_ = r;
            selectedCol_ = c;
        }
    } else {
        const Piece* selectedPiece = getPiece(selectedRow_, selectedCol_);
        const Piece* targetPiece = getPiece(r, c);

        if (selectedPiece && selectedPiece->isLegalMove(selectedRow_, selectedCol_, r, c, *this)) {
            if (!targetPiece || targetPiece->getColor() != selectedPiece->getColor()) {
                state_[r][c] = std::move(state_[selectedRow_][selectedCol_]);
                state_[selectedRow_][selectedCol_] = nullptr;
            }
        }
        selectedRow_ = -1;
        selectedCol_ = -1;
    }
}
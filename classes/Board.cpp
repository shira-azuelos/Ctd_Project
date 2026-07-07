#include "Board.h"

#include <iostream>
#include <sstream>

Board::Board() : rows_(0), cols_(0), selectedRow_(-1), selectedCol_(-1), gameTime_(0) {}

void Board::addRow(const std::string& line) {
    if (line.empty()) {
        return;
    }

    std::istringstream rowStream(line);
    std::vector<std::string> row;
    std::string token;

    while (rowStream >> token) {
        row.push_back(token);
    }

    if (!row.empty()) {
        state_.push_back(row);
    }
}

bool Board::validate() {
    if (state_.empty()) {
        error_ = "ERROR UNKNOWN_TOKEN";
        return false;
    }

    int expectedWidth = state_[0].size();
    for (const auto& row : state_) {
        if (static_cast<int>(row.size()) != expectedWidth) {
            error_ = "ERROR ROW_WIDTH_MISMATCH";
            return false;
        }

        for (const auto& token : row) {
            if (!isValidToken(token)) {
                error_ = "ERROR UNKNOWN_TOKEN";
                return false;
            }
        }
    }

    rows_ = static_cast<int>(state_.size());
    cols_ = expectedWidth;
    return true;
}

void Board::printCanonical(std::ostream& out) const {
    if (state_.empty()) {
        return;
    }

    for (const auto& row : state_) {
        for (size_t i = 0; i < row.size(); ++i) {
            out << row[i];
            if (i < row.size() - 1) {
                out << " ";
            }
        }
        out << '\n';
    }
}

std::string Board::getError() const {
    return error_;
}

void Board::click(int x, int y) {
    if (rows_ <= 0 || cols_ <= 0) {
        return;
    }

    int row = y / 100;
    int col = x / 100;

    if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
        return;
    }

    const std::string& target = state_[row][col];

    if (selectedRow_ == -1 && selectedCol_ == -1) {
        if (target != "." && (target[0] == 'w' || target[0] == 'b')) {
            selectedRow_ = row;
            selectedCol_ = col;
        }
        return;
    }

    if (row == selectedRow_ && col == selectedCol_) {
        selectedRow_ = -1;
        selectedCol_ = -1;
        return;
    }

    char selectedColor = state_[selectedRow_][selectedCol_][0];

    if (target == "." || target[0] != selectedColor) {
        if (!isMoveLegal(selectedRow_, selectedCol_, row, col)) {
            return;
        }

        state_[row][col] = state_[selectedRow_][selectedCol_];
        state_[selectedRow_][selectedCol_] = ".";
        selectedRow_ = -1;
        selectedCol_ = -1;
        return;
    }

    if (target[0] == selectedColor) {
        selectedRow_ = row;
        selectedCol_ = col;
    }
}

void Board::wait(int ms) {
    gameTime_ += ms;
}

bool Board::isMoveLegal(int startRow, int startCol, int endRow, int endCol) const {
    if (startRow == endRow && startCol == endCol) {
        return false;
    }

    if (startRow < 0 || startRow >= rows_ || startCol < 0 || startCol >= cols_ ||
        endRow < 0 || endRow >= rows_ || endCol < 0 || endCol >= cols_) {
        return false;
    }

    const std::string& piece = state_[startRow][startCol];
    const std::string& target = state_[endRow][endCol];

    if (piece == "." || piece.size() != 2) {
        return false;
    }

    if (target != "." && target[0] == piece[0]) {
        return false;
    }

    char type = piece[1];
    int rowDiff = endRow - startRow;
    int colDiff = endCol - startCol;
    int absRow = rowDiff < 0 ? -rowDiff : rowDiff;
    int absCol = colDiff < 0 ? -colDiff : colDiff;

    switch (type) {
    case 'K':
        return absRow <= 1 && absCol <= 1;
    case 'Q':
        if (rowDiff == 0 || colDiff == 0 || absRow == absCol) {
            int stepRow = (rowDiff == 0) ? 0 : (rowDiff > 0 ? 1 : -1);
            int stepCol = (colDiff == 0) ? 0 : (colDiff > 0 ? 1 : -1);
            int steps = std::max(absRow, absCol);
            for (int i = 1; i < steps; ++i) {
                if (state_[startRow + stepRow * i][startCol + stepCol * i] != ".") {
                    return false;
                }
            }
            return true;
        }
        return false;
    case 'R':
        if (rowDiff == 0 || colDiff == 0) {
            int stepRow = (rowDiff == 0) ? 0 : (rowDiff > 0 ? 1 : -1);
            int stepCol = (colDiff == 0) ? 0 : (colDiff > 0 ? 1 : -1);
            int steps = std::max(absRow, absCol);
            for (int i = 1; i < steps; ++i) {
                if (state_[startRow + stepRow * i][startCol + stepCol * i] != ".") {
                    return false;
                }
            }
            return true;
        }
        return false;
    case 'B':
        if (absRow == absCol) {
            int stepRow = (rowDiff > 0) ? 1 : -1;
            int stepCol = (colDiff > 0) ? 1 : -1;
            for (int i = 1; i < absRow; ++i) {
                if (state_[startRow + stepRow * i][startCol + stepCol * i] != ".") {
                    return false;
                }
            }
            return true;
        }
        return false;
    case 'N':
        return (absRow == 2 && absCol == 1) || (absRow == 1 && absCol == 2);
    default:
        return false;
    }
}

bool Board::isValidToken(const std::string& token) {
    if (token == ".") {
        return true;
    }

    if (token.size() == 2) {
        char color = token[0];
        char piece = token[1];

        if ((color == 'w' || color == 'b') &&
            (piece == 'K' || piece == 'Q' || piece == 'R' ||
             piece == 'B' || piece == 'N' || piece == 'P')) {
            return true;
        }
    }

    return false;
}

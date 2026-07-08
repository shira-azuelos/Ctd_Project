#include "../include/Board.h"
#include <sstream>

Board::Board() : rows_(0), cols_(0), selectedRow_(-1), selectedCol_(-1) {}

bool Board::validate() {
    if (error_ == "ERROR UNKNOWN_TOKEN") {
        return false;
    }

    if (state_.empty()) {
        error_ = "Invalid board dimensions";
        return false;
    }

    size_t expectedCols = state_[0].size();
    for (size_t i = 1; i < state_.size(); ++i) {
        if (state_[i].size() != expectedCols) {
            error_ = "ERROR ROW_WIDTH_MISMATCH";
            return false;
        }
    }

    return true;
}

std::string Board::getError() const {
    return error_;
}

void Board::wait(int ms) {
    gameTime_ += ms;
    processPendingMoves();
}

void Board::printCanonical(std::ostream& out) const {
    for (const auto& row : state_) {
        for (size_t i = 0; i < row.size(); ++i) {
            out << (row[i] ? row[i]->toString() : ".");
            if (i + 1 < row.size()) {
                out << " ";
            }
        }
        out << "\n";
    }
}

void Board::addRow(const std::string& line) {
    std::istringstream rowStream(line);
    std::vector<std::unique_ptr<Piece>> row;
    std::string token;
    
    std::string emptyCellStr(1, EMPTY_CELL_CHAR);

    while (rowStream >> token) {
        if (token == emptyCellStr) {
            row.push_back(nullptr);
        } 
        else if (token.length() == 2) {
            char c = token[0], t = token[1];
            
            bool validColor = (c == COLOR_WHITE || c == COLOR_BLACK);
            bool validPiece = (t == TYPE_KING || t == TYPE_QUEEN || t == TYPE_ROOK || 
                               t == TYPE_BISHOP || t == TYPE_KNIGHT || t == TYPE_PAWN);
            
            if (validColor && validPiece) {
                if (t == TYPE_KING) row.push_back(std::make_unique<King>(c));
                else if (t == TYPE_QUEEN) row.push_back(std::make_unique<Queen>(c));
                else if (t == TYPE_ROOK) row.push_back(std::make_unique<Rook>(c));
                else if (t == TYPE_BISHOP) row.push_back(std::make_unique<Bishop>(c));
                else if (t == TYPE_KNIGHT) row.push_back(std::make_unique<Knight>(c));
                else if (t == TYPE_PAWN) row.push_back(std::make_unique<Pawn>(c));
            } else {
                error_ = "ERROR UNKNOWN_TOKEN";
                row.push_back(nullptr); 
            }
        } 
        else {
            error_ = "ERROR UNKNOWN_TOKEN";
            row.push_back(nullptr); 
        }
    }
    state_.push_back(std::move(row));
    rows_ = state_.size();
    if (rows_ > 0) cols_ = state_[0].size();
}

const Piece* Board::getPiece(int r, int c) const {
    if (r < 0 || r >= (int)state_.size() || c < 0 || c >= (int)state_[0].size()) return nullptr;
    return state_[r][c].get();
}

bool Board::isPieceMoving(int r, int c) const {
    for (const auto& move : pendingMoves_) {
        if (move.startRow == r && move.startCol == c) {
            return true;
        }
    }
    return false;
}

bool Board::isOppositeColorMoving(char myColor) const {
    for (const auto& move : pendingMoves_) {
        const Piece* movingPiece = getPiece(move.startRow, move.startCol);
        if (movingPiece && movingPiece->getColor() != myColor) {
            return true; 
        }
    }
    return false;
}

bool Board::isTargetOccupied(int r, int c, char myColor) const {
    const Piece* targetPiece = getPiece(r, c);
    if (targetPiece != nullptr && targetPiece->getColor() == myColor) {
        return true; 
    }

    for (const auto& move : pendingMoves_) {
        if (move.endRow == r && move.endCol == c) return true;
    }
    return false;
}

bool Board::isPieceJumping(int r, int c) const {
    for (const auto& jump : activeJumps_) {
        if (jump.row == r && jump.col == c) return true;
    }
    return false;
}

void Board::click(int x, int y) {
    if (gameOver_) return;

    int r = y / 100, c = x / 100;
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) return;
    
    const Piece* targetPiece = getPiece(r, c);

    if (selectedRow_ == -1) {
        if (targetPiece && !isPieceMoving(r, c) && !isPieceJumping(r, c)) {
            selectedRow_ = r;
            selectedCol_ = c;
        }
    } else {
        const Piece* selectedPiece = getPiece(selectedRow_, selectedCol_);

        if (targetPiece && targetPiece->getColor() == selectedPiece->getColor()) {
            if (!isPieceMoving(r, c) && !isPieceJumping(r, c)) {
                selectedRow_ = r;
                selectedCol_ = c;
            }
            return; 
        }

        if (selectedPiece && selectedPiece->isLegalMove(selectedRow_, selectedCol_, r, c, *this)) {
            if (!isOppositeColorMoving(selectedPiece->getColor()) && !isTargetOccupied(r, c, selectedPiece->getColor())) {
                int distance = std::max(std::abs(r - selectedRow_), std::abs(c - selectedCol_));
                int arrivalTime = gameTime_ + (distance * 1000);
                pendingMoves_.push_back({selectedRow_, selectedCol_, r, c, arrivalTime});
            }
        }
        
        selectedRow_ = -1;
        selectedCol_ = -1;
    }
}

void Board::jump(int x, int y) {
    if (gameOver_) return;

    int r = y / 100, c = x / 100;
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) return;
    
    const Piece* targetPiece = getPiece(r, c);

    if (targetPiece && !isPieceMoving(r, c) && !isPieceJumping(r, c)) {
        activeJumps_.push_back({r, c, gameTime_ + 1000, targetPiece->getColor()});
        
        if (selectedRow_ == r && selectedCol_ == c) {
            selectedRow_ = -1;
            selectedCol_ = -1;
        }
    }
}

void Board::processPendingMoves() {
    for (auto it = pendingMoves_.begin(); it != pendingMoves_.end(); ) {
        if (gameTime_ >= it->arrivalTime) {
            const Piece* movingPiece = getPiece(it->startRow, it->startCol);
            char movingColor = movingPiece ? movingPiece->getColor() : ' ';

            bool airborneKill = false;
            for (const auto& jump : activeJumps_) {
                if (jump.row == it->endRow && jump.col == it->endCol && jump.color != movingColor) {
                    int jumpStartTime = jump.landTime - 1000;
                    if (it->arrivalTime >= jumpStartTime && it->arrivalTime <= jump.landTime) {
                        airborneKill = true;
                        break;
                    }
                }
            }

            if (airborneKill) {
                if (movingPiece && movingPiece->getType() == TYPE_KING) gameOver_ = true; 
                
                state_[it->startRow][it->startCol] = nullptr;
            } else {
                const Piece* target = getPiece(it->endRow, it->endCol);
                if (target && target->getType() == TYPE_KING) gameOver_ = true; 

                state_[it->endRow][it->endCol] = std::move(state_[it->startRow][it->startCol]);
                state_[it->startRow][it->startCol] = nullptr;
                
                Piece* movedPiece = state_[it->endRow][it->endCol].get();
                if (movedPiece && movedPiece->getType() == TYPE_PAWN) {
                    if ((movedPiece->getColor() == COLOR_WHITE && it->endRow == 0) ||
                        (movedPiece->getColor() == COLOR_BLACK && it->endRow == rows_ - 1)) {
                        state_[it->endRow][it->endCol] = std::make_unique<Queen>(movedPiece->getColor());
                    }
                }
            }
            
            it = pendingMoves_.erase(it);
        } else {
            ++it;
        }
    }

    for (auto jt = activeJumps_.begin(); jt != activeJumps_.end(); ) {
        if (gameTime_ >= jt->landTime) {
            jt = activeJumps_.erase(jt);
        } else {
            ++jt;
        }
    }
}
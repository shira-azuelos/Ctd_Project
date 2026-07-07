#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Piece.h"

class Board {
public:
    Board();
    void addRow(const std::string& line);
    bool validate();
    void printCanonical(std::ostream& out) const;
    void click(int x, int y);
    void wait(int ms);
    const Piece* getPiece(int r, int c) const;
    std::string getError() const;
private:
    std::vector<std::vector<std::unique_ptr<Piece>>> state_;
    int rows_ = 0, cols_ = 0;
    int selectedRow_ = -1, selectedCol_ = -1;
    std::string error_;
};
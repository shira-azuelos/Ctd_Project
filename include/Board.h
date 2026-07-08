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
    void jump(int x, int y); 
    void wait(int ms);
    int getGameTime() const { return gameTime_; }
    int getRows() const { return rows_; } 
    const Piece* getPiece(int r, int c) const;
    std::string getError() const;
    void processPendingMoves();
    bool isPieceMoving(int r, int c) const;
    bool isOppositeColorMoving(char myColor) const;
    bool isTargetOccupied(int r, int c, char myColor) const;
    bool isPieceJumping(int r, int c) const;

private:
    std::vector<std::vector<std::unique_ptr<Piece>>> state_;
    int rows_ = 0, cols_ = 0;
    int selectedRow_ = -1, selectedCol_ = -1;
    int gameTime_ = 0;
    std::string error_;
    bool gameOver_ = false; 

    struct PendingMove {
        int startRow, startCol;
        int endRow, endCol;
        int arrivalTime; 
    };
    std::vector<PendingMove> pendingMoves_;

    struct ActiveJump {
        int row, col;
        int landTime;
        char color;
    };
    std::vector<ActiveJump> activeJumps_;
};
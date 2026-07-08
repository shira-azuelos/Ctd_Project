#pragma once
#include <string>
#include <vector>
#include <memory>

constexpr char COLOR_WHITE = 'w';
constexpr char COLOR_BLACK = 'b';
constexpr char TYPE_KING = 'K';
constexpr char TYPE_QUEEN = 'Q';
constexpr char TYPE_ROOK = 'R';
constexpr char TYPE_BISHOP = 'B';
constexpr char TYPE_KNIGHT = 'N';
constexpr char TYPE_PAWN = 'P';
constexpr char EMPTY_CELL_CHAR = '.';

class Board; 

class Piece {
public:
    Piece(char color, char type) : color_(color), type_(type) {}
    virtual ~Piece() = default;
    char getColor() const { return color_; }
    char getType() const { return type_; }
    std::string toString() const { return std::string(1, color_) + type_; }
    virtual bool isLegalMove(int sR, int sC, int eR, int eC, const Board& board) const = 0;
protected:
    char color_, type_;
};

class King : public Piece { public: King(char c) : Piece(c, TYPE_KING) {} bool isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const override; };
class Queen : public Piece { public: Queen(char c) : Piece(c, TYPE_QUEEN) {} bool isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const override; };
class Rook : public Piece { public: Rook(char c) : Piece(c, TYPE_ROOK) {} bool isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const override; };
class Bishop : public Piece { public: Bishop(char c) : Piece(c, TYPE_BISHOP) {} bool isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const override; };
class Knight : public Piece { public: Knight(char c) : Piece(c, TYPE_KNIGHT) {} bool isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const override; };
class Pawn : public Piece { public: Pawn(char c) : Piece(c, TYPE_PAWN) {} bool isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const override; };
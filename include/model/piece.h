#pragma once
#include <string>
#include "position.h"

namespace model {

enum class PieceColor { WHITE, BLACK };
enum class PieceKind { KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN };
enum class PieceState { IDLE, MOVING, CAPTURED }; 

class Piece {
public:
    std::string id;  //?מזהה יחודי     
    PieceColor color;     
    PieceKind kind;       
    Position cell;        
    PieceState state;     

    Piece(std::string id, PieceColor color, PieceKind kind, Position cell);
};

} 
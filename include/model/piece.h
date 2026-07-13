#pragma once
#include <string>
#include <unordered_map>
#include "position.h"

namespace model {

enum class PieceColor { WHITE, BLACK };
enum class PieceKind { KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN };
enum class PieceState { IDLE, MOVING, CAPTURED }; 

inline const std::unordered_map<PieceKind, char> KIND_TO_CHAR = {
    {PieceKind::KING, 'K'},
    {PieceKind::QUEEN, 'Q'},
    {PieceKind::ROOK, 'R'},
    {PieceKind::BISHOP, 'B'},
    {PieceKind::KNIGHT, 'N'},
    {PieceKind::PAWN, 'P'}
};

inline const std::unordered_map<char, PieceKind> CHAR_TO_KIND = {
    {'K', PieceKind::KING},
    {'Q', PieceKind::QUEEN},
    {'R', PieceKind::ROOK},
    {'B', PieceKind::BISHOP},
    {'N', PieceKind::KNIGHT},
    {'P', PieceKind::PAWN}
};

class Piece {
public:
    std::string id;      
    PieceColor color;     
    PieceKind kind;       
    Position cell;        
    PieceState state;     

    Piece(std::string id, PieceColor color, PieceKind kind, Position cell);
};

} 
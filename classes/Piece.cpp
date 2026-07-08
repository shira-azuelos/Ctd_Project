#include "Piece.h"
#include "Board.h"
#include <cmath>
#include <algorithm>

bool King::isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const {
    int rD = std::abs(eR - sR), cD = std::abs(eC - sC);
    return rD <= 1 && cD <= 1 && !(rD == 0 && cD == 0);
}

bool Rook::isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const {
    if (sR != eR && sC != eC) return false;
    int rSt = (eR > sR) ? 1 : (eR < sR ? -1 : 0);
    int cSt = (eC > sC) ? 1 : (eC < sC ? -1 : 0);
    for (int i = 1; i < std::max(std::abs(eR - sR), std::abs(eC - sC)); ++i)
        if (b.getPiece(sR + i * rSt, sC + i * cSt)) return false;
    return true;
}

bool Knight::isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const {
    int rD = std::abs(eR - sR), cD = std::abs(eC - sC);
    return (rD == 2 && cD == 1) || (rD == 1 && cD == 2);
}

bool Bishop::isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const {
    if (std::abs(eR - sR) != std::abs(eC - sC)) return false;
    int rSt = (eR > sR) ? 1 : -1;
    int cSt = (eC > sC) ? 1 : -1;
    for (int i = 1; i < std::abs(eR - sR); ++i)
        if (b.getPiece(sR + i * rSt, sC + i * cSt)) return false;
    return true;
}

bool Queen::isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const {
    return Rook(color_).isLegalMove(sR, sC, eR, eC, b) || Bishop(color_).isLegalMove(sR, sC, eR, eC, b);
}

bool Pawn::isLegalMove(int sR, int sC, int eR, int eC, const Board& b) const {
    int dir = (color_ == COLOR_WHITE) ? -1 : 1;
    return (eC == sC && eR == sR + dir && b.getPiece(eR, eC) == nullptr);
}
#pragma once
#include <memory>
#include "../include/model/board.h"
#include "../include/model/position.h"

namespace rules {

class RuleEngine {
public:
    static bool validate_move(const model::Board& board, const model::Position& src, const model::Position& dest);
};

} 
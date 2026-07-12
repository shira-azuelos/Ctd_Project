#pragma once
#include <string>
#include <vector>
#include <memory>
#include "model/board.h"

namespace io {

class BoardParser {
public:
    static std::shared_ptr<model::Board> parse(const std::vector<std::string>& lines);
};

} 
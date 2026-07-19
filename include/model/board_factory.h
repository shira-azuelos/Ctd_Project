#pragma once
#include <memory>
#include "model/board.h"

namespace model {

class BoardFactory {
public:
    static std::shared_ptr<Board> create_default_board();
};

}

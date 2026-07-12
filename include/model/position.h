#pragma once
#include <string>

namespace model {

class Position {
public:
    int row;
    int col;

    Position(int r, int c);

    bool operator==(const Position& other) const;
    bool operator!=(const Position& other) const;
    
    std::string to_string() const;
};

} 
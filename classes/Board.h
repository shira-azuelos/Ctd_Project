#pragma once

#include <iostream>
#include <string>
#include <vector>

class Board {
public:
    Board();

    void addRow(const std::string& line);
    bool validate();
    void printCanonical(std::ostream& out) const;
    std::string getError() const;
    void click(int x, int y);
    void wait(int ms);

private:
    static bool isValidToken(const std::string& token);
    bool isMoveLegal(int startRow, int startCol, int endRow, int endCol) const;

    std::vector<std::vector<std::string>> state_;
    int rows_;
    int cols_;
    int selectedRow_;
    int selectedCol_;
    int gameTime_;
    std::string error_;
};

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include "Board.h" 

// פונקציית עזר לניקוי רווחים
std::string trim(const std::string& input) {
    size_t first = input.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = input.find_last_not_of(" \t\n\r");
    return input.substr(first, (last - first + 1));
}

int main() {
    Board board;
    std::string line;
    bool inBoard = false, inCommands = false;

    while (std::getline(std::cin, line)) {
        std::string trimmed = trim(line);
        if (trimmed == "Board:") { inBoard = true; inCommands = false; continue; }
        if (trimmed == "Commands:") {
            inBoard = false; inCommands = true;
            if (!board.validate()) { std::cout << board.getError() << '\n'; return 0; }
            continue;
        }

        if (inBoard && !trimmed.empty()) board.addRow(trimmed);
        else if (inCommands) {
            std::istringstream ss(trimmed);
            std::string cmd; ss >> cmd;
            if (cmd == "print") { std::string t; ss >> t; if(t == "board") board.printCanonical(std::cout); }
            else if (cmd == "click") { int x, y; ss >> x >> y; board.click(x, y); }
            else if (cmd == "wait") { int ms; ss >> ms; board.wait(ms); }
        }
    }
    return 0;
}
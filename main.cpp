#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include "classes/Board.h"

namespace {
std::string trim(const std::string& input) {
    std::size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }

    std::size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }

    return input.substr(start, end - start);
}
}

int main() {
    Board board;
    std::string line;
    bool inBoard = false;
    bool inCommands = false;
    bool isBoardValid = false;

    while (std::getline(std::cin, line)) {
        std::string trimmed = trim(line);

        if (trimmed == "Board:") {
            inBoard = true;
            inCommands = false;
            continue;
        }

        if (trimmed == "Commands:") {
            inBoard = false;
            inCommands = true;

            if (!board.validate()) {
                std::cout << board.getError() << '\n';
                return 0;
            }

            isBoardValid = true;
            continue;
        }

        if (inBoard && !trimmed.empty()) {
            board.addRow(trimmed);
            continue;
        }

        if (inCommands && isBoardValid) {
            std::istringstream commandStream(trimmed);
            std::string command;
            commandStream >> command;

            if (command == "print") {
                std::string target;
                commandStream >> target;
                if (target == "board") {
                    board.printCanonical(std::cout);
                }
            } else if (command == "click") {
                int x = 0;
                int y = 0;
                commandStream >> x >> y;
                board.click(x, y);
            } else if (command == "wait") {
                int ms = 0;
                commandStream >> ms;
                board.wait(ms);
            }
        }
    }

    return 0;
}
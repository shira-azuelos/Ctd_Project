#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "include/io/board_parser.h"
#include "include/view/renderer.h"
#include "include/input/controller.h"
#include "include/engine/game_engine.h"

int main() {
    std::vector<std::string> board_lines;
    std::string line;
    bool reading_board = false;
    
    std::shared_ptr<model::Board> board;
    std::shared_ptr<engine::GameEngine> game_engine;
    std::shared_ptr<input::Controller> controller;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        if (line == "Board") {
            reading_board = true;
            board_lines.clear();
            continue;
        }

        if (reading_board) {
            if (line.rfind("click", 0) == 0 || line.rfind("wait", 0) == 0 || line.rfind("print board", 0) == 0) {
                reading_board = false;
                board = io::BoardParser::parse(board_lines);
                game_engine = std::make_shared<engine::GameEngine>(board);
                controller = std::make_shared<input::Controller>(game_engine, board);
            } else {
                board_lines.push_back(line);
                continue;
            }
        }

        if (!reading_board && board) {
            if (line.rfind("click", 0) == 0) {
                int x = 0, y = 0;
                sscanf(line.c_str(), "click %d %d", &x, &y);
                controller->click(x, y);
            } 
            else if (line.rfind("wait", 0) == 0) {
                int ms = 0;
                sscanf(line.c_str(), "wait %d", &ms);
                game_engine->wait(ms);
            } 
            else if (line == "print board") {
                auto output_lines = view::Renderer::render_board(game_engine->get_state());
                for (const auto& out_line : output_lines) {
                    std::cout << out_line << "\n";
                }
            }
        }
    }

    return 0;
}
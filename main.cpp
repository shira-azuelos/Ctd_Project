#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "include/io/board_parser.h"
#include "include/view/renderer.h"
#include "include/input/controller.h"
#include "include/input/board_mapper.h"
#include "include/engine/game_engine.h"

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

int main() {
    std::vector<std::string> board_lines;
    std::string line;
    bool reading_board = false;
    
    std::shared_ptr<model::Board> board;
    std::shared_ptr<engine::GameEngine> game_engine;
    std::shared_ptr<input::Controller> controller;

    while (std::getline(std::cin, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line == "Board" || line == "Board:") {
            reading_board = true;
            board_lines.clear();
            continue;
        }

        if (line == "Commands" || line == "Commands:") {
            reading_board = false;
            try {
                board = io::BoardParser::parse(board_lines);
                game_engine = std::make_shared<engine::GameEngine>(board);
                controller = std::make_shared<input::Controller>(game_engine, board);
            } catch (const std::exception& e) {
                std::cout << e.what() << "\n";
                return 0; 
            }
            continue;
        }

        if (reading_board) {
            board_lines.push_back(line);
            continue;
        }

        if (!reading_board && board) {
            if (line.rfind("click", 0) == 0) {
                int x = 0, y = 0;
                sscanf(line.c_str(), "click %d %d", &x, &y);
                controller->click(x, y);
            } 
            else if (line.rfind("jump", 0) == 0) {
                int x = 0, y = 0;
                sscanf(line.c_str(), "jump %d %d", &x, &y);
                auto cell_opt = input::BoardMapper::pixel_to_cell(x, y, board->get_width(), board->get_height());
                if (cell_opt) {
                    game_engine->request_jump(*cell_opt);
                }
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
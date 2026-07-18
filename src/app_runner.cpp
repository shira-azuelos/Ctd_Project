#include "app_runner.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <io.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "io/board_parser.h"
#include "view/renderer.h"
#include "input/controller.h"
#include "input/board_mapper.h"
#include "engine/game_engine.h"
#include "view/img.h"

namespace app {

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static std::shared_ptr<model::Board> create_default_board() {
    auto board = std::make_shared<model::Board>(8, 8);

    std::vector<model::PieceKind> kinds = {
        model::PieceKind::ROOK, model::PieceKind::KNIGHT, model::PieceKind::BISHOP, model::PieceKind::QUEEN,
        model::PieceKind::KING, model::PieceKind::BISHOP, model::PieceKind::KNIGHT, model::PieceKind::ROOK
    };
    std::vector<std::string> ids = {"R1", "N1", "B1", "Q", "K", "B2", "N2", "R2"};

    for (int col = 0; col < 8; ++col) {
        board->add_piece(std::make_shared<model::Piece>("b" + ids[col], model::PieceColor::BLACK, kinds[col], model::Position(0, col)));
        board->add_piece(std::make_shared<model::Piece>("w" + ids[col], model::PieceColor::WHITE, kinds[col], model::Position(7, col)));
        
        board->add_piece(std::make_shared<model::Piece>("bP" + std::to_string(col), model::PieceColor::BLACK, model::PieceKind::PAWN, model::Position(1, col)));
        board->add_piece(std::make_shared<model::Piece>("wP" + std::to_string(col), model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(6, col)));
    }

    return board;
}

struct GuiState {
    std::shared_ptr<engine::GameEngine> game_engine;
    std::shared_ptr<model::Board> board;
    std::optional<model::Position> selected_cell;
    std::shared_ptr<model::Piece> dragged_piece = nullptr;
    int drag_x = 0;
    int drag_y = 0;
};

static void on_mouse(int event, int x, int y, int flags, void* userdata) {
    auto* g_state = static_cast<GuiState*>(userdata);
    if (g_state->game_engine->get_state()->is_game_over()) {
        g_state->dragged_piece = nullptr;
        return;
    }

    int board_x = x - 100;

    if (event == cv::EVENT_MOUSEMOVE) {
        if (g_state->dragged_piece) {
            g_state->drag_x = x;
            g_state->drag_y = y;
        }
    }
    
    auto cell_opt = input::BoardMapper::pixel_to_cell(board_x, y, g_state->board->get_width(), g_state->board->get_height());
    if (!cell_opt) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
        return;
    }

    model::Position cell = *cell_opt;

    if (event == cv::EVENT_LBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        auto selected_piece = g_state->selected_cell ? g_state->board->get_piece_at(*g_state->selected_cell) : nullptr;

        if (clicked_piece && (!selected_piece || clicked_piece->color == selected_piece->color)) {
            if (!g_state->game_engine->is_piece_cooling_down(clicked_piece)) {
                g_state->selected_cell = cell;
                g_state->dragged_piece = clicked_piece;
                g_state->drag_x = x;
                g_state->drag_y = y;
            }
        } else if (g_state->selected_cell) {
            g_state->game_engine->request_move(*g_state->selected_cell, cell);
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        if (g_state->dragged_piece) {
            auto source_cell = g_state->dragged_piece->cell;
            if (source_cell != cell) {
                g_state->game_engine->request_move(source_cell, cell);
                g_state->selected_cell.reset();
            }
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        if (clicked_piece) {
            g_state->game_engine->request_jump(cell);
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
    }
}

void AppRunner::run_gui_mode() {
    auto board = create_default_board();
    auto game_engine = std::make_shared<engine::GameEngine>(board);
    view::Renderer renderer;

    GuiState gui_state{game_engine, board, std::nullopt};

    cv::namedWindow("KungFu Chess", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("KungFu Chess", on_mouse, &gui_state);

    std::cout << "Starting KungFu Chess GUI. Click on pieces to move them." << std::endl;
    std::cout << "Press ESC on the game window to exit." << std::endl;

    Img canvas;
    while (true) {
        game_engine->wait(30);

        canvas.create(1000, 800, cv::Scalar(15, 15, 15));
        renderer.draw(canvas, game_engine->get_state(), gui_state.selected_cell, game_engine->get_active_motions(), game_engine->get_active_jumps(), &game_engine->get_arbiter(), view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y});

        cv::imshow("KungFu Chess", canvas.get_mat());

        int key = cv::waitKey(30);
        if (key == 27) { // ESC 
            break;
        }
    };
    cv::destroyAllWindows();
}

void AppRunner::run_cli_mode() {
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
                return; 
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
                std::sscanf(line.c_str(), "click %d %d", &x, &y);
                controller->click(x, y);
            } 
            else if (line.rfind("jump", 0) == 0) {
                int x = 0, y = 0;
                std::sscanf(line.c_str(), "jump %d %d", &x, &y);
                auto cell_opt = input::BoardMapper::pixel_to_cell(x, y, board->get_width(), board->get_height());
                if (cell_opt) {
                    game_engine->request_jump(*cell_opt);
                }
            }
            else if (line.rfind("wait", 0) == 0) {
                int ms = 0;
                std::sscanf(line.c_str(), "wait %d", &ms);
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
}

}

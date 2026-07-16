#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <io.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "include/io/board_parser.h"
#include "include/view/renderer.h"
#include "include/input/controller.h"
#include "include/input/board_mapper.h"
#include "include/engine/game_engine.h"
#include "include/view/img.h"

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::shared_ptr<model::Board> create_default_board() {
    auto board = std::make_shared<model::Board>(8, 8);

    board->add_piece(std::make_shared<model::Piece>("bR1", model::PieceColor::BLACK, model::PieceKind::ROOK, model::Position(0, 0)));
    board->add_piece(std::make_shared<model::Piece>("bN1", model::PieceColor::BLACK, model::PieceKind::KNIGHT, model::Position(0, 1)));
    board->add_piece(std::make_shared<model::Piece>("bB1", model::PieceColor::BLACK, model::PieceKind::BISHOP, model::Position(0, 2)));
    board->add_piece(std::make_shared<model::Piece>("bQ", model::PieceColor::BLACK, model::PieceKind::QUEEN, model::Position(0, 3)));
    board->add_piece(std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(0, 4)));
    board->add_piece(std::make_shared<model::Piece>("bB2", model::PieceColor::BLACK, model::PieceKind::BISHOP, model::Position(0, 5)));
    board->add_piece(std::make_shared<model::Piece>("bN2", model::PieceColor::BLACK, model::PieceKind::KNIGHT, model::Position(0, 6)));
    board->add_piece(std::make_shared<model::Piece>("bR2", model::PieceColor::BLACK, model::PieceKind::ROOK, model::Position(0, 7)));
    
    for (int col = 0; col < 8; ++col) {
        board->add_piece(std::make_shared<model::Piece>("bP" + std::to_string(col), model::PieceColor::BLACK, model::PieceKind::PAWN, model::Position(1, col)));
    }

    for (int col = 0; col < 8; ++col) {
        board->add_piece(std::make_shared<model::Piece>("wP" + std::to_string(col), model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(6, col)));
    }

    board->add_piece(std::make_shared<model::Piece>("wR1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(7, 0)));
    board->add_piece(std::make_shared<model::Piece>("wN1", model::PieceColor::WHITE, model::PieceKind::KNIGHT, model::Position(7, 1)));
    board->add_piece(std::make_shared<model::Piece>("wB1", model::PieceColor::WHITE, model::PieceKind::BISHOP, model::Position(7, 2)));
    board->add_piece(std::make_shared<model::Piece>("wQ", model::PieceColor::WHITE, model::PieceKind::QUEEN, model::Position(7, 3)));
    board->add_piece(std::make_shared<model::Piece>("wK", model::PieceColor::WHITE, model::PieceKind::KING, model::Position(7, 4)));
    board->add_piece(std::make_shared<model::Piece>("wB2", model::PieceColor::WHITE, model::PieceKind::BISHOP, model::Position(7, 5)));
    board->add_piece(std::make_shared<model::Piece>("wN2", model::PieceColor::WHITE, model::PieceKind::KNIGHT, model::Position(7, 6)));
    board->add_piece(std::make_shared<model::Piece>("wR2", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(7, 7)));

    return board;
}

struct GuiState {
    std::shared_ptr<engine::GameEngine> game_engine;
    std::shared_ptr<model::Board> board;
    std::optional<model::Position> selected_cell;
    std::optional<model::Position> hovered_cell;
    std::shared_ptr<model::Piece> dragged_piece = nullptr;
    int drag_x = 0;
    int drag_y = 0;
};

void on_mouse(int event, int x, int y, int flags, void* userdata) {
    auto* g_state = static_cast<GuiState*>(userdata);
    if (g_state->game_engine->get_state()->is_game_over()) {
        g_state->hovered_cell.reset();
        g_state->dragged_piece = nullptr;
        return;
    }

    if (event == cv::EVENT_MOUSEMOVE) {
        auto hover_opt = input::BoardMapper::pixel_to_cell(x, y, g_state->board->get_width(), g_state->board->get_height());
        if (hover_opt) {
            g_state->hovered_cell = *hover_opt;
        } else {
            g_state->hovered_cell.reset();
        }
        
        if (g_state->dragged_piece) {
            g_state->drag_x = x;
            g_state->drag_y = y;
        }
    }
    
    auto cell_opt = input::BoardMapper::pixel_to_cell(x, y, g_state->board->get_width(), g_state->board->get_height());
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

        if (!g_state->selected_cell) {
            if (clicked_piece && !g_state->game_engine->is_piece_cooling_down(clicked_piece)) {
                g_state->selected_cell = cell;
                g_state->dragged_piece = clicked_piece;
                g_state->drag_x = x;
                g_state->drag_y = y;
                std::cout << "[UI] Selected piece & started drag: " << clicked_piece->id << " at (" << cell.row << ", " << cell.col << ")" << std::endl;
            }
        } else {
            auto selected_piece = g_state->board->get_piece_at(*g_state->selected_cell);
            if (clicked_piece && selected_piece && clicked_piece->color == selected_piece->color) {
                if (!g_state->game_engine->is_piece_cooling_down(clicked_piece)) {
                    g_state->selected_cell = cell;
                    g_state->dragged_piece = clicked_piece;
                    g_state->drag_x = x;
                    g_state->drag_y = y;
                    std::cout << "[UI] Changed selection & started drag: " << clicked_piece->id << " at (" << cell.row << ", " << cell.col << ")" << std::endl;
                }
            } else {
                std::cout << "[UI] Clicked destination: Requesting move from (" << g_state->selected_cell->row << ", " << g_state->selected_cell->col << ") to (" << cell.row << ", " << cell.col << ")" << std::endl;
                g_state->game_engine->request_move(*g_state->selected_cell, cell);
                g_state->selected_cell.reset();
                g_state->dragged_piece = nullptr;
            }
        }
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        if (g_state->dragged_piece) {
            auto source_cell = g_state->dragged_piece->cell;
            if (source_cell != cell) {
                std::cout << "[UI] Drag-dropped: Requesting move from (" << source_cell.row << ", " << source_cell.col << ") to (" << cell.row << ", " << cell.col << ")" << std::endl;
                g_state->game_engine->request_move(source_cell, cell);
                g_state->selected_cell.reset();
            }
            g_state->dragged_piece = nullptr;
        }
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        auto clicked_piece = g_state->board->get_piece_at(cell);
        if (clicked_piece) {
            std::cout << "[UI] Right-clicked: Requesting jump for " << clicked_piece->id << " at (" << cell.row << ", " << cell.col << ")" << std::endl;
            g_state->game_engine->request_jump(cell);
            g_state->selected_cell.reset();
            g_state->dragged_piece = nullptr;
        }
    }
}

void run_gui_mode() {
    auto board = create_default_board();
    auto game_engine = std::make_shared<engine::GameEngine>(board);
    view::Renderer renderer;

    GuiState gui_state{game_engine, board, std::nullopt};

    cv::namedWindow("KungFu Chess", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("KungFu Chess", on_mouse, &gui_state);

    std::cout << "Starting KungFu Chess GUI. Click on pieces to move them." << std::endl;
    std::cout << "Press ESC on the game window to exit." << std::endl;

    while (true) {
        game_engine->wait(30);

        Img canvas;
        canvas.read("assets/board.png", {800, 800}, false);
        renderer.draw(canvas, game_engine->get_state(), gui_state.selected_cell, gui_state.hovered_cell, game_engine->get_active_motions(), game_engine->get_active_jumps(), &game_engine->get_arbiter(), view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y});

        cv::imshow("KungFu Chess", canvas.get_mat());

        int key = cv::waitKey(30);
        if (key == 27) { // ESC key
            break;
        }
    }
    cv::destroyAllWindows();
}

int main() {
    if (_isatty(0)) {
        try {
            run_gui_mode();
        } catch (const std::exception& e) {
            std::cerr << "GUI Error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }

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
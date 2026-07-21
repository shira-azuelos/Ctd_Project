#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <opencv2/opencv.hpp>
#include "model/board_factory.h"
#include "input/gui_controller.h"
#include "engine/game_engine.h"
#include "view/renderer.h"
#include "pubsub/message_bus.h"
#include "network/socket_server.h"
#include "network/socket_client.h"
#include "view/img.h"
#include "io/sound_player.h"

namespace {
constexpr int DEFAULT_PORT = 8080;
constexpr int FRAME_TIME_MS = 16;
constexpr int CANVAS_WIDTH = 1000;
constexpr int CANVAS_HEIGHT = 800;
constexpr int KEY_ESC = 27;
constexpr int KEY_SPACE = ' ';
constexpr int KEY_ENTER_CR = 13;
constexpr int KEY_ENTER_LF = 10;
}

int main(int argc, char* argv[]) {
    std::string mode = "local";
    std::string ip = "127.0.0.1";
    int port = DEFAULT_PORT;

    if (argc > 1) {
        mode = argv[1];
    }
    if (argc > 2) {
        ip = argv[2];
    }

    if (mode == "server") {
        network::SocketServer server;
        if (!server.start(port)) {
            std::cerr << "Failed to start server on port " << port << std::endl;
            return 1;
        }
        std::cout << "Server is running. Press Enter to exit." << std::endl;
        std::cin.get();
        server.stop();
        return 0;
    }
    else if (mode == "client") {
        try {
            std::cout << "==========================================" << std::endl;
            std::cout << "   KungFu Chess Client Login (CLI)       " << std::endl;
            std::cout << "==========================================" << std::endl;
            std::string username = (argc > 3) ? argv[3] : "";
            std::string password = (argc > 4) ? argv[4] : "";
            
            if (username.empty()) {
                std::cout << "Enter Username: ";
                std::cin >> username;
            }
            if (password.empty()) {
                std::cout << "Enter Password: ";
                std::cin >> password;
            }

            auto client = std::make_shared<network::SocketClient>(ip, port);
            if (!client->connect_to_server()) {
                std::cerr << "Failed to connect to server at " << ip << ":" << port << std::endl;
                return 1;
            }

            client->send_login(username, password);

            int wait_count = 0;
            while (!client->is_logged_in() && wait_count < 20) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                wait_count++;
            }

            view::Renderer renderer;
            input::GuiState gui_state;
            gui_state.socket_client = client;
            gui_state.board = client->get_board();

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::SCORE_CHANGED, [](const pubsub::Event& ev) {});

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::PLAY_SOUND, [](const pubsub::Event& ev) {
                auto payload = std::any_cast<pubsub::SoundPayload>(ev.payload);
                io::SoundPlayer::play(payload.sound_name);
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::MOVE_LOGGED, [](const pubsub::Event& ev) {});

            cv::namedWindow("KungFu Chess", cv::WINDOW_AUTOSIZE);
            cv::setMouseCallback("KungFu Chess", input::GuiController::on_mouse, &gui_state);

            Img canvas;
            gui_state.in_opening_screen = true;
            auto search_start = std::chrono::steady_clock::now();
            network::MatchState last_state = network::MatchState::IDLE;

            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_TIME_MS));
                client->advance_animations(FRAME_TIME_MS);
                gui_state.board = client->get_board();

                auto match_st = client->get_match_state();
                if (match_st == network::MatchState::MATCHED) {
                    gui_state.in_opening_screen = false;
                }

                if (match_st == network::MatchState::SEARCHING && last_state != network::MatchState::SEARCHING) {
                    search_start = std::chrono::steady_clock::now();
                }
                last_state = match_st;

                int search_elapsed_sec = 0;
                if (match_st == network::MatchState::SEARCHING) {
                    auto now = std::chrono::steady_clock::now();
                    search_elapsed_sec = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - search_start).count());
                }

                canvas.create(CANVAS_WIDTH, CANVAS_HEIGHT, cv::Scalar(15, 15, 15));

                if (gui_state.in_opening_screen) {
                    renderer.draw_opening(canvas, client->get_username(), client->get_elo(), 
                                          (match_st == network::MatchState::SEARCHING), search_elapsed_sec, 
                                          client->show_popup(), client->get_popup_msg(),
                                          gui_state.show_room_dialog, gui_state.room_input_text);
                } else {
                    renderer.draw(canvas, client->get_game_state(), gui_state.selected_cell, 
                                  client->get_active_motions(), client->get_active_jumps(), 
                                  client->get_arbiter(), 
                                  view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y},
                                  client->get_white_username(), client->get_white_elo(),
                                  client->get_black_username(), client->get_black_elo(),
                                  client->get_room_id(), client->get_room_name(), client->is_viewer());
                }

                cv::imshow("KungFu Chess", canvas.get_mat());

                int key = cv::waitKey(FRAME_TIME_MS);
                if (key == KEY_ESC) {
                    break;
                }

                if (key != -1) {
                    if (input::GuiController::on_key(key, &gui_state)) {
                        continue;
                    }
                }

                if (gui_state.in_opening_screen && (key == KEY_SPACE || key == KEY_ENTER_CR || key == KEY_ENTER_LF)) {
                    if (client->show_popup()) {
                        client->dismiss_popup();
                    } else if (match_st == network::MatchState::IDLE || match_st == network::MatchState::TIMEOUT) {
                        client->send_find_match();
                    } else if (match_st == network::MatchState::SEARCHING) {
                        client->send_cancel_match();
                    }
                }
            }
            client->disconnect();
            cv::destroyAllWindows();
        } catch (const std::exception& e) {
            std::cerr << "Client Fatal Error: " << e.what() << std::endl;
            return 1;
        }
    }
    else {
        try {
            auto board = model::BoardFactory::create_default_board();
            auto game_engine = std::make_shared<engine::GameEngine>(board);
            view::Renderer renderer;

            input::GuiState gui_state{game_engine, nullptr, board, std::nullopt};

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::SCORE_CHANGED, [](const pubsub::Event& ev) {});

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::PLAY_SOUND, [](const pubsub::Event& ev) {
                auto payload = std::any_cast<pubsub::SoundPayload>(ev.payload);
                io::SoundPlayer::play(payload.sound_name);
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::MOVE_LOGGED, [](const pubsub::Event& ev) {});

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::GAME_STATUS, [](const pubsub::Event& ev) {});

            cv::namedWindow("KungFu Chess", cv::WINDOW_AUTOSIZE);
            cv::setMouseCallback("KungFu Chess", input::GuiController::on_mouse, &gui_state);

            std::cout << "Starting Offline KungFu Chess GUI. Click on pieces to move them." << std::endl;
            std::cout << "Press ESC on the game window to exit." << std::endl;

            Img canvas;
            gui_state.in_opening_screen = true;

            while (true) {
                game_engine->update(FRAME_TIME_MS);

                canvas.create(CANVAS_WIDTH, CANVAS_HEIGHT, cv::Scalar(15, 15, 15));

                if (gui_state.in_opening_screen) {
                    renderer.draw_opening(canvas, "PLAYER", 1200, false);
                } else {
                    renderer.draw(canvas, game_engine->get_state(), gui_state.selected_cell, 
                                  game_engine->get_active_motions(), game_engine->get_active_jumps(), 
                                  &game_engine->get_arbiter(), 
                                  view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y});
                }

                cv::imshow("KungFu Chess", canvas.get_mat());

                int key = cv::waitKey(FRAME_TIME_MS);
                if (key == KEY_ESC) {
                    break;
                }
                if (gui_state.in_opening_screen && (key == KEY_SPACE || key == KEY_ENTER_CR || key == KEY_ENTER_LF)) {
                    gui_state.in_opening_screen = false;
                }
            }
            cv::destroyAllWindows();
        } catch (const std::exception& e) {
            std::cerr << "Local Fatal Error: " << e.what() << std::endl;
            return 1;
        }
    }
    return 0;
}
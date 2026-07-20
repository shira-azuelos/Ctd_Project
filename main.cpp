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

namespace {
constexpr int DEFAULT_PORT = 8080;
constexpr int FRAME_TIME_MS = 30;
constexpr int CANVAS_WIDTH = 1000;
constexpr int CANVAS_HEIGHT = 800;
constexpr int KEY_ESC = 27;
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
            auto client = std::make_shared<network::SocketClient>(ip, port);
            if (!client->connect_to_server()) {
                std::cerr << "Failed to connect to server at " << ip << ":" << port << std::endl;
                return 1;
            }

            view::Renderer renderer;
            input::GuiState gui_state;
            gui_state.socket_client = client;
            gui_state.board = client->get_board();

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::SCORE_CHANGED, [](const pubsub::Event& ev) {
                auto payload = std::any_cast<pubsub::ScorePayload>(ev.payload);
                std::cout << "[Score HUD] " << payload.color << " score updated to " << payload.new_score << std::endl;
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::PLAY_SOUND, [](const pubsub::Event& ev) {
                auto payload = std::any_cast<pubsub::SoundPayload>(ev.payload);
                std::cout << "[Sound Engine] Playing sound: " << payload.sound_name << std::endl;
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::MOVE_LOGGED, [](const pubsub::Event& ev) {
                auto log_msg = std::any_cast<std::string>(ev.payload);
                std::cout << "[Move Log] " << log_msg << std::endl;
            });

            cv::namedWindow("KungFu Chess", cv::WINDOW_AUTOSIZE);
            cv::setMouseCallback("KungFu Chess", input::GuiController::on_mouse, &gui_state);

            std::cout << "Starting KungFu Chess GUI Client. Click on pieces to move them." << std::endl;
            std::cout << "Press ESC on the game window to exit." << std::endl;

            Img canvas;
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_TIME_MS));
                client->advance_animations(FRAME_TIME_MS);
                gui_state.board = client->get_board();

                canvas.create(CANVAS_WIDTH, CANVAS_HEIGHT, cv::Scalar(15, 15, 15));
                renderer.draw(canvas, client->get_game_state(), gui_state.selected_cell, 
                              client->get_active_motions(), client->get_active_jumps(), 
                              client->get_arbiter(), 
                              view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y});

                cv::imshow("KungFu Chess", canvas.get_mat());

                int key = cv::waitKey(FRAME_TIME_MS);
                if (key == KEY_ESC) {
                    break;
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

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::SCORE_CHANGED, [](const pubsub::Event& ev) {
                auto payload = std::any_cast<pubsub::ScorePayload>(ev.payload);
                std::cout << "[Score HUD] " << payload.color << " score updated to " << payload.new_score << std::endl;
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::PLAY_SOUND, [](const pubsub::Event& ev) {
                auto payload = std::any_cast<pubsub::SoundPayload>(ev.payload);
                std::cout << "[Sound Engine] Playing sound: " << payload.sound_name << std::endl;
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::MOVE_LOGGED, [](const pubsub::Event& ev) {
                auto log_msg = std::any_cast<std::string>(ev.payload);
                std::cout << "[Move Log] " << log_msg << std::endl;
            });

            pubsub::MessageBus::get_instance().subscribe(pubsub::EventType::GAME_STATUS, [](const pubsub::Event& ev) {
                auto status = std::any_cast<std::string>(ev.payload);
                std::cout << "[Game Status] " << status << std::endl;
            });

            cv::namedWindow("KungFu Chess", cv::WINDOW_AUTOSIZE);
            cv::setMouseCallback("KungFu Chess", input::GuiController::on_mouse, &gui_state);

            std::cout << "Starting Offline KungFu Chess GUI. Click on pieces to move them." << std::endl;
            std::cout << "Press ESC on the game window to exit." << std::endl;

            Img canvas;
            while (true) {
                game_engine->wait(FRAME_TIME_MS);

                canvas.create(CANVAS_WIDTH, CANVAS_HEIGHT, cv::Scalar(15, 15, 15));
                renderer.draw(canvas, game_engine->get_state(), gui_state.selected_cell, 
                              game_engine->get_active_motions(), game_engine->get_active_jumps(), 
                              &game_engine->get_arbiter(), 
                              view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y});

                cv::imshow("KungFu Chess", canvas.get_mat());

                int key = cv::waitKey(FRAME_TIME_MS);
                if (key == KEY_ESC) {
                    break;
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
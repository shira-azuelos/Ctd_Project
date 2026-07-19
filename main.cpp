#include <iostream>
#include <memory>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "model/board_factory.h"
#include "input/gui_controller.h"
#include "engine/game_engine.h"
#include "view/renderer.h"
#include "pubsub/message_bus.h"
#include "view/img.h"

int main() {
    try {
        auto board = model::BoardFactory::create_default_board();
        auto game_engine = std::make_shared<engine::GameEngine>(board);
        view::Renderer renderer;

        input::GuiState gui_state{game_engine, board, std::nullopt};

        // Register event subscriptions
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

        std::cout << "Starting KungFu Chess GUI. Click on pieces to move them." << std::endl;
        std::cout << "Press ESC on the game window to exit." << std::endl;

        Img canvas;
        while (true) {
            game_engine->wait(30);

            canvas.create(1000, 800, cv::Scalar(15, 15, 15));
            renderer.draw(canvas, game_engine->get_state(), gui_state.selected_cell, 
                          game_engine->get_active_motions(), game_engine->get_active_jumps(), 
                          &game_engine->get_arbiter(), 
                          view::DragInfo{gui_state.dragged_piece, gui_state.drag_x, gui_state.drag_y});

            cv::imshow("KungFu Chess", canvas.get_mat());

            int key = cv::waitKey(30);
            if (key == 27) { // ESC key
                break;
            }
        }
        cv::destroyAllWindows();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
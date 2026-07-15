#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "Preparing assets in C++..." << std::endl;
        
        cv::Mat board(800, 800, CV_8UC3);
        cv::Vec3b color_light(255, 255, 255); // White (BGR)
        cv::Vec3b color_dark(0, 0, 0);       // Black (BGR)
        
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                cv::Vec3b color = ((r + c) % 2 == 0) ? color_light : color_dark;
                cv::Mat cell = board(cv::Rect(c * 100, r * 100, 100, 100));
                cell.setTo(color);
            }
        }
        
        cv::imwrite("assets/board.png", board);
        std::cout << "Generated perfect 800x800 chessboard image." << std::endl;
                       
        std::cout << "Assets prepared successfully (board only)!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

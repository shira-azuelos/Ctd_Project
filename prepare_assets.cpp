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
                       
        std::string pieces_dir = "assets/pieces";
        if (!fs::exists(pieces_dir)) {
            std::cerr << "Error: assets/pieces directory not found!" << std::endl;
            return 1;
        }
        
        for (const auto& entry : fs::directory_iterator(pieces_dir)) {
            if (!entry.is_directory()) continue;
            
            std::string folder = entry.path().filename().string();
            std::string sprite_path = pieces_dir + "/" + folder + "/states/idle/sprites/1.png";
            if (!fs::exists(sprite_path)) continue;
            
            cv::Mat sprite = cv::imread(sprite_path, cv::IMREAD_UNCHANGED);
            if (sprite.empty() || sprite.channels() != 4) {
                std::cout << "Skipping " << sprite_path << std::endl;
                continue;
            }
            
            cv::resize(sprite, sprite, cv::Size(100, 100), 0, 0, cv::INTER_AREA);
            
            std::vector<cv::Mat> src_channels;
            cv::split(sprite, src_channels);
            
            cv::Mat alpha;
            src_channels[3].convertTo(alpha, CV_32F, 1.0 / 255.0);
            
            cv::Mat blended_light(100, 100, CV_8UC3, cv::Scalar(color_light[0], color_light[1], color_light[2]));
            cv::Mat blended_dark(100, 100, CV_8UC3, cv::Scalar(color_dark[0], color_dark[1], color_dark[2]));
            
            std::vector<cv::Mat> dst_light_channels;
            cv::split(blended_light, dst_light_channels);
            
            std::vector<cv::Mat> dst_dark_channels;
            cv::split(blended_dark, dst_dark_channels);
            
            for (int c = 0; c < 3; ++c) {
                cv::Mat src_c, dst_light_c, dst_dark_c;
                src_channels[c].convertTo(src_c, CV_32F);
                dst_light_channels[c].convertTo(dst_light_c, CV_32F);
                dst_dark_channels[c].convertTo(dst_dark_c, CV_32F);
                
                cv::Mat blend_light_c = dst_light_c.mul(1.0 - alpha) + src_c.mul(alpha);
                cv::Mat blend_dark_c = dst_dark_c.mul(1.0 - alpha) + src_c.mul(alpha);
                
                blend_light_c.convertTo(dst_light_channels[c], CV_8U);
                blend_dark_c.convertTo(dst_dark_channels[c], CV_8U);
            }
            
            cv::merge(dst_light_channels, blended_light);
            cv::merge(dst_dark_channels, blended_dark);
            
            cv::imwrite(pieces_dir + "/" + folder + "/states/idle/sprites/1_light.png", blended_light);
            cv::imwrite(pieces_dir + "/" + folder + "/states/idle/sprites/1_dark.png", blended_dark);
            
            cv::Mat selected_light = blended_light.clone();
            cv::Mat selected_dark = blended_dark.clone();
            cv::rectangle(selected_light, cv::Point(0,0), cv::Point(99,99), cv::Scalar(0, 255, 255), 4); 
            cv::rectangle(selected_dark, cv::Point(0,0), cv::Point(99,99), cv::Scalar(0, 255, 255), 4); 
            cv::imwrite(pieces_dir + "/" + folder + "/states/idle/sprites/1_light_selected.png", selected_light);
            cv::imwrite(pieces_dir + "/" + folder + "/states/idle/sprites/1_dark_selected.png", selected_dark);
            
            cv::Mat attack_light = blended_light.clone();
            cv::Mat attack_dark = blended_dark.clone();
            cv::rectangle(attack_light, cv::Point(0,0), cv::Point(99,99), cv::Scalar(0, 0, 255), 4); 
            cv::rectangle(attack_dark, cv::Point(0,0), cv::Point(99,99), cv::Scalar(0, 0, 255), 4);  
            cv::imwrite(pieces_dir + "/" + folder + "/states/idle/sprites/1_light_attack.png", attack_light);
            cv::imwrite(pieces_dir + "/" + folder + "/states/idle/sprites/1_dark_attack.png", attack_dark);
            
            std::cout << "Generated blended sprites for " << folder << std::endl;
        }
        
        std::cout << "Assets prepared successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

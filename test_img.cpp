#include "img.h"
#include <iostream>

int main() {
    try {
        std::cout << "Testing Img class..." << std::endl;
        
        Img img;
        img.read("assets/board.png", {740, 580}, true);
        img.put_text("Hello!", 150, 360, 1.0, {0,0,0});
        img.show();
        
        std::cout << "Img class test completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 
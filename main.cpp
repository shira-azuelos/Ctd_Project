#include "app_runner.h"
#include <io.h>
#include <iostream>
#include <stdexcept>

int main() {
    if (_isatty(0)) {
        try {
            app::AppRunner::run_gui_mode();
        } catch (const std::exception& e) {
            std::cerr << "GUI Error: " << e.what() << std::endl;
            return 1;
        }
    } else {
        try {
            app::AppRunner::run_cli_mode();
        } catch (const std::exception& e) {
            std::cerr << "CLI Error: " << e.what() << std::endl;
            return 1;
        }
    }
    return 0;
}
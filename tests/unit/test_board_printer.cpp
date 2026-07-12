#include "io/board_printer.h"
#include "io/board_parser.h"
#include "model/game_state.h"
#include <vector>
#include <string>
#include "../../doctest.h"

TEST_CASE("BoardPrinter: Performs round-trip for a simple board") {
    // 1. הגדרת טקסט מקורי
    std::vector<std::string> original_lines = {
        "wK .",
        ". bR"
    };
    
    // 2. יצירת הלוח הלוגי ומצב המשחק בעזרת ה-Parser
    auto board = io::BoardParser::parse(original_lines);
    auto state = std::make_shared<model::GameState>(board);
    
    // 3. הדפסה חזרה לטקסט בעזרת ה-Printer
    auto printed_lines = io::BoardPrinter::print(state);
    
    // 4. השוואה (Round-Trip)
    REQUIRE(printed_lines.size() == 2);
    CHECK(printed_lines[0] == original_lines[0]);
    CHECK(printed_lines[1] == original_lines[1]);
}
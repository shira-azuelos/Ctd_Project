#include "io/board_printer.h"
#include "io/board_parser.h"
#include "model/game_state.h"
#include <vector>
#include <string>
#include "../../doctest.h"

TEST_CASE("BoardPrinter: Performs round-trip for a simple board") {
    std::vector<std::string> original_lines = {
        "wK .",
        ". bR"
    };
    auto board = io::BoardParser::parse(original_lines);
    auto state = std::make_shared<model::GameState>(board);
    
    auto printed_lines = io::BoardPrinter::print(state);
    
    REQUIRE(printed_lines.size() == 2);
    CHECK(printed_lines[0] == original_lines[0]);
    CHECK(printed_lines[1] == original_lines[1]);
}
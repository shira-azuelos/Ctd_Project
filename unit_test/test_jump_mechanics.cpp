//בודק את יכולות ההתחמקות והמלכודת באוויר

#include "../doctest.h"
#include "../include/Board.h"
#include <sstream>

TEST_CASE("שלב 11: מלכודת אווירית - תוקף מושמד באוויר") {
    Board board;
    board.addRow("wR . .");
    board.addRow(". . .");
    board.addRow("bR . .");
    board.validate();

    board.click(50, 50);
    board.click(50, 250);
    
    board.wait(1500); 

    board.jump(50, 250); 
    
    board.wait(1000); 

    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == ". . .\n. . .\nbR . .\n");
}

TEST_CASE("שלב 11: קפיצה מוקדמת מדי - התוקף דורס") {
    Board board;
    board.addRow("wR . .");
    board.addRow(". . .");
    board.addRow("bR . .");
    board.validate();

    board.click(50, 50);
    board.click(50, 250);
    
    board.jump(50, 250); 
    
    board.wait(2500); 

    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == ". . .\n. . .\nwR . .\n");
}
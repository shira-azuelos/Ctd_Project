//לוגיקת התנועה הגיאומטרית הכללית (צריח, פרש וכדומה)

#include "../doctest.h"
#include "../include/Board.h"
#include <sstream>

TEST_CASE("שלב 3: בדיקת תנועה חוקית ולא חוקית") {
    Board board;
    board.addRow("wR . ."); 
    board.addRow(". . .");
    board.addRow(". . .");  
    board.validate();

    board.click(50, 50);   
    board.click(150, 150); 
    board.wait(10000);     
    
    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == "wR . .\n. . .\n. . .\n");
}

TEST_CASE("שלב 4: חסימות ולכידה") {
    Board board;
    board.addRow("wR wP ."); 
    board.addRow(". . .");
    board.addRow("bP . .");  
    board.validate();

    board.click(50, 50);   
    board.click(250, 50);  
    board.wait(10000); 
    std::stringstream ss1;
    board.printCanonical(ss1);
    CHECK(ss1.str() == "wR wP .\n. . .\nbP . .\n");

    board.click(50, 50);   
    board.click(50, 250);  
    board.wait(10000); 
    
    std::stringstream ss2;
    board.printCanonical(ss2);
    CHECK(ss2.str() == ". wP .\n. . .\nwR . .\n"); 
}

TEST_CASE("שלב 4: פרש מדלג מעל כלים") {
    Board board;
    board.addRow("wN wP .");
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);   
    board.click(150, 250); 
    board.wait(10000); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == ". wP .\n. . .\n. wN .\n");
}
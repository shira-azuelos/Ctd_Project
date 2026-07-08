//חוקי החייל

#include "../doctest.h"
#include "../include/Board.h"
#include <sstream>

TEST_CASE("שלב 5: תנועת חיילים קדימה (לבן למעלה, שחור למטה)") {
    Board board;
    board.addRow(". bP ."); 
    board.addRow(". . .");  
    board.addRow("wP . ."); 
    board.validate();

    board.click(150, 50);  
    board.click(150, 150); 
    board.wait(1000); 

    board.click(50, 250);  
    board.click(50, 150);  
    board.wait(1000); 

    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == ". . .\nwP bP .\n. . .\n");
}

TEST_CASE("שלב 5: לכידה באלכסון וחסימה קדימה") {
    Board board;
    board.addRow(". . .");
    board.addRow("bP bP ."); 
    board.addRow("wP . .");  
    board.validate();

    board.click(50, 250);  
    board.click(50, 150);  
    board.wait(10000);
    std::stringstream ss1;
    board.printCanonical(ss1);
    CHECK(ss1.str() == ". . .\nbP bP .\nwP . .\n"); 

    board.click(50, 250);  
    board.click(150, 150); 
    board.wait(10000); 
    std::stringstream ss2;
    board.printCanonical(ss2);
    CHECK(ss2.str() == ". . .\nbP wP .\n. . .\n"); 
}

TEST_CASE("שלב 5: חייל לא יכול לזוז שתי משבצות או לזוז אחורה") {
    Board board;
    board.addRow(". . ."); 
    board.addRow(". . ."); 
    board.addRow("wP . ."); 
    board.addRow(". . ."); 
    board.addRow(". . ."); 
    board.validate();

    board.click(50, 250);  
    board.click(50, 50);   
    board.wait(10000);
    std::stringstream ss1;
    board.printCanonical(ss1);
    CHECK(ss1.str() == ". . .\n. . .\nwP . .\n. . .\n. . .\n"); 

    board.click(50, 250);  
    board.click(50, 350);  
    board.wait(10000);
    std::stringstream ss2;
    board.printCanonical(ss2);
    CHECK(ss2.str() == ". . .\n. . .\nwP . .\n. . .\n. . .\n"); 
}

TEST_CASE("שלב 10: תנועה כפולה של חייל בתור הראשון") {
    Board board;
    board.addRow("bP . ."); 
    board.addRow(". . .");  
    board.addRow(". . .");  
    board.addRow(". . .");  
    board.addRow(". wP ."); 
    board.validate();

    board.click(150, 450); 
    board.click(150, 250); 
    board.wait(2000);

    board.click(50, 50);  
    board.click(50, 250); 
    board.wait(2000);

    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == ". . .\n. . .\nbP wP .\n. . .\n. . .\n");
}

TEST_CASE("שלב 10: הכתרת חייל למלכה בסוף הלוח") {
    Board board;
    board.addRow(". . .");   
    board.addRow("wP . .");  
    board.addRow("bP . .");  
    board.addRow(". . .");   
    board.validate();

    board.click(50, 150);
    board.click(50, 50);
    board.wait(1000);

    board.click(50, 250);
    board.click(50, 350);
    board.wait(1000);

    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == "wQ . .\n. . .\n. . .\nbQ . .\n");
}
//בודקים את תקינות הלוח, מנגנון הזמן, חסימות תנועה מקבילות, התנגשויות וסיום משחק

#include "../doctest.h"
#include "../include/Board.h"
#include <sstream>

TEST_CASE("שלב 1: בדיקת תקינות לוח בסיסית") {
    Board board;
    board.addRow("wK wR .");
    board.addRow(". . .");
    board.addRow(". . .");
    CHECK(board.validate() == true); 
}

TEST_CASE("שלב 2: בדיקת לחיצה והמתנה") {
    Board board;
    board.addRow("wK . .");
    board.validate();
    board.click(50, 50); 
    board.wait(100);     
    CHECK(true);         
}

TEST_CASE("שלב 6: מהלכים מבוססי זמן (שנייה אחת לכל משבצת)") {
    Board board;
    board.addRow("bP . .");
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);  
    board.click(50, 150); 

    std::stringstream ss1;
    board.printCanonical(ss1);
    CHECK(ss1.str() == "bP . .\n. . .\n. . .\n"); 

    board.wait(500); 
    std::stringstream ss2;
    board.printCanonical(ss2);
    CHECK(ss2.str() == "bP . .\n. . .\n. . .\n"); 

    board.wait(500); 
    std::stringstream ss3;
    board.printCanonical(ss3);
    CHECK(ss3.str() == ". . .\nbP . .\n. . .\n"); 
}

TEST_CASE("שלב 7: התעלמות משינוי מסלול תוך כדי תנועה (No Redirect)") {
    Board board;
    board.addRow("wR . ."); 
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);   
    board.click(250, 50);  
    
    board.wait(1000); 
    
    board.click(50, 50);
    board.click(150, 50);
    
    board.wait(1000); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == ". . wR\n. . .\n. . .\n");
}

TEST_CASE("שלב 7: תנועה מיידית לאחר הגעה ללא זמן קירור (No Cooldown)") {
    Board board;
    board.addRow("wR . .");
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);
    board.click(150, 50);
    board.wait(1000); 
    
    board.click(150, 50);
    board.click(250, 50);
    board.wait(1000); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == ". . wR\n. . .\n. . .\n");
}

TEST_CASE("שלב 7: מניעת תנועה מקבילה של צבעים מנוגדים") {
    Board board;
    board.addRow("wR . ."); 
    board.addRow(". . .");
    board.addRow("bR . ."); 
    board.validate();

    board.click(50, 50);
    board.click(250, 50);
    
    board.click(50, 250);
    board.click(250, 250);
    
    board.wait(2000); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == ". . wR\n. . .\nbR . .\n");
}

TEST_CASE("שלב 8: מניעת התנגשות - שני כלים לאותה משבצת") {
    Board board;
    board.addRow("wR . .");
    board.addRow(". . .");
    board.addRow(". . wR");
    board.validate();

    board.click(50, 50);
    board.click(250, 50);
    
    board.click(250, 250);
    board.click(250, 50);
    
    board.wait(2000); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == ". . wR\n. . .\n. . wR\n");
}

TEST_CASE("שלב 8: מניעת נחיתה על כלי ידידותי") {
    Board board;
    board.addRow("wK wR .");
    board.addRow(". . .");
    board.validate();

    board.click(150, 50);
    board.click(50, 50);
    
    board.wait(1000);
    
    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == "wK wR .\n. . .\n");
}

TEST_CASE("שלב 8: מניעת תנועה דרך חסימה") {
    Board board;
    board.addRow("wR wP ."); 
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);
    board.click(250, 50);
    
    board.wait(2000);
    
    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == "wR wP .\n. . .\n");
}

TEST_CASE("שלב 9: לכידת מלך מסיימת את המשחק ומפסיקה אינטראקציה") {
    Board board;
    board.addRow("wQ . .");
    board.addRow(". bK .");
    board.validate();

    board.click(50, 50);  
    board.click(150, 150); 
    board.wait(1000);      

    board.click(150, 150);
    board.click(250, 150); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == ". . .\n. wQ .\n"); 
}

TEST_CASE("שלב 9: מניעת בחירת כלים לאחר סיום משחק") {
    Board board;
    board.addRow("wQ . .");
    board.addRow(". bK .");
    board.validate();

    board.click(50, 50);
    board.click(150, 150);
    board.wait(1000);

    board.click(150, 150); 
    
    board.click(250, 150); 
    
    std::stringstream ss;
    board.printCanonical(ss);
    
    CHECK(ss.str() == ". . .\n. wQ .\n"); 
}
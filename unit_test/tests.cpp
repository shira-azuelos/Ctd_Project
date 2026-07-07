#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "Board.h"
#include <sstream>

TEST_CASE("שלב א': בדיקת תקינות לוח בסיסית") {
    Board board;
    board.addRow("wK wR .");
    board.addRow(". . .");
    board.addRow(". . .");
    // ה-validate צריך להחזיר true כי הלוח תקין
    CHECK(board.validate() == true); 
}

TEST_CASE("שלב ב': בדיקת לחיצה והמתנה") {
    Board board;
    board.addRow("wK . .");
    board.validate();
    
    board.click(50, 50); // בחירת wK
    board.wait(100);     // בדיקת פונקציית המתנה
    CHECK(true);         // מוודא שהתוכנית לא קורסת
}

TEST_CASE("שלב ג': בדיקת תנועה חוקית ולא חוקית") {
    Board board;
    board.addRow("wR . ."); // Rook ב-(0,0)
    board.addRow(". . .");
    board.validate();

    // ניסיון מהלך לא חוקי (אלכסוני לצריח)
    board.click(50, 50);   // בחירה
    board.click(150, 150); // ניסיון אלכסוני
    
    std::stringstream ss;
    board.printCanonical(ss);
    // הלוח לא אמור להשתנות כי המהלך לא חוקי
    CHECK(ss.str() == "wR . .\n. . .\n. . .\n");
}

TEST_CASE("שלב ד': חסימות ולכידה") {
    Board board;
    board.addRow("wR wP ."); // חסימה של הצריח על ידי חייל ידידותי
    board.addRow(". . .");
    board.addRow("bP . .");  // חייל אויב בהמשך השורה
    board.validate();

    // 1. בדיקת חסימה (הצריח לא יכול לדלג מעל wP)
    board.click(50, 50);   // בחירת wR
    board.click(250, 50);  // ניסיון להגיע ל-(0,2)
    
    std::stringstream ss1;
    board.printCanonical(ss1);
    CHECK(ss1.str() == "wR wP .\n. . .\nbP . .\n");

    // 2. בדיקת לכידת אויב (חוקי)
    board.click(50, 50);   // בחירת wR
    board.click(50, 250);  // לכידת bP ב-(2,0)
    
    std::stringstream ss2;
    board.printCanonical(ss2);
    CHECK(ss2.str() == ". wP .\n. . .\nwR . .\n");
}

TEST_CASE("שלב ד': פרש מדלג מעל כלים") {
    Board board;
    board.addRow("wN wP .");
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);   // בחירת פרש
    board.click(150, 250); // קפיצה מעל החייל
    
    std::stringstream ss;
    board.printCanonical(ss);
    CHECK(ss.str() == ". wP .\n. . .\n. wN .\n");
}
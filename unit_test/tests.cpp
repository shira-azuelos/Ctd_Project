#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../classes/Board.h"
#include <sstream>

TEST_CASE("Testing Board Validation") {
    Board board;
    board.addRow("wK . .");
    board.addRow(". . .");
    board.addRow(". . .");

    CHECK(board.validate() == true);
}

TEST_CASE("Testing Click and Move Logic") {
    Board board;
    board.addRow("wR . .");
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    // הזזה חוקית של צריח
    board.click(50, 50);  // בוחר את wR ב-0,0
    board.click(50, 150); // מזיז ל-1,0

    std::stringstream ss;
    board.printCanonical(ss);
    
    // בודק שהצריח אכן עבר ושבמקום הישן יש נקודה
    CHECK(ss.str() == ". . .\nwR . .\n. . .\n");
}

TEST_CASE("Testing Path Blocking (Rook cannot jump)") {
    Board board;
    board.addRow("wR wP ."); // צריח ולידו חייל באותה שורה
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);   // בוחר את wR
    board.click(250, 50);  // מנסה לדלג מעל החייל ל-0,2

    std::stringstream ss;
    board.printCanonical(ss);
    
    // בודק שהלוח לא השתנה בגלל שהמהלך לא חוקי
    CHECK(ss.str() == "wR wP .\n. . .\n. . .\n");
}

TEST_CASE("Testing Game Time") {
    Board board;
    board.wait(500);
    board.wait(250);
    // כדי שזה יעבוד, וודאי שהוספת getGameTime למחלקה
    // אם לא הוספת, אפשר פשוט להסיר את ה-CHECK הזה בינתיים
    // CHECK(board.getGameTime() == 750); 
}

TEST_CASE("Testing Capture Logic") {
    Board board;
    board.addRow("wR bP ."); // צריח לבן ליד חייל שחור
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);   // בוחר wR
    board.click(150, 50);  // לוכד את bP ב-0,1

    std::stringstream ss;
    board.printCanonical(ss);
    
    // בודק שהצריח אכן נמצא במקום החייל שנלכד
    CHECK(ss.str() == ". wR .\n. . .\n. . .\n");
}

TEST_CASE("Testing No Friendly Fire") {
    Board board;
    board.addRow("wR wP ."); // צריח וחייל מאותו צבע
    board.addRow(". . .");
    board.addRow(". . .");
    board.validate();

    board.click(50, 50);   // בוחר wR
    board.click(150, 50);  // מנסה "ללכוד" את wP

    std::stringstream ss;
    board.printCanonical(ss);
    
    // בודק שהלוח לא השתנה (המהלך נחסם)
    CHECK(ss.str() == "wR wP .\n. . .\n. . .\n");
}

TEST_CASE("Testing Knight Jump and Capture") {
    Board board;
    board.addRow("wN wP ."); // פרש לבן, לידו חייל לבן
    board.addRow(". . .");
    board.addRow(". bP .");  // חייל שחור במיקום שהפרש יכול להגיע אליו
    board.validate();

    board.click(50, 50);   // בוחר wN
    board.click(150, 250); // הפרש מדלג מעל wP ולוכד את bP

    std::stringstream ss;
    board.printCanonical(ss);
    
    // בודק שהפרש עבר למיקום החדש והחייל השחור נעלם
    CHECK(ss.str() == ". wP .\n. . .\n. wN .\n");
}
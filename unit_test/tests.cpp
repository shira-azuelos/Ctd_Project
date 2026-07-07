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
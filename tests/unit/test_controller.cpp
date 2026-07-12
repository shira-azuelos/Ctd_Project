#include "../../doctest.h"
#include "input/controller.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include <memory>

TEST_CASE("Controller & Engine: Movement takes correct amount of time") {
    // 1.  לוח פשוט של 2x2 עם חייל אחד
    auto board = std::make_shared<model::Board>(2, 2);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(0, 0));
    board->add_piece(piece);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    input::Controller controller(engine, board);

    // 2. קליק על התא (0,0) - בחירת החייל (פיקסלים: 50, 50)
    controller.click(50, 50);
    
    // 3. קליק על היעד (0,1) - שליחת בקשת תנועה (פיקסלים: 150, 50)
    controller.click(150, 50);

    // 4. בדיקה שהחייל עדיין לא זז (כי לא עבר זמן!)
    CHECK(board->get_piece_at(model::Position(0, 0)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);

    // 5. המרחק הוא משבצת אחת, לכן נדרשים 1000 מ"ש. נמתין חצי מהזמן:
    engine->wait(500);
    
    // מוודאים שהוא עדיין במקור כי הזמן לא תם
    CHECK(board->get_piece_at(model::Position(0, 0)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);

    // 6. נמתין את השארית של הזמן (עוד 500 מ"ש)
    engine->wait(500);
    
    // עכשיו החייל היה אמור להגיע ליעדו הלוגי
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == nullptr);
    auto moved_piece = board->get_piece_at(model::Position(0, 1));
    REQUIRE(moved_piece.get() != nullptr);
    CHECK(moved_piece->id == "p1");
}
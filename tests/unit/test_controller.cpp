#include "../../doctest.h"
#include "input/controller.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include <memory>

TEST_CASE("Controller & Engine: Movement takes correct amount of time") {
    auto board = std::make_shared<model::Board>(2, 2);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(0, 0));
    board->add_piece(piece);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    input::Controller controller(engine, board);

    // קליק על המקור (0,0) -> פיקסלים (50, 50)
    controller.click(50, 50);
    
    // קליק על היעד (1,0) -> פיקסלים (50, 150)
    controller.click(50, 150);

    // בדיקה שהכלי עדיין לא זז (לא עבר זמן)
    CHECK(board->get_piece_at(model::Position(0, 0)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(1, 0)).get() == nullptr);

    // המתנה לסיום התנועה (1000 מ"ש)
    engine->wait(1000);
    
    // בדיקה שהכלי הגיע ליעד
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == nullptr);
    auto moved_piece = board->get_piece_at(model::Position(1, 0));
    REQUIRE(moved_piece.get() != nullptr);
    CHECK(moved_piece->id == "p1");
}
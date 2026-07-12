#include "../../doctest.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include "model/position.h"
#include <memory>

TEST_CASE("Movement blocking and immediate re-move") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    board->add_piece(piece);
    auto engine = std::make_shared<engine::GameEngine>(board);

    // תנועה ראשונה
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    // ניסיון תנועה שנייה בזמן שהראשונה פעילה (צריך להיכשל)
    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    CHECK(board->get_piece_at(model::Position(0, 2)).get() == nullptr);

    // המתנה לסיום
    engine->wait(1000);
    
    // תנועה נוספת מיד לאחר מכן
    engine->request_move(model::Position(0, 1), model::Position(0, 2));
    engine->wait(1000);
    CHECK(board->get_piece_at(model::Position(0, 2)).get() != nullptr);
}

TEST_CASE("Advanced Interaction Cases") {
    auto board = std::make_shared<model::Board>(8, 8);
    // ... (הצבת כלים) ...
    auto engine = std::make_shared<engine::GameEngine>(board);

    // תנועה ראשונה (1000ms)
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    // חובה להמתין שהתנועה תסתיים כדי לשחרר את ה-Arbiter
    engine->wait(1000); 
    
    // עכשיו המנוע יהיה פנוי לתנועה הבאה
    CHECK_FALSE(engine->is_moving());
}
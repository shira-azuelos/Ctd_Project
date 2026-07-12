#include "../../doctest.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include <memory>

TEST_CASE("Piece movement happens over time") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    board->add_piece(piece);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    // תחילת תנועה (1000ms)
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    // בדיקה: אחרי 500ms, הכלי עדיין במקור
    engine->wait(500);
    CHECK(board->get_piece_at(model::Position(0, 0)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);
    
    // בדיקה: אחרי עוד 500ms (סה"כ 1000), הכלי הגיע ליעד
    engine->wait(500);
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == nullptr);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() != nullptr);
}
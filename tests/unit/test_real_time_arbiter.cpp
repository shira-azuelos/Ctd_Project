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

TEST_CASE("Airborne mechanics: Mid-air capture") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto arbiter = std::make_shared<realtime::RealTimeArbiter>();

    auto white_piece = std::make_shared<model::Piece>("wR", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    auto black_piece = std::make_shared<model::Piece>("bR", model::PieceColor::BLACK, model::PieceKind::ROOK, model::Position(0, 1));

    board->add_piece(white_piece);
    board->add_piece(black_piece);

    // 1. האויב (שחור) מתחיל לנוע מ-(0,1) ל-(0,0) - זמן תנועה של 1000ms
    arbiter->start_motion(black_piece, model::Position(0, 1), model::Position(0, 0), 1000);

    // 2. אחרי 500ms, הלבן קופץ לאוויר במשבצת שלו (0,0)
    arbiter->advance_time(500, board);
    arbiter->start_jump(white_piece, model::Position(0, 0), 1000);

    // 3. ממתינים עוד 500ms. בשלב זה השחור סיים את התנועה שלו ו"נחת" על הלבן
    arbiter->advance_time(500, board);

    // 4. בדיקת התוצאה:
    // השחור (האויב) אמור להימחק מהלוח (נמחק ממקום המוצא שלו כי הוא מושמד)
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);
    // הלבן אמור לשרוד במשבצת המקורית שלו
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == white_piece.get());
}
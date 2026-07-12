#include "../../doctest.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include "model/position.h"
#include <memory>

TEST_CASE("Movement blocking and immediate re-move") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    
    // מוסיפים שני מלכים "פיקטיביים" בפינות הלוח כדי שהמשחק לא יוכרז כ-Game Over בטעות
    auto dummy_wk = std::make_shared<model::Piece>("wK", model::PieceColor::WHITE, model::PieceKind::KING, model::Position(7, 0));
    auto dummy_bk = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(7, 7));

    board->add_piece(piece);
    board->add_piece(dummy_wk);
    board->add_piece(dummy_bk);

    auto engine = std::make_shared<engine::GameEngine>(board);

    // תנועה ראשונה
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    // ניסיון תנועה שנייה בזמן שהראשונה פעילה (צריך להיכשל)
    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    CHECK(board->get_piece_at(model::Position(0, 2)).get() == nullptr);

    // המתנה לסיום תנועה ראשונה
    engine->wait(1000);
    
    // תנועה נוספת מיד לאחר מכן (עכשיו תעבוד כי המלכים בחיים)
    engine->request_move(model::Position(0, 1), model::Position(0, 2));
    engine->wait(1000); // המתנה לסיום תנועה שנייה

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

TEST_CASE("Game-over behavior: Capturing King ends game and ignores moves") {
    auto board = std::make_shared<model::Board>(8, 8);
    // נציב צריח לבן שמוכן לתקוף מלך שחור
    auto white_rook = std::make_shared<model::Piece>("wR", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    auto black_king = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(0, 2));
    
    // נוסיף צריח שחור סתם כדי לנסות להזיז אותו אחרי שהמשחק ייגמר
    auto black_rook = std::make_shared<model::Piece>("bR", model::PieceColor::BLACK, model::PieceKind::ROOK, model::Position(7, 7));

    board->add_piece(white_rook);
    board->add_piece(black_king);
    board->add_piece(black_rook);

    auto engine = std::make_shared<engine::GameEngine>(board);
    
    CHECK_FALSE(engine->is_game_over()); // המשחק לא נגמר בהתחלה

    // צריח לבן תוקף את המלך השחור
    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    engine->wait(2000); // מחכים שהתנועה תסתיים

    // המלך נאכל - המשחק אמור להסתיים
    CHECK(engine->is_game_over());

    // ניסיון להזיז כלי נוסף אחרי שהמשחק נגמר (פקודה שאמורה להיות מסורבת)
    engine->request_move(model::Position(7, 7), model::Position(7, 6));
    engine->wait(1000);
    
    // וידוא שהצריח השחור לא זז כי הפקודה סורבה
    CHECK(board->get_piece_at(model::Position(7, 7)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(7, 6)).get() == nullptr);
}

TEST_CASE("Pawn special rules: Double move, path blocking, and promotion") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto engine = std::make_shared<engine::GameEngine>(board);

    // חייל לבן בעמדת הפתיחה שלו, ומלך שחור כדמי
    auto pawn = std::make_shared<model::Piece>("wP", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(1, 0));
    auto dummy_bk = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(7, 7));
    auto dummy_wk = std::make_shared<model::Piece>("wK", model::PieceColor::WHITE, model::PieceKind::KING, model::Position(0, 7));
    
    board->add_piece(pawn);
    board->add_piece(dummy_bk);
    board->add_piece(dummy_wk);

    SUBCASE("Pawn can move 2 cells forward from start, but not if blocked") {
        auto blocking_piece = std::make_shared<model::Piece>("bN", model::PieceColor::BLACK, model::PieceKind::KNIGHT, model::Position(2, 0));
        board->add_piece(blocking_piece);

        // ניסיון קפיצה מעל כלי - אמור להיכשל
        engine->request_move(model::Position(1, 0), model::Position(3, 0));
        CHECK_FALSE(engine->is_moving());

        // נזיז את הכלי החוסם וננסה שוב
        board->move_piece(model::Position(2, 0), model::Position(2, 1));
        
        // עכשיו הקפיצה אמורה לעבוד
        engine->request_move(model::Position(1, 0), model::Position(3, 0));
        engine->wait(2000);
        
        CHECK(board->get_piece_at(model::Position(3, 0)).get() == pawn.get());
    }

    SUBCASE("Pawn promotes to Queen on the last row") {
        // נזיז את החייל ידנית לשורה 6 לצורך הבדיקה
        board->move_piece(model::Position(1, 0), model::Position(6, 0));
        
        // תנועה אחת קדימה לשורה האחרונה (7)
        engine->request_move(model::Position(6, 0), model::Position(7, 0));
        engine->wait(1000);

        auto piece_at_end = board->get_piece_at(model::Position(7, 0));
        REQUIRE(piece_at_end.get() != nullptr);
        // וידוא שהכלי שינה את הסוג שלו למלכה
        CHECK(piece_at_end->kind == model::PieceKind::QUEEN);
    }
}
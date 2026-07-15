#include "../../doctest.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include "model/position.h"
#include <memory>

TEST_CASE("Movement blocking and immediate re-move") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    
    auto dummy_wk = std::make_shared<model::Piece>("wK", model::PieceColor::WHITE, model::PieceKind::KING, model::Position(7, 0));
    auto dummy_bk = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(7, 7));

    board->add_piece(piece);
    board->add_piece(dummy_wk);
    board->add_piece(dummy_bk);

    auto engine = std::make_shared<engine::GameEngine>(board);

    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    CHECK(board->get_piece_at(model::Position(0, 2)).get() == nullptr);

    engine->wait(1000);
    
    engine->wait(3000);
    
    engine->request_move(model::Position(0, 1), model::Position(0, 2));
    engine->wait(1000);

    CHECK(board->get_piece_at(model::Position(0, 2)).get() != nullptr);
}

TEST_CASE("Advanced Interaction Cases") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto engine = std::make_shared<engine::GameEngine>(board);

    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    engine->wait(1000); 
    
    CHECK_FALSE(engine->is_moving());
}

TEST_CASE("Game-over behavior: Capturing King ends game and ignores moves") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto white_rook = std::make_shared<model::Piece>("wR", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    auto black_king = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(0, 2));
    
    auto black_rook = std::make_shared<model::Piece>("bR", model::PieceColor::BLACK, model::PieceKind::ROOK, model::Position(7, 7));

    board->add_piece(white_rook);
    board->add_piece(black_king);
    board->add_piece(black_rook);

    auto engine = std::make_shared<engine::GameEngine>(board);
    
    CHECK_FALSE(engine->get_state()->is_game_over());

    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    engine->wait(2000);

    CHECK(engine->get_state()->is_game_over());

    engine->request_move(model::Position(7, 7), model::Position(7, 6));
    engine->wait(1000);
    
    CHECK(board->get_piece_at(model::Position(7, 7)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(7, 6)).get() == nullptr);
}

TEST_CASE("Pawn special rules: Double move, path blocking, and promotion") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto engine = std::make_shared<engine::GameEngine>(board);

    auto pawn = std::make_shared<model::Piece>("wP", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(6, 0));
    auto dummy_bk = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(7, 7));
    auto dummy_wk = std::make_shared<model::Piece>("wK", model::PieceColor::WHITE, model::PieceKind::KING, model::Position(0, 7));
    
    board->add_piece(pawn);
    board->add_piece(dummy_bk);
    board->add_piece(dummy_wk);

    SUBCASE("Pawn can move 2 cells forward from start, but not if blocked") {
        auto blocking_piece = std::make_shared<model::Piece>("bN", model::PieceColor::BLACK, model::PieceKind::KNIGHT, model::Position(5, 0));
        board->add_piece(blocking_piece);

        engine->request_move(model::Position(6, 0), model::Position(4, 0));
        CHECK_FALSE(engine->is_moving());

        board->remove_piece(model::Position(5, 0));
        
        engine->request_move(model::Position(6, 0), model::Position(4, 0));
        engine->wait(2000);
        
        CHECK(board->get_piece_at(model::Position(4, 0)).get() == pawn.get());
    }

    SUBCASE("Pawn promotes to Queen on the last row") {
        board->move_piece(model::Position(6, 0), model::Position(1, 0));
        
        engine->request_move(model::Position(1, 0), model::Position(0, 0));
        engine->wait(1000);

        auto piece_at_end = board->get_piece_at(model::Position(0, 0));
        REQUIRE(piece_at_end.get() != nullptr);
        CHECK(piece_at_end->kind == model::PieceKind::QUEEN);
    }
}
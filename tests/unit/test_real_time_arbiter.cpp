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
    
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    engine->wait(500);
    CHECK(board->get_piece_at(model::Position(0, 0)).get() != nullptr);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);
    
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

    arbiter->start_motion(black_piece, model::Position(0, 1), model::Position(0, 0), 1000);

    arbiter->advance_time(500, board);
    arbiter->start_jump(white_piece, model::Position(0, 0), 1000);

    arbiter->advance_time(500, board);

    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == white_piece.get());
}

TEST_CASE("Piece cooldown after movement") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    board->add_piece(piece);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    engine->wait(1000);
    
    CHECK(engine->is_piece_cooling_down(piece) == true);
    CHECK(engine->get_piece_cooldown_remaining_ms(piece) == 3000);
    
    engine->request_move(model::Position(0, 1), model::Position(0, 2));
    CHECK(engine->is_moving() == false);
    
    engine->wait(1500);
    CHECK(engine->is_piece_cooling_down(piece) == true);
    CHECK(engine->get_piece_cooldown_remaining_ms(piece) == 1500);
    
    engine->request_move(model::Position(0, 1), model::Position(0, 2));
    CHECK(engine->is_moving() == false);
    
    engine->wait(1500);
    CHECK(engine->is_piece_cooling_down(piece) == false);
    CHECK(engine->get_piece_cooldown_remaining_ms(piece) == 0);
    engine->request_move(model::Position(0, 1), model::Position(0, 2));
    CHECK(engine->is_moving() == true);
}
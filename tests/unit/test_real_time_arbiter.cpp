#include "../../doctest.h"
#include "engine/game_engine.h"
#include "model/board.h"
#include "model/piece.h"
#include "rules/rule_engine.h"
#include <memory>
#include <iostream>

TEST_CASE("Piece movement happens over time") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto piece = std::make_shared<model::Piece>("p1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    board->add_piece(piece);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    
    engine->wait(500);
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == nullptr);
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

    arbiter->start_motion(black_piece, model::Position(0, 1), model::Position(0, 0), 1000, board);

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

TEST_CASE("Concurrent movement of multiple pieces") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto p1 = std::make_shared<model::Piece>("wR1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    auto p2 = std::make_shared<model::Piece>("wR2", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(7, 7));
    board->add_piece(p1);
    board->add_piece(p2);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(0, 0), model::Position(0, 1));
    engine->request_move(model::Position(7, 7), model::Position(7, 6));
    
    CHECK(engine->get_active_motions().size() == 2);
    
    engine->wait(500);
    CHECK(engine->get_active_motions().size() == 2);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == nullptr);
    CHECK(board->get_piece_at(model::Position(7, 6)).get() == nullptr);
    
    engine->wait(500);
    CHECK(engine->get_active_motions().size() == 0);
    CHECK(board->get_piece_at(model::Position(0, 1)).get() == p1.get());
    CHECK(board->get_piece_at(model::Position(7, 6)).get() == p2.get());
}

TEST_CASE("Collision - Different colors (late arrival captures early)") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto white_rook = std::make_shared<model::Piece>("wR", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    auto black_rook = std::make_shared<model::Piece>("bR", model::PieceColor::BLACK, model::PieceKind::ROOK, model::Position(0, 3));
    
    board->add_piece(white_rook);
    board->add_piece(black_rook);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    
    engine->wait(500);
    
    engine->request_move(model::Position(0, 3), model::Position(0, 2));
    
    engine->wait(1500);
    
    CHECK(board->get_piece_at(model::Position(0, 2)).get() == white_rook.get());
    CHECK(board->get_piece_at(model::Position(0, 3)).get() == nullptr);
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == nullptr);
}

TEST_CASE("Collision - Same color (late arrival gets blocked and stuck)") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto rook1 = std::make_shared<model::Piece>("wR1", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 0));
    auto rook2 = std::make_shared<model::Piece>("wR2", model::PieceColor::WHITE, model::PieceKind::ROOK, model::Position(0, 3));
    
    board->add_piece(rook1);
    board->add_piece(rook2);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(0, 0), model::Position(0, 2));
    
    engine->wait(500);
    
    engine->request_move(model::Position(0, 3), model::Position(0, 2));
    
    engine->wait(1500);
    
    CHECK(board->get_piece_at(model::Position(0, 2)).get() == rook2.get());
    CHECK(board->get_piece_at(model::Position(0, 0)).get() == rook1.get());
    CHECK(board->get_piece_at(model::Position(0, 3)).get() == nullptr);
}

TEST_CASE("Escaping piece survives and attacker lands on empty cell") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto white_knight = std::make_shared<model::Piece>("wN", model::PieceColor::WHITE, model::PieceKind::KNIGHT, model::Position(4, 5));
    auto black_pawn = std::make_shared<model::Piece>("bP", model::PieceColor::BLACK, model::PieceKind::PAWN, model::Position(6, 4));
    
    board->add_piece(white_knight);
    board->add_piece(black_pawn);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(4, 5), model::Position(6, 4));
    
    engine->wait(500);
    
    engine->request_move(model::Position(6, 4), model::Position(7, 4));
    
    engine->request_move(model::Position(6, 4), model::Position(7, 4));
    
    engine->wait(1000);
    
    CHECK(board->get_piece_at(model::Position(7, 4)).get() == black_pawn.get());
    CHECK(board->get_piece_at(model::Position(6, 4)).get() == nullptr);
    
    engine->wait(500);
    
    CHECK(board->get_piece_at(model::Position(6, 4)).get() == white_knight.get());
    CHECK(board->get_piece_at(model::Position(7, 4)).get() == black_pawn.get());
}

TEST_CASE("Game Over is not triggered prematurely when King is under threat by a moving piece") {
    auto board = std::make_shared<model::Board>(8, 8);
    auto white_pawn = std::make_shared<model::Piece>("wP", model::PieceColor::WHITE, model::PieceKind::PAWN, model::Position(1, 1));
    auto black_king = std::make_shared<model::Piece>("bK", model::PieceColor::BLACK, model::PieceKind::KING, model::Position(0, 2));
    auto white_king = std::make_shared<model::Piece>("wK", model::PieceColor::WHITE, model::PieceKind::KING, model::Position(7, 7));
    
    board->add_piece(white_pawn);
    board->add_piece(black_king);
    board->add_piece(white_king);
    
    auto engine = std::make_shared<engine::GameEngine>(board);
    
    engine->request_move(model::Position(1, 1), model::Position(0, 2));
    
    CHECK(engine->get_state()->is_game_over() == false);
    
    engine->wait(500);
    CHECK(engine->get_state()->is_game_over() == false);
    
    engine->wait(500);
    CHECK(engine->get_state()->is_game_over() == true);
}
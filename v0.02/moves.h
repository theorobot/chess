#ifndef MOVES_H
#define MOVES_H

#include "settings.h"
#include "board.h"

struct MoveList {
    uint16_t* moves;
    uint16_t num_moves;
};

extern uint64_t kingMovesBySquareBB[64];
extern uint64_t knightMovesBySquareBB[64];
extern uint64_t slidingPiecesMovesBySquareBB[64][8];

void initKingMovesBySquareBB();
void initKnightMovesBySquareBB();
void initSlidingPiecesMovesBySquareBB();
void initMovesBySquareBBs();

void AddMoveToMoveList(struct MoveList* list, uint16_t move);

uint64_t KingMovesBB(struct Game* g, uint8_t square);
uint64_t KingAttacksBB(struct Game* g, uint8_t square);
struct MoveList KingMovesList(struct Game* g);
uint64_t KnightMovesBB(struct Game* g, uint8_t square);
uint64_t KnightAttacksBB(struct Game* g, uint8_t square);
struct MoveList KnightMovesList(struct Game* g);
uint64_t PawnMovesBB(struct Game* g, uint8_t square);
uint64_t PawnAttacksBB(struct Game* g, uint8_t square);
struct MoveList PawnMovesList(struct Game* g);
uint64_t BishopMovesBB(struct Game* g, uint8_t square);
uint64_t BishopAttacksBB(struct Game* g, uint8_t square);
struct MoveList BishopMovesList(struct Game* g);
uint64_t RookMovesBB(struct Game* g, uint8_t square);
uint64_t RookAttacksBB(struct Game* g, uint8_t square);
struct MoveList RookMovesList(struct Game* g);
uint64_t QueenMovesBB(struct Game* g, uint8_t square);
uint64_t QueenAttacksBB(struct Game* g, uint8_t square);
struct MoveList QueenMovesList(struct Game* g);
struct MoveList PseudoLegalMoves(struct Game* g);
uint64_t allSquaresAttacking(struct Game* g);
bool InCheck(struct Game* g);
struct MoveList LegalMoves(struct Game* g);
uint64_t perft(struct Game* g, int depth);

#endif

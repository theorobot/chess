#ifndef BOARD_H
#define BOARD_H

#include "settings.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Game {
	uint64_t pieces_bb[9];
	struct Game* prev_state;
};

uint8_t ConvertSquareStringToSquareIndex(char* sq_str);
uint16_t ConvertStartEndSquareToMove(uint8_t start_sq, uint8_t end_sq);
uint16_t ConvertStartEndSquareFlagToMove(uint8_t start_sq, uint8_t end_sq, uint8_t flag);
uint16_t ConvertStartEndSquareStringFlagToMove(char* start_sq_str, char* end_sq_str, uint8_t flag);
uint16_t ConvertMoveStringFlagToMove(char* move_str, uint8_t flag);
char* ConvertMoveToStringMove(uint16_t move);
void GameReset(struct Game* g);
void GameAddPiece(struct Game* g, uint8_t square, uint8_t piece);
void GameDeletePiece(struct Game* g, uint8_t square);
void GameMovePiece(struct Game* g, uint8_t start_sq, uint8_t end_sq);
void GameMakeMove(struct Game* g, uint16_t move);
void GameUnmakeMove(struct Game* g);
void GameLoadFen(struct Game* g, char* fen);
void GamePrintDetails(struct Game* g);
void GameDelete(struct Game* g);

#endif
#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

enum pieceIndex {
	iWHITE, iBLACK, iKING, iQUEEN, iROOK, iBISHOP, iKNIGHT, iPAWN, iSTATE
};

enum pieceValue {
	vWHITE  = 8,
	vBLACK  = 16,
	vKING   = 1,
	vQUEEN  = 2,
	vROOK   = 3,
	vBISHOP = 4,
	vKNIGHT = 5,
	vPAWN   = 6
};

enum slidingPieceDir {
	DIRRIGHT, DIRUP, DIRLEFT, DIRDOWN, DIRUPRIGHT, DIRUPLEFT, DIRDOWNLEFT, DIRDOWNRIGHT
};

#define pieceValueColorMask 0x18
#define pieceValueTypeMask 0x7

#define gameStateTurnMask 0x1ull
#define gameStateWhiteKingsideCastlingMask 0x2ull
#define gameStateWhiteQueensideCastlingMask 0x4ull
#define gameStateBlackKingsideCastlingMask 0x8ull
#define gameStateBlackQueensideCastlingMask 0x10ull
#define gameStateEnpassantMask 0x7e0ull
#define gameStateHalfmovesMask 0x3f800ull
#define gameStateFullmovesMask 0xfffffffffffc0000ull

#define moveStartSquareMask 0x3f
#define moveEndSquareMask 0xfc0
#define moveFlagMask 0x7000

#define noMoveFlag 0x0
#define castlingMoveFlag 0x1
#define pawnTwoUpMoveFlag 0x2
#define enPassantMoveFlag 0x3
#define queenPromotionMoveFlag 0x4
#define rookPromotionMoveFlag 0x5
#define bishopPromotionMoveFlag 0x6
#define knightPromotionMoveFlag 0x7

#define startingPositionFen "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#endif
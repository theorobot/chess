#include "moves.h"

uint64_t kingMovesBySquareBB[64];
uint64_t knightMovesBySquareBB[64];
uint64_t slidingPiecesMovesBySquareBB[64][8];

void initKingMovesBySquareBB() {
    memset(kingMovesBySquareBB, 0, 64 * 8);
    for (int square = 0; square < 64; square++) {
        uint8_t on_afile = square % 8 == 0;
        uint8_t on_hfile = square % 8 == 7;
        uint8_t on_toprank = square >= 56;
        uint8_t on_bottomrank = square <= 7;
        kingMovesBySquareBB[square] |= on_afile ? 0ULL : 1ULL << (square - 1);
        kingMovesBySquareBB[square] |= on_hfile ? 0ULL : 1ULL << (square + 1);
        kingMovesBySquareBB[square] |= on_toprank ? 0ULL : 1ULL << (square + 8);
        kingMovesBySquareBB[square] |= on_bottomrank ? 0ULL : 1ULL << (square - 8);
        kingMovesBySquareBB[square] |= on_hfile || on_bottomrank ? 0ULL : 1ULL << (square - 7);
        kingMovesBySquareBB[square] |= on_afile || on_toprank ? 0ULL : 1ULL << (square + 7);
        kingMovesBySquareBB[square] |= on_afile || on_bottomrank ? 0ULL : 1ULL << (square - 9);
        kingMovesBySquareBB[square] |= on_hfile || on_toprank ? 0ULL : 1ULL << (square + 9);
    }
}

void initKnightMovesBySquareBB() {
    memset(knightMovesBySquareBB, 0, 64 * 8);
    for (int square = 0; square < 64; square++) {
        uint8_t on_afile = square % 8 == 0;
        uint8_t on_abfile = square % 8 <= 1;
        uint8_t on_hfile = square % 8 == 7;
        uint8_t on_ghfile = square % 8 >= 6;
        uint8_t on_toprank = square >= 56;
        uint8_t on_top2rank = square >= 48;
        uint8_t on_bottomrank = square <= 7;
        uint8_t on_bottom2rank = square <= 15;
        knightMovesBySquareBB[square] |= on_afile || on_top2rank ? 0ULL : 1ULL << (square + 15);
        knightMovesBySquareBB[square] |= on_hfile || on_top2rank ? 0ULL : 1ULL << (square + 17);
        knightMovesBySquareBB[square] |= on_abfile || on_toprank ? 0ULL : 1ULL << (square + 6);
        knightMovesBySquareBB[square] |= on_ghfile || on_toprank ? 0ULL : 1ULL << (square + 10);
        knightMovesBySquareBB[square] |= on_abfile || on_bottomrank ? 0ULL : 1ULL << (square - 10);
        knightMovesBySquareBB[square] |= on_ghfile || on_bottomrank ? 0ULL : 1ULL << (square - 6);
        knightMovesBySquareBB[square] |= on_afile || on_bottom2rank ? 0ULL : 1ULL << (square - 17);
        knightMovesBySquareBB[square] |= on_hfile || on_bottom2rank ? 0ULL : 1ULL << (square - 15);
    }
}

void initSlidingPiecesMovesBySquareBB() {
    memset(slidingPiecesMovesBySquareBB, 0, 64 * 8 * 8);
    int directions[8][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
    for (int square = 0; square < 64; square++) {
        for (int dir = 0; dir < 8; dir++) {
            int file = square % 8;
            int rank = square / 8;
            file += directions[dir][0];
            rank += directions[dir][1];
            while (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
                slidingPiecesMovesBySquareBB[square][dir] |= 1ULL << (file + 8 * rank);
                file += directions[dir][0];
                rank += directions[dir][1];
            }
        }
    }
}

void initMovesBySquareBBs() {
    initKingMovesBySquareBB();
    initKnightMovesBySquareBB();
    initSlidingPiecesMovesBySquareBB();
}

void AddMoveToMoveList(struct MoveList* list, uint16_t move) {
    list->moves = (uint16_t*)realloc(list->moves, 2 * (list->num_moves + 1));
    list->moves[list->num_moves] = move;
    list->num_moves++;
}

uint64_t KingMovesBB(struct Game* g, uint8_t square) {
    return kingMovesBySquareBB[square] & ~(g->pieces_bb[iWHITE] & (1ULL << square) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
}

uint64_t KingAttacksBB(struct Game* g, uint8_t square) {
	return kingMovesBySquareBB[square];
}

struct MoveList KingMovesList(struct Game* g) {
    struct MoveList king_moves_list;
    king_moves_list.num_moves = 0;
    king_moves_list.moves = (uint16_t*)malloc(0);

    uint8_t king_square;
    if (g->pieces_bb[iSTATE] & gameStateTurnMask)
        king_square = __builtin_ffsll(g->pieces_bb[iWHITE] & g->pieces_bb[iKING]) - 1;
    else
        king_square = __builtin_ffsll(g->pieces_bb[iBLACK] & g->pieces_bb[iKING]) - 1;
    
    uint64_t king_moves = KingMovesBB(g, king_square);
    while (king_moves) {
        uint8_t destination_square = __builtin_ffsll(king_moves) - 1;
        AddMoveToMoveList(&king_moves_list, ConvertStartEndSquareFlagToMove(king_square, destination_square, noMoveFlag));
        king_moves &= ~(1ULL << destination_square);
    }

    if (g->pieces_bb[iSTATE] & gameStateTurnMask) {
        if (g->pieces_bb[iSTATE] & gameStateWhiteKingsideCastlingMask) {
            if (!((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & 0x60))
                AddMoveToMoveList(&king_moves_list, ConvertStartEndSquareFlagToMove(king_square, 6, castlingMoveFlag));
        }
        if (g->pieces_bb[iSTATE] & gameStateWhiteQueensideCastlingMask) {
            if (!((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & 0xE))
                AddMoveToMoveList(&king_moves_list, ConvertStartEndSquareFlagToMove(king_square, 2, castlingMoveFlag));
        }
    } else {
        if (g->pieces_bb[iSTATE] & gameStateBlackKingsideCastlingMask) {
            if (!((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & 0x6000000000000000))
                AddMoveToMoveList(&king_moves_list, ConvertStartEndSquareFlagToMove(king_square, 62, castlingMoveFlag));
        }
        if (g->pieces_bb[iSTATE] & gameStateBlackQueensideCastlingMask) {
            if (!((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & 0xE00000000000000))
                AddMoveToMoveList(&king_moves_list, ConvertStartEndSquareFlagToMove(king_square, 58, castlingMoveFlag));
        }
    }

    return king_moves_list;
}

uint64_t KnightMovesBB(struct Game* g, uint8_t square) {
    return knightMovesBySquareBB[square] & ~(g->pieces_bb[iWHITE] & (1ULL << square) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
}

uint64_t KnightAttacksBB(struct Game* g, uint8_t square) {
	return knightMovesBySquareBB[square];
}

struct MoveList KnightMovesList(struct Game* g) {
    struct MoveList knight_moves_list;
    knight_moves_list.num_moves = 0;
    knight_moves_list.moves = (uint16_t*)malloc(0);

    uint64_t knights_bb = g->pieces_bb[iKNIGHT] & ((g->pieces_bb[iSTATE] & gameStateTurnMask) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
    while (knights_bb) {
        uint8_t square = __builtin_ffsll(knights_bb) - 1;
        uint64_t knight_moves_bb = KnightMovesBB(g, square);
        while (knight_moves_bb) {
            uint8_t destination_square = __builtin_ffsll(knight_moves_bb) - 1;
            AddMoveToMoveList(&knight_moves_list, ConvertStartEndSquareFlagToMove(square, destination_square, noMoveFlag));
            knight_moves_bb &= ~(1ULL << destination_square);
        }
        knights_bb &= ~(1ULL << square);
    }

    return knight_moves_list;
}

uint64_t PawnMovesBB(struct Game* g, uint8_t square) {
    uint64_t pawn_moves = 0ULL;
    uint8_t en_passant_square = (g->pieces_bb[iSTATE] & gameStateEnpassantMask) >> 5;

    if (g->pieces_bb[iWHITE] & (1ULL << square)) {
        if (!((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & (1ULL << (square + 8)))) {
            pawn_moves |= 1ULL << (square + 8);
            if (square <= 15 && !((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & (1ULL << (square + 16)))) {
                pawn_moves |= 1ULL << (square + 16);
            }
        }

        if (square % 8 > 0 && (g->pieces_bb[iBLACK] & (1ULL << (square + 7)) || en_passant_square == square + 7)) {
            pawn_moves |= 1ULL << (square + 7);
        }

        if (square % 8 < 7 && (g->pieces_bb[iBLACK] & (1ULL << (square + 9)) || en_passant_square == square + 9)) {
            pawn_moves |= 1ULL << (square + 9);
        }
    } else {
        if (!((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & (1ULL << (square - 8)))) {
            pawn_moves |= 1ULL << (square - 8);
            if (square >= 48 && !((g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) & (1ULL << (square - 16)))) {
                pawn_moves |= 1ULL << (square - 16);
            }
        }

        if (square % 8 > 0 && (g->pieces_bb[iWHITE] & (1ULL << (square - 9)) || en_passant_square == square - 9)) {
            pawn_moves |= 1ULL << (square - 9);
        }

        if (square % 8 < 7 && (g->pieces_bb[iWHITE] & (1ULL << (square - 7)) || en_passant_square == square - 7)) {
            pawn_moves |= 1ULL << (square - 7);
        }
    }

    return pawn_moves;
}

uint64_t PawnAttacksBB(struct Game* g, uint8_t square) {
	uint64_t attacks_bb = 0ULL;
	if (g->pieces_bb[iWHITE] & (1ULL << square)) {
		if (square % 8 > 0)
			attacks_bb |= 1ULL << (square + 7);
		if (square % 8 < 7)
			attacks_bb |= 1ULL << (square + 9);
	} else {
		if (square % 8 > 0)
			attacks_bb |= 1ULL <<  (square - 9);
		if (square % 8 < 7)
			attacks_bb |= 1ULL << (square - 7);
	}

	return attacks_bb;
}	

struct MoveList PawnMovesList(struct Game* g) {
    struct MoveList pawn_moves;
    pawn_moves.num_moves = 0;
    pawn_moves.moves = (uint16_t*)malloc(0);

    if (g->pieces_bb[iSTATE] & gameStateTurnMask) {
        uint64_t pawns_bb = g->pieces_bb[iWHITE] & g->pieces_bb[iPAWN];
        while (pawns_bb) {
            uint8_t square = __builtin_ffsll(pawns_bb) - 1;
            uint64_t pawn_moves_bb = PawnMovesBB(g, square);
            
            while (pawn_moves_bb) {
                uint8_t destination_square = __builtin_ffsll(pawn_moves_bb) - 1;

                if (destination_square - square == 16) {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, pawnTwoUpMoveFlag));
                } else if (destination_square == (g->pieces_bb[iSTATE] & gameStateEnpassantMask) >> 5) {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, enPassantMoveFlag));
                } else if (destination_square >= 56) {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, queenPromotionMoveFlag));
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, rookPromotionMoveFlag));
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, bishopPromotionMoveFlag));
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, knightPromotionMoveFlag));
                } else {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, noMoveFlag));
                }

                pawn_moves_bb &= ~(1ULL << destination_square);
            }

            pawns_bb &= ~(1ULL << square);
        }
    } else {
        uint64_t pawns_bb = g->pieces_bb[iBLACK] & g->pieces_bb[iPAWN];
        while (pawns_bb) {
            uint8_t square = __builtin_ffsll(pawns_bb) - 1;
            uint64_t pawn_moves_bb = PawnMovesBB(g, square);

            while (pawn_moves_bb) {
                uint8_t destination_square = __builtin_ffsll(pawn_moves_bb) - 1;

                if (square - destination_square == 16) {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, pawnTwoUpMoveFlag));
                } else if (destination_square == (g->pieces_bb[iSTATE] & gameStateEnpassantMask) >> 5) {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, enPassantMoveFlag));
                } else if (destination_square <= 7) {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, queenPromotionMoveFlag));
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, rookPromotionMoveFlag));
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, bishopPromotionMoveFlag));
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, knightPromotionMoveFlag));
                } else {
                    AddMoveToMoveList(&pawn_moves, ConvertStartEndSquareFlagToMove(square, destination_square, noMoveFlag));
                }

                pawn_moves_bb &= ~(1ULL << destination_square);
            }

            pawns_bb &= ~(1ULL << square);
        }
    }

    return pawn_moves;
}

uint64_t BishopMovesBB(struct Game* g, uint8_t square) {
    uint64_t bishop_moves = 0ULL;

    uint64_t intersection = slidingPiecesMovesBySquareBB[square][DIRUPRIGHT] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    int first_square = __builtin_ffsll(intersection) - 1;
    bishop_moves |= slidingPiecesMovesBySquareBB[square][DIRUPRIGHT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRUPRIGHT] : 0);

    intersection = slidingPiecesMovesBySquareBB[square][DIRUPLEFT] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    first_square = __builtin_ffsll(intersection) - 1;
    bishop_moves |= slidingPiecesMovesBySquareBB[square][DIRUPLEFT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRUPLEFT] : 0);

    intersection = slidingPiecesMovesBySquareBB[square][DIRDOWNLEFT] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    first_square = 63 - __builtin_clzll(intersection);
    bishop_moves |= slidingPiecesMovesBySquareBB[square][DIRDOWNLEFT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRDOWNLEFT] : 0);

    intersection = slidingPiecesMovesBySquareBB[square][DIRDOWNRIGHT] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    first_square = 63 - __builtin_clzll(intersection);
    bishop_moves |= slidingPiecesMovesBySquareBB[square][DIRDOWNRIGHT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRDOWNRIGHT] : 0);

    return bishop_moves & ~(g->pieces_bb[iWHITE] & (1ULL << square) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
}

uint64_t BishopAttacksBB(struct Game* g, uint8_t square) {
	uint64_t bishop_attacks = 0ULL;
	uint64_t occupied = (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) ^ (g->pieces_bb[iKING] & ((g->pieces_bb[iWHITE] & (1ULL << square)) ? g->pieces_bb[iBLACK] : g->pieces_bb[iWHITE]));

	uint64_t intersection = slidingPiecesMovesBySquareBB[square][DIRUPRIGHT] & occupied;
	int first_square = __builtin_ffsll(intersection) - 1;
	bishop_attacks |= slidingPiecesMovesBySquareBB[square][DIRUPRIGHT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRUPRIGHT] : 0);

	intersection = slidingPiecesMovesBySquareBB[square][DIRUPLEFT] & occupied;
	first_square = __builtin_ffsll(intersection) - 1;
	bishop_attacks |= slidingPiecesMovesBySquareBB[square][DIRUPLEFT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRUPLEFT] : 0);

	intersection = slidingPiecesMovesBySquareBB[square][DIRDOWNLEFT] & occupied;
	first_square = 63 - __builtin_clzll(intersection);
	bishop_attacks |= slidingPiecesMovesBySquareBB[square][DIRDOWNLEFT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRDOWNLEFT] : 0);

	intersection = slidingPiecesMovesBySquareBB[square][DIRDOWNRIGHT] & occupied;
	first_square = 63 - __builtin_clzll(intersection);
	bishop_attacks |= slidingPiecesMovesBySquareBB[square][DIRDOWNRIGHT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRDOWNRIGHT] : 0);

	return bishop_attacks;
}

struct MoveList BishopMovesList(struct Game* g) {
    struct MoveList bishop_moves_list;
    bishop_moves_list.num_moves = 0;
    bishop_moves_list.moves = (uint16_t*)malloc(0);

    uint64_t bishop_bb = g->pieces_bb[iBISHOP] & ((g->pieces_bb[iSTATE] & gameStateTurnMask) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
    while (bishop_bb) {
        uint8_t square = __builtin_ffsll(bishop_bb) - 1;
        uint64_t bishop_moves_bb = BishopMovesBB(g, square);
        while (bishop_moves_bb) {
            uint8_t destination_square = __builtin_ffsll(bishop_moves_bb) - 1;
            AddMoveToMoveList(&bishop_moves_list, ConvertStartEndSquareFlagToMove(square, destination_square, noMoveFlag));
            bishop_moves_bb &= ~(1ULL << destination_square);
        }
        bishop_bb &= ~(1ULL << square);
    }

    return bishop_moves_list;
}

uint64_t RookMovesBB(struct Game* g, uint8_t square) {
    uint64_t rook_moves = 0ULL;

    uint64_t intersection = slidingPiecesMovesBySquareBB[square][DIRUP] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    int first_square = __builtin_ffsll(intersection) - 1;
    rook_moves |= slidingPiecesMovesBySquareBB[square][DIRUP] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRUP] : 0);

    intersection = slidingPiecesMovesBySquareBB[square][DIRLEFT] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    first_square = 63 - __builtin_clzll(intersection);
    rook_moves |= slidingPiecesMovesBySquareBB[square][DIRLEFT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRLEFT] : 0);

    intersection = slidingPiecesMovesBySquareBB[square][DIRDOWN] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    first_square = 63 - __builtin_clzll(intersection);
    rook_moves |= slidingPiecesMovesBySquareBB[square][DIRDOWN] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRDOWN] : 0);

    intersection = slidingPiecesMovesBySquareBB[square][DIRRIGHT] & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]);
    first_square = __builtin_ffsll(intersection) - 1;
    rook_moves |= slidingPiecesMovesBySquareBB[square][DIRRIGHT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRRIGHT] : 0);

    return rook_moves & ~(g->pieces_bb[iWHITE] & (1ULL << square) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
}

uint64_t RookAttacksBB(struct Game* g, uint8_t square) {
	uint64_t rook_attacks = 0ULL;
	uint64_t occupied = (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]) ^ (g->pieces_bb[iKING] & ((g->pieces_bb[iWHITE] & (1ULL << square)) ? g->pieces_bb[iBLACK] : g->pieces_bb[iWHITE]));

	uint64_t intersection = slidingPiecesMovesBySquareBB[square][DIRUP] & occupied;
	int first_square = __builtin_ffsll(intersection) - 1;
	rook_attacks |= slidingPiecesMovesBySquareBB[square][DIRUP] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRUP] : 0);

	intersection = slidingPiecesMovesBySquareBB[square][DIRLEFT] & occupied;
	first_square = 63 - __builtin_clzll(intersection);
	rook_attacks |= slidingPiecesMovesBySquareBB[square][DIRLEFT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRLEFT] : 0);

	intersection = slidingPiecesMovesBySquareBB[square][DIRDOWN] & occupied;
	first_square = 63 - __builtin_clzll(intersection);
	rook_attacks |= slidingPiecesMovesBySquareBB[square][DIRDOWN] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRDOWN] : 0);

	intersection = slidingPiecesMovesBySquareBB[square][DIRRIGHT] & occupied;
	first_square = __builtin_ffsll(intersection) - 1;
	rook_attacks |= slidingPiecesMovesBySquareBB[square][DIRRIGHT] ^ (intersection ? slidingPiecesMovesBySquareBB[first_square][DIRRIGHT] : 0);

	return rook_attacks;
}

struct MoveList RookMovesList(struct Game* g) {
    struct MoveList rook_moves_list;
    rook_moves_list.num_moves = 0;
    rook_moves_list.moves = (uint16_t*)malloc(0);

    uint64_t rook_bb = g->pieces_bb[iROOK] & ((g->pieces_bb[iSTATE] & gameStateTurnMask) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
    while (rook_bb) {
        uint8_t square = __builtin_ffsll(rook_bb) - 1;
        uint64_t rook_moves_bb = RookMovesBB(g, square);
        while (rook_moves_bb) {
            uint8_t destination_square = __builtin_ffsll(rook_moves_bb) - 1;
            AddMoveToMoveList(&rook_moves_list, ConvertStartEndSquareFlagToMove(square, destination_square, noMoveFlag));
            rook_moves_bb &= ~(1ULL << destination_square);
        }
        rook_bb &= ~(1ULL << square);
    }

    return rook_moves_list;
}

uint64_t QueenMovesBB(struct Game* g, uint8_t square) {
    return BishopMovesBB(g, square) | RookMovesBB(g, square);
}

uint64_t QueenAttacksBB(struct Game* g, uint8_t square) {
    return BishopAttacksBB(g, square) | RookAttacksBB(g, square);
}

struct MoveList QueenMovesList(struct Game* g) {
    struct MoveList queen_moves_list;
    queen_moves_list.num_moves = 0;
    queen_moves_list.moves = (uint16_t*)malloc(0);

    uint64_t queen_bb = g->pieces_bb[iQUEEN] & ((g->pieces_bb[iSTATE] & gameStateTurnMask) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK]);
    while (queen_bb) {
        uint8_t square = __builtin_ffsll(queen_bb) - 1;
        uint64_t queen_moves_bb = QueenMovesBB(g, square);
        while (queen_moves_bb) {
            uint8_t destination_square = __builtin_ffsll(queen_moves_bb) - 1;
            AddMoveToMoveList(&queen_moves_list, ConvertStartEndSquareFlagToMove(square, destination_square, noMoveFlag));
            queen_moves_bb &= ~(1ULL << destination_square);
        }
        queen_bb &= ~(1ULL << square);
    }

    return queen_moves_list;
}

struct MoveList PseudoLegalMoves(struct Game* g) {
    struct MoveList move_list;
    move_list.num_moves = 0;
    move_list.moves = (uint16_t*)malloc(0);

    struct MoveList king_moves = KingMovesList(g);
    move_list.num_moves += king_moves.num_moves;
    move_list.moves = (uint16_t*)realloc(move_list.moves, 2 * move_list.num_moves);
    memcpy(&move_list.moves[move_list.num_moves - king_moves.num_moves], king_moves.moves, 2 * king_moves.num_moves);

    struct MoveList queen_moves = QueenMovesList(g);
    move_list.num_moves += queen_moves.num_moves;
    move_list.moves = (uint16_t*)realloc(move_list.moves, 2 * move_list.num_moves);
    memcpy(&move_list.moves[move_list.num_moves - queen_moves.num_moves], queen_moves.moves, 2 * queen_moves.num_moves);

    struct MoveList rook_moves = RookMovesList(g);
    move_list.num_moves += rook_moves.num_moves;
    move_list.moves = (uint16_t*)realloc(move_list.moves, 2 * move_list.num_moves);
    memcpy(&move_list.moves[move_list.num_moves - rook_moves.num_moves], rook_moves.moves, 2 * rook_moves.num_moves);

    struct MoveList bishop_moves = BishopMovesList(g);
    move_list.num_moves += bishop_moves.num_moves;
    move_list.moves = (uint16_t*)realloc(move_list.moves, 2 * move_list.num_moves);
    memcpy(&move_list.moves[move_list.num_moves - bishop_moves.num_moves], bishop_moves.moves, 2 * bishop_moves.num_moves);

    struct MoveList knight_moves = KnightMovesList(g);
    move_list.num_moves += knight_moves.num_moves;
    move_list.moves = (uint16_t*)realloc(move_list.moves, 2 * move_list.num_moves);
    memcpy(&move_list.moves[move_list.num_moves - knight_moves.num_moves], knight_moves.moves, 2 * knight_moves.num_moves);

    struct MoveList pawn_moves = PawnMovesList(g);
    move_list.num_moves += pawn_moves.num_moves;
    move_list.moves = (uint16_t*)realloc(move_list.moves, 2 * move_list.num_moves);
    memcpy(&move_list.moves[move_list.num_moves - pawn_moves.num_moves], pawn_moves.moves, 2 * pawn_moves.num_moves);

    return move_list;
}

uint64_t allSquaresAttacking(struct Game* g) {
    uint64_t pieces = (g->pieces_bb[iSTATE] & gameStateTurnMask) ? g->pieces_bb[iWHITE] : g->pieces_bb[iBLACK];
    uint64_t squares_attacking = 0ULL;
    while (pieces) {
        uint8_t square = __builtin_ffsll(pieces) - 1;
        if (g->pieces_bb[iKING] & (1ULL << square)) {
            squares_attacking |= KingAttacksBB(g, square);
        } else if (g->pieces_bb[iQUEEN] & (1ULL << square)) {
            squares_attacking |= QueenAttacksBB(g, square);
        } else if (g->pieces_bb[iROOK] & (1ULL << square)) {
            squares_attacking |= RookAttacksBB(g, square);
        } else if (g->pieces_bb[iBISHOP] & (1ULL << square)) {
            squares_attacking |= BishopAttacksBB(g, square);
        } else if (g->pieces_bb[iKNIGHT] & (1ULL << square)) {
            squares_attacking |= KnightAttacksBB(g, square);
        } else if (g->pieces_bb[iPAWN] & (1ULL << square)) {
            squares_attacking |= PawnAttacksBB(g, square);
        }
        pieces ^= 1ULL << square;
    }

    return squares_attacking;
}

bool InCheck(struct Game* g) {
    uint64_t king_bb = g->pieces_bb[iKING] & ((g->pieces_bb[iSTATE] & gameStateTurnMask) ? g->pieces_bb[iBLACK] : g->pieces_bb[iWHITE]);
    uint64_t squares_attacking = allSquaresAttacking(g);
    return squares_attacking & king_bb;
}

struct MoveList LegalMoves(struct Game* g) {
    struct MoveList legal_moves;
    legal_moves.num_moves = 0;
    legal_moves.moves = (uint16_t*)malloc(0);

    struct MoveList pseudo_legal_moves = PseudoLegalMoves(g);
    for (int m = 0; m < pseudo_legal_moves.num_moves; m++) {
        if ((pseudo_legal_moves.moves[m] & moveFlagMask) >> 12 == castlingMoveFlag) {
            switch ((pseudo_legal_moves.moves[m] & moveEndSquareMask) >> 6) {
                case 6:
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e1f1", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e1g1", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    AddMoveToMoveList(&legal_moves, pseudo_legal_moves.moves[m]);
                    break;
                case 2:
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e1d1", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e1c1", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    AddMoveToMoveList(&legal_moves, pseudo_legal_moves.moves[m]);
                    break;
                case 62:
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e8f8", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e8g8", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    AddMoveToMoveList(&legal_moves, pseudo_legal_moves.moves[m]);
                    break;
                case 58:
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e8d8", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    GameMakeMove(g, ConvertMoveStringFlagToMove("e8c8", noMoveFlag));
                    if (InCheck(g)) {
                        GameUnmakeMove(g);
                        break;
                    }
                    GameUnmakeMove(g);
                    AddMoveToMoveList(&legal_moves, pseudo_legal_moves.moves[m]);
                    break;
            }
        } else {
            GameMakeMove(g, pseudo_legal_moves.moves[m]);
            if (!InCheck(g)) {
                AddMoveToMoveList(&legal_moves, pseudo_legal_moves.moves[m]);
            }
            GameUnmakeMove(g);
        }
    }

    return legal_moves;
}

uint64_t perft(struct Game* g, int depth) {
    if (depth == 1) {
        struct MoveList moves = LegalMoves(g);
        return moves.num_moves;
    } else {
        uint64_t n_moves = 0ULL;
        struct MoveList moves = LegalMoves(g);
        for (int m = 0; m < moves.num_moves; m++) {
            GameMakeMove(g, moves.moves[m]);
            n_moves += perft(g, depth - 1);
            GameUnmakeMove(g);
        }

        return n_moves;
    }
}

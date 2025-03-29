#include "board.h"

uint8_t ConvertSquareStringToSquareIndex(char* sq_str) {
	return 8 * (sq_str[1] - '1') + (sq_str[0] - 'a');
}

uint16_t ConvertStartEndSquareToMove(uint8_t start_sq, uint8_t end_sq) {
	return start_sq | ((uint16_t)end_sq << 6);
}

uint16_t ConvertStartEndSquareFlagToMove(uint8_t start_sq, uint8_t end_sq, uint8_t flag) {
	return start_sq | ((uint16_t)end_sq << 6) | ((uint16_t)flag << 12);
}

uint16_t ConvertStartEndSquareStringFlagToMove(char* start_sq_str, char* end_sq_str, uint8_t flag) {
	return ConvertStartEndSquareFlagToMove(ConvertSquareStringToSquareIndex(start_sq_str), ConvertSquareStringToSquareIndex(end_sq_str), flag);
}

uint16_t ConvertMoveStringFlagToMove(char* move_str, uint8_t flag) {
	char start_sq_str[2] = {move_str[0], move_str[1]};
	char end_sq_str[2] = {move_str[2], move_str[3]};
	return ConvertStartEndSquareStringFlagToMove(start_sq_str, end_sq_str, flag);
}

char* ConvertSquareIndexToSquareString(uint8_t square) {
	char* sq_str = (char*)malloc(3 * sizeof(char));
	sq_str[0] = 'a' + (square % 8);
	sq_str[1] = '1' + (square / 8);
	sq_str[2] = '\0';

	return sq_str;
}

char* ConvertMoveToStringMove(uint16_t move) {
	char* move_str = (char*)malloc(5 * sizeof(char));
	char* start_sq_str = ConvertSquareIndexToSquareString(move & moveStartSquareMask);
	char* end_sq_str = ConvertSquareIndexToSquareString((move & moveEndSquareMask) >> 6);
	move_str[0] = start_sq_str[0];
	move_str[1] = start_sq_str[1];
	move_str[2] = end_sq_str[0];
	move_str[3] = end_sq_str[1];
	move_str[4] = '\0';

	free(start_sq_str);
	free(end_sq_str);
	return move_str;
}

void GameReset(struct Game* g) {
	memset(g->pieces_bb, 0, sizeof(g->pieces_bb));
	g->prev_state = NULL;
}

void GameAddPiece(struct Game* g, uint8_t square, uint8_t piece) {
	switch (piece & pieceValueColorMask) {
		case vWHITE:
			g->pieces_bb[iWHITE] |= 1ULL << square;
			break;
		case vBLACK:
			g->pieces_bb[iBLACK] |= 1ULL << square;
			break;
	}
	
	switch (piece & pieceValueTypeMask) {
		case vKING:
			g->pieces_bb[iKING] |= 1ULL << square;
			break;
		case vQUEEN:
			g->pieces_bb[iQUEEN] |= 1ULL << square;
			break;
		case vROOK:
			g->pieces_bb[iROOK] |= 1ULL << square;
			break;
		case vBISHOP:
			g->pieces_bb[iBISHOP] |= 1ULL << square;
			break;
		case vKNIGHT:
			g->pieces_bb[iKNIGHT] |= 1ULL << square;
			break;
		case vPAWN:
			g->pieces_bb[iPAWN] |= 1ULL << square;
			break;
	}
}

void GameDeletePiece(struct Game* g, uint8_t square) {
	uint64_t square_bb = 1ULL << square;
	
	if (g->pieces_bb[iWHITE] & square_bb) g->pieces_bb[iWHITE] -= square_bb;
	else if (g->pieces_bb[iBLACK] & square_bb) g->pieces_bb[iBLACK] -= square_bb;
	
	if (g->pieces_bb[iKING] & square_bb) g->pieces_bb[iKING] -= square_bb;
	else if (g->pieces_bb[iQUEEN] & square_bb) g->pieces_bb[iQUEEN] -= square_bb;
	else if (g->pieces_bb[iROOK] & square_bb) g->pieces_bb[iROOK] -= square_bb;
	else if (g->pieces_bb[iBISHOP] & square_bb) g->pieces_bb[iBISHOP] -= square_bb;
	else if (g->pieces_bb[iKNIGHT] & square_bb) g->pieces_bb[iKNIGHT] -= square_bb;
	else if (g->pieces_bb[iPAWN] & square_bb) g->pieces_bb[iPAWN] -= square_bb;
}

void GameMovePiece(struct Game* g, uint8_t start_sq, uint8_t end_sq) {
	uint64_t square_bb = 1ULL << start_sq;
	
	GameDeletePiece(g, end_sq);
	
	if (g->pieces_bb[iWHITE] & square_bb) GameAddPiece(g, end_sq, vWHITE);
	else if (g->pieces_bb[iBLACK] & square_bb) GameAddPiece(g, end_sq, vBLACK);
	
	if (g->pieces_bb[iKING] & square_bb) GameAddPiece(g, end_sq, vKING);
	else if (g->pieces_bb[iQUEEN] & square_bb) GameAddPiece(g, end_sq, vQUEEN);
	else if (g->pieces_bb[iROOK] & square_bb) GameAddPiece(g, end_sq, vROOK);
	else if (g->pieces_bb[iBISHOP] & square_bb) GameAddPiece(g, end_sq, vBISHOP);
	else if (g->pieces_bb[iKNIGHT] & square_bb) GameAddPiece(g, end_sq, vKNIGHT);
	else if (g->pieces_bb[iPAWN] & square_bb) GameAddPiece(g, end_sq, vPAWN);
	
	GameDeletePiece(g, start_sq);
}

void GameMakeMove(struct Game* g, uint16_t move) {
	uint8_t startSquare = (move & moveStartSquareMask);
	uint8_t endSquare = (move & moveEndSquareMask) >> 6;
	uint8_t flag = (move & moveFlagMask) >> 12;
	uint64_t startSquareBitboard = 1ULL << startSquare;
	uint64_t endSquareBitboard = 1ULL << endSquare;

	struct Game* prev_game = (struct Game*)malloc(sizeof(struct Game));
	memcpy(prev_game, g, sizeof(struct Game));

	g->prev_state = prev_game;
	
	g->pieces_bb[iSTATE] ^= gameStateTurnMask;
	
	if (startSquare == 4)
		g->pieces_bb[iSTATE] &= ~(gameStateWhiteKingsideCastlingMask | gameStateWhiteQueensideCastlingMask);
	else if (startSquare == 60)
		g->pieces_bb[iSTATE] &= ~(gameStateBlackKingsideCastlingMask | gameStateBlackQueensideCastlingMask);
	
	if (startSquare == 0 || endSquare == 0)
		g->pieces_bb[iSTATE] &= ~gameStateWhiteQueensideCastlingMask;
	else if (startSquare == 7 || endSquare == 7)
		g->pieces_bb[iSTATE] &= ~gameStateWhiteKingsideCastlingMask;
	else if (startSquare == 56 || endSquare == 56)
		g->pieces_bb[iSTATE] &= ~gameStateBlackQueensideCastlingMask;
	else if (startSquare == 63 || endSquare == 63)
		g->pieces_bb[iSTATE] &= ~gameStateBlackKingsideCastlingMask;
	
	if (flag == castlingMoveFlag) {
		if (endSquare == 2)
			GameMovePiece(g, 0, 3);
		else if (endSquare == 6)
			GameMovePiece(g, 7, 5);
		else if (endSquare == 58)
			GameMovePiece(g, 56, 59);
		else
			GameMovePiece(g, 63, 61);
	}
	
	if (flag == enPassantMoveFlag) {
		if (endSquare >= 32)
			GameDeletePiece(g, endSquare - 8);
		else
			GameDeletePiece(g, endSquare + 8);
	}
	
	g->pieces_bb[iSTATE] &= ~gameStateEnpassantMask;
	if (flag == pawnTwoUpMoveFlag) {
		if (endSquare <= 31)
			g->pieces_bb[iSTATE] |= (endSquare - 8) * 0x20ull;
		else
			g->pieces_bb[iSTATE] |= (endSquare + 8) * 0x20ull;
	}
	
	if (startSquareBitboard & g->pieces_bb[iPAWN] || endSquareBitboard & (g->pieces_bb[iWHITE] | g->pieces_bb[iBLACK]))
		g->pieces_bb[iSTATE] &= ~gameStateHalfmovesMask;
	else
		g->pieces_bb[iSTATE] += 1ULL << 11;
	
	if (g->pieces_bb[iSTATE] & gameStateTurnMask)
		g->pieces_bb[iSTATE] += 1ULL << 18;
	
	GameMovePiece(g, startSquare, endSquare);
	
	if (flag == queenPromotionMoveFlag) {
		GameDeletePiece(g, endSquare);
		if (g->pieces_bb[iSTATE] & gameStateTurnMask)
			GameAddPiece(g, endSquare, vBLACK | vQUEEN);
		else
			GameAddPiece(g, endSquare, vWHITE | vQUEEN);
	} else if (flag == rookPromotionMoveFlag) {
		GameDeletePiece(g, endSquare);
		if (g->pieces_bb[iSTATE] & gameStateTurnMask)
			GameAddPiece(g, endSquare, vBLACK | vROOK);
		else
			GameAddPiece(g, endSquare, vWHITE | vROOK);
	} else if (flag == bishopPromotionMoveFlag) {
		GameDeletePiece(g, endSquare);
		if (g->pieces_bb[iSTATE] & gameStateTurnMask)
			GameAddPiece(g, endSquare, vBLACK | vBISHOP);
		else
			GameAddPiece(g, endSquare, vWHITE | vBISHOP);
	} else if (flag == knightPromotionMoveFlag) {
		GameDeletePiece(g, endSquare);
		if (g->pieces_bb[iSTATE] & gameStateTurnMask)
			GameAddPiece(g, endSquare, vBLACK | vKNIGHT);
		else
			GameAddPiece(g, endSquare, vWHITE | vKNIGHT);
	}
}

void GameUnmakeMove(struct Game* g) {
	struct Game* prev_game = g->prev_state;
	memcpy(g, prev_game, sizeof(struct Game));
	free(prev_game);
}

void GameLoadFen(struct Game* g, char* fen) {
	GameReset(g);
	
	char fenCopy[strlen(fen) + 1];
	strcpy(fenCopy, fen);
	
	char* subFenString;
	
	subFenString = strtok(fenCopy, " ");
	
	char* letter = subFenString;
	uint8_t file = 0;
	uint8_t rank = 7;
	
	while (*letter != '\0') {
		if (*letter == '/') {
			file = 0;
			rank--;
		} else if (*letter >= '0' && *letter <= '9') {
			file += *letter - '0';
		} else {
			GameAddPiece(g, rank * 8 + file, (*letter >= 'a') ? vBLACK : vWHITE);
			switch ((*letter <= 'Z') ? (*letter + 'a' - 'A') : *letter) {
				case 'k':
					GameAddPiece(g, rank * 8 + file, vKING);
					break;
				case 'q':
					GameAddPiece(g, rank * 8 + file, vQUEEN);
					break;
				case 'r':
					GameAddPiece(g, rank * 8 + file, vROOK);
					break;
				case 'b':
					GameAddPiece(g, rank * 8 + file, vBISHOP);
					break;
				case 'n':
					GameAddPiece(g, rank * 8 + file, vKNIGHT);
					break;
				case 'p':
					GameAddPiece(g, rank * 8 + file, vPAWN);
					break;
			}
			file++;
		}
		
		letter++;
	}
	
	subFenString = strtok(NULL, " ");
	if (*subFenString == 'w') 
		g->pieces_bb[iSTATE] |= gameStateTurnMask;
	
	subFenString = strtok(NULL, " ");
	letter = subFenString;
	if (*letter != '-') {
		while (*letter != '\0') {
			if (*letter == 'K') g->pieces_bb[iSTATE] |= gameStateWhiteKingsideCastlingMask;
			if (*letter == 'Q') g->pieces_bb[iSTATE] |= gameStateWhiteQueensideCastlingMask;
			if (*letter == 'k') g->pieces_bb[iSTATE] |= gameStateBlackKingsideCastlingMask;
			if (*letter == 'q') g->pieces_bb[iSTATE] |= gameStateBlackQueensideCastlingMask;
			letter++;
		}
	}
	
	subFenString = strtok(NULL, " ");
	if (*subFenString != '-') {
		g->pieces_bb[iSTATE] |= 0x20ull * ConvertSquareStringToSquareIndex(letter);
	}
	
	subFenString = strtok(NULL, " ");
	g->pieces_bb[iSTATE] |= 0x800ull * atoi(subFenString);
	
	subFenString = strtok(NULL, " ");
	g->pieces_bb[iSTATE] |= 0x40000ull * atoi(subFenString);
}

void GamePrintDetails(struct Game* g) {
	printf("WHITE:    %lu\n", g->pieces_bb[iWHITE]);
	printf("BLACK:    %lu\n", g->pieces_bb[iBLACK]);
	printf("KING:     %lu\n", g->pieces_bb[iKING]);
	printf("QUEEN:    %lu\n", g->pieces_bb[iQUEEN]);
	printf("ROOK:     %lu\n", g->pieces_bb[iROOK]);
	printf("BISHOP:   %lu\n", g->pieces_bb[iBISHOP]);
	printf("KNIGHT:   %lu\n", g->pieces_bb[iKNIGHT]);
	printf("PAWN:     %lu\n", g->pieces_bb[iPAWN]);
	printf("TURN:     %llu\n", g->pieces_bb[iSTATE] & gameStateTurnMask);
	printf("WHITE KINGSIDE:  %llu\n", (g->pieces_bb[iSTATE] & gameStateWhiteKingsideCastlingMask) >> 1);
	printf("WHITE QUEENSIDE: %llu\n", (g->pieces_bb[iSTATE] & gameStateWhiteQueensideCastlingMask) >> 2);
	printf("BLACK KINGSIDE:  %llu\n", (g->pieces_bb[iSTATE] & gameStateBlackKingsideCastlingMask) >> 3);
	printf("BLACK QUEENSIDE: %llu\n", (g->pieces_bb[iSTATE] & gameStateBlackQueensideCastlingMask) >> 4);
	printf("EN PASSANT: %llu\n", (g->pieces_bb[iSTATE] & gameStateEnpassantMask) >> 5);
	printf("HALFMOVES:  %llu\n", (g->pieces_bb[iSTATE] & gameStateHalfmovesMask) >> 11);
	printf("FULLMOVES:  %llu\n", (g->pieces_bb[iSTATE] & gameStateFullmovesMask) >> 18);
}

void GameDelete(struct Game* g) {
	if (g->prev_state != NULL) {
		GameDelete(g->prev_state);
		free(g->prev_state);
	}
}
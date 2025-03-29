#include "best_move.h"

float eval(struct Game* g) {
    float eval = 0;
    eval += 9.0f * __builtin_popcountll(g->pieces_bb[iWHITE] & g->pieces_bb[iQUEEN]);
    eval += 5.0f * __builtin_popcountll(g->pieces_bb[iWHITE] & g->pieces_bb[iROOK]);
    eval += 3.0f * __builtin_popcountll(g->pieces_bb[iWHITE] & g->pieces_bb[iBISHOP]);
    eval += 3.0f * __builtin_popcountll(g->pieces_bb[iWHITE] & g->pieces_bb[iKNIGHT]);
    eval += 1.0f * __builtin_popcountll(g->pieces_bb[iWHITE] & g->pieces_bb[iPAWN]);
    eval -= 9.0f * __builtin_popcountll(g->pieces_bb[iBLACK] & g->pieces_bb[iQUEEN]);
    eval -= 5.0f * __builtin_popcountll(g->pieces_bb[iBLACK] & g->pieces_bb[iROOK]);
    eval -= 3.0f * __builtin_popcountll(g->pieces_bb[iBLACK] & g->pieces_bb[iBISHOP]);
    eval -= 3.0f * __builtin_popcountll(g->pieces_bb[iBLACK] & g->pieces_bb[iKNIGHT]);
    eval -= 1.0f * __builtin_popcountll(g->pieces_bb[iBLACK] & g->pieces_bb[iPAWN]);

    return eval;
}

float eval_depth(struct Game* g, float a, float b, int depth) {
    if (depth == 0) return eval(g);

    if (g->pieces_bb[iSTATE] & gameStateTurnMask) {
        float best_eval = -FLT_MAX;
        struct MoveList moves = LegalMoves(g);
        
        if (moves.num_moves == 0) {
            g->pieces_bb[iSTATE] ^= gameStateTurnMask;
            if (InCheck(g)) {
                g->pieces_bb[iSTATE] ^= gameStateTurnMask;
                return -FLT_MAX;
            } else {
                g->pieces_bb[iSTATE] ^= gameStateTurnMask;
                return 0.f;
            }
        }

        float alpha = a;
        for (int m = 0; m < moves.num_moves; m++) {
            GameMakeMove(g, moves.moves[m]);
            float e = eval_depth(g, alpha, b, depth-1);
            if (e > best_eval) best_eval = e;
            if (e > alpha) alpha = e;
            if (alpha >= b) {
                GameUnmakeMove(g);
                break;
            }

            GameUnmakeMove(g);
        }
        return best_eval;
    } else {
        float best_eval = FLT_MAX;
        struct MoveList moves = LegalMoves(g);

        if (moves.num_moves == 0) {
            g->pieces_bb[iSTATE] ^= gameStateTurnMask;
            if (InCheck(g)) {
                g->pieces_bb[iSTATE] ^= gameStateTurnMask;
                return FLT_MAX;
            } else {
                g->pieces_bb[iSTATE] ^= gameStateTurnMask;
                return 0.f;
            }
        }

        float beta = b;
        for (int m = 0; m < moves.num_moves; m++) {
            GameMakeMove(g, moves.moves[m]);
            float e = eval_depth(g, a, beta, depth-1);
            if (e < best_eval) best_eval = e;
            if (e < beta) beta = e;
            if (a >= beta) {
                GameUnmakeMove(g);
                break;
            }

            GameUnmakeMove(g);
        }
        return best_eval;
    }
}

uint16_t bestMove(struct Game* g, int depth) {
    if (g->pieces_bb[iSTATE] & gameStateTurnMask) {
        float best_eval = -FLT_MAX;
        uint16_t best_move = 0;
        struct MoveList moves = LegalMoves(g);
        for (int m = 0; m < moves.num_moves; m++) {
            GameMakeMove(g, moves.moves[m]);
            float e = eval_depth(g, -FLT_MAX, FLT_MAX, depth-1);
            printf("%s: %f\n", ConvertMoveToStringMove(moves.moves[m]), e);
            if (e > best_eval) {
                best_eval = e;
                best_move = moves.moves[m];
            }
            GameUnmakeMove(g);
        }
        return best_move;
    } else {
        float best_eval = FLT_MAX;
        uint16_t best_move = 0;
        struct MoveList moves = LegalMoves(g);
        for (int m = 0; m < moves.num_moves; m++) {
            GameMakeMove(g, moves.moves[m]);
            float e = eval_depth(g, -FLT_MAX, FLT_MAX, depth-1);
            printf("%s: %f\n", ConvertMoveToStringMove(moves.moves[m]), e);
            if (e < best_eval) {
                best_eval = e;
                best_move = moves.moves[m];
            }
            GameUnmakeMove(g);
        }
        return best_move;
    }
}
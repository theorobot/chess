#ifndef BEST_MOVE_H
#define BEST_MOVE_H

#include <float.h>

#include "settings.h"
#include "board.h"
#include "moves.h"

float eval(struct Game* g);
float eval_depth(struct Game* g, float a, float b, int depth);
uint16_t bestMove(struct Game* g, int depth);

#endif
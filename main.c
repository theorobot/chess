#include <stdio.h>
#include <time.h>

#include "board.h"
#include "moves.h"
#include "settings.h"

int main() {
	clock_t start, end;
	double cpu_time_used;

	start = clock();

	initMovesBySquareBBs();

	struct Game g;
	GameLoadFen(&g, startingPositionFen);

	uint64_t perft_moves = perft(&g, 6);
	printf("%llu\n", perft_moves);

	GameDelete(&g);

	end = clock();

	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("Time taken: %f seconds\n", cpu_time_used);

	fflush(stdout);
	
	return 0;
}
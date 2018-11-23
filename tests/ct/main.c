#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#include "test_che_make_moves.h"
#include "external_perft.h"
#include "chester.h"

int test_count, error_count;

int
main()
{
    /*
    test_CHE_MAKE_MOVES_with_pawn_promotions();
    test_CHE_MAKE_MOVES_with_short_game();

    test_CHE_MAKE_MOVES_with_long_game_1();
    test_CHE_MAKE_MOVES_with_long_game_2();
    test_CHE_MAKE_MOVES_with_long_game_3();

    ex_perft("8/8/8/8/8/8/1RP3q1/k6K w - - 19 83", 2, 1);

    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n", test_count);
    */

    // ex_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902);

    printf("London calling");
    for(int i = 1; i <= 10; i++) {
        printf("."), fflush(stdout);
        char *results = che_make_moves(
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            NULL);
        free(results);
    }
    printf("\n");
}

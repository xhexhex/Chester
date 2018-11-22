#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "test_che_make_moves.h"

int test_count, error_count;

int
main()
{
    test_CHE_MAKE_MOVES_with_pawn_promotions();
    test_CHE_MAKE_MOVES_with_short_game();

    test_CHE_MAKE_MOVES_with_long_game_1();
    test_CHE_MAKE_MOVES_with_long_game_2();
    test_CHE_MAKE_MOVES_with_long_game_3();

    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n", test_count);
}

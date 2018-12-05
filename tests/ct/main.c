#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ct_che_make_moves.h"
#include "chester.h"

// Remove!
#include "../../src/base.h"
#include "../../src/utils.h"
#include "../../src/extra.h"
#include "../../src/move_gen.h"

int test_count, error_count;

int
main()
{
    /*
    char ring_verse[] =
        "Three Rings for the Elven-kings under the sky,\n"
        "Seven for the Dwarf-lords in their halls of stone,\n"
        "Nine for Mortal Men doomed to die,\n"
        "One for the Dark Lord on his dark throne\n"
        "In the Land of Mordor where the Shadows lie.\n"
        "One Ring to rule them all, One Ring to find them,\n"
        "One Ring to bring them all, and in the darkness bind them,\n"
        "In the Land of Mordor where the Shadows lie.\n";
    char *unmod_ptr = malloc(strlen(ring_verse) + 1), *copy = unmod_ptr, *line;
    strcpy(copy, ring_verse);

    const int REPS = 1000 * 1000;
    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= REPS; count++) {
        copy = unmod_ptr, strcpy(copy, ring_verse);
        while((line = next_line(&copy))) {
            // printf("Line: \"%s\"\n", line);
        }
        // printf("~\n");
    }
    printf("The operation took %lld ms\n", time_in_milliseconds() - t0);

    free(unmod_ptr);
    */

    /*
    rawcodes_performance_test();
    return 0;
    */

    che_make_moves_tested_with_pawn_promotions();
    che_make_moves_tested_with_short_game();
    che_make_moves_tested_with_long_game_1();
    che_make_moves_tested_with_long_game_2();
    che_make_moves_tested_with_long_game_3();

    ct_perft_v1(FEN_STD_START_POS, 0, 1, false);
    ct_perft_v1(FEN_STD_START_POS, 1, 20, false);
    ct_perft_v1(FEN_STD_START_POS, 2, 400, false);
    ct_perft_v1(FEN_STD_START_POS, 3, 8902, false);
    // ct_perft_v1(FEN_STD_START_POS, 4, 197281, false);
    // ct_perft_v1(FEN_STD_START_POS, 5, 4865609, true);
    // ct_perft_v1(FEN_STD_START_POS, 6, 119060324, true);

    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
}

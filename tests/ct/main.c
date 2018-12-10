#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ct_che_make_moves.h"
#include "chester.h"

// Remove!
#include "../../src/base.h"
#include "../../src/utils.h"
#include "../../src/extra.h"
#include "../../src/move_gen.h"
#include "../../src/pgn.h"

int test_count, error_count;

static void
x_che_perft_with_mt( int first, int last )
{
    assert(first <= last);
    for(int level = first; level <= last; level++) {
        ++test_count;
        printf("Level %i: %lld\n", level, che_perft(INIT_POS, level, true));
    }
}

int
main()
{
    // const Pos *p = fen_to_pos(INIT_POS);

    /*
    const long long expected = 4865609, level = 5;
    printf("Should be %lld: %lld\n", expected, che_perft(INIT_POS, level, true));
    return 0;
    */

    /*
    che_make_moves_tested_with_single_moves();
    che_make_moves_tested_with_short_game();
    che_make_moves_tested_with_long_game_1();
    che_make_moves_tested_with_long_game_2();
    che_make_moves_tested_with_long_game_3();

    ct_perft_v1(INIT_POS, 0, 1, false);
    ct_perft_v1(INIT_POS, 1, 20, false);
    ct_perft_v1(INIT_POS, 2, 400, false);
    ct_perft_v1(INIT_POS, 3, 8902, false);
    ct_perft_v1(INIT_POS, 4, 197281, false);
    */
    // ct_perft_v1(INIT_POS, 5, 4865609, true);
    // ct_perft_v1(INIT_POS, 6, 119060324, true);

    x_che_perft_with_mt(0, 6);

    /*
    ct_perft_v2(p, 0, 1, false);
    ct_perft_v2(p, 1, 20, false);
    ct_perft_v2(p, 2, 400, false);
    ct_perft_v2(p, 3, 8902, false);
    ct_perft_v2(p, 4, 197281, false);
    */
    // ct_perft_v2(p, 5, 4865609, false);
    // ct_perft_v2(p, 6, 119060324, true);
    // ct_perft_v2(p, 7, 3195901860, true);

    // free((void *) p);

    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
}

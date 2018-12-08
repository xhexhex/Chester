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

/*
static void
x_tell_the_moves(const Pos *p)
{
    const Bitboard ONE = 1; // Make global?

    printf("King on %s: ", SNA[glo_orig_k]);
    for(int dest = 0; dest < 64; dest++) {
        if((ONE << dest) & glo_dest_k) {
            // char s[4 + 1];
            // rawmove(ORIG_DEST_RC[glo_orig_k][dest], s);
            // printf("%s ", s);
            Rawcode rc = ORIG_DEST_RC[glo_orig_k][dest];
            Pos copy;
            copy_pos(p, &copy);
            make_move(&copy, rc, '-');
            if(!forsaken_king(&copy)) printf("%s ", SNA[dest]);
        }
    }
    printf("\n");
    if(glo_orig_q1 != -1) {
        printf("There's a queen on %s\n", SNA[glo_orig_q1]);
    }
    if(glo_orig_q2 != -1) {
        printf("There's a queen on %s\n", SNA[glo_orig_q2]);
    }
}
*/

int
main()
{
    // ALKU >>
    // INIT_POS: The operation took 2611 ms, 153 ms
    // FEN_GAME_OF_THE_CENTURY: The operation took 547 ms, 241 ms
    // "q6k/8/8/8/8/8/8/7K b - - 12 34": The operation took 2246 ms, 1170 ms
    // FEN_SUPERPOSITION_1: The operation took 233 ms, 89 ms
    // FEN_SUPERPOSITION_2: The operation took 453 ms, 196 ms
    /*
    const Pos *p = fen_to_pos(FEN_SUPERPOSITION_2);
    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= 1000 * 1000; count++)
        forsaken_king(p);
    printf("The operation took %lld ms\n", time_in_milliseconds() - t0);
    free((void *) p);
    */
    // << LOPPU

    /*
    // Three queens
    const Pos *p = fen_to_pos("4k3/5q2/3q4/1q6/8/8/8/4K3 b - - 12 34");
    // Two legal moves for king
    // const Pos *p = fen_to_pos("3qk3/8/5p2/8/4K3/8/6n1/8 w - - 12 34");
    move_gen(p);
    x_tell_the_moves(p);
    free((void *) p);
    return 0;
    */

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
    // ct_perft_v1(INIT_POS, 5, 4865609, true);
    // ct_perft_v1(INIT_POS, 6, 119060324, true);

    const Pos *p = fen_to_pos(INIT_POS);

    ct_perft_v2(p, 0, 1, false);
    ct_perft_v2(p, 1, 20, false);
    ct_perft_v2(p, 2, 400, false);
    ct_perft_v2(p, 3, 8902, false);
    ct_perft_v2(p, 4, 197281, false);
    ct_perft_v2(p, 5, 4865609, false);
    // ct_perft_v2(p, 6, 119060324, true);
    // ct_perft_v2(p, 7, 3195901860, true);

    free((void *) p);

    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
}

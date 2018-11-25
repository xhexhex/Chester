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

int test_count, error_count;

int
main()
{
    /*
    che_make_moves_tested_with_pawn_promotions();
    che_make_moves_tested_with_short_game();
    che_make_moves_tested_with_long_game_1();
    che_make_moves_tested_with_long_game_2();
    che_make_moves_tested_with_long_game_3();
    */

    // ct_perft_v1(FEN_STD_START_POS, 0, 1, false);
    // ct_perft_v1(FEN_STD_START_POS, 1, 20, false);
    // ct_perft_v1(FEN_STD_START_POS, 2, 400, false);
    // ct_perft_v1(FEN_STD_START_POS, 3, 8902, true);
    // ct_perft_v1(FEN_STD_START_POS, 4, 197281, true);
    // ct_perft_v1(FEN_STD_START_POS, 5, 4865609, true);

    /*
    Pos *p = fen_to_pos(FEN_STD_START_POS);
    for(int i = 1; i <= 5 * 10000; i++) {
        if(i % 10000 == 0) printf("%d\n", i);
        char *fen = pos_to_fen(p);
        assert( !strcmp(fen, FEN_STD_START_POS) );
        free(fen);
    }
    free(p);
    */

    const uint16_t nine_1s = 511; // 256 + 0xffU
    const uint16_t ppa1[] = {nine_1s, 0, 0}, ppa2[] = {174, 257, 80};
    char *ppf;
    long long t1 = time_in_milliseconds(), t2;
    for(int count = 1; count <= 500 * 1000 * 1000; count++) {
        if(count % (10 * 1000 * 1000) == 0) printf("%d\n", count);
        ppf = dyn_ttt_ppa_to_ppf_conv(ppa1), free(ppf);
        ppf = dyn_ttt_ppa_to_ppf_conv(ppa2), free(ppf); }
    t2 = time_in_milliseconds();
    printf("The operation took %lld ms\n", t2 - t1);

    /*
    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
    */
}

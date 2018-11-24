#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ct_che_make_moves.h"
#include "chester.h"
// Remove!
#include "../../src/base.h"

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
    ct_perft_v1(FEN_STD_START_POS, 3, 8902, true);
    // ct_perft_v1(FEN_STD_START_POS, 4, 197281, true);
    // ct_perft_v1(FEN_STD_START_POS, 5, 4865609, true);

    printf("Create an alt implementation of pos_to_fen()!\n");
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

    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
}

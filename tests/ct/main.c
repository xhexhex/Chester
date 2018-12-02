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
    make_move_performance_test();

    /*
    const Pos *p = fen_to_pos(
        FEN_STD_START_POS
        // "r1bqk2r/2ppbppp/p1n2n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 b kq - 1 7"
        // "3b4/P3P3/8/8/8/8/8/K6k w - - 1 123"
        // "b3k3/8/8/8/3P4/8/8/4K2R b K d3 0 9"
        // "4k2r/8/8/8/6Pp/8/8/7K b k g3 0 75"
    );

    Pos copy;

    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= 5 * 1000; count++) {
        copy_pos( p, &copy );
        make_move(&copy, 936, '-');
        // make_move(&copy, 1140, '-');
        // make_move(&copy, 1098, 'q');
        // make_move(&copy, 193, '-');
        // make_move(&copy, 1685, '-');
    }
    printf("The operation took %lld ms\n", time_in_milliseconds() - t0);
    free((void *) p);
    */

    // >>
    // long long t0, /*d1,*/ d2;
    // const int REPS = 1000;

    /*
    t0 = time_in_milliseconds();
    printf("Calling che_move_gen() %d times...\n", REPS);
    for(int count = 1; count <= REPS; count++) {
        if(count % (10) == 0) fprintf(stderr, "-");
        char *sans = che_move_gen(FEN_STD_START_POS);
        free(sans);
    }
    d1 = time_in_milliseconds() - t0;
    printf("\nThe operation took %lld ms\n", d1);
    */

    /*
    const Pos *p = fen_to_pos(FEN_STD_START_POS);
    t0 = time_in_milliseconds();
    printf("Calling rawcodes() %d times...\n", REPS);
    for(int count = 1; count <= REPS; count++) {
        if(count % (10) == 0) fprintf(stderr, "+");
        Rawcode *rc = rawcodes(p);
        free(rc);
    }
    d2 = time_in_milliseconds() - t0;
    printf("\nThe operation took %lld ms\n", d2);
    free((void *) p);
    */

    // printf("Performance ratio: %.2f\n", (double) d1 / d2);
    // <<

    /*
    const uint16_t one = 1;
    char line[30 + 1], tmp[10 + 1];
    for(uint8_t byte = 0; ; byte++) {
        uint8_t bit = (one << 7), bc = 0;
        // for(int i = 0; i < 30; i++) line[i] = '\0';
        // printf("[%3d]  ", byte);
        sprintf(line, "[%3d]  ", byte);
        while(true) {
            sprintf(tmp, "%c ", (bit & byte) ? '1' : '0');
            strcat(line, tmp);
            if(bit & byte) ++bc;
            if(bit == 1) break;
            bit >>= 1;
        }
        if(bc == 2) printf("%s\n", line);

        if(byte == 255) break;
    }
    */

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
    // ct_perft_v1(FEN_STD_START_POS, 3, 8902, false);
    // ct_perft_v1(FEN_STD_START_POS, 4, 197281, true);
    // ct_perft_v1(FEN_STD_START_POS, 5, 4865609, true);
    // ct_perft_v1(FEN_STD_START_POS, 6, 119060324, true);

    /*
    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
    */
}

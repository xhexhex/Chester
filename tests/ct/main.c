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
    // fen_to_pos(FEN_STD_START_POS);
    // fen_to_pos("6k1/r1q1b2n/6QP/p3R3/1p3p2/1P6/1PP2P2/2K4R b - - 1 35");

    const int REPS = 10 * 1000 * 1000;
    long long t0, d1, d2;

    t0 = time_in_milliseconds();
    for(int count = 1; count <= REPS; count++) {
        Pos *p = fen_to_pos(FEN_STD_START_POS);
        free(p); }
    d1 = time_in_milliseconds() - t0;
    printf("The operation took %lld ms\n", d1);

    const Pos *p = fen_to_pos(FEN_STD_START_POS);
    t0 = time_in_milliseconds();
    for(int count = 1; count <= REPS; count++) {
        char *fen = pos_to_fen((Pos *) p);
        free(fen); }
    d2 = time_in_milliseconds() - t0;
    printf("The operation took %lld ms\n", d2);
    free((void *) p);

    printf("Performance ratio: %.2f\n", (double) d1 / d2);
    printf("If ratio < 1, then fen_to_pos() is faster than pos_to_fen()\n");

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

    /* Put in extra.*
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

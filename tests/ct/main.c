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
    const int reps = 80 * 1000;

    long long t1, t2, result_1, result_2;
    Pos *p = fen_to_pos(FEN_STD_START_POS);

    t1 = time_in_milliseconds();
    printf("Calling pos_to_fen() %d times...\n", reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000) == 0) fprintf(stderr, "-");
        char *fen = pos_to_fen(p);
        free(fen); }
    t2 = time_in_milliseconds(), result_1 = t2 - t1;
    printf("\nThe operation took %lld ms\n", result_1);

    t1 = time_in_milliseconds();
    printf("Calling new_pos_to_fen() %d times...\n", reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000) == 0) fprintf(stderr, "+");
        char *fen = new_pos_to_fen(p);
        free(fen); }
    t2 = time_in_milliseconds(), result_2 = t2 - t1;
    printf("\nThe operation took %lld ms\n", result_2);

    free(p);

    printf("Performance ratio: %.2f\n", (double) result_1 / result_2);

    /*
    Pos *p = fen_to_pos(FEN_STD_START_POS);
    char *s;
    for(int count = 1; count <= 1 * 1000 * 1000; count++) {
        s = ecaf(p);
        free(s);
    }
    free(p);
    */

    /*
    Pos *p = fen_to_pos(FEN_STD_START_POS);
    char *fen = new_pos_to_fen(p);
    printf("\"%s\"\n", fen);
    free(p), free(fen);
    */

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

    // https://stackoverflow.com/questions/4864453/associative-arrays-in-c

    /*
    Pos *p = fen_to_pos(FEN_STD_START_POS);
    char *fen = pos_to_fen(p);
    assert(!strcmp(fen, FEN_STD_START_POS));
    free(fen);
    free(p);
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
    // ct_perft_v1(FEN_STD_START_POS, 3, 8902, true);
    // ct_perft_v1(FEN_STD_START_POS, 4, 197281, true);
    // ct_perft_v1(FEN_STD_START_POS, 5, 4865609, true);

    /*
    const int reps = 10000;

    long long t1 = time_in_milliseconds(), t2, result_1, result_2;
    Pos *p = fen_to_pos(FEN_STD_START_POS);
    printf("Calling pos_to_fen() %d times...\n", reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000) == 0) fprintf(stderr, "-");
        char *fen = pos_to_fen(p);
        free(fen); }
    free(p);
    t2 = time_in_milliseconds(), result_1 = t2 - t1;
    printf("\nThe operation took %lld ms\n", result_1);

    t1 = time_in_milliseconds();
    printf("Calling fen_to_pos() %d times...\n", reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000) == 0) fprintf(stderr, "+");
        Pos *p = fen_to_pos(FEN_STD_START_POS);
        // assert(p->ppa[WHITE_KING] == SB.e1);
        // assert(p->ppa[BLACK_ROOK] == (SB.a8 | SB.h8));
        free(p); }
    t2 = time_in_milliseconds(), result_2 = t2 - t1;
    printf("\nThe operation took %lld ms\n", result_2);

    printf("Performance ratio: %.2f\n", (double) result_1 / result_2);

    t1 = time_in_microseconds();
    char *ptr = malloc(1024 * 1024);
    // for(int i = 0; i < 1024 * 1024; i++) ptr[i] = 0;
    free(ptr);
    t2 = time_in_microseconds();
    printf("\nThe thing was done in %lld µs\n", t2 - t1);
    */

    /*
    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
    */
}

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
    char rawmove[4 + 1] = {'\0'}, *ptr = rawmove;
    for(int orig = 0; orig < 64; orig++) {
        printf("{");
        for(int dest = 0; dest < 64; dest++, ptr = rawmove) {
            strcpy(ptr, SNA[orig]);
            ptr += 2;
            strcpy(ptr, SNA[dest]);
            printf("%u", rawcode(rawmove));
            if(dest == 63) printf("},\n");
            else printf(", ");
            assert(str_m_pat(rawmove, "^[a-h][1-8][a-h][1-8]$"));
        }
    }
    */

    // rawcodes_performance_test();

    /*
    // assert(!che_fen_validator("k4r2/8/8/2p5/8/4K3/r7/8 w - c6 0 123"));
    const Pos *p = fen_to_pos(
        // FEN_STD_START_POS
        "k4r2/8/8/2p5/8/4K3/r7/8 w - c6 0 123"
    );
    Rawcode *rc = rawcodes(p);
    free(rc);

    const int REPS = 10 * 1000;
    long long t0 = time_in_milliseconds();

    for(int count = 1; count <= REPS; count++) {
        rc = rawcodes(p);
        free(rc); }
    printf("The operation took %lld ms\n", time_in_milliseconds() - t0);

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

    /*
    ct_perft_v1(FEN_STD_START_POS, 0, 1, false);
    ct_perft_v1(FEN_STD_START_POS, 1, 20, false);
    ct_perft_v1(FEN_STD_START_POS, 2, 400, false);
    ct_perft_v1(FEN_STD_START_POS, 3, 8902, false);
    */
    // ct_perft_v1(FEN_STD_START_POS, 4, 197281, false);
    // ct_perft_v1(FEN_STD_START_POS, 5, 4865609, true);
    // ct_perft_v1(FEN_STD_START_POS, 6, 119060324, true);

    /*
    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
    */
}

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ct_che_make_moves.h"
#include "ct_che_build_fen_gt.h"
#include "chester.h"
// Remove!
#include "../../src/utils.h"

int test_count, error_count;

static void
x_test_che_perft_with_range_of_levels( const char *fen, int first,
    int last, bool mt, const long long *expected )
{
    for(int level = first; level <= last; ++level, ++test_count) {
        long long result = che_perft(fen, level, mt);
        if(expected && result != expected[level]) {
            assert(expected[level]);
            printf("FAIL: %s: Invalid node count for level %d: %lld\n",
                __func__, level, result);
            ++error_count;
            return;
        }
        // printf("Level %i: %lld\n", level, result);
    }
}

int
main()
{
    const char FEN[] = FEN_PERFT_POS_5;
    const int COUNT = 5000;

    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= COUNT; count++) {
        char *fens = che_make_moves(FEN, NULL);
        free(fens); }
    long long t1 = time_in_milliseconds();

    printf("The che_make_moves() test took %lld ms\n", t1 - t0);

    // [ 1] The che_make_moves() test took 15943 ms

    return 0;

    //
    //
    //

    /*
    long long t0 = time_in_milliseconds();
    struct fen_game_tree gt_ip4 = che_build_fen_gt(NULL, 4);
    long long t1 = time_in_milliseconds();
    // 2019-01-17: Building gt_ip4 took 12641 ms
    printf("Building gt_ip4 took %lld ms\n", t1 - t0);

    assert(gt_ip4.nc == 197281 + 8902 + 400 + 20 + 1);
    che_build_fen_gt_tw_fools_mate(gt_ip4);
    che_build_fen_gt_tw_ip4_stats_v1(gt_ip4);

    che_free_fen_gt(gt_ip4);
    return 0;
    */

    //
    //
    //

    const bool quick = true, mt = true; // mt, multithreading
    char test_group_id = 'A';

    // Test group A
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    che_make_moves_tested_with_single_moves();
    che_make_moves_tested_with_short_game();
    che_make_moves_tested_with_long_game_1();
    che_make_moves_tested_with_long_game_2();
    che_make_moves_tested_with_long_game_3();

    // Test group B
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    ct_fen_based_perft(INIT_POS, 0, 1, false);
    ct_fen_based_perft(INIT_POS, 1, 20, false);
    ct_fen_based_perft(INIT_POS, 2, 400, false);
    ct_fen_based_perft(INIT_POS, 3, 8902, false);
    x_test_che_perft_with_range_of_levels(FEN_SUPERPOSITION_1, 0, 5,
        mt, NULL);
    if(!quick) ct_fen_based_perft(INIT_POS, 4, 197281, false);

    // Test group C
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    x_test_che_perft_with_range_of_levels(INIT_POS, 0, quick ? 5 : 6, mt,
        EPR_INIT_POS);

    // Test group D
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    x_test_che_perft_with_range_of_levels(FEN_KIWIPETE, 0, quick ? 4 : 5,
        mt, EPR_KIWIPETE);

    // Test group E
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    x_test_che_perft_with_range_of_levels(FEN_PERFT_POS_3, 0, quick ? 6 : 7,
        mt, EPR_PERFT_POS_3);

    // Test group F
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    x_test_che_perft_with_range_of_levels(FEN_PERFT_POS_4, 0, quick ? 5 : 6,
        mt, EPR_PERFT_POS_4);

    // Test group G
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    x_test_che_perft_with_range_of_levels(FEN_PERFT_POS_5, 0, quick ? 4 : 5,
        mt, EPR_PERFT_POS_5);

    // Test group H
    if(!quick) fprintf(stderr, "%c", test_group_id++);
    x_test_che_perft_with_range_of_levels(FEN_PERFT_POS_6, 0, quick ? 4 : 5,
        mt, EPR_PERFT_POS_6);

    if(!quick) printf("\n");
    if(!error_count)
        printf("All of the %i chester_tester tests succeeded\n",
            test_count);
}

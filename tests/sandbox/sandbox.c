#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "chester.h"
#include "../../src/utils.h"
#include "../../src/move_gen.h"
#include "../../src/pgn.h"

#define NO_EP_INIT_POS \
    "rnbqkbnr/8/pppppppp/8/8/PPPPPPPP/8/RNBQKBNR w KQkq - 0 1"
#define SOME_POSITION "k7/8/8/8/8/P7/8/K7 w - - 0 1"

int
main()
{
    assert( che_fen_validator(SOME_POSITION) == FEN_NO_ERRORS );
    // printf("Hello from the sandbox!\n");
    // long long che_perft( const char *fen, int depth, bool mt );
    // struct fen_game_tree che_build_fen_gt( const char *fen, uint8_t height );
    // for(int depth = 0; depth <= 4; ++depth)
    //    printf( "%lld\n", che_perft(NO_EP_INIT_POS, depth, true) );

    struct fen_game_tree gt = che_build_fen_gt(SOME_POSITION, 3);

    printf("#: %d\n", gt.num_ufen);

    for(uint32_t id = 1; id <= gt.num_ufen; id++) {
        printf("%s\n", gt.ufen[id]);
    }

    // che_free_fen_gt(gt);
}

/**************
 ** Junkyard **
 **************/

    /*
    const Pos *p = fen_to_pos(FEN_KIWIPETE);
    Rawcode *rc = rawcodes(p);
    assert(*rc == 48);

    const int COUNT = 100 * 1000;

    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= COUNT; count++) {
        for(int i = 1; i <= *rc; i++) {
            char *san = rawcode_to_san(p, rc[i], '-');
            // printf("\"%s\"\n", san);
            free(san);
        }
    }
    long long t1 = time_in_milliseconds();
    free((void *) p);

    printf("The rawcode_to_san() test took %lld ms\n", t1 - t0);

    // [ 1] The rawcode_to_san() test took 4915 ms
    // [ 2] The rawcode_to_san() test took 4669 ms
    // [ 3] The rawcode_to_san() test took 3357 ms

    return 0;
    */

    /*
    const char FEN[] = FEN_PERFT_POS_5;
    const int COUNT = 10 * 1000;

    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= COUNT; count++) {
        char *moves = single_fen_move_gen(FEN);
        free(moves); }
    long long t1 = time_in_milliseconds();

    printf("The single_fen_move_gen() test took %lld ms\n", t1 - t0);

    // [ 1] The single_fen_move_gen() test took 17025 ms
    // [ 2] The single_fen_move_gen() test took 802 ms
    // [ 3] The single_fen_move_gen() test took 641 ms

    return 0;
    */

    //
    //
    //

    /*
    const char FENS[] =
        INIT_POS "\n"
        FEN_KIWIPETE "\n"
        FEN_PERFT_POS_3 "\n"
        FEN_PERFT_POS_5 "\n"
        FEN_PERFT_POS_6 "\n"
        INIT_POS "\n"
        FEN_KIWIPETE "\n"
        FEN_PERFT_POS_3 "\n"
        FEN_PERFT_POS_5 "\n"
        FEN_PERFT_POS_6 "\n";
    const int COUNT = 2000;

    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= COUNT; count++) {
        char *san_list = che_move_gen(FENS);
        free(san_list); }
    long long t1 = time_in_milliseconds();

    printf("The che_move_gen() test took %lld ms\n", t1 - t0);

    // [ 1] The che_move_gen() test took 21518 ms
    // [ 2] The che_move_gen() test took 1546 ms
    // [ 3] The che_move_gen() test took 1328 ms

    return 0;
    */

    //
    //
    //

    /*
    const char FEN[] = FEN_PERFT_POS_5;
    const int COUNT = 5000;

    long long t0 = time_in_milliseconds();
    for(int count = 1; count <= COUNT; count++) {
        char *fens = che_make_moves(FEN, NULL);
        free(fens); }
    long long t1 = time_in_milliseconds();

    printf("The che_make_moves() test took %lld ms\n", t1 - t0);

    // [ 1] The che_make_moves() test took 15943 ms
    // [ 2] The che_make_moves() test took 7333 ms
    // (The rawcode_to_san() optimization didn't help here)

    // Remember to look at san_to_rawcode()!

    return 0;
    */

    //
    //
    //

    /*
    long long t0 = time_in_milliseconds();
    struct fen_game_tree gt_ip5 = che_build_fen_gt(NULL, 5);
    long long t1 = time_in_milliseconds();
    printf("Building gt_ip5 took %lld ms\n", t1 - t0);

    // [ 1] Building gt_ip5 took 187782 ms
    // [ 2] Building gt_ip5 took 185058 ms
    // [ 3] Building gt_ip5 took  19175 ms
    // [ 4] Building gt_ip5 took  15377 ms

    assert(gt_ip5.nc == 4865609 + 197281 + 8902 + 400 + 20 + 1);

    che_free_fen_gt(gt_ip5);
    return 0;
    */

    //
    //
    //

    /*
    long long t0 = time_in_milliseconds();
    struct fen_game_tree gt_ip4 = che_build_fen_gt(NULL, 4);
    long long t1 = time_in_milliseconds();
    // 2019-01-17: Building gt_ip4 took 12641 ms
    // 2019-01-17: Building gt_ip4 took 6594 ms
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

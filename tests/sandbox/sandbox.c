#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chester.h"
#include "../../src/extra.h"
#include "../../src/utils.h"
#include "../../src/move_gen.h"
#include "../../src/pgn.h"

struct fen_game_tree gt;
uint32_t visit_count, checkmate_count;

void
dfs(uint32_t node)
{
    ++visit_count;
    if(!gt.cc[node]) {
        char *fen = gt.ufen[gt.findex[node]];
        const Pos *p = fen_to_pos(fen);
        if(king_in_check(p)) ++checkmate_count;
        free((void *) p); }

    if(node >= gt.lo[gt.height])
        return;

    for(int i = 0; i < gt.cc[node]; ++i)
        dfs(gt.children[node][i]);
}

int
main()
{
    // assert(!che_fen_validator(FEN_PERFT_POS_4));
    printf("Working very hard...\n");

    long long t0 = time_in_milliseconds();
    // gt = che_build_fen_gt(FEN_PERFT_POS_4, 5);
    gt = che_build_fen_gt(NULL, 5);
    long long t1 = time_in_milliseconds();
    printf("Building gt took %lld ms\n", t1 - t0);

    long long t2 = time_in_milliseconds();
    dfs(1);
    long long t3 = time_in_milliseconds();
    printf("Doing the DFS took %lld ms\n", t3 - t2);

    assert(visit_count == gt.nc);
    /*
    assert(visit_count == 1 + 6 + 264 + 9467 + 422333 + 15833292);
    assert(checkmate_count == 22 + 5 + 50562);
    */
    assert(visit_count == 1 + 20 + 400 + 8902 + 197281 + 4865609);
    assert(checkmate_count == 8 + 347);
    printf("Nodes: %u\n", gt.nc);
    printf("Checkmates: %u\n", checkmate_count);
    printf("gt.num_ufen: %u\n", gt.num_ufen);

    che_free_fen_gt(gt);
}

/**************
 ** Junkyard **
 **************/

    // #define MATE_IN_3 "r1b1k2r/ppp2ppp/2p5/8/3Qn1q1/8/PPPB2PP/2KR1B1R w kq - 0 1"

    /*
    char *sta_fen[] = {
        INIT_POS,
        "4k3/8/8/8/4Pp2/8/8/4K3 b - e3 0 1",
        "4k3/8/8/pP6/8/8/8/4K3 w - a6 0 2",
        FEN_PERSONALLY_SIGNIFICANT,
        FEN_GAME_OF_THE_CENTURY,
        "4kr2/8/8/3PpP2/8/5K2/8/8 w - e6 0 2",
        "b3k3/8/8/3PpP2/8/5K2/8/8 w - e6 0 2",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "8/8/8/6k1/6pP/8/8/4K3 b - h3 0 123",
        "4k3/8/8/4pP2/8/8/8/4K3 w - e6 0 2",
        "4k3/8/8/8/4P3/8/8/4K3 b - e3 0 1",
        "5k2/8/8/8/4Pp2/8/8/4KR2 b - e3 0 1",
        "b3kr2/8/8/3PpP2/8/5K2/8/8 w - e6 0 2",
        "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1",
        "4k3/8/8/1pp5/1P6/p7/8/4K2R b K b3 0 1",
        "4k3/8/6P1/6Kp/6P1/8/8/8 w - h6 0 2",
        "8/8/4k3/8/4pPp1/7Q/6K1/4R3 b - f3 0 1",
        NULL };

    char **dyn_fen = malloc(1 * sizeof(void *));
    for(int i = 0; sta_fen[i]; ++i) {
        *dyn_fen = malloc((strlen(sta_fen[i]) + 1) * sizeof(char));
        strcpy(*dyn_fen, sta_fen[i]);
        // printf("%2d: \"%s\"\n", i, *dyn_fen);
        size_t num_removed = che_remove_redundant_epts(dyn_fen, 1);
        printf("Return: %lu\n", num_removed);
        free(*dyn_fen);
    }
    free(dyn_fen);
    */

    /*
    #define NO_EP_INIT_POS \
        "rnbqkbnr/8/pppppppp/8/8/PPPPPPPP/8/RNBQKBNR w KQkq - 0 1"
    #define SOME_POSITION "k7/8/8/8/8/P7/8/K7 w - - 0 1"
    */

    /*
    assert( che_fen_validator(SOME_POSITION) == FEN_NO_ERRORS );
    // printf("Hello from the sandbox!\n");
    // long long che_perft( const char *fen, int depth, bool mt );
    // struct fen_game_tree che_build_fen_gt( const char *fen, uint8_t height );
    // for(int depth = 0; depth <= 4; ++depth)
    //    printf( "%lld\n", che_perft(NO_EP_INIT_POS, depth, true) );

    struct fen_game_tree gt = che_build_fen_gt(SOME_POSITION, 3);

    // printf("#: %d\n", gt.num_ufen);

    for(uint32_t id = 1; id <= gt.nc; id++) {
        // printf("%s\n", gt.ufen[gt.findex[id]]);
        printf("%u\n", gt.findex[id]);
    }

    // che_free_fen_gt(gt);
    */

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

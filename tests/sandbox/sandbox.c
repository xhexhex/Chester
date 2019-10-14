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

int
main()
{
    struct explicit_game_tree gt =
        che_build_explicit_gt(FEN_PERFT_POS_4, 5, false);
    che_free_explicit_gt(gt);
    /*
    // long long t0 = time_in_milliseconds(), t1;
    struct explicit_game_tree gt =
        che_build_explicit_gt(FEN_KIWIPETE, 3, false);
    // t1 = time_in_milliseconds();
    // printf("Building the tree took %lld ms\n", t1 - t0);
    assert(gt.nc == 1 + 48 + 2039 + 97862);

    uint32_t captures, en_passants, castles, promotions, checks, checkmates;
    assert(gt.nc == che_explicit_gt_stats(gt,
        &captures, &en_passants, &castles, &promotions, &checks, &checkmates));

    assert(captures == 0 + 8 + 351 + 17102);
    assert(en_passants == 0 + 0 + 1 + 45);
    assert(castles == 0 + 2 + 91 + 3162);
    assert(promotions == 0 + 0 + 0 + 0);
    assert(checks == 0 + 0 + 3 + 993);
    assert(checkmates == 0 + 0 + 0 + 1);

    che_free_explicit_gt(gt);
    */
}

/**************
 ** Junkyard **
 **************/

/*
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
*/

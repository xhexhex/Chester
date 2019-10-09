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
    // assert(!che_fen_validator("4k2r/7P/8/8/8/8/8/4K3 b k - 12 34"));

    struct explicit_game_tree gt = che_build_explicit_gt(
        "4k2r/7P/8/8/8/8/8/4K3 b k - 12 34", 2);

    // struct explicit_game_tree gt = che_build_explicit_gt(FEN_PERFT_POS_4, 2);
    // assert(gt.nc == 1 + 6 + 264);

    uint32_t captures, en_passants, castles, promotions, checks, checkmates;
    assert(gt.nc == che_explicit_gt_stats(gt,
        &captures, &en_passants, &castles, &promotions, &checks, &checkmates));

    printf("castles: %u\n", castles);
    assert(castles == 0);

    che_free_explicit_gt(gt);
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

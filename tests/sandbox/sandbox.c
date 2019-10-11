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
    const char *fen = "4k2r/8/8/8/3B4/8/4pPPP/r5K1 w k - 12 34";
    struct explicit_game_tree gt = che_build_explicit_gt(fen, 2);
    uint32_t capture_count, check_count;
    uint32_t node_count = che_explicit_gt_stats(
        gt, &capture_count, NULL, NULL, NULL, &check_count, NULL);
    printf("%u  %u  %u\n", node_count, capture_count, check_count);
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

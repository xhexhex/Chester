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
    struct explicit_game_tree gt = che_build_explicit_gt(FEN_KIWIPETE, 3);
    che_free_explicit_gt(gt);

    //
    // Do a Valgrind at some point on che_build_explicit_gt().
    //
    /*
    long long t0 = time_in_milliseconds(), t1;
    struct explicit_game_tree gt = che_build_explicit_gt(FEN_KIWIPETE, 4);
    t1 = time_in_milliseconds();
    printf("Building the tree took %lld ms\n", t1 - t0);
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

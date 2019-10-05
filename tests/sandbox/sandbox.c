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
    printf("%s\n", INIT_POS);

    char ca[9];
    nth_ppf_rank(INIT_POS, 1, ca);
    printf("\"%s\"\n", ca);
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

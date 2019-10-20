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
    const char fen[] = "7k/4P3/8/8/8/8/8/K7 w - - 12 34";
    char *children = che_children(fen);
    printf("%s", children);

    /*
    naive_bst_for_che_children = init_naive_bst(1);
    char *children;
    for(int count = 1; count <= 10 * 1000; ++count) {
        children = che_children(FEN_KIWIPETE);
        // children = che_make_moves(FEN_KIWIPETE, NULL);
        free(children); }
    destroy_naive_bst(naive_bst_for_che_children);
    naive_bst_for_che_children = NULL;

    naive_bst_for_che_children = NULL;
    struct explicit_game_tree gt =
        che_build_explicit_gt(FEN_KIWIPETE, 4, false);
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

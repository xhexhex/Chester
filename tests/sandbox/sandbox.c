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
    const uint32_t NODE_LIMIT = 4;

    struct naive_binary_search_tree *nbst = init_naive_bst(NODE_LIMIT);
    insert_into_naive_bst(nbst, "Antti", "1");
    insert_into_naive_bst(nbst, "Bertta", "2");
    insert_into_naive_bst(nbst, "Cecilia", "3");
    insert_into_naive_bst(nbst, "Daavid", "4");
    // insert_into_naive_bst(nbst, "Pekka", "5" );

    destroy_naive_bst(nbst);
    printf("Done!\n");
    return 0;
    printf("Still here!\n");
    /*
    const uint32_t NODE_LIMIT = 10;

    struct naive_binary_search_tree nbst = che_init_naive_bst(NODE_LIMIT);
    che_insert_into_naive_bst(&nbst, "mahdoton", "1");
    che_insert_into_naive_bst(&nbst, "rankka", "2");
    che_insert_into_naive_bst(&nbst, "painava", "3");
    che_insert_into_naive_bst(&nbst, "pakkaus", "4");
    che_insert_into_naive_bst(&nbst, "aamu", "5");
    che_insert_into_naive_bst(&nbst, "paino", "6");
    che_insert_into_naive_bst(&nbst, "aakkonen", "7");
    che_insert_into_naive_bst(&nbst, "paini", "8");
    che_insert_into_naive_bst(&nbst, "lumi", "9");
    che_insert_into_naive_bst(&nbst, "paita", "10");

    printf("\"%s\"\n", nbst.root->right->left->right->left->right->key);

    che_destroy_naive_bst(nbst);
    */

    naive_bst_for_che_children = NULL;
    struct explicit_game_tree gt =
        che_build_explicit_gt(FEN_KIWIPETE, 4, false);
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

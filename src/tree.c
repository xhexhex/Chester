#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "chester.h"
#include "utils.h"
#include "move_gen.h"

static void x_che_build_fen_gt_set_findex(struct fen_game_tree *gt,
    char **id_to_fen);

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// TODO: doc
struct fen_game_tree
che_build_fen_gt( const char *fen, const uint8_t height )
{
    // What if 'fen' is a stalemate position?
    // List of terminal nodes?

    assert(height < FGT_LO_SIZE);
    if(!fen) fen = INIT_POS;

    struct fen_game_tree gt;

    gt.nc = 0, gt.lo[0] = 1, gt.height = UINT8_MAX; // gt.height = height;
    long long nodes_on_level;
    for(int level = 0; level <= height; ++level) {
        if(!(nodes_on_level = che_perft(fen, level, true)))
            break;
        ++gt.height, gt.lo[level] = gt.nc + 1, gt.nc += nodes_on_level; }
    // if(nodes_on_level)
    //    gt.nc += che_perft(fen, height, true);
    // printf("The two heights: %u %u\n", height, gt.height);
    // printf("gt.nc: %u\n", gt.nc);

    assert((gt.cc = malloc((gt.nc + 1) * sizeof(uint8_t))));
    gt.cc[0] = UINT8_MAX;
    memset((void *) (gt.cc + 1), 0, (size_t) gt.nc);
    // for(uint32_t id = 1; id <= gt.nc; id++) gt.cc[id] = 0;


    // BHNC, below height node count
    const uint32_t BHNC = gt.lo[gt.height] - 1;
    // printf("BHNC: %u\n", BHNC);
    assert((gt.children = malloc((BHNC + 1) * sizeof(void *))));
    const int SLOT_COUNT = 50;
    uint8_t *num_alloc_slots = malloc((BHNC + 1) * sizeof(uint8_t));
    for(uint32_t id = 1; id <= BHNC; id++) {
        assert((gt.children[id] = malloc(SLOT_COUNT * sizeof(uint32_t))));
        num_alloc_slots[id] = SLOT_COUNT; }

    assert((gt.parent = malloc((gt.nc + 1) * sizeof(uint32_t))));
    gt.parent[1] = 0;

    char **id_to_fen, **sorted_id_to_fen; // Free!
    assert((id_to_fen = malloc((gt.nc + 1) * sizeof(void *))));
    assert((id_to_fen[1] = malloc(strlen(fen) + 1)));
    strcpy(id_to_fen[1], fen);

    uint32_t cur = 1, vac = 1; // current, vacant
    for(; cur < gt.lo[gt.height]; cur++) {
        char *unmod_ptr = che_children(id_to_fen[cur]),
            *children = unmod_ptr, *child;

        while((child = next_line(&children))) {
            // 'vac' is a child of 'cur'
            assert((id_to_fen[++vac] = malloc(strlen(child) + 1)));
            strcpy(id_to_fen[vac], child);

            if(gt.cc[cur] == num_alloc_slots[cur]) {
                num_alloc_slots[cur] += SLOT_COUNT;
                gt.children[cur] = realloc(gt.children[cur],
                    num_alloc_slots[cur] * sizeof(uint32_t)); }

            gt.parent[vac] = cur, gt.children[cur][gt.cc[cur]] = vac;
            ++gt.cc[cur]; }

        free(unmod_ptr);
    } // End for

    // printf("\"%s\"\n", *(id_to_fen + 1));
    // printf("Hello...\n");
    che_remove_redundant_epts(id_to_fen + 1, gt.nc);
    // printf("...there!\n");

    /*
    printf("BEGIN TMP_FEN >>\n");
    for(uint32_t id = 1; id <= gt.nc; id++) printf("%s\n", id_to_fen[id]);
    printf("<< END TMP_FEN\n");
    */

    assert((sorted_id_to_fen = malloc((gt.nc + 1) * sizeof(void *))));
    sorted_id_to_fen[0] = "";
    for(uint32_t id = 1; id <= gt.nc; ++id) {
        sorted_id_to_fen[id] = id_to_fen[id];
        /*
        sorted_id_to_fen[id] = malloc((strlen(id_to_fen[id]) + 1) * sizeof(char));
        strcpy(sorted_id_to_fen[id], id_to_fen[id]);
        */
    }
    string_sort(sorted_id_to_fen + 1, gt.nc);
    size_t size = gt.nc + 1;
    gt.ufen = unique_strings(sorted_id_to_fen, &size);
    gt.num_ufen = size - 1;
    assert(!strcmp("", gt.ufen[0]));

    // for(uint32_t id = 1; id <= gt.num_ufen; ++id) printf("%s\n", gt.ufen[id]);
    x_che_build_fen_gt_set_findex(&gt, id_to_fen);
    for(uint32_t id = 1; id <= gt.nc; ++id)
        free(id_to_fen[id]); // free(sorted_id_to_fen[id]);
    free(id_to_fen), free(sorted_id_to_fen);

    // const uint32_t high_node_count = gt.nc - BHNC;
    uint8_t *move_count = malloc((gt.nc + 1) * sizeof(uint8_t));
    memset(move_count + 1, UINT8_MAX, gt.nc);
    for(uint32_t id = gt.lo[gt.height]; id <= gt.nc; ++id) {
        uint32_t index = gt.findex[id];
        // assert(index < high_node_count);

        if(move_count[index] != UINT8_MAX) {
            gt.cc[id] = move_count[index];
            continue; }

        Pos *p = fen_to_pos(gt.ufen[index]);
        Rawcode *rc = rawcodes(p);
        gt.cc[id] = rc[0], move_count[index] = rc[0];
        free(p), free(rc);
    }

    free(move_count);

    for(uint32_t id = 1; id <= BHNC; id++)
        gt.children[id] = realloc(gt.children[id],
            gt.cc[id] * sizeof(uint32_t));
    free(num_alloc_slots);

    return gt;
}

// TODO: doc
void
che_free_fen_gt( struct fen_game_tree gt )
{
/*
struct fen_game_tree {
    char **ufen; // ufen, unique FENs
    uint8_t height, *cc; // cc, child count
    // nc, node count; lo, level offset
    uint32_t nc, num_ufen, lo[FGT_LO_SIZE], *parent, **children,
        *findex;
};
 */

    for(uint32_t j = 0; j <= gt.num_ufen; ++j)
        free(gt.ufen[j]);
    free(gt.ufen), free(gt.cc), free(gt.parent);

    for(uint32_t id = 1; id < gt.lo[gt.height]; id++)
        free(gt.children[id]);
    free(gt.children), free(gt.findex);
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

// findex, FEN index
static void
x_che_build_fen_gt_set_findex(struct fen_game_tree *gt, char **id_to_fen)
{
    assert((gt->findex = malloc((gt->nc + 1) * sizeof(uint32_t) )));
    for(uint32_t id = 0; id <= gt->nc; ++id)
        gt->findex[id] = 0;
    // memset!

    for(uint32_t id = 1; id <= gt->nc; ++id) {
        // printf("Looking for \"%s\"\n", id_to_fen[id]);
        uint32_t left = 1, right = gt->num_ufen;

        // Binary search algorithm
        while(left <= right) {
            uint32_t middle = (left + right) / 2;
            // printf("%u\n", middle);
            assert( middle >= 1 && middle <= gt->num_ufen );

            if(strcmp(gt->ufen[middle], id_to_fen[id]) < 0) {
                left = middle + 1;
                continue;
            } else if(strcmp(gt->ufen[middle], id_to_fen[id]) > 0) {
                right = middle - 1;
                continue;
            }

            gt->findex[id] = middle;
            break;
        } // End while

        if(!gt->findex[id]) assert(false);
    } // End for
}

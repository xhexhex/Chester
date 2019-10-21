#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tree.h"
#include "chester.h"
#include "utils.h"
#include "move_gen.h"

static void x_che_build_explicit_gt_set_findex(struct explicit_game_tree *gt,
    char **id_to_fen);
static void x_che_explicit_gt_stats_dfs( uint32_t node );
static void x_che_build_explicit_gt_set_highest_level_child_counts(
    struct explicit_game_tree *gt );

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// TODO: doc
#define SUFFICIENT_AMOUNT_OF_NODES (20 * 1000 * 1000)
struct explicit_game_tree
che_build_explicit_gt( const char *fen, const uint8_t height,
    bool set_highest_level_child_counts, bool use_naive_bst )
{
    if(!fen) fen = INIT_POS;

    struct explicit_game_tree gt;
    gt.nc = 0, gt.lo[0] = 1, gt.height = UINT8_MAX;
    long long nodes_on_level;
    for(int level = 0; level <= height; ++level) {
        if(!(nodes_on_level = che_perft(fen, level, true))) break;
        ++gt.height, gt.lo[level] = gt.nc + 1, gt.nc += nodes_on_level; }

    assert((gt.cc = malloc((gt.nc + 1) * sizeof(uint8_t))));
    memset((void *) (gt.cc + 1), 0, (size_t) gt.nc);

    // BHNC, below height node count
    const uint32_t BHNC = gt.lo[gt.height] - 1;
    assert((gt.children = malloc((BHNC + 1) * sizeof(void *))));
    const int SLOT_COUNT = 50;
    uint8_t *num_alloc_slots = malloc((BHNC + 1) * sizeof(uint8_t));
    for(uint32_t id = 1; id <= BHNC; id++) {
        assert((gt.children[id] = malloc(SLOT_COUNT * sizeof(uint32_t))));
        num_alloc_slots[id] = SLOT_COUNT; }

    assert((gt.parent = malloc((gt.nc + 1) * sizeof(uint32_t))));
    gt.parent[1] = 0;

    char **id_to_fen, **sorted_id_to_fen;
    assert((id_to_fen = malloc((gt.nc + 1) * sizeof(void *))));
    assert((id_to_fen[1] = malloc(strlen(fen) + 1)));
    strcpy(id_to_fen[1], fen);

    assert(!naive_bst_for_che_children);
    if(use_naive_bst)
        naive_bst_for_che_children =
            init_naive_bst(SUFFICIENT_AMOUNT_OF_NODES);

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

    if(naive_bst_for_che_children) {
        destroy_naive_bst(naive_bst_for_che_children);
        naive_bst_for_che_children = NULL; }

    che_remove_redundant_epts(id_to_fen + 1, gt.nc);

    assert((sorted_id_to_fen = malloc((gt.nc + 1) * sizeof(void *))));
    sorted_id_to_fen[0] = "";
    for(uint32_t id = 1; id <= gt.nc; ++id)
        sorted_id_to_fen[id] = id_to_fen[id];
    string_sort(sorted_id_to_fen + 1, gt.nc);
    size_t size = gt.nc + 1;
    gt.ufen = unique_strings(sorted_id_to_fen, &size);
    gt.num_ufen = size - 1;

    x_che_build_explicit_gt_set_findex(&gt, id_to_fen);
    for(uint32_t id = 1; id <= gt.nc; ++id)
        free(id_to_fen[id]);
    free(id_to_fen), free(sorted_id_to_fen);

    if(set_highest_level_child_counts)
        x_che_build_explicit_gt_set_highest_level_child_counts(&gt);

    for(uint32_t id = 1; id <= BHNC; id++)
        gt.children[id] = realloc(gt.children[id],
            gt.cc[id] * sizeof(uint32_t));
    free(num_alloc_slots);

    return gt;
}
#undef SUFFICIENT_AMOUNT_OF_NODES

// TODO: doc
void
che_free_explicit_gt( struct explicit_game_tree gt )
{
    for(uint32_t j = 0; j <= gt.num_ufen; ++j)
        free(gt.ufen[j]);
    free(gt.ufen), free(gt.cc), free(gt.parent);

    for(uint32_t id = 1; id < gt.lo[gt.height]; id++)
        free(gt.children[id]);
    free(gt.children), free(gt.findex);
}

static uint32_t x_nodes, x_captures, x_en_passants, x_castles, x_proms,
    x_checks, x_checkmates;
static struct explicit_game_tree x_gt;

// The function is used to obtain statistical information about the chess
// game tree represented by 'gt'. This stat info is similar to what can be
// found on the web page https://www.chessprogramming.org/Perft_Results.
//
// We'll use the position
//
//     const char *fen = "4k2r/8/8/8/3B4/8/4pPPP/r5K1 w k - 12 34"
//
// as an example. The first three lines of the stat info for 'fen' is
// as follows.
//
// | Depth | Nodes | Captures | EPs | Castles | Proms | Checks | Checkmates |
// +------------------------------------------------------------------------+
// |     0 |     1 |        0 |   0 |       0 |     0 |      1 |          0 |
// |     1 |     1 |        1 |   0 |       0 |     0 |      0 |          0 |
// |     2 |    18 |        1 |   0 |       1 |     4 |      2 |          2 |
//
// Let's consider the code fragment
//
//     struct explicit_game_tree gt = che_build_explicit_gt(fen, 2);
//     uint32_t capture_count, check_count;
//     uint32_t node_count = che_explicit_gt_stats(
//         gt, &capture_count, NULL, NULL, NULL, &check_count, NULL);
//
// After the second function call the three uint32_t variables will
// have the following values:
// > node_count    = 1 + 1 + 18 = 20
// > capture_count = 0 + 1 +  1 =  2
// > check_count   = 1 + 0 +  2 =  3
//
// As can be observed from the example, the return value of the function
// is the number of nodes in the chess game tree represented by 'gt'.
uint32_t
che_explicit_gt_stats( struct explicit_game_tree gt, uint32_t *captures,
    uint32_t *en_passants, uint32_t *castles, uint32_t *proms,
    uint32_t *checks, uint32_t *checkmates )
{
    x_gt = gt;

    x_nodes = 0;
    x_captures = captures ? 0 : UINT32_MAX;
    x_en_passants = en_passants ? 0 : UINT32_MAX;
    x_castles = castles ? 0 : UINT32_MAX;
    x_proms = proms ? 0 : UINT32_MAX;
    x_checks = checks ? 0 : UINT32_MAX;
    x_checkmates = checkmates ? 0 : UINT32_MAX;

    x_che_explicit_gt_stats_dfs(1);

    if(captures) *captures = x_captures;
    if(en_passants) *en_passants = x_en_passants;
    if(castles) *castles = x_castles;
    if(proms) *proms = x_proms;
    if(checks) *checks = x_checks;
    if(checkmates) *checkmates = x_checkmates;

    return x_nodes;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

// findex, FEN index
static void
x_che_build_explicit_gt_set_findex( struct explicit_game_tree *gt,
    char **id_to_fen )
{
    assert((gt->findex = calloc(gt->nc + 1, sizeof(uint32_t))));

    for(uint32_t id = 1; id <= gt->nc; ++id) {
        uint32_t left = 1, right = gt->num_ufen;

        // Binary search algorithm
        while(left <= right) {
            uint32_t middle = (left + right) / 2;
            assert(middle >= 1 && middle <= gt->num_ufen);

            if(strcmp(gt->ufen[middle], id_to_fen[id]) < 0) {
                left = middle + 1;
                continue; }
            else if(strcmp(gt->ufen[middle], id_to_fen[id]) > 0) {
                right = middle - 1;
                continue; }

            gt->findex[id] = middle;
            break;
        } // End while

        if(!gt->findex[id]) assert(false);
    } // End for
}

static void
x_che_explicit_gt_stats_dfs( uint32_t node )
{
    ++x_nodes;

    const char *fen = x_gt.ufen[x_gt.findex[node]];
    const Pos *p = fen_to_pos(fen);
    const Rawcode *rc = rawcodes(p);
    const bool node_is_at_max_level = (node >= x_gt.lo[x_gt.height]);

    if(king_in_check(p)) {
        if(x_checks != UINT32_MAX) ++x_checks;
        if(x_checkmates != UINT32_MAX && !rc[0]) ++x_checkmates; }

    if(node_is_at_max_level) {
        free((void *) p), free((void *) rc);
        return; }

    if(x_captures != UINT32_MAX)
        for(int i = 1; i <= *rc; ++i)
            if(is_capture(p, rc[i]))
                x_captures += is_promotion(p, rc[i]) ? 4 : 1;
    if(x_en_passants != UINT32_MAX)
        for(int i = 1; i <= *rc; ++i)
            if(is_en_passant_capture(p, rc[i]))
                ++x_en_passants;
    if(x_castles != UINT32_MAX) {
        // kingside/queenside castling move
        Rawcode kcm = castle(p, "kingside"), qcm = castle(p, "queenside");
        Pos after_castling;
        if(kcm) {
            copy_pos(p, &after_castling);
            make_move(&after_castling, kcm, '-');
            if(!forsaken_king(&after_castling)) ++x_castles; }
        if(qcm) {
            copy_pos(p, &after_castling);
            make_move(&after_castling, qcm, '-');
            if(!forsaken_king(&after_castling)) ++x_castles; } }
    if(x_proms != UINT32_MAX)
        for(int i = 1; i <= *rc; ++i)
            if(is_promotion(p, rc[i]))
                x_proms += 4;

    free((void *) p), free((void *) rc);

    for(int i = 0; i < x_gt.cc[node]; ++i)
        x_che_explicit_gt_stats_dfs(x_gt.children[node][i]);
}

static void
x_che_build_explicit_gt_set_highest_level_child_counts(
    struct explicit_game_tree *gt )
{
    uint8_t *move_count = malloc((gt->nc + 1) * sizeof(uint8_t));
    memset(move_count + 1, UINT8_MAX, gt->nc);

    for(uint32_t id = gt->lo[gt->height]; id <= gt->nc; ++id) {
        uint32_t index = gt->findex[id];

        // Not strictly necessary but enhances performance.
        if(move_count[index] != UINT8_MAX) {
            gt->cc[id] = move_count[index];
            continue; }

        Pos *p = fen_to_pos(gt->ufen[index]);
        Rawcode *rc = rawcodes(p);
        gt->cc[id] = rc[0], move_count[index] = rc[0];
        free(p), free(rc); }

    free(move_count);
}

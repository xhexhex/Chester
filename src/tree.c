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
static void x_che_explicit_gt_stats_dfs_check_for_2nd_ep( const char *fen,
    int eptsf_offset );
static void x_che_explicit_gt_stats_dfs_count_promotions( const Pos *p,
    const Rawcode *rc );

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// TODO: doc
struct explicit_game_tree
che_build_explicit_gt( const char *fen, const uint8_t height )
{
    // What if 'fen' is a stalemate position?
    // printf("%s: I work but am a mess\n", __func__);

    assert(height < FGT_LO_SIZE);
    if(!fen) fen = INIT_POS;

    struct explicit_game_tree gt;

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
    x_che_build_explicit_gt_set_findex(&gt, id_to_fen);
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

// TODO: doc
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

    if(en_passants) *en_passants = x_en_passants;
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
x_che_build_explicit_gt_set_findex(struct explicit_game_tree *gt, char **id_to_fen)
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

static void
x_che_explicit_gt_stats_dfs( uint32_t node )
{
    ++x_nodes;

    char *fen = x_gt.ufen[x_gt.findex[node]];
    const Pos *p = fen_to_pos(fen);
    const Rawcode *rc = rawcodes(p);
    bool w = whites_turn(p);
    Bitboard rank_1 = 255;

    if(x_en_passants != UINT32_MAX && node < x_gt.lo[x_gt.height]) {
        int space_count = 0, index = 15;
        while(space_count < 3) if(fen[index++] == ' ') ++space_count;
        if(fen[index] != '-') {
            ++x_en_passants;
            x_che_explicit_gt_stats_dfs_check_for_2nd_ep(fen, index);
        }
    }
    if(x_en_passants != UINT32_MAX && node < x_gt.lo[x_gt.height]) {
        if( (w && ((rank_1 << 48) & p->ppa[WHITE_PAWN])) ||
                (!w && ((rank_1 << 8) & p->ppa[BLACK_PAWN]))
        ) x_che_explicit_gt_stats_dfs_count_promotions(p, rc);
    }
    if(x_checks != UINT32_MAX && king_in_check(p))
        ++x_checks;
    if(x_checkmates != UINT32_MAX && !rc[0] && king_in_check(p))
        ++x_checkmates;

    free((void *) p), free((void *) rc);
    if(node >= x_gt.lo[x_gt.height]) return;
    for(int i = 0; i < x_gt.cc[node]; ++i)
        x_che_explicit_gt_stats_dfs(x_gt.children[node][i]);
}

static void
x_che_explicit_gt_stats_dfs_check_for_2nd_ep( const char *fen,
    int eptsf_offset )
{
    char epts_file = fen[eptsf_offset];
    if(epts_file == 'a' || epts_file == 'h') return;

    int epts_rank = fen[eptsf_offset + 1] - '0';
    char rank[8 + 1], expanded_rank[8 + 1];
    bool w = (fen[eptsf_offset + 1] == '6');

    nth_ppf_rank(fen, epts_rank + (w ? -1 : 1), rank);
    expand_ppf_rank(rank, expanded_rank);
    // printf("expanded_rank is \"%s\"\n", expanded_rank);

    int index_of_epts_file = epts_file - 'a'; // Between 1 and 6
    char ac_pawn = w ? 'P' : 'p'; // Active color pawn
    if(expanded_rank[index_of_epts_file - 1] != ac_pawn) return;
    if(expanded_rank[index_of_epts_file + 1] == ac_pawn) ++x_en_passants;
}

static void
x_che_explicit_gt_stats_dfs_count_promotions( const Pos *p,
    const Rawcode *rc )
{
    // It's already established that it's either White's turn and there's
    // a white pawn on rank 7 or it's Black's turn and there's a black
    // pawn on rank 2.

    bool w = whites_turn(p);
    int num_moves = rc[0];
    for(int i = 1; i <= num_moves; ++i) {
        int orig = RC_ORIG_SQ_BINDEX[rc[i]];
        // Continue if the chessman being moved is not a pawn
        if(!(ONE << orig & (p->ppa[WHITE_PAWN] | p->ppa[BLACK_PAWN])))
            continue;
        if((w && orig >= 48 && orig <= 55) || (!w && orig >= 8 && orig <= 15))
            x_proms += 4;
    }
}

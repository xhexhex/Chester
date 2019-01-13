#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/chester.h"
#include "../../src/base.h"
#include "../../src/move_gen.h"

static int x_fen_find_and_count( struct fen_game_tree gt,
    const char *search_key, uint32_t *ids );

// tw, tested with
void
che_build_fen_gt_tw_fools_mate( struct fen_game_tree gt_ip4 )
{
    uint32_t cc = 0;
    for(uint32_t id = 1; id < gt_ip4.lo[gt_ip4.height]; id++)
        cc += gt_ip4.cc[id];
    assert(cc == gt_ip4.nc - 1);

    uint32_t ids[2];
    assert(x_fen_find_and_count(gt_ip4,
        "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3",
        ids) == 2);
    assert(!strcmp("rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq g3 0 2",
        gt_ip4.fen[ gt_ip4.parent[ids[0]] ]));
    assert(!strcmp("rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2",
        gt_ip4.fen[ gt_ip4.parent[ids[1]] ]));
}

// ip4, initial position (height 4)
void
che_build_fen_gt_tw_ip4_stats( struct fen_game_tree gt_ip4 )
{
    for(uint32_t id = 1; id <= gt_ip4.nc; id++) {
        const Pos *p = fen_to_pos(gt_ip4.fen[id]);
        if(checkmate(p)) printf("\"%s\"\n", gt_ip4.fen[id]);
        free((void *) p);
    }
}

//
// Static functions
//

static int
x_fen_find_and_count( struct fen_game_tree gt, const char *search_key,
    uint32_t *ids )
{
    int match_count = 0, index = -1;

    for(uint32_t id = 1; id <= gt.nc; id++) {
        if(!strcmp(gt.fen[id], search_key)) {
            ++match_count;
            if(ids) ids[++index] = id;
        }
    }

    return match_count;
}

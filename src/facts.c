#include <assert.h>
#include <stdlib.h>

#include "facts.h"
#include "chester.h"
#include "utils.h"

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// TODO: doc
// The child-parent relationship is not checked, it is assumed
bool
che_ep_has_occurred( const char *child, const char *parent )
{
    assert(child);
    if(!parent) return false;

    const Pos *p_ch = fen_to_pos(child);

    if( (whites_turn(p_ch) && !(p_ch->ppa[BLACK_PAWN] & rank('3'))) ||
            (!whites_turn(p_ch) && !(p_ch->ppa[WHITE_PAWN] & rank('6'))) ) {
        free((void *) p_ch); return false; }

    const Pos *p_pa = fen_to_pos(parent);
    // remove_redundant_epts(p_pa);
    if(!get_epts(p_pa)) { free((void *) p_pa); return false; }

    free((void *) p_ch), free((void *) p_pa);
    return true;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

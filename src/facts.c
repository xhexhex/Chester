#include <assert.h>
#include <stdlib.h>

#include "facts.h"
#include "chester.h"

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
    if(!parent) return false;
    const Pos *p_pa = fen_to_pos(parent);
    // remove_redundant_epts(p_pa);
    if(!get_epts(p_pa)) {free((void *) p_pa); return false;}
    const Pos *p_ch = fen_to_pos(child);

    free((void *) p_ch), free((void *) p_pa);
    return true;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

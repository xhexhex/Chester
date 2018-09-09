#include <string.h>
#include <stdio.h>

#include "chester.h"
#include "base.h"
#include "pgn.h"
#include "utils.h"

static void x_san_to_rawcode_find_dest_sq( const Pos *p,
    const char *san, char *dest );

// TODO: doc
Rawcode
san_to_rawcode( const Pos *p, const char *san )
{
    if( !che_is_san(san) ) return 0;

    char orig[2 + 1], dest[2 + 1], move[4 + 1] = {0};

    x_san_to_rawcode_find_dest_sq(p, san, dest);

    if( san[0] == 'O' )
        strcpy( orig, sq_bit_to_sq_name( p->ppa[whites_turn(p) ?
            WHITE_KING : BLACK_KING] ) );

    move[0] = orig[0], move[1] = orig[1],
        move[2] = dest[0], move[3] = dest[1];
    printf( "move = \"%s\"\n", move );

    return rawcode(move);
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

static void
x_san_to_rawcode_find_dest_sq( const Pos *p, const char *san, char *dest )
{
    if( san[0] == 'O' ) {
        const char *castling_rook =
            sq_bit_to_sq_name( p->irp[strlen(san) < 5 ? 1 : 0] );
        strcpy( dest, castling_rook );
        return;
    }
}

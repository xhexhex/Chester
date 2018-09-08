#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdbool.h>
#include <stdint.h>

#include "base.h"

enum status_code_che_is_san {
    CIS_DEFAULT,
    CIS_INVALID_LENGTH,
    CIS_INVALID_CHAR,
    CIS_CASTLING_MOVE,
    CIS_PAWN_ADVANCE,
    CIS_PAWN_CAPTURE,
    CIS_NON_DISAMBIGUATED_PIECE_MOVE,
    CIS_SINGLY_DISAMBIGUATED_PIECE_MOVE,
    CIS_DOUBLY_DISAMBIGUATED_PIECE_MOVE
};

extern enum status_code_che_is_san sc_che_is_san;

bool is_sq_name( const char *sq_name );
const char *ppa_integrity_check( const Bitboard *ppa );
bool is_sq_bit( Bitboard bb );

#endif
// end VALIDATION_H

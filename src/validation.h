#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdbool.h>
#include <stdint.h>

#include "base.h"

bool is_sq_name( const char *sq_name );
const char *ppa_integrity_check( const Bitboard *ppa );
bool is_sq_bit( Bitboard bb );

#endif
// end VALIDATION_H

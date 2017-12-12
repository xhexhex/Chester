#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdbool.h>

#include "types.h"

// The shortest possible FEN is something like "k7/8/8/8/8/8/8/K7 w - - 0 1"
// which is 27 characters long.
#define FEN_STR_MIN_LENGTH 27
// The following FEN is 90 chars long:
// "rrrrkrrr/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/RRRRKRRR w KQkq - 4095 4095"
// Allowing FENs longer than this wouldn't seem to make sense
#define FEN_STR_MAX_LENGTH 90

// The shortest possible PPF is "k7/8/8/8/8/8/8/K7" (17 chars)
#define PPF_MIN_LENGTH 17
// A PPF can be no longer than
// "pppppppp/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/PPPPPPPP"
#define PPF_MAX_LENGTH 71

extern const uint64_t CFSV_BF_CHESS960;

bool valid_sq_name( const char *sq_name );

#endif
// end VALIDATION_H

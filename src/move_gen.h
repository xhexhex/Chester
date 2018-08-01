#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <stdbool.h>

#include "base.h"

enum castle_error_codes {
    CASTLE_OK,
    CASTLE_NO_CASTLING_RIGHT,
    CASTLE_KING_IN_CHECK,
    CASTLE_KINGS_PATH_BLOCKED,
    CASTLE_KINGS_EXCLUSIVE_PATH_IN_CHECK
};

extern enum castle_error_codes castle_error;

extern const Bitboard KNIGHT_SQS[];
extern const Bitboard KING_SQS[];
extern const Bitboard ROOK_SQS[];
extern const Bitboard BISHOP_SQS[];

bool king_can_be_captured( const Pos *p );
Bitboard kerc( const Bitboard sq_bit ); // knight's effective range circle
Bitboard dest_sqs( const Pos *p, Bitboard origin_sq );
Bitboard attackers( const Bitboard *ppa, Bitboard sq, int num_arg, ... );
Bitboard white_attackers( const Bitboard *ppa, Bitboard sq );
Bitboard black_attackers( const Bitboard *ppa, Bitboard sq );
Rawcode *rawcodes( const Pos *p );
Rawcode castle( const Pos *p, const char *castle_type );
uint32_t move_info( const Pos *p, Rawcode code );

#endif
// end MOVE_GEN_H

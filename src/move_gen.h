#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <stdbool.h>

#include "base.h"

// Return values of the function castling_move_status()
enum cms {
	CMS_AVAILABLE,
	CMS_CA_BIT_UNSET,
	CMS_KINGS_PATH_BLOCKED,
	CMS_ROOKS_PATH_BLOCKED,
	CMS_KINGS_PATH_IN_CHECK,
	CMS_CASTLED_KING_IN_CHECK
};

extern const Bitboard KNIGHT_SQS[];
extern const Bitboard KING_SQS[];
extern const Bitboard ROOK_SQS[];
extern const Bitboard BISHOP_SQS[];

bool king_can_be_captured( const Pos *p );
Bitboard kerc( const Bitboard sq_bit ); // knight's effective range circle
Bitboard dest_sqs( const Pos *p, Bitboard origin_sq );
Bitboard cm_attacking_sq( const Pos *p, Bitboard sq, int num_arg, ... );
Bitboard white_cm_attacking_sq( const Pos *p, Bitboard sq );
Bitboard black_cm_attacking_sq( const Pos *p, Bitboard sq );
enum cms castling_move_status( const Pos *p, const char *castle_type );

#endif
// end MOVE_GEN_H

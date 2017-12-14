#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <stdbool.h>

#include "types.h"

// Return values of the function castling_move_status()
enum cms {
	CMS_AVAILABLE,
	CMS_INVALID_CASTLE_TYPE,
	CMS_INVALID_IRPF, // Initial rook placement/position field
	CMS_CA_FLAG_UNSET,
	CMS_KING_IN_CHECK
};

extern const Bitboard KNIGHT_SQS[];
extern const Bitboard KING_SQS[];
extern const Bitboard ROOK_SQS[];
extern const Bitboard BISHOP_SQS[];
extern const char *APM[];

bool king_can_be_captured( const Pos *p );
Bitboard kerc( const Bitboard sq_bit ); // knight's effective range circle
Bitboard dest_sqs( const Pos *p, Bitboard origin_sq );
Bitboard cm_attacking_sq( const Pos *p, Bitboard sq, int num_arg, ... );
Bitboard white_cm_attacking_sq( const Pos *p, Bitboard sq );
Bitboard black_cm_attacking_sq( const Pos *p, Bitboard sq );
enum cms castling_move_status( const Pos *p, const char *castle_type );

#endif
// end MOVE_GEN_H

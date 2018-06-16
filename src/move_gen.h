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

enum castle_error_codes {
    CASTLE_OK,
    CASTLE_NO_CASTLING_RIGHT
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
enum cms castling_move_status( const Pos *p, const char *castle_type );
Rawcode *rawcodes( const Pos *p );
Rawcode castle( const Pos *p, const char *castle_type );

#endif
// end MOVE_GEN_H

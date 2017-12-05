#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <stdbool.h>

#include "types.h"

// Used to get the "knight squares" of a particular square. For example,
// the knight squares of square b1 is the set { d2, c3, a3 }.
extern const Bitboard KNIGHT_SQS[];

// The "king squares" of a given square. For example, KING_SQS[ 4 ] is the
// Bitboard 0x3828u (which is the square set { d1, f1, d2, e2, f2 })
extern const Bitboard KING_SQS[];

// Example: ROOK_SQS[ 28 ] is 0x10101010ef101010U. That is, the rook squares
// of the square e4 is the set { e1, e2, e3, a4, b4, c4, d4, f4, g4, h4, e5,
// e6, e7, e8 }.
extern const Bitboard ROOK_SQS[];

// Example: BISHOP_SQS[ 28 ] is 0x182442800284482U. That is, the bishop squares
// of e4 is the set { b1, c2, d3, f5, g6, h7, h1, g2, f3, d5, c6, b7, a8 }.
extern const Bitboard BISHOP_SQS[];

// APM, all possible moves. A move in this context means a valid source and
// destination square such as "a1d1" or "e4f6" but not "d4d4", "h1g4" or "x1b2".
extern const char *APM[];

bool king_can_be_captured( const Pos *p );
Bitboard kerc( const Bitboard sq_bit ); // knight's effective range circle
Bitboard dest_sqs( const Pos *p, Bitboard origin_sq );

#endif
// end MOVE_GEN_H

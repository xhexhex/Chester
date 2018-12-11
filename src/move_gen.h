#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <stdbool.h>

#include "base.h"

enum castle_error_codes {
    CASTLE_OK,
    CASTLE_NO_CASTLING_RIGHT,
    CASTLE_KING_IN_CHECK,
    CASTLE_KINGS_PATH_BLOCKED,
    CASTLE_KINGS_EXCLUSIVE_PATH_IN_CHECK,
    CASTLE_ROOKS_PATH_BLOCKED };

extern enum castle_error_codes castle_error;

/*
extern int8_t
    glo_orig_k,
    glo_orig_q1, glo_orig_q2,
    glo_orig_r1, glo_orig_r2,
    glo_orig_b1, glo_orig_b2,
    glo_orig_n1, glo_orig_n2,
    glo_orig_p1, glo_orig_p2, glo_orig_p3, glo_orig_p4,
    glo_orig_p5, glo_orig_p6, glo_orig_p7, glo_orig_p8;
extern Bitboard
    glo_dest_k,
    glo_dest_q1, glo_dest_q2,
    glo_dest_r1, glo_dest_r2,
    glo_dest_b1, glo_dest_b2,
    glo_dest_n1, glo_dest_n2,
    glo_dest_p1, glo_dest_p2, glo_dest_p3, glo_dest_p4,
    glo_dest_p5, glo_dest_p6, glo_dest_p7, glo_dest_p8;
*/

bool forsaken_king( const Pos *p );
Bitboard kerc( const Bitboard sq_bit );
Bitboard attackers( const Bitboard *ppa, Bitboard sq, int num_arg, ... );
Bitboard white_attackers( const Bitboard *ppa, Bitboard sq );
Bitboard black_attackers( const Bitboard *ppa, Bitboard sq );
Rawcode *rawcodes( const Pos *p );
Rawcode castle( const Pos *p, const char *castle_type );
bool is_short_castle( const Pos *p, Rawcode move );
bool is_long_castle( const Pos *p, Rawcode move );
bool is_capture( const Pos *p, Rawcode move );
bool is_pawn_advance( const Pos *p, Rawcode move );
bool is_single_step_pawn_advance( const Pos *p, Rawcode move );
bool is_double_step_pawn_advance( const Pos *p, Rawcode move );
bool is_promotion( const Pos *p, Rawcode rc );
bool is_en_passant_capture( const Pos *p, Rawcode move );
bool is_castle( const Pos *p, Rawcode move );
bool king_in_check( const Pos *p );
bool checkmate( const Pos *p );
char *single_fen_move_gen( const char *fen );
void move_gen( const Pos *p );

#endif
// end MOVE_GEN_H

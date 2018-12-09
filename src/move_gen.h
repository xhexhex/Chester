#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include <stdbool.h>

#include "base.h"

#define FEN_PERSONALLY_SIGNIFICANT \
    "6k1/r1q1b2n/6QP/p3R3/1p3p2/1P6/1PP2P2/2K4R b - - 1 35"
#define FEN_GAME_OF_THE_CENTURY \
    "r2q1rk1/pp2ppbp/1np2np1/2Q3B1/3PP1b1/2N2N2/PP3PPP/3RKB1R b K - 6 11"
#define FEN_SUPERPOSITION_1 \
    "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1"
#define FEN_SUPERPOSITION_2 \
    "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1"

enum castle_error_codes {
    CASTLE_OK,
    CASTLE_NO_CASTLING_RIGHT,
    CASTLE_KING_IN_CHECK,
    CASTLE_KINGS_PATH_BLOCKED,
    CASTLE_KINGS_EXCLUSIVE_PATH_IN_CHECK,
    CASTLE_ROOKS_PATH_BLOCKED };

extern enum castle_error_codes castle_error;

extern const Bitboard KNIGHT_SQS[];
extern const Bitboard KING_SQS[];
extern const Bitboard ROOK_SQS[];
extern const Bitboard BISHOP_SQS[];
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

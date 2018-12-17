#ifndef EXTRA_H
#define EXTRA_H

#include "base.h"

#define FEN_PERSONALLY_SIGNIFICANT \
    "6k1/r1q1b2n/6QP/p3R3/1p3p2/1P6/1PP2P2/2K4R b - - 1 35"
#define FEN_GAME_OF_THE_CENTURY \
    "r2q1rk1/pp2ppbp/1np2np1/2Q3B1/3PP1b1/2N2N2/PP3PPP/3RKB1R b K - 6 11"

// The square sets for the 15 diagonals and 15 antidiagonals of the chessboard
extern const Bitboard
    SS_DIAG_H1H1, SS_DIAG_G1H2, SS_DIAG_F1H3, SS_DIAG_E1H4, SS_DIAG_D1H5,
    SS_DIAG_C1H6, SS_DIAG_B1H7, SS_DIAG_A1H8, SS_DIAG_A2G8, SS_DIAG_A3F8,
    SS_DIAG_A4E8, SS_DIAG_A5D8, SS_DIAG_A6C8, SS_DIAG_A7B8, SS_DIAG_A8A8,
    SS_ANTIDIAG_A1A1, SS_ANTIDIAG_B1A2, SS_ANTIDIAG_C1A3,
    SS_ANTIDIAG_D1A4, SS_ANTIDIAG_E1A5, SS_ANTIDIAG_F1A6,
    SS_ANTIDIAG_G1A7, SS_ANTIDIAG_H1A8, SS_ANTIDIAG_H2B8,
    SS_ANTIDIAG_H3C8, SS_ANTIDIAG_H4D8, SS_ANTIDIAG_H5E8,
    SS_ANTIDIAG_H6F8, SS_ANTIDIAG_H7G8, SS_ANTIDIAG_H8H8;

char *dyn_ttt_ppa_to_ppf_conv( const uint16_t *ppa );
void sta_ttt_ppa_to_ppf_conv( const uint16_t *ppa, char *s );
void comparative_ttt_ppa_to_ppf_conv_test( const int reps );
void make_move_performance_test();
void rawcode_bit_indexes( Rawcode code, int *orig, int *dest );
void rawcodes_performance_test();
int sq_bit_index( Bitboard sq_bit );
Bitboard diag_of_sq( Bitboard sq_bit );
Bitboard antidiag_of_sq( Bitboard sq_bit );
Bitboard sq_set_of_diag( const int index );
Bitboard sq_set_of_antidiag( const int index );
void make_move_sanity_checks( const Pos *p, Rawcode rc, char promotion,
    int orig, int dest, Chessman mover, Chessman target );
Bitboard kerc( const Bitboard sq_bit );
Bitboard rectangle_of_sqs( int upper_left, int lower_right );

#endif
// end EXTRA_H

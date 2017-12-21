#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "chester.h"

// Bitboard, the fundamental (internal) data type of Chester. It is a 64-bit
// unsigned integer used to deliver information that relates to the chessboard
// in one way or another. Each bit corresponds to a particular square of
// the chessboard.
typedef uint64_t Bitboard;

struct const_single_var_struct_SB {
	const Bitboard
		a1, b1, c1, d1, e1, f1, g1, h1,
		a2, b2, c2, d2, e2, f2, g2, h2,
		a3, b3, c3, d3, e3, f3, g3, h3,
		a4, b4, c4, d4, e4, f4, g4, h4,
		a5, b5, c5, d5, e5, f5, g5, h5,
		a6, b6, c6, d6, e6, f6, g6, h6,
		a7, b7, c7, d7, e7, f7, g7, h7,
		a8, b8, c8, d8, e8, f8, g8, h8;
};

extern const struct const_single_var_struct_SB SB;
extern const Bitboard SBA[];
extern const char *SNA[];
extern const Bitboard SQ_NAV[][ 16 ];
extern const char STD_START_POS_FEN[];
extern const char FEN_PIECE_LETTERS[];

// The pieces array of Pos should be indexed with these, e.g.,
// pos.pieces[ WHITE_KING ]
typedef enum {
	EMPTY_SQUARE,
	WHITE_KING, WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN,
	BLACK_KING, BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN
} Chessman;

// TODO: Doc
enum sq_dir {
	NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST,
	ONE_OCLOCK, TWO_OCLOCK, FOUR_OCLOCK, FIVE_OCLOCK,
	SEVEN_OCLOCK, EIGHT_OCLOCK, TEN_OCLOCK, ELEVEN_OCLOCK
};

// Chester's internal representation of a position. The Pos type contains
// the same information as a FEN string. See the the documention for a
// comprehensive description of the Pos type.
// TODO: Mention BM_C960IRPF.
typedef struct {
	Bitboard pieces[ 13 ];
	uint64_t info;
} Pos;

// Used for inspecting and manipulating the data in the 'info' member
// of Pos variables. As the 'info' member corresponds to the last five
// fields of a FEN record, there are five bit fields of interest in 'info'.
// The bitmasks below specify those bit fields (or the  individual bits
// in the bit fields).
extern const uint64_t
	BM_AC,
	BM_CA_WK, BM_CA_WQ, BM_CA_BK, BM_CA_BQ,
	BM_EPTS_FILE_A, BM_EPTS_FILE_B, BM_EPTS_FILE_C, BM_EPTS_FILE_D,
		BM_EPTS_FILE_E, BM_EPTS_FILE_F, BM_EPTS_FILE_G, BM_EPTS_FILE_H,
	BM_HMC,
	BM_FMN,
	BM_C960IRPF;

#define BM_CA_ALL ( BM_CA_WK | BM_CA_WQ | BM_CA_BK | BM_CA_BQ )

#define BM_EPTS_ALL \
( BM_EPTS_FILE_A | BM_EPTS_FILE_B | BM_EPTS_FILE_C | BM_EPTS_FILE_D | \
	BM_EPTS_FILE_E | BM_EPTS_FILE_F | BM_EPTS_FILE_G | BM_EPTS_FILE_H )

#define BM_UNUSED_INFO_BITS \
( ~( BM_AC | BM_CA_ALL | BM_EPTS_ALL | BM_HMC | BM_FMN | BM_C960IRPF ) )

// The square sets that correspond to the eight files and
// the eight ranks of the chessboard
extern const Bitboard
	SS_FILE_A, SS_FILE_B, SS_FILE_C, SS_FILE_D,
	SS_FILE_E, SS_FILE_F, SS_FILE_G, SS_FILE_H,
	SS_RANK_1, SS_RANK_2, SS_RANK_3, SS_RANK_4,
	SS_RANK_5, SS_RANK_6, SS_RANK_7, SS_RANK_8;

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

// Function prototypes
Pos *fen_str_to_pos_var( const char *fen_str );
char active_color( const Pos *p );
bool whites_turn( const Pos *p );
const char *pos_var_sq_integrity_check( const Pos *p );
Bitboard sq_set_of_file( const char file );
Bitboard sq_set_of_rank( const char rank );
Bitboard sq_set_of_diag( const int index );
Bitboard sq_set_of_antidiag( const int index );
Bitboard ss_white_army( const Pos *p );
Bitboard ss_black_army( const Pos *p );
Bitboard sq_nav( Bitboard sq, enum sq_dir dir );
bool white_has_a_side_castling_right( const Pos *p );
bool white_has_h_side_castling_right( const Pos *p );
bool black_has_a_side_castling_right( const Pos *p );
bool black_has_h_side_castling_right( const Pos *p );
uint64_t value_BM_C960IRPF( const Pos *p );
void set_BM_C960IRPF( uint64_t *info, uint8_t irpf );
bool chess960_start_pos( const Pos *p );

#endif
// end TYPES_H
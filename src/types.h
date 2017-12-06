#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "chester.h"

// The shortest possible FEN string is something like
// "k7/8/8/8/8/8/8/K7 w - - 0 1" which is 27 characters long
#define FEN_STR_MIN_LENGTH 27
// The following FEN string is 90 chars long:
// "pppppppp/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/PPPPPPPP w KQkq - 4095 4095"
// Allowing FENs longer than this wouldn't seem to make sense
#define FEN_STR_MAX_LENGTH 90

// The shortest possible PPF is "k7/8/8/8/8/8/8/K7" (17 chars)
#define PPF_MIN_LENGTH 17
// A PPF can be no longer than
// "pppppppp/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/PPPPPPPP"
// 8*8 + 7 = 71
#define PPF_MAX_LENGTH 71

// Bitboard, the fundamental (internal) data type of Chester. It is a 64-bit
// unsigned integer used to deliver information that relates to the chessboard
// in one way or another. Each bit corresponds to a particular square of
// the chessboard.
typedef uint64_t Bitboard;

// The following structure should have only one variable (or instance), the
// one declared and initialized in types.c with the identifier SB. The constant
// struct variable SB serves as a mapping between a square name (such as "e4")
// and its corresponding square bit (a Bitboard with exactly one bit set).
struct const_single_var_struct_SB { // SB, square bit
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

// The constant struct variable SB is declared and initialized in types.c
// and should be available in any source file that includes types.h
extern const struct const_single_var_struct_SB SB;

// The 64 members of the struct variable SB as an array. SBA[ 0 ] is
// SB.a1, SBA[ 63 ] is SB.h8.
extern const Bitboard SBA[];

// SNA, square names array
// SNA[ 0 ] is "a1", SNA[ 1 ] is "b1", ..., SNA[ 63 ] is "h8"
extern const char *SNA[];

extern const Bitboard SQ_NAV[][ 16 ];

// The pieces array of Pos should be indexed with these, e.g.,
// pos.pieces[ WHITE_KING ]
typedef enum {
	EMPTY_SQUARE,
	WHITE_KING, WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN,
	BLACK_KING, BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN
} Chessman;

// Chester's internal representation of a position. The Pos type contains
// the same information as a FEN string. See the the documention for a
// comprehensive description of the Pos type.
typedef struct {
	Bitboard pieces[ 13 ];
	uint64_t info;
} Pos;

//
// Misc. external data
//

// The FEN string for the (standard) starting position
extern const char STD_START_POS_FEN[];

// The 12 letters that identify the different chessmen in the PPF
// of a FEN record. The first element of the array is '-' and as such
// isn't one of the 12 letters. The order of the letters correspond
// to the Chessman enum type.
extern const char FEN_PIECE_LETTERS[];

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
	BM_FMN;

#define BM_CA_ALL ( BM_CA_WK | BM_CA_WQ | BM_CA_BK | BM_CA_BQ )

#define BM_EPTS_ALL \
( BM_EPTS_FILE_A | BM_EPTS_FILE_B | BM_EPTS_FILE_C | BM_EPTS_FILE_D | \
	BM_EPTS_FILE_E | BM_EPTS_FILE_F | BM_EPTS_FILE_G | BM_EPTS_FILE_H )

#define BM_UNUSED_INFO_BITS \
( ~( BM_AC | BM_CA_ALL | BM_EPTS_ALL | BM_HMC | BM_FMN ) )

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

/*****************************
 **** Function prototypes ****
 *****************************/

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

// ???
// Bitboard Squares_to_bb( const Squares *s );
// Squares *bb_to_Squares( const Bitboard bb );

#endif
// end TYPES_H

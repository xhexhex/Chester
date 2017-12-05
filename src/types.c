#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "utils.h"

/***********************
 **** External data ****
 ***********************/

#define ALL_SQ_BIT_VALUES \
	0x1u, 0x2u, 0x4u, 0x8u, 0x10u, 0x20u, 0x40u, 0x80u, 0x100u, 0x200u, 0x400u, \
	0x800u, 0x1000u, 0x2000u, 0x4000u, 0x8000u, 0x10000u, 0x20000u, 0x40000u, \
	0x80000u, 0x100000u, 0x200000u, 0x400000u, 0x800000u, 0x1000000u, 0x2000000u, \
	0x4000000u, 0x8000000u, 0x10000000u, 0x20000000u, 0x40000000u, 0x80000000u, \
	0x100000000u, 0x200000000u, 0x400000000u, 0x800000000u, 0x1000000000u, \
	0x2000000000u, 0x4000000000u, 0x8000000000u, 0x10000000000u, 0x20000000000u, \
	0x40000000000u, 0x80000000000u, 0x100000000000u, 0x200000000000u, \
	0x400000000000u, 0x800000000000u, 0x1000000000000u, 0x2000000000000u, \
	0x4000000000000u, 0x8000000000000u, 0x10000000000000u, 0x20000000000000u, \
	0x40000000000000u, 0x80000000000000u, 0x100000000000000u, 0x200000000000000u, \
	0x400000000000000u, 0x800000000000000u, 0x1000000000000000u, 0x2000000000000000u, \
	0x4000000000000000u, 0x8000000000000000u

const struct const_single_var_struct_SB SB = {
	ALL_SQ_BIT_VALUES
};

const Bitboard SBA[] = {
	ALL_SQ_BIT_VALUES
};

const char *SNA[] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

const char STD_START_POS_FEN[] =
	"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const char FEN_PIECE_LETTERS[] = "-KQRBNPkqrbnp";

const uint64_t
	BM_AC = 0x1u,
	BM_CA_WK = 0x2u, BM_CA_WQ = 0x4u, BM_CA_BK = 0x8u, BM_CA_BQ = 0x10u,
	BM_EPTS_FILE_A = 0x20u, BM_EPTS_FILE_B = 0x40u,
		BM_EPTS_FILE_C = 0x80u, BM_EPTS_FILE_D = 0x100u,
		BM_EPTS_FILE_E = 0x200u, BM_EPTS_FILE_F = 0x400u,
		BM_EPTS_FILE_G = 0x800u, BM_EPTS_FILE_H = 0x1000u,
	BM_HMC = 0x1ffe000u,
	BM_FMN = 0x1ffe000000u;

const Bitboard
	SS_FILE_A = 0x101010101010101u, SS_FILE_B = 0x202020202020202u,
	SS_FILE_C = 0x404040404040404u, SS_FILE_D = 0x808080808080808u,
	SS_FILE_E = 0x1010101010101010u, SS_FILE_F = 0x2020202020202020u,
	SS_FILE_G = 0x4040404040404040u, SS_FILE_H = 0x8080808080808080u,
	SS_RANK_1 = 0xffu, SS_RANK_2 = 0xff00u, SS_RANK_3 = 0xff0000u,
	SS_RANK_4 = 0xff000000u, SS_RANK_5 = 0xff00000000u,
	SS_RANK_6 = 0xff0000000000u, SS_RANK_7 = 0xff000000000000u,
	SS_RANK_8 = 0xff00000000000000u;

const Bitboard
	SS_DIAG_H1H1 = 0x80U, SS_DIAG_G1H2 = 0x8040U, SS_DIAG_F1H3 = 0x804020U,
	SS_DIAG_E1H4 = 0x80402010U, SS_DIAG_D1H5 = 0x8040201008U,
	SS_DIAG_C1H6 = 0x804020100804U, SS_DIAG_B1H7 = 0x80402010080402U,
	SS_DIAG_A1H8 = 0x8040201008040201U, SS_DIAG_A2G8 = 0x4020100804020100U,
	SS_DIAG_A3F8 = 0x2010080402010000U, SS_DIAG_A4E8 = 0x1008040201000000U,
	SS_DIAG_A5D8 = 0x804020100000000U, SS_DIAG_A6C8 = 0x402010000000000U,
	SS_DIAG_A7B8 = 0x201000000000000U, SS_DIAG_A8A8 = 0x100000000000000U,
	SS_ANTIDIAG_A1A1 = 0x1U, SS_ANTIDIAG_B1A2 = 0x102U,
	SS_ANTIDIAG_C1A3 = 0x10204U, SS_ANTIDIAG_D1A4 = 0x1020408U,
	SS_ANTIDIAG_E1A5 = 0x102040810U, SS_ANTIDIAG_F1A6 = 0x10204081020U,
	SS_ANTIDIAG_G1A7 = 0x1020408102040U, SS_ANTIDIAG_H1A8 = 0x102040810204080U,
	SS_ANTIDIAG_H2B8 = 0x204081020408000U, SS_ANTIDIAG_H3C8 = 0x408102040800000U,
	SS_ANTIDIAG_H4D8 = 0x810204080000000U, SS_ANTIDIAG_H5E8 = 0x1020408000000000U,
	SS_ANTIDIAG_H6F8 = 0x2040800000000000U, SS_ANTIDIAG_H7G8 = 0x4080000000000000U,
	SS_ANTIDIAG_H8H8 = 0x8000000000000000U;

/************************************
 **** Static function prototypes ****
 ************************************/

static void x_gen_slashless_constant_length_pp_str(
	const char *fen_str, char * const target_array );
static void x_init_empty_square_bb( Pos *p );
static void x_init_pos_var_pieces_array(
	Pos *p, const char *slashless_constant_length_pp_str );
static void x_set_active_color( Pos *p, char color );
static void x_set_castling_availability( Pos *p, const char *ca );
static void x_set_en_passant_target_square( Pos *p, const char *epts );
static void x_set_halfmove_clock( Pos *p, const char *hmc );
static void x_set_fullmove_number( Pos *p, const char *fmn );
static Bitboard x_sq_set_of_diag( const int index );
static Bitboard x_sq_set_of_antidiag( const int index );

/*********************************
 **** Static global variables ****
 *********************************/

// Used by nth_field_of_fen_str() of utils.c
static char x_writable_mem[ FEN_STR_MAX_LENGTH + 1 ];

/****************************
 **** External functions ****
 ****************************/

// Converts a FEN string to a Pos variable, the internal position
// representation in Chester
Pos *
fen_str_to_pos_var( const char *fen_str ) // Argument assumed to be valid
{
	Pos *p = (Pos *) malloc( sizeof( Pos ) );
	char writable_array_of_65_bytes[ 64 + 1 ];

	if( !p ) {
		fprintf( stderr, "%s(): malloc() returned a null pointer\n", __func__ );
		abort();
	}

	x_gen_slashless_constant_length_pp_str( fen_str, writable_array_of_65_bytes );
	x_init_pos_var_pieces_array( p, writable_array_of_65_bytes );

	char ac = ( !strcmp(
		"w", nth_field_of_fen_str( fen_str, x_writable_mem, 2 )
	) ) ? 'w' : 'b';
	x_set_active_color( p, ac );

	const char *ca = nth_field_of_fen_str( fen_str, x_writable_mem, 3 );
	x_set_castling_availability( p, ca );

	const char *epts = nth_field_of_fen_str( fen_str, x_writable_mem, 4 );
	x_set_en_passant_target_square( p, epts );

	const char *hmc = nth_field_of_fen_str( fen_str, x_writable_mem, 5 );
	x_set_halfmove_clock( p, hmc );

	const char *fmn = nth_field_of_fen_str( fen_str, x_writable_mem, 6 );
	x_set_fullmove_number( p, fmn );

	unset_bits( &p->info, BM_UNUSED_INFO_BITS );

	assert( !pos_var_sq_integrity_check( p ) );
	return p;
}

// Returns the active color of the given Pos variable
inline char
active_color( const Pos *p )
{
	// BM_AC is the least significant digit of the 'info' integer, and
	// it being set means it is White's turn.
	return ( p->info % 2 ) ? 'w' : 'b';
}

// Using this one might be a bit more convenient than using active_color()
inline bool
whites_turn( const Pos *p ) {
	return active_color( p ) == 'w';
}

// In the pieces array of a Pos var, exactly one bit should be set per
// bit index. In logical terms this means that each square is either
// empty or contains one of the twelve types of chessmen; a square cannot
// be empty AND be occupied by a chessman, nor can it be occupied by more
// than one type of chessman. The following function checks that exactly
// one bit is set for each bit index of the pieces array.
const char *
pos_var_sq_integrity_check( const Pos *p )
{
	for( int bit_index = 0; bit_index < 64; bit_index++ ) {
		int number_of_set_bits = 0;

		for( Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++ )
			if( p->pieces[ cm ] & SBA[ bit_index ] )
				++number_of_set_bits;

		if( number_of_set_bits != 1 )
			return SNA[ bit_index ];
	}

	return NULL;
}

// The call sq_set_of_file( 'a' ) would return the constant SS_FILE_A.
// The main purpose of the function is to not have to define an array
// version of the SS_FILE_? constants.
Bitboard
sq_set_of_file( const char file )
{
	switch( file ) {
	case 'a':
		return SS_FILE_A;
	case 'b':
		return SS_FILE_B;
	case 'c':
		return SS_FILE_C;
	case 'd':
		return SS_FILE_D;
	case 'e':
		return SS_FILE_E;
	case 'f':
		return SS_FILE_F;
	case 'g':
		return SS_FILE_G;
	case 'h':
		return SS_FILE_H;
	default:
		assert( false );
		return 0;
	}
}

// The call sq_set_of_rank( '1' ) would return the constant SS_RANK_1.
// The main purpose of the function is the same as in sq_set_of_file().
Bitboard
sq_set_of_rank( const char rank )
{
	switch( rank ) {
	case '1':
		return SS_RANK_1;
	case '2':
		return SS_RANK_2;
	case '3':
		return SS_RANK_3;
	case '4':
		return SS_RANK_4;
	case '5':
		return SS_RANK_5;
	case '6':
		return SS_RANK_6;
	case '7':
		return SS_RANK_7;
	case '8':
		return SS_RANK_8;
	default:
		assert( false );
		return 0;
	}
}

// Can be used to collectively access the SS_DIAG_* constants as if they
// were elements of the same array. The index of diagonal h1h1 is 0 and
// the index of a8a8 is 14.
Bitboard
sq_set_of_diag( const int index )
{
	return x_sq_set_of_diag( index );
}

// Same as above for the SS_ANTIDIAG_* constants. The index of antidiagonal
// a1a1 is 0 and the index of h8h8 14.
Bitboard
sq_set_of_antidiag( const int index )
{
	return x_sq_set_of_antidiag( index );
}

// Returns the square set of the white army. That means all the squares
// that contain a white chessman.
Bitboard
ss_white_army( const Pos *p )
{
	return p->pieces[ WHITE_KING ] | p->pieces[ WHITE_QUEEN ] |
		p->pieces[ WHITE_ROOK ] | p->pieces[ WHITE_BISHOP ] |
		p->pieces[ WHITE_KNIGHT ] | p->pieces[ WHITE_PAWN ];
}

// Returns the square set of the black army. That means all the squares
// that contain a black chessman.
Bitboard
ss_black_army( const Pos *p )
{
	return p->pieces[ BLACK_KING ] | p->pieces[ BLACK_QUEEN ] |
		p->pieces[ BLACK_ROOK ] | p->pieces[ BLACK_BISHOP ] |
		p->pieces[ BLACK_KNIGHT ] | p->pieces[ BLACK_PAWN ];
}

/*
// ...
Bitboard
Squares_to_bb( const Squares *s )
{
	return 0;
}

// ...
Squares *
bb_to_Squares( const Bitboard bb )
{
	return NULL;
}
*/

/**************************
 **** Static functions ****
 **************************/

// If the first argument is the FEN for the standard starting position,
// then (after the function call completes), target_array will contain the
// string "RNBQKBNRPPPPPPPP--------------------------------pppppppprnbqkbnr".
static void
x_gen_slashless_constant_length_pp_str( // pp, piece placement
	const char *fen_str, char * const target_array )
{
	char ppf[ PPF_MAX_LENGTH + 1 ];
	strcpy( ppf, nth_field_of_fen_str( fen_str, x_writable_mem, 1 ) );

	char *moving_ptr = target_array;
	for( int rank = 1; rank <= 8; rank++ ) {
		// Making sure an "array out of bounds" type of error doesn't occur
		assert( moving_ptr - target_array <= 56 );

		// This will copy eight non-null chars (plus a null char) somewhere
		// into target_array. The eight non-null chars are a rank from
		// the PPF with digits replaced by a corresponding number of dashes.
		convert_ppf_rank_digits_to_dashes(
			nth_rank_of_ppf( ppf, x_writable_mem, rank ),
			moving_ptr );

		moving_ptr += 8;

		assert( !*moving_ptr ); // moving_ptr points to a null character
	}

	assert( strlen( target_array ) == 64 );
	assert( str_matches_pattern( target_array, "^[KQRBNPkqrbnp-]{64}$" ) );
}

// Initializes the pieces[] array of a Pos variable with the piece placement
// specified in the second argument. For the standard starting position the
// second argument would be
// "RNBQKBNRPPPPPPPP--------------------------------pppppppprnbqkbnr".
static void
x_init_pos_var_pieces_array( Pos *p, const char *slashless_constant_length_pp_str )
{
	for( Chessman cm = WHITE_KING; cm <= BLACK_PAWN; cm++ ) {
		p->pieces[ cm ] = 0u;

		for( int i = 0; i < 64; i++ ) {
			if( slashless_constant_length_pp_str[ i ] == FEN_PIECE_LETTERS[ cm ] ) {
				p->pieces[ cm ] |= SBA[ i ];
			}
		}
	}

	x_init_empty_square_bb( p );
}

// Called from x_init_pos_var_pieces_array()
static void
x_init_empty_square_bb( Pos *p )
{
	Bitboard occupied_squares = 0u;
	for( Chessman cm = WHITE_KING; cm <= BLACK_PAWN; cm++ ) {
		occupied_squares |= p->pieces[ cm ];
	}

	p->pieces[ EMPTY_SQUARE ] = ~occupied_squares;
}

// Used to set the active color (AC). The AC is 'w' if the BM_AC bit is set;
// otherwise the AC is 'b'.
static void
x_set_active_color( Pos *p, char color ) {
	assert( p ); // p must not be a null pointer
	assert( color == 'w' || color == 'b' );

	// Sets the BM_AC bit if it is White's turn; otherwise the bit is unset
	set_or_unset_bits( &p->info, BM_AC, color == 'w' );
}

// Sets the castling availability bit field of a Pos variable. Should
// only be called from fen_str_to_pos_var(). The ca argument is assumed
// to be valid.
static void
x_set_castling_availability( Pos *p, const char *ca )
{
	// Unsetting all of the four castling availability flags
	set_or_unset_bits( &p->info, BM_CA_ALL, 0 );

	// We're done if none of the four types of castling availability
	// are available
	if( !strcmp( ca, "-" ) ) return;

	// Setting the CA flags as needed
	if( str_matches_pattern( ca, "^K.*$" ) )
		set_or_unset_bits( &p->info, BM_CA_WK, 1 );
	if( str_matches_pattern( ca, "^.*Q.*$" ) )
		set_or_unset_bits( &p->info, BM_CA_WQ, 1 );
	if( str_matches_pattern( ca, "^.*k.*$" ) )
		set_or_unset_bits( &p->info, BM_CA_BK, 1 );
	if( str_matches_pattern( ca, "^.*q$" ) )
		set_or_unset_bits( &p->info, BM_CA_BQ, 1 );
}

// Should only be called from fen_str_to_pos_var(). The 'epts' argument
// is assumed to be valid.
static void
x_set_en_passant_target_square( Pos *p, const char *epts )
{
	// Unsetting all of the eight EPTS flags
	set_or_unset_bits( &p->info, BM_EPTS_ALL, 0 );

	// We are done if there is no EPTS set
	if( !strcmp( "-", epts ) ) return;

	char file = *epts; // Same as epts[ 0 ], the letter indicating the file
	int num_of_left_shifts = file - (int) 'a';

	set_bits( &p->info, BM_EPTS_FILE_A << num_of_left_shifts );
}

// Should only be called from fen_str_to_pos_var(). The argument 'hmc'
// is assumed to be valid.
static void
x_set_halfmove_clock( Pos *p, const char *hmc )
{
	unset_bits( &p->info, BM_HMC );
	set_bits( &p->info, ( (uint64_t) atoi( hmc ) ) << 13 );
}

// Should only be called from fen_str_to_pos_var(). The argument 'fmn'
// is assumed to be valid.
static void
x_set_fullmove_number( Pos *p, const char *fmn )
{
	unset_bits( &p->info, BM_FMN );
	set_bits( &p->info, ( (uint64_t) atoi( fmn ) ) << 25 );
}

static Bitboard
x_sq_set_of_diag( const int index )
{
	switch( index ) {
	case 0:
		return SS_DIAG_H1H1;
	case 1:
		return SS_DIAG_G1H2;
	case 2:
		return SS_DIAG_F1H3;
	case 3:
		return SS_DIAG_E1H4;
	case 4:
		return SS_DIAG_D1H5;
	case 5:
		return SS_DIAG_C1H6;
	case 6:
		return SS_DIAG_B1H7;
	case 7:
		return SS_DIAG_A1H8;
	case 8:
		return SS_DIAG_A2G8;
	case 9:
		return SS_DIAG_A3F8;
	case 10:
		return SS_DIAG_A4E8;
	case 11:
		return SS_DIAG_A5D8;
	case 12:
		return SS_DIAG_A6C8;
	case 13:
		return SS_DIAG_A7B8;
	case 14:
		return SS_DIAG_A8A8;
	default:
		assert( false );
		return 0u;
	}
}

static Bitboard
x_sq_set_of_antidiag( const int index )
{
	switch( index ) {
	case 0:
		return SS_ANTIDIAG_A1A1;
	case 1:
		return SS_ANTIDIAG_B1A2;
	case 2:
		return SS_ANTIDIAG_C1A3;
	case 3:
		return SS_ANTIDIAG_D1A4;
	case 4:
		return SS_ANTIDIAG_E1A5;
	case 5:
		return SS_ANTIDIAG_F1A6;
	case 6:
		return SS_ANTIDIAG_G1A7;
	case 7:
		return SS_ANTIDIAG_H1A8;
	case 8:
		return SS_ANTIDIAG_H2B8;
	case 9:
		return SS_ANTIDIAG_H3C8;
	case 10:
		return SS_ANTIDIAG_H4D8;
	case 11:
		return SS_ANTIDIAG_H5E8;
	case 12:
		return SS_ANTIDIAG_H6F8;
	case 13:
		return SS_ANTIDIAG_H7G8;
	case 14:
		return SS_ANTIDIAG_H8H8;
	default:
		assert( false );
		return 0u;
	}
}

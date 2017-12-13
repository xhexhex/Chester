#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "validation.h"
#include "utils.h"
#include "chester.h"
#include "move_gen.h"

// Bit flags for the 'settings' parameter of function che_fen_str_validator()
const uint64_t
	CFSV_BF_CHESS960 = 0x1U; // The FEN has Chess960 castling rules in effect

static bool x_validate_fen_str_test_1( const char *fen_str );
static bool x_validate_fen_str_test_2( const char *fen_str );
static bool x_validate_fen_str_test_3( const char *fen_str );
static bool x_validate_fen_str_test_4( const char *fen_str );
static bool x_validate_fen_str_test_5( const char *fen_str );
static bool x_validate_fen_str_test_6( const char *fen_str );
static bool x_validate_fen_str_test_7( const char *fen_str );
static bool x_validate_fen_str_test_8( const char *fen_str );
static bool x_validate_fen_str_test_9( const char *fen_str );
static bool x_validate_fen_str_test_10( const char *fen_str );
static bool x_validate_fen_str_test_11( const char *fen_str );
static bool x_validate_fen_str_test_12( const char *fen_str, uint64_t settings );
static bool x_validate_fen_str_test_13( const char *fen_str );
static bool x_validate_fen_str_test_14( const char *fen_str );
static bool x_validate_fen_str_test_15( const Pos *p );
static bool x_validate_fen_str_test_16( const Pos *p );
static bool x_validate_fen_str_test_17( const Pos *p );
static bool x_validate_fen_str_test_18( const Pos *p );
static bool x_validate_fen_str_test_19( const Pos *p );
static bool x_validate_fen_str_test_20( const Pos *p );
static bool x_validate_fen_str_test_21( const Pos *p );
static bool x_validate_fen_str_test_22( const Pos *p );
static bool x_validate_fen_str_test_23( const Pos *p );
static bool x_validate_fen_str_test_24( const Pos *p );
static int x_calc_rank_sum( const char *rank );
static bool x_ppf_and_caf_agree( const char *ppf, const char *ca );
static char x_occupant_of_sq( const char *ppf, const char *sq );

static char x_writable_mem[ FEN_STR_MAX_LENGTH + 1 ];

/***************************
 **** Chester interface ****
 ***************************/

// The function for FEN string validation
enum fen_str_error
che_validate_fen_str( const char *fen_str, const uint64_t settings )
{
	// Test 1
	if( !x_validate_fen_str_test_1( fen_str ) )
		return FEN_STR_LENGTH_ERROR;
	// Test 2
	if( !x_validate_fen_str_test_2( fen_str ) )
		return FEN_STR_CHARS_ERROR;
	// Test 3
	if( !x_validate_fen_str_test_3( fen_str ) )
		return FEN_STR_FIELD_STRUCTURE_ERROR;
	// Test 4
	if( !x_validate_fen_str_test_4( fen_str ) )
		return FEN_STR_PPF_STRUCTURE_ERROR;
	// Test 5
	if( !x_validate_fen_str_test_5( fen_str ) )
		return FEN_STR_PPF_CONSECUTIVE_DIGITS_ERROR;
	// Test 6
	if( !x_validate_fen_str_test_6( fen_str ) )
		return FEN_STR_PPF_RANK_SUM_ERROR;
	// Test 7
	if( !x_validate_fen_str_test_7( fen_str ) )
		return FEN_STR_ACF_ERROR;
	// Test 8
	if( !x_validate_fen_str_test_8( fen_str ) )
		return FEN_STR_CAF_ERROR;
	// Test 9
	if( !x_validate_fen_str_test_9( fen_str ) )
		return FEN_STR_EPTSF_ERROR;
	// Test 10
	if( !x_validate_fen_str_test_10( fen_str ) )
		return FEN_STR_HMCF_ERROR;
	// Test 11
	if( !x_validate_fen_str_test_11( fen_str ) )
		return FEN_STR_FMNF_ERROR;
	// Test 12
	if( !x_validate_fen_str_test_12( fen_str, settings ) )
		return FEN_STR_PPF_CONTRADICTS_CAF_ERROR;
	// Test 13
	if( !x_validate_fen_str_test_13( fen_str ) )
		return FEN_STR_EPTSF_CONTRADICTS_HMCF_ERROR;
	// Test 14
	if( !x_validate_fen_str_test_14( fen_str ) )
		return FEN_STR_EPTSF_CONTRADICTS_ACF_ERROR;

	// At this point the FEN string is found to be "sufficiently valid"
	// for the conversion to succeed
	Pos *p = fen_str_to_pos_var( fen_str );

	// Test 15
	if( !x_validate_fen_str_test_15( p ) )
		return FEN_STR_EPTSF_CONTRADICTS_PPF_ERROR;
	// Test 16
	if( !x_validate_fen_str_test_16( p ) )
		return FEN_STR_WHITE_PAWN_ON_FIRST_RANK;
	// Test 17
	if( !x_validate_fen_str_test_17( p ) )
		return FEN_STR_BLACK_PAWN_ON_FIRST_RANK;
	// Test 18
	if( !x_validate_fen_str_test_18( p ) )
		return FEN_STR_WHITE_PAWN_ON_LAST_RANK;
	// Test 19
	if( !x_validate_fen_str_test_19( p ) )
		return FEN_STR_BLACK_PAWN_ON_LAST_RANK;
	// Test 20
	if( !x_validate_fen_str_test_20( p ) )
		return FEN_STR_INVALID_NUMBER_OF_WHITE_KINGS;
	// Test 21
	if( !x_validate_fen_str_test_21( p ) )
		return FEN_STR_INVALID_NUMBER_OF_BLACK_KINGS;
	// Test 22
	if( !x_validate_fen_str_test_22( p ) )
		return FEN_STR_WHITE_KING_CAN_BE_CAPTURED;
	// Test 23
	if( !x_validate_fen_str_test_23( p ) )
		return FEN_STR_BLACK_KING_CAN_BE_CAPTURED;
	// Test 24
	if( !x_validate_fen_str_test_24( p ) )
		return FEN_STR_CHESS960_PPF_CONTRADICTS_CAF_ERROR;

	return FEN_STR_NO_ERRORS;
}

/****************************
 **** External functions ****
 ****************************/

// Validates a square name such as "a1" or "e4"
bool
valid_sq_name( const char *sq_name )
{
	return sq_name &&
		str_matches_pattern( sq_name, "^[abcdefgh][12345678]$" );
}

/**************************
 **** Static functions ****
 **************************/

static bool
x_validate_fen_str_test_1( const char *fen_str )
{
	size_t fen_string_length = strlen( fen_str );

	return ( fen_string_length < FEN_STR_MIN_LENGTH ||
		fen_string_length > FEN_STR_MAX_LENGTH ) ? false : true;
}

static bool
x_validate_fen_str_test_2( const char *fen_str )
{
	return str_matches_pattern(
		fen_str, "^[KkQqRrBbNnPp/acdefghw 0123456789-]*$" );
}

static bool
x_validate_fen_str_test_3( const char *fen_str )
{
	return str_matches_pattern(
		fen_str, "^[^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+$" );
}

static bool
x_validate_fen_str_test_4( const char *fen_str )
{
// RRE, row regex
#define RRE "[KkQqRrBbNnPp12345678]{1,8}"

	char *ppf_structure_regex =
		"^"
			RRE "/" RRE "/" RRE "/" RRE "/" RRE "/" RRE "/" RRE "/" RRE
		"$";
	char *ppf = nth_field_of_fen_str( fen_str, x_writable_mem, 1 );

	return str_matches_pattern( ppf, ppf_structure_regex );
}

static bool
x_validate_fen_str_test_5( const char *fen_str )
{
	char *ppf = nth_field_of_fen_str( fen_str, x_writable_mem, 1 );

	return ! str_matches_pattern( ppf, "^.*[12345678]{2,}.*$" );
}

static bool
x_validate_fen_str_test_6( const char *fen_str )
{
	const char *ppf = nth_field_of_fen_str( fen_str, x_writable_mem, 1 );

	char more_writable_mem[ PPF_MAX_LENGTH + 1 ];
	for( int i = 1; i <= 8; i++ ) {
		const char *rank = nth_rank_of_ppf( ppf, more_writable_mem, i );
		int rank_sum = x_calc_rank_sum( rank );
		if( rank_sum != 8 ) {
			return false;
		}
	}

	return true;
}

static bool
x_validate_fen_str_test_7( const char *fen_str )
{
	char *active_color = nth_field_of_fen_str( fen_str, x_writable_mem, 2 );

	if( !strcmp( active_color, "w" ) || !strcmp( active_color, "b" ) ) {
		return true;
	}

	return false;
}

static bool
x_validate_fen_str_test_8( const char *fen_str )
{
	const char *ca = // ca, castling availability
		nth_field_of_fen_str( fen_str, x_writable_mem, 3 );

	return
		!strcmp( ca,   "-" ) || !strcmp( ca,    "K" ) ||
		!strcmp( ca,   "Q" ) || !strcmp( ca,    "k" ) ||
		!strcmp( ca,   "q" ) || !strcmp( ca,   "KQ" ) ||
		!strcmp( ca,  "Kk" ) || !strcmp( ca,   "Kq" ) ||
		!strcmp( ca,  "Qk" ) || !strcmp( ca,   "Qq" ) ||
		!strcmp( ca,  "kq" ) || !strcmp( ca,  "KQk" ) ||
		!strcmp( ca, "KQq" ) || !strcmp( ca,  "Kkq" ) ||
		!strcmp( ca, "Qkq" ) || !strcmp( ca, "KQkq" );
}

static bool
x_validate_fen_str_test_9( const char *fen_str )
{
	const char *epts = nth_field_of_fen_str( fen_str, x_writable_mem, 4 );

	return !strcmp( epts, "-" ) ||
		str_matches_pattern( epts, "^[abcdefgh][36]$" );
}

static bool
x_validate_fen_str_test_10( const char *fen_str )
{
	const char *hmc = nth_field_of_fen_str( fen_str, x_writable_mem, 5 );

	return
		!strcmp( hmc, "0" ) ||
		(
			str_matches_pattern( hmc, "^[123456789][0123456789]{0,3}$" ) &&
			atoi( hmc ) > 0 &&
			atoi( hmc ) < 4096
		);
}

static bool
x_validate_fen_str_test_11( const char *fen_str )
{
	const char *fmn = nth_field_of_fen_str( fen_str, x_writable_mem, 6 );

	return
		str_matches_pattern( fmn, "^[123456789][0123456789]{0,3}$" ) &&
		atoi( fmn ) > 0 &&
		atoi( fmn ) < 4096;
}

static bool
x_validate_fen_str_test_12( const char *fen_str, uint64_t settings )
{
	if( settings & CFSV_BF_CHESS960 )
		return true;

	const char *ca = nth_field_of_fen_str( fen_str, x_writable_mem, 3 );
	// We want ca pointing somewhere else than writable_mem before
	// reusing the array
	char five_bytes[ 4 + 1 ];
	strcpy( five_bytes, ca );
	ca = five_bytes;

	const char *ppf = nth_field_of_fen_str( fen_str, x_writable_mem, 1 );

	return x_ppf_and_caf_agree( ppf, ca );
}

static bool
x_validate_fen_str_test_13( const char *fen_str )
{
	const char *eptsf = nth_field_of_fen_str( fen_str, x_writable_mem, 4 ),
		*hmcf = nth_field_of_fen_str( fen_str, x_writable_mem, 5 );

	assert( strlen( eptsf ) > 0 && strlen( eptsf ) < 3 &&
		strlen( hmcf ) > 0 && strlen( hmcf ) < 5 );

	// ( ( EPTS field is something else than "-" ) --> ( HMC field is "0" ) )
	// <=>
	// ( ( EPTS field is "-" ) OR ( HMC field is "0" ) )
	return !strcmp( eptsf, "-" ) || !strcmp( hmcf, "0" );
}

static bool
x_validate_fen_str_test_14( const char *fen_str )
{
	const char *eptsf = nth_field_of_fen_str( fen_str, x_writable_mem, 4 );
	char three_bytes[ 2 + 1 ];
	strcpy( three_bytes, eptsf );
	eptsf = three_bytes;

	const char *ac = nth_field_of_fen_str( fen_str, x_writable_mem, 2 );

	return
		!strcmp( eptsf, "-" ) ||
		( !strcmp( ac, "w" ) && str_matches_pattern( eptsf, "^[abcdefgh]6$" ) ) ||
		( !strcmp( ac, "b" ) && str_matches_pattern( eptsf, "^[abcdefgh]3$" ) );
}

static bool
x_validate_fen_str_test_15( const Pos *p )
{
	const char *epts = epts_from_pos_var( p );
	char sq_of_double_advanced_pawn[ 3 ] = { 0 };

	if( !strcmp( "-", epts ) )
		return true;

	sq_of_double_advanced_pawn[ 0 ] = *epts;
	sq_of_double_advanced_pawn[ 1 ] = whites_turn( p ) ? '5' : '4';

	return whites_turn( p ) ?
		occupant_of_sq( p, sq_of_double_advanced_pawn ) == BLACK_PAWN :
		occupant_of_sq( p, sq_of_double_advanced_pawn ) == WHITE_PAWN;
}

static bool
x_validate_fen_str_test_16( const Pos *p )
{
	return !( p->pieces[ WHITE_PAWN ] & SS_RANK_1 );
}

static bool
x_validate_fen_str_test_17( const Pos *p )
{
	return !( p->pieces[ BLACK_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_str_test_18( const Pos *p )
{
	return !( p->pieces[ WHITE_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_str_test_19( const Pos *p )
{
	return !( p->pieces[ BLACK_PAWN ] & SS_RANK_1 );
}

static bool
x_validate_fen_str_test_20( const Pos *p )
{
	return bb_is_sq_bit( p->pieces[ WHITE_KING ] );
}

static bool
x_validate_fen_str_test_21( const Pos *p )
{
	return bb_is_sq_bit( p->pieces[ BLACK_KING ] );
}

static bool
x_validate_fen_str_test_22( const Pos *p )
{
	return whites_turn( p ) || !king_can_be_captured( p );
}

static bool
x_validate_fen_str_test_23( const Pos *p )
{
	return !whites_turn( p ) || !king_can_be_captured( p );
}

#define FIND_ROOKS_POSSIBLE_SQS( king, dir, bb ) \
sq = p->pieces[ king ]; \
while( ( sq = sq_nav( sq, dir ) ) ) \
	bb |= sq;

static bool
x_validate_fen_str_test_24( const Pos *p )
{
	bool K = white_has_h_side_castling_right( p ),
		Q = white_has_a_side_castling_right( p ),
		k = black_has_h_side_castling_right( p ),
		q = black_has_a_side_castling_right( p );

	Bitboard
		white_kings_possible_sqs = ( SB.b1 | SB.c1 | SB.d1 | SB.e1 | SB.f1 | SB.g1 ),
		black_kings_possible_sqs = ( SB.b8 | SB.c8 | SB.d8 | SB.e8 | SB.f8 | SB.g8 );

	if( ( ( K || Q ) && !( p->pieces[ WHITE_KING ] & white_kings_possible_sqs ) ) ||
		( ( k || q ) && !( p->pieces[ BLACK_KING ] & black_kings_possible_sqs ) ) )
		return false;

	Bitboard sq,
		white_rooks_possible_sqs_to_east = 0, white_rooks_possible_sqs_to_west = 0,
		black_rooks_possible_sqs_to_east = 0, black_rooks_possible_sqs_to_west = 0;

	FIND_ROOKS_POSSIBLE_SQS( WHITE_KING, EAST, white_rooks_possible_sqs_to_east )
	FIND_ROOKS_POSSIBLE_SQS( WHITE_KING, WEST, white_rooks_possible_sqs_to_west )
	FIND_ROOKS_POSSIBLE_SQS( BLACK_KING, EAST, black_rooks_possible_sqs_to_east )
	FIND_ROOKS_POSSIBLE_SQS( BLACK_KING, WEST, black_rooks_possible_sqs_to_west )

	return !(
		( K && !( p->pieces[ WHITE_ROOK ] & white_rooks_possible_sqs_to_east ) ) ||
		( Q && !( p->pieces[ WHITE_ROOK ] & white_rooks_possible_sqs_to_west ) ) ||
		( k && !( p->pieces[ BLACK_ROOK ] & black_rooks_possible_sqs_to_east ) ) ||
		( q && !( p->pieces[ BLACK_ROOK ] & black_rooks_possible_sqs_to_west ) ) );
}

#undef FIND_ROOKS_POSSIBLE_SQS

// Returns the rank sum of a FEN string representation of a rank
static int
x_calc_rank_sum( const char *rank )
{
	// 'rank' should not be null or an empty string and it should be at most
	// eight chars long
	assert( rank && strcmp( rank, "" ) && strlen( rank ) < 9 );

	int rank_sum = 0;
	while( *rank ) { // Loop until rank points to '\0'
		if( *rank == 'K' || *rank == 'k' || *rank == 'Q' || *rank == 'q' ||
			*rank == 'R' || *rank == 'r' || *rank == 'B' || *rank == 'b' ||
			*rank == 'N' || *rank == 'n' || *rank == 'P' || *rank == 'p' ) {
			++rank_sum;
		} else if( *rank > '0' && *rank < '9' ) {
			rank_sum += ( *rank - 48 );
		} else {
			assert( 1 == 2 );
		}

		++rank; // Point to the next char of the string
	}

	return rank_sum;
}

// Checks that castling rights make sense what comes to king and rook
// placement. For example, if the castling availability field is "K",
// then there must be a white king on e1 and a white rook on h1.
static bool
x_ppf_and_caf_agree( const char *ppf, const char *ca )
{
	assert( strlen( ca ) > 0 && str_matches_pattern( ca, "^(-|K?Q?k?q?)$" ) );

	bool white_has_kingside_castling = str_matches_pattern( ca, "^KQ?k?q?$" ),
		white_has_queenside_castling = str_matches_pattern( ca, "^K?Qk?q?$" ),
		black_has_kingside_castling = str_matches_pattern( ca, "^K?Q?kq?$" ),
		black_has_queenside_castling = str_matches_pattern( ca, "^K?Q?k?q$" ),
		white_king_on_e1 = ( x_occupant_of_sq( ppf, "e1" ) == 'K' ),
		black_king_on_e8 = ( x_occupant_of_sq( ppf, "e8" ) == 'k' ),
		white_rook_on_a1 = ( x_occupant_of_sq( ppf, "a1" ) == 'R' ),
		white_rook_on_h1 = ( x_occupant_of_sq( ppf, "h1" ) == 'R' ),
		black_rook_on_a8 = ( x_occupant_of_sq( ppf, "a8" ) == 'r' ),
		black_rook_on_h8 = ( x_occupant_of_sq( ppf, "h8" ) == 'r' );

	bool
		white_kingside_castling_is_valid =
			( !white_has_kingside_castling  || ( white_king_on_e1 && white_rook_on_h1 ) ),
		white_queenside_castling_is_valid =
			( !white_has_queenside_castling || ( white_king_on_e1 && white_rook_on_a1) ),
		black_kingside_castling_is_valid =
			( !black_has_kingside_castling  || ( black_king_on_e8 && black_rook_on_h8 ) ),
		black_queenside_castling_is_valid =
			( !black_has_queenside_castling || ( black_king_on_e8 && black_rook_on_a8 ) );

	return white_kingside_castling_is_valid && white_queenside_castling_is_valid &&
		black_kingside_castling_is_valid && black_queenside_castling_is_valid;
}

// Returns the type of chessman on square 'sq'. For example, if there is
// a white king on square e1, then x_occupant_of_sq( ppf, "e1" ) returns "K".
// For an empty square "-" is returned.
static char
x_occupant_of_sq( const char *ppf, const char *sq )
{
	assert( str_matches_pattern( sq, "^[abcdefgh][12345678]$" ) );

	char rank_num_str[ 2 ] = { sq[ 1 ], '\0' };
	char writable_mem_for_ppf[ PPF_MAX_LENGTH + 1 ];
	const char *rank =
		nth_rank_of_ppf( ppf, writable_mem_for_ppf, atoi( rank_num_str ) );
	char constant_length_rank[ 8 + 1 ];

	convert_ppf_rank_digits_to_dashes( rank, constant_length_rank );

	char file = sq[ 0 ];
	return constant_length_rank[ file - 97 ]; // 'a' - 97 = 0, 'h' - 97 = 7
}

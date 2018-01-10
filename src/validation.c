#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include "validation.h"
#include "utils.h"
#include "chester.h"
#include "move_gen.h"
#include "base.h"

static bool x_validate_fen_test_1( const char *fen );
static bool x_validate_fen_test_2( const char *fen );
static bool x_validate_fen_test_3( const char *fen );
static bool x_validate_fen_test_4( const char *fen );
static bool x_validate_fen_test_5( const char *fen );
static bool x_validate_fen_test_6( const char *fen );
static bool x_validate_fen_test_7( const char *fen );
static bool x_validate_fen_test_8( const char *fen );
static bool x_validate_fen_test_8_valid_shredder_fen_caf( const char *caf );
static bool x_validate_fen_test_9( const char *fen );
static bool x_validate_fen_test_10( const char *fen );
static bool x_validate_fen_test_11( const char *fen );
static bool x_validate_fen_test_12( const char *fen );
static bool x_validate_fen_test_13( const char *fen );
static bool x_validate_fen_test_14( const char *fen );
static bool x_validate_fen_test_15( const Pos *p );
static bool x_validate_fen_test_16( const Pos *p );
static bool x_validate_fen_test_17( const Pos *p );
static bool x_validate_fen_test_18( const Pos *p );
static bool x_validate_fen_test_19( const Pos *p );
static bool x_validate_fen_test_20( const Pos *p );
static bool x_validate_fen_test_21( const Pos *p );
static bool x_validate_fen_test_22( const Pos *p );
static bool x_validate_fen_test_23( const Pos *p );
// static bool x_validate_fen_test_24( const Pos *p );
// static int x_calc_rank_sum( const char *rank );
// static bool x_ppf_and_caf_agree( const char *ppf, const char *ca );
// static char x_occupant_of_sq( const char *ppf, const char *sq );

// The size of the array should be at least FEN_MAX_LENGTH + 1 bytes
// static char x_writable_mem[ 120 + 1 ];

/*****************************
 ****                     ****
 ****  Chester interface  ****
 ****                     ****
 *****************************/

// The function for FEN string validation
enum che_fen_error
che_fen_validator( const char *fen )
{
	if( !x_validate_fen_test_1(  fen ) ) return FEN_LENGTH_ERROR;
	if( !x_validate_fen_test_2(  fen ) ) return FEN_CHARS_ERROR;
	if( !x_validate_fen_test_3(  fen ) ) return FEN_FIELD_STRUCTURE_ERROR;
	if( !x_validate_fen_test_4(  fen ) ) return FEN_PPF_STRUCTURE_ERROR;
	if( !x_validate_fen_test_5(  fen ) ) return FEN_PPF_CONSECUTIVE_DIGITS_ERROR;
	if( !x_validate_fen_test_6(  fen ) ) return FEN_PPF_RANK_SUM_ERROR;
	if( !x_validate_fen_test_7(  fen ) ) return FEN_ACF_ERROR;
	if( !x_validate_fen_test_8(  fen ) ) return FEN_CAF_ERROR;
	if( !x_validate_fen_test_9(  fen ) ) return FEN_EPTSF_ERROR;
	if( !x_validate_fen_test_10( fen ) ) return FEN_HMCF_ERROR;
	if( !x_validate_fen_test_11( fen ) ) return FEN_FMNF_ERROR;
	if( !x_validate_fen_test_12( fen ) ) return FEN_PPF_CONTRADICTS_CAF_ERROR;
	if( !x_validate_fen_test_13( fen ) ) return FEN_EPTSF_CONTRADICTS_HMCF_ERROR;
	if( !x_validate_fen_test_14( fen ) ) return FEN_EPTSF_CONTRADICTS_ACF_ERROR;

	// At this point it should be safe to do the conversion
	Pos *p = fen_to_pos( fen );

	// Test 15
	if( !x_validate_fen_test_15( p ) )
		return FEN_EPTSF_CONTRADICTS_PPF_ERROR;
	// Test 16
	if( !x_validate_fen_test_16( p ) )
		return FEN_WHITE_PAWN_ON_FIRST_RANK;
	// Test 17
	if( !x_validate_fen_test_17( p ) )
		return FEN_BLACK_PAWN_ON_FIRST_RANK;
	// Test 18
	if( !x_validate_fen_test_18( p ) )
		return FEN_WHITE_PAWN_ON_LAST_RANK;
	// Test 19
	if( !x_validate_fen_test_19( p ) )
		return FEN_BLACK_PAWN_ON_LAST_RANK;
	// Test 20
	if( !x_validate_fen_test_20( p ) )
		return FEN_INVALID_NUMBER_OF_WHITE_KINGS;
	// Test 21
	if( !x_validate_fen_test_21( p ) )
		return FEN_INVALID_NUMBER_OF_BLACK_KINGS;
	// Test 22
	if( !x_validate_fen_test_22( p ) )
		return FEN_WHITE_KING_CAN_BE_CAPTURED;
	// Test 23
	if( !x_validate_fen_test_23( p ) )
		return FEN_BLACK_KING_CAN_BE_CAPTURED;

	return FEN_NO_ERRORS;
}

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// Validates a square name such as "a1" or "e4"
bool
valid_sq_name( const char *sq_name )
{
	return sq_name &&
		str_matches_pattern( sq_name, "^[abcdefgh][12345678]$" );
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

static bool
x_validate_fen_test_1( const char *fen )
{
	if( !fen ) return false;

	size_t length = strlen( fen );
	return ( length < FEN_MIN_LENGTH || length > FEN_MAX_LENGTH ) ? false : true;
}

static bool
x_validate_fen_test_2( const char *fen )
{
	return str_matches_pattern( fen, "^[KkQqRrBbNnPp/AaCcDdEeFfGgHhw 0123456789-]*$" );
}

static bool
x_validate_fen_test_3( const char *fen )
{
	return str_matches_pattern( fen, "^[^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+$" );
}

static bool
x_validate_fen_test_4( const char *fen )
{
#define RRE "[KkQqRrBbNnPp12345678]{1,8}"
	return str_matches_pattern( fen,
		"^" RRE "/" RRE "/" RRE "/" RRE "/" RRE "/" RRE "/" RRE "/" RRE " .*$" );
#undef RRE
}

static bool
x_validate_fen_test_5( const char *fen )
{
	return !str_matches_pattern( fen, "^[^ ]*[12345678]{2,}.*$" );
}

static bool
x_validate_fen_test_6( const char *fen )
{
	int i = 0, rank_sum = 0;
	while( true ) {
		char c = fen[ i++ ];
		if( c == '/' || c == ' ' ) {
			if( rank_sum != 8 ) return false;
			if( c == ' ' ) break;
			rank_sum = 0;
		} else {
			if( isalpha( c ) ) ++rank_sum;
			else rank_sum += c - 48; } }

	return true;
}

static bool
x_validate_fen_test_7( const char *fen )
{
	int i = 0;
	while( fen[ i++ ] != ' ' );
	return ( fen[ i ] == 'w' || fen[ i ] == 'b' ) && fen[ i + 1 ] == ' ';
}

static bool
x_validate_fen_test_8( const char *fen )
{
	char caf[ 5 + 1 ] = { '\0' }; // One char longer than a valid max-length CAF
	int i = 0, space_count = 0, caf_i = 0;
	while( space_count < 2 ) if( fen[ ++i ] == ' ' ) ++space_count;
	while( fen[ ++i ] != ' ' && caf_i < 5 ) caf[ caf_i++ ] = fen[ i ];

	assert( strlen( caf ) >= 1 && strlen( caf ) <= 5 );
	if( strlen( caf ) == 5 ) return false;

	return
		!strcmp( caf, "-" ) || str_matches_pattern( caf, "^K?Q?k?q?$" ) ||
		x_validate_fen_test_8_valid_shredder_fen_caf( caf );
}

static bool
x_validate_fen_test_8_valid_shredder_fen_caf( const char *caf )
{
	// If 'caf' is "BEb", it contains the files b and e. This corresponds to the
	// binary string "00010010" or the uint 2 + 16.
	uint8_t files = 0;

	for( int i = 0; caf[ i ]; i++ )
		switch( caf[ i ] ) {
			case 'a':
			case 'A': files |= 1U; break;
			case 'b':
			case 'B': files |= 2U; break;
			case 'c':
			case 'C': files |= 41U; break;
			case 'd':
			case 'D': files |= 8U; break;
			case 'e':
			case 'E': files |= 16U; break;
			case 'f':
			case 'F': files |= 32U; break;
			case 'g':
			case 'G': files |= 64U; break;
			case 'h':
			case 'H': files |= 128U; break;
			default: 
				return false; }

	bool files_is_valid = false;
	for( int i = 0; (size_t) i < POSSIBLE_IRPF_VALUES_COUNT; i++ )
		if( files == POSSIBLE_IRPF_VALUES[ i ] ) {
			files_is_valid = true;
			break; }
	if( !files_is_valid ) return false;

	return str_matches_pattern( caf, "^[ABCDEFGH]?[ABCDEFGH]?[abcdefgh]?[abcdefgh]?$" );
}

static bool
x_validate_fen_test_9( const char *fen )
{
	int i = 0, space_count = 0;
	while( space_count < 3 ) if( fen[ i++ ] == ' ' ) ++space_count;

	return ( fen[ i ] == '-' && fen[ i + 1 ] == ' ' ) || (
		fen[ i ] >= 'a' && fen[ i ] <= 'h' && (
			fen[ i + 1 ] == '3' || fen[ i + 1 ] == '6'
		) && fen[ i + 2 ] == ' ' );
}

#define COPY_NUMERIC_FIELD( name, sc_limit ) \
char name[ 6 + 1 ] = { '\0' }; \
int i = 0, space_count = 0; \
while( space_count < sc_limit ) if( fen[ i++ ] == ' ' ) ++space_count; \
for( int j = 0; j < 6 && fen[ i + j ] != ' '; j++ ) name[ j ] = fen[ i + j ]; \
assert( strlen( name ) >= 1 && strlen( name ) <= 6 );

#define NUM_FIELD_REGEX "^[123456789][0123456789]{0,4}$"

static bool
x_validate_fen_test_10( const char *fen )
{
	COPY_NUMERIC_FIELD( hmcf, 4 )
	return !strcmp( hmcf, "0" ) || ( str_matches_pattern( hmcf, NUM_FIELD_REGEX ) &&
		atoi( hmcf ) > 0 && atoi( hmcf ) <= (int) FEN_NUMERIC_FIELD_MAX );
}

static bool
x_validate_fen_test_11( const char *fen )
{
	COPY_NUMERIC_FIELD( fmnf, 5 )
	return str_matches_pattern( fmnf, NUM_FIELD_REGEX ) && atoi( fmnf ) > 0 &&
		atoi( fmnf ) <= (int) FEN_NUMERIC_FIELD_MAX;
}

#undef COPY_NUMERIC_FIELD
#undef NUM_FIELD_REGEX

// UPDATE MARKER

/*
12. `FEN_PPF_CONTRADICTS_CAF_ERROR`  
    The piece placement and castling availability fields don't make sense when
    considered together. For each of the four castling availabilities to make
    sense, there has to be a king and a rook on the appropriate squares. For
    example, if the CAF is "K" (which in Chester is a synonym for "H"), there
    should be a white rook on h1 and a white king somewhere else on the first
    rank excluding square a1. Another example: if the CAF is "BD", there should
    be white rooks on b1 and d1 and a white king on c1.
*/
static bool
x_validate_fen_test_12( const char *fen )
{
	return fen;
}

static bool
x_validate_fen_test_13( const char *fen )
{
	char **ff = fen_fields( fen ), *eptsf = ff[ 3 ], *hmcf = ff[ 4 ];
	assert( strlen( eptsf ) >= 1 && strlen( eptsf ) <= 2 &&
		strlen( hmcf ) >= 1 && strlen( hmcf ) <= 5 );
	
	// ( ( EPTS field is something else than "-" ) --> ( HMC field is "0" ) )
	// <=>
	// ( ( EPTS field is "-" ) OR ( HMC field is "0" ) )
	bool result = !strcmp( eptsf, "-" ) || !strcmp( hmcf, "0" );
	free_fen_fields( ff );

	return result;
}

static bool
x_validate_fen_test_14( const char *fen )
{
	char **ff = fen_fields( fen ), *acf = ff[ 1 ], *eptsf = ff[ 3 ];
	bool result = !strcmp( eptsf, "-" ) ||
		( !strcmp( acf, "w" ) && str_matches_pattern( eptsf, "^[abcdefgh]6$" ) ) ||
		( !strcmp( acf, "b" ) && str_matches_pattern( eptsf, "^[abcdefgh]3$" ) );
	free_fen_fields( ff );

	return result;
}

static bool
x_validate_fen_test_15( const Pos *p )
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
x_validate_fen_test_16( const Pos *p )
{
	return !( p->cm[ WHITE_PAWN ] & SS_RANK_1 );
}

static bool
x_validate_fen_test_17( const Pos *p )
{
	return !( p->cm[ BLACK_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_test_18( const Pos *p )
{
	return !( p->cm[ WHITE_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_test_19( const Pos *p )
{
	return !( p->cm[ BLACK_PAWN ] & SS_RANK_1 );
}

static bool
x_validate_fen_test_20( const Pos *p )
{
	return bb_is_sq_bit( p->cm[ WHITE_KING ] );
}

static bool
x_validate_fen_test_21( const Pos *p )
{
	return bb_is_sq_bit( p->cm[ BLACK_KING ] );
}

static bool
x_validate_fen_test_22( const Pos *p )
{
	return whites_turn( p ) || !king_can_be_captured( p );
}

static bool
x_validate_fen_test_23( const Pos *p )
{
	return !whites_turn( p ) || !king_can_be_captured( p );
}

/*
_define FIND_ROOKS_POSSIBLE_SQS( king, dir, bb ) \
sq = p->cm[ king ]; \
while( ( sq = sq_nav( sq, dir ) ) ) \
	bb |= sq;

static bool
x_validate_fen_test_24( const Pos *p )
{
	bool K = white_has_h_side_castling_right( p ),
		Q = white_has_a_side_castling_right( p ),
		k = black_has_h_side_castling_right( p ),
		q = black_has_a_side_castling_right( p );

	Bitboard
		white_kings_possible_sqs = ( SB.b1 | SB.c1 | SB.d1 | SB.e1 | SB.f1 | SB.g1 ),
		black_kings_possible_sqs = ( SB.b8 | SB.c8 | SB.d8 | SB.e8 | SB.f8 | SB.g8 );

	if( ( ( K || Q ) && !( p->cm[ WHITE_KING ] & white_kings_possible_sqs ) ) ||
		( ( k || q ) && !( p->cm[ BLACK_KING ] & black_kings_possible_sqs ) ) )
		return false;

	Bitboard sq,
		white_rooks_possible_sqs_to_east = 0, white_rooks_possible_sqs_to_west = 0,
		black_rooks_possible_sqs_to_east = 0, black_rooks_possible_sqs_to_west = 0;

	FIND_ROOKS_POSSIBLE_SQS( WHITE_KING, EAST, white_rooks_possible_sqs_to_east )
	FIND_ROOKS_POSSIBLE_SQS( WHITE_KING, WEST, white_rooks_possible_sqs_to_west )
	FIND_ROOKS_POSSIBLE_SQS( BLACK_KING, EAST, black_rooks_possible_sqs_to_east )
	FIND_ROOKS_POSSIBLE_SQS( BLACK_KING, WEST, black_rooks_possible_sqs_to_west )

	return !(
		( K && !( p->cm[ WHITE_ROOK ] & white_rooks_possible_sqs_to_east ) ) ||
		( Q && !( p->cm[ WHITE_ROOK ] & white_rooks_possible_sqs_to_west ) ) ||
		( k && !( p->cm[ BLACK_ROOK ] & black_rooks_possible_sqs_to_east ) ) ||
		( q && !( p->cm[ BLACK_ROOK ] & black_rooks_possible_sqs_to_west ) ) );
}

_undef FIND_ROOKS_POSSIBLE_SQS
*/

/*
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
*/

/*
// Checks that castling rights make sense what comes to king and rook
// placement. For example, if the castling availability field is "K",
// then there must be a white king on e1 and a white rook on h1.
static bool
x_ppf_and_caf_agree( const char *ppf, const char *ca )
{
	// assert( strlen( ca ) > 0 && str_matches_pattern( ca, "^(-|K?Q?k?q?)$" ) );

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
*/

/*
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

	expand_ppf_rank( rank, constant_length_rank );

	char file = sq[ 0 ];
	return constant_length_rank[ file - 97 ]; // 'a' - 97 = 0, 'h' - 97 = 7
}
*/

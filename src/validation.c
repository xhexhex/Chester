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
#include "extra.h"

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
static void x_validate_fen_test_12_remove_irrelevant_chessmen(
	char *rank, bool rank_1 );
static bool x_validate_fen_test_12_king_placement_contradicts_caf(
	const char *r1, const char *r8, const char *caf );
static char x_validate_fen_test_12_file_of_king( const char *rank );
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

	if( !x_validate_fen_test_15( p ) ) return FEN_EPTSF_CONTRADICTS_PPF_ERROR;
	if( !x_validate_fen_test_16( p ) ) return FEN_WHITE_PAWN_ON_FIRST_RANK;
	if( !x_validate_fen_test_17( p ) ) return FEN_BLACK_PAWN_ON_FIRST_RANK;
	if( !x_validate_fen_test_18( p ) ) return FEN_WHITE_PAWN_ON_LAST_RANK;
	if( !x_validate_fen_test_19( p ) ) return FEN_BLACK_PAWN_ON_LAST_RANK;
	if( !x_validate_fen_test_20( p ) ) return FEN_INVALID_NUMBER_OF_WHITE_KINGS;
	if( !x_validate_fen_test_21( p ) ) return FEN_INVALID_NUMBER_OF_BLACK_KINGS;
	if( !x_validate_fen_test_22( p ) ) return FEN_WHITE_KING_CAN_BE_CAPTURED;
	if( !x_validate_fen_test_23( p ) ) return FEN_BLACK_KING_CAN_BE_CAPTURED;

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
	return sq_name && str_matches_pattern( sq_name, "^[abcdefgh][12345678]$" );
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

	return  str_matches_pattern( caf, STD_FEN_CAF_REGEX ) ||
		x_validate_fen_test_8_valid_shredder_fen_caf( caf );
}

static bool
x_validate_fen_test_8_valid_shredder_fen_caf( const char *caf )
{
	for( int i = 0; i < (int) SHREDDER_FEN_CAFS_COUNT; i++ )
		if( !strcmp( caf, SHREDDER_FEN_CAFS[ i ] ) )
			return true;
	
	return false;
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
	// if( strcmp( fen, "2r1k3/8/8/8/4P3/8/8/4K2R b Hc e3 0 100" ) ) return true;
	
	if( str_matches_pattern( fen, "^[^ ]+ [wb] - .+$" ) ) return true;
	
	char **ff = fen_fields( fen ), eppf[ PPF_MAX_LENGTH + 1 ],
		r1[ 8 + 1 ] = { '\0' }, r8[ 8 + 1 ] = { '\0' }, // Rank 1, 8
		*caf = ff[ 2 ], ecaf[ 9 + 1 ] = { '\0' }; // ecaf, expanded CAF
	
	expand_ppf( ff[ 0 ], eppf );
	for( int i = PPF_MAX_LENGTH - 1, j = 7; j >= 0; i--, j-- ) r1[ j ] = eppf[ i ];
	for( int i = 0, j = 0; j < 8; i++, j++ ) r8[ j ] = eppf[ i ];

	x_validate_fen_test_12_remove_irrelevant_chessmen( r1, true );
	x_validate_fen_test_12_remove_irrelevant_chessmen( r8, false );
	assert( strlen( r1 ) == 8 && strlen( r8 ) == 8 );
	assert( str_matches_pattern( r1, "^[-KR]*$" ) );
	assert( str_matches_pattern( r8, "^[-kr]*$" ) );

	bool early_return =
		x_validate_fen_test_12_king_placement_contradicts_caf( r1, r8, caf );
	expand_caf( caf, ecaf );
	free_fen_fields( ff );
	if( early_return ) return false;

	bool K = ( ecaf[ 1 ] != '-' ), Q = ( ecaf[ 0 ] != '-' ),
		k = ( ecaf[ 3 ] != '-' ), q = ( ecaf[ 2 ] != '-' );
	char file_of_wk = x_validate_fen_test_12_file_of_king( r1 ),
		file_of_bk = x_validate_fen_test_12_file_of_king( r8 );
	if( file_of_wk != file_of_bk && ( K || Q ) && ( k || q ) ) return false;
	// Same kind of test should be done on the rooks
	
	char *rook_left_or_right_of_king[] = {
		"^.*R.*K[-R]+$", "^[-R]+K.*R.*$", "^.*r.*k[-r]+$", "^[-r]+k.*r.*$" };
	for( int i = 0; i < 4; i++ )
		if( ecaf[ i ] != '-' && !str_matches_pattern(
				i < 2 ? r1 : r8, rook_left_or_right_of_king[ i ] ) ) {
			// printf( "### i, ecaf[ i ]: %d %c\n", i, ecaf[ i ] );
			return false;
		}
		
	for( int i = 0; i < 4; i++ ) {
		if( ecaf[ i ] == '-' ) continue;
		// printf( "### \"%s\" %d\n", i < 2 ? r1 : r8, ecaf[ i ] - ( i < 2 ? 'A' : 'a' ) );
		int rrri = ecaf[ i ] - ( i < 2 ? 'A' : 'a' ); // rrri, relevant rook rank index
		if( ( i < 2 && r1[ rrri ] != 'R' ) || ( i >= 2 && r8[ rrri ] != 'r' ) )
		{
			// "nnrkrbqb/pppppppp/8/8/8/8/PPPPPPPP/NNRKRBQB w KQkq - 0 1"
			// printf( "### \"%s\"\n", fen );
			// printf( "### r1, r8, i, rrri, r1[ rrri ]: \"%s\"  \"%s\"  %d  %d  %c\n",
			//	r1, r8, i, rrri, r1[ rrri ] );
			// printf( "\n" );
			return false;
		}
	}

	return true;
}

static char x_validate_fen_test_12_file_of_king( const char *rank )
{
	for( int i = 0; i < 8; i++ )
		if( rank[ i ] == 'K' || rank[ i ] == 'k' ) return 'a' + i;

	return '\0'; // Should not happen
}

static void
x_validate_fen_test_12_remove_irrelevant_chessmen( char *rank, bool rank_1 )
{
	for( int i = 0; i < 8; i++ ) {
		char c = rank[ i ];
		if( ( rank_1 && c != 'K' && c != 'R' ) || ( !rank_1 && c != 'k' && c != 'r' ) )
			rank[ i ] = '-'; }
}

static bool
x_validate_fen_test_12_king_placement_contradicts_caf(
	const char *r1, const char *r8, const char *caf )
{
	bool wk_on_suitable_sq = false, bk_on_suitable_sq = false;
	for( int i = 1; i < 7; i++ ) {
		if( r1[ i ] == 'K' ) wk_on_suitable_sq = true;
		if( r8[ i ] == 'k' ) bk_on_suitable_sq = true; }

	return ( !wk_on_suitable_sq && str_matches_pattern( caf, "^[ABCDEFGHKQ].*$" ) ) ||
		( !bk_on_suitable_sq && str_matches_pattern( caf, "^.*[abcdefghkq]$" ) );
}

/*
static void // It's not possible for 'caf' to have the value "-"
x_validate_fen_test_12_expand_caf( const char *caf, char *ecaf )
{
	for( int i = 0; i < 4; i++ ) ecaf[ i ] = '-';
	size_t ca_count = strlen( caf );
	
	if( ca_count == 1 ) {
		ecaf[ isupper( caf[ 0 ] ) ? 0 : 2 ] = caf[ 0 ];
	} else if( ca_count == 2 ) {
		bool upper_1st = isupper( caf[ 0 ] ), upper_2nd = isupper( caf[ 1 ] );
		if( !upper_1st ) {
			ecaf[ 2 ] = caf[ 0 ];
			ecaf[ 3 ] = caf[ 1 ];
		} else if( !upper_2nd ) {
			ecaf[ 0 ] = caf[ 0 ];
			ecaf[ 2 ] = caf[ 1 ];
		} else {
			ecaf[ 0 ] = caf[ 0 ];
			ecaf[ 1 ] = caf[ 1 ]; }
	} else if( ca_count == 3 ) {
		int uc_count = 0;
		for( int i = 0; i < 3; i++ ) if( isupper( caf[ i ] ) ) ++uc_count;
		ecaf[ 0 ] = caf[ 0 ];
		ecaf[ uc_count == 2 ? 1 : 2 ] = caf[ 1 ];
		ecaf[ uc_count == 2 ? 2 : 3 ] = caf[ 2 ];
		// Avoiding the erratic conversion "Qkq" --> "Q-kq"
		if( uc_count == 1 && tolower( ecaf[ 0 ] ) != ecaf[ 0 ] ) {
			char tmp = ecaf[ 0 ];
			ecaf[ 0 ] = ecaf[ 1 ];
			ecaf[ 1 ] = tmp; }
			MORE PROBLEMS:
			"Kkq" --> "-Kkq"
			"Kq" --> "K-q-"
			"KQq" --> "KQq-"
			"Qk" --> "Q-k-"
	} else if( ca_count == 4 ) {
		strcpy( ecaf, caf );
	} else assert( false );
	
	assert( strlen( ecaf ) >= 1 && strlen( ecaf ) <= 4 );
}
*/

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

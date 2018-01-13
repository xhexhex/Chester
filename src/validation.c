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
// static bool x_validate_fen_test_13( const char *fen );
static bool x_validate_fen_test_14( const char *fen );
static bool x_validate_fen_test_15( const char *fen );
static bool x_validate_fen_test_16( const Pos *p );
static bool x_validate_fen_test_17( const Pos *p );
static bool x_validate_fen_test_18( const Pos *p );
static bool x_validate_fen_test_19( const Pos *p );
static bool x_validate_fen_test_20( const Pos *p );
static bool x_validate_fen_test_21( const Pos *p );
static bool x_validate_fen_test_22( const Pos *p );
static bool x_validate_fen_test_23( const Pos *p );
static bool x_validate_fen_test_24( const Pos *p );

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
	if( !x_validate_fen_test_12( fen ) ) return FEN_KING_PLACEMENT_CONTRADICTS_CAF_ERROR;

	if( !x_validate_fen_test_14( fen ) ) return FEN_EPTSF_CONTRADICTS_HMCF_ERROR;
	if( !x_validate_fen_test_15( fen ) ) return FEN_EPTSF_CONTRADICTS_ACF_ERROR;

	// At this point it should be safe to do the conversion
	Pos *p = fen_to_pos( fen );

	if( !x_validate_fen_test_16( p ) ) return FEN_EPTSF_CONTRADICTS_PPF_ERROR;
	if( !x_validate_fen_test_17( p ) ) return FEN_WHITE_PAWN_ON_FIRST_RANK;
	if( !x_validate_fen_test_18( p ) ) return FEN_BLACK_PAWN_ON_FIRST_RANK;
	if( !x_validate_fen_test_19( p ) ) return FEN_WHITE_PAWN_ON_LAST_RANK;
	if( !x_validate_fen_test_20( p ) ) return FEN_BLACK_PAWN_ON_LAST_RANK;
	if( !x_validate_fen_test_21( p ) ) return FEN_INVALID_NUMBER_OF_WHITE_KINGS;
	if( !x_validate_fen_test_22( p ) ) return FEN_INVALID_NUMBER_OF_BLACK_KINGS;
	if( !x_validate_fen_test_23( p ) ) return FEN_WHITE_KING_CAN_BE_CAPTURED;
	if( !x_validate_fen_test_24( p ) ) return FEN_BLACK_KING_CAN_BE_CAPTURED;

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

	// FEN_KP_CONTRADICTS_CAF_ERROR ... king placement
	bool early_return =
		x_validate_fen_test_12_king_placement_contradicts_caf( r1, r8, caf );
	expand_caf( caf, ecaf );
	free_fen_fields( ff );
	if( early_return ) return false;

	bool K = ( ecaf[ 1 ] != '-' ), Q = ( ecaf[ 0 ] != '-' ),
		k = ( ecaf[ 3 ] != '-' ), q = ( ecaf[ 2 ] != '-' );
	char file_of_wk = x_validate_fen_test_12_file_of_king( r1 ),
		file_of_bk = x_validate_fen_test_12_file_of_king( r8 );
	// FEN_KMA_CONTRADICTS_CAF_ERROR ... king misalignment
	if( file_of_wk != file_of_bk && ( K || Q ) && ( k || q ) ) return false;
	
	char *rook_left_or_right_of_king[] = {
		"^.*R.*K[-R]+$", "^[-R]+K.*R.*$", "^.*r.*k[-r]+$", "^[-r]+k.*r.*$" };
	for( int i = 0; i < 4; i++ )
		if( ecaf[ i ] != '-' && !str_matches_pattern(
				i < 2 ? r1 : r8, rook_left_or_right_of_king[ i ] ) ) {
			// printf( "### i, ecaf[ i ]: %d %c\n", i, ecaf[ i ] );
			return false;
			// FEN_ROOK_ABS_CONTRADICTS_CAF_ERROR
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
			// FEN_ROOK_P_CONTRADICTS_CAF_ERROR
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

static bool
x_validate_fen_test_14( const char *fen )
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
x_validate_fen_test_15( const char *fen )
{
	char **ff = fen_fields( fen ), *acf = ff[ 1 ], *eptsf = ff[ 3 ];
	bool result = !strcmp( eptsf, "-" ) ||
		( !strcmp( acf, "w" ) && str_matches_pattern( eptsf, "^[abcdefgh]6$" ) ) ||
		( !strcmp( acf, "b" ) && str_matches_pattern( eptsf, "^[abcdefgh]3$" ) );
	free_fen_fields( ff );

	return result;
}

static bool
x_validate_fen_test_16( const Pos *p )
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
x_validate_fen_test_17( const Pos *p )
{
	return !( p->cm[ WHITE_PAWN ] & SS_RANK_1 );
}

static bool
x_validate_fen_test_18( const Pos *p )
{
	return !( p->cm[ BLACK_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_test_19( const Pos *p )
{
	return !( p->cm[ WHITE_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_test_20( const Pos *p )
{
	return !( p->cm[ BLACK_PAWN ] & SS_RANK_1 );
}

static bool
x_validate_fen_test_21( const Pos *p )
{
	return bb_is_sq_bit( p->cm[ WHITE_KING ] );
}

static bool
x_validate_fen_test_22( const Pos *p )
{
	return bb_is_sq_bit( p->cm[ BLACK_KING ] );
}

static bool
x_validate_fen_test_23( const Pos *p )
{
	return whites_turn( p ) || !king_can_be_captured( p );
}

static bool
x_validate_fen_test_24( const Pos *p )
{
	return !whites_turn( p ) || !king_can_be_captured( p );
}

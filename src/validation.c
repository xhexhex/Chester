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
static bool x_validate_fen_test_9( const char *fen );
static bool x_validate_fen_test_10( const char *fen );
static bool x_validate_fen_test_11( const char *fen );
static bool x_validate_fen_test_12( const char *fen );
static bool x_validate_fen_test_13( const char *fen );
static bool x_validate_fen_test_14( const char *fen );
static bool x_validate_fen_test_15( const char *fen );
static bool x_validate_fen_test_16( const Pos *p );
static bool x_validate_fen_test_17( const Pos *p );
static bool x_validate_fen_test_18( const Pos *p );
static bool x_validate_fen_test_19( const Pos *p );
static bool x_validate_fen_test_20( const Pos *p );
static void x_remove_irrelevant_chessmen_from_rank( char *rank, bool rank_1 );
static bool x_king_with_ca_right_on_impossible_square(
	const char *r1, const char *r8, const char *ecaf );
static bool x_kings_with_ca_rights_on_different_files(
	const char *r1, const char *r8, const char *ecaf );
static bool x_valid_shredder_fen_caf( const char *caf );
static char x_file_of_king( const char *rank );
static bool x_no_rook_on_square_indicated_by_caf(
	const char *r1, const char *r8, const char *ecaf );
static bool x_rook_on_wrong_side_of_king(
	const char *r1, const char *r8, const char *ecaf );

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
	if( !x_validate_fen_test_13( fen ) ) return FEN_ROOK_PLACEMENT_CONTRADICTS_CAF_ERROR;
	if( !x_validate_fen_test_14( fen ) ) return FEN_EPTSF_CONTRADICTS_HMCF_ERROR;
	if( !x_validate_fen_test_15( fen ) ) return FEN_EPTSF_CONTRADICTS_ACF_ERROR;

	// At this point it should be safe to do the conversion
	Pos *p = fen_to_pos( fen );

	if( !x_validate_fen_test_16( p ) ) return FEN_EPTSF_CONTRADICTS_PPF_ERROR;
	if( !x_validate_fen_test_17( p ) ) return FEN_PAWN_ON_INVALID_RANK_ERROR;
	if( !x_validate_fen_test_18( p ) ) return FEN_INVALID_NUMBER_OF_KINGS_ERROR;
	if( !x_validate_fen_test_19( p ) ) return FEN_KING_CAN_BE_CAPTURED_ERROR;
	if( !x_validate_fen_test_20( p ) ) return FEN_HMCF_CONTRADICTS_FMNF_ERROR;

	return FEN_NO_ERRORS;
}

/*************************
 ****                 ****
 ****  External data  ****
 ****                 ****
 *************************/

// (empty)

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
	return fen && ( strlen( fen ) >= FEN_MIN_LENGTH && strlen( fen ) <= FEN_MAX_LENGTH );
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

	return str_matches_pattern( caf, STD_FEN_CAF_REGEX ) || x_valid_shredder_fen_caf( caf );
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

// Expanded PPF and CAF, PPF ranks 1 and 8 with irrelevant chessmen removed
#define INIT_EPPF_ECAF_R1_R8 char **ff = fen_fields( fen ), eppf[ PPF_MAX_LENGTH + 1 ], \
	ecaf[ 9 + 1 ] = { '\0' }, r1[ 8 + 1 ] = { '\0' }, r8[ 8 + 1 ] = { '\0' }; \
expand_ppf( ff[ 0 ], eppf ); \
expand_caf( ff[ 2 ], ecaf ); \
free_fen_fields( ff ); \
resolve_ambiguous_ecaf( ecaf, fen ); \
for( int i = PPF_MAX_LENGTH - 1, j = 7; j >= 0; i--, j-- ) r1[ j ] = eppf[ i ]; \
for( int i = 0, j = 0; j < 8; i++, j++ ) r8[ j ] = eppf[ i ]; \
x_remove_irrelevant_chessmen_from_rank( r1, true ); \
x_remove_irrelevant_chessmen_from_rank( r8, false ); \
assert( strlen( r1 ) == 8 && strlen( r8 ) == 8 ); \
assert( str_matches_pattern( r1, "^[-KR]*$" ) ); \
assert( str_matches_pattern( r8, "^[-kr]*$" ) );

static bool
x_validate_fen_test_12( const char *fen )
{
	INIT_EPPF_ECAF_R1_R8

	return !strcmp( ecaf, "----" ) || (
		!x_king_with_ca_right_on_impossible_square( r1, r8, ecaf ) &&
		!x_kings_with_ca_rights_on_different_files( r1, r8, ecaf ) );
}

static bool x_validate_fen_test_13( const char *fen )
{
	INIT_EPPF_ECAF_R1_R8

	return !strcmp( ecaf, "----" ) || (
		!x_no_rook_on_square_indicated_by_caf( r1, r8, ecaf ) &&
		!x_rook_on_wrong_side_of_king( r1, r8, ecaf ) );
}

#undef INIT_EPPF_ECAF_R1_R8

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

	if( !strcmp( "-", epts ) ) return true;

	sq_of_double_advanced_pawn[ 0 ] = *epts;
	sq_of_double_advanced_pawn[ 1 ] = whites_turn( p ) ? '5' : '4';

	return whites_turn( p ) ?
		occupant_of_sq( p, sq_of_double_advanced_pawn ) == BLACK_PAWN :
		occupant_of_sq( p, sq_of_double_advanced_pawn ) == WHITE_PAWN;
}

static bool
x_validate_fen_test_17( const Pos *p )
{
	return
		!( p->cm[ WHITE_PAWN ] & SS_RANK_1 ) && !( p->cm[ BLACK_PAWN ] & SS_RANK_1 ) &&
		!( p->cm[ WHITE_PAWN ] & SS_RANK_8 ) && !( p->cm[ BLACK_PAWN ] & SS_RANK_8 );
}

static bool
x_validate_fen_test_18( const Pos *p )
{
	return bb_is_sq_bit( p->cm[ WHITE_KING ] ) && bb_is_sq_bit( p->cm[ BLACK_KING ] );
}

static bool
x_validate_fen_test_19( const Pos *p )
{
	return !king_can_be_captured( p );
}

static bool
x_validate_fen_test_20( const Pos *p )
{
	return p->hmc <= 2 * ( p->fmn - 1 ) + ( whites_turn( p ) ? 0 : 1 );
}

static void
x_remove_irrelevant_chessmen_from_rank( char *rank, bool rank_1 )
{
	for( int i = 0; i < 8; i++ ) {
		char c = rank[ i ];
		if( ( rank_1 && c != 'K' && c != 'R' ) || ( !rank_1 && c != 'k' && c != 'r' ) )
			rank[ i ] = '-'; }
}

static bool
x_king_with_ca_right_on_impossible_square( const char *r1, const char *r8,
	const char *ecaf )
{
	bool wk_on_possible_sq = false, bk_on_possible_sq = false;
	for( int i = 1; i <= 6; i++ ) {
		if( r1[ i ] == 'K' ) wk_on_possible_sq = true;
		if( r8[ i ] == 'k' ) bk_on_possible_sq = true; }

	return ( !wk_on_possible_sq && str_matches_pattern( ecaf, "^-?[ABCDEFGHKQ].*$" ) ) ||
		( !bk_on_possible_sq && str_matches_pattern( ecaf, "^.*[abcdefghkq]-?$" ) );
}

static bool
x_kings_with_ca_rights_on_different_files( const char *r1, const char *r8,
	const char *ecaf )
{
	bool K = ( ecaf[ 1 ] != '-' ), Q = ( ecaf[ 0 ] != '-' ),
		k = ( ecaf[ 3 ] != '-' ), q = ( ecaf[ 2 ] != '-' );
	char file_of_wk = x_file_of_king( r1 ), file_of_bk = x_file_of_king( r8 );
	return file_of_wk != file_of_bk && ( K || Q ) && ( k || q );
}

static bool
x_valid_shredder_fen_caf( const char *caf )
{
	for( int i = 0; i < (int) SHREDDER_FEN_CAFS_COUNT; i++ )
		if( !strcmp( caf, SHREDDER_FEN_CAFS[ i ] ) )
			return true;
	
	return false;
}

static char x_file_of_king( const char *rank )
{
	for( int i = 0; i < 8; i++ )
		if( rank[ i ] == 'K' || rank[ i ] == 'k' ) return 'a' + i;

	return '\0'; // Should not happen
}

static bool
x_no_rook_on_square_indicated_by_caf( const char *r1, const char *r8, const char *ecaf )
{
	for( int i = 0; i < 4; i++ ) {
		if( ecaf[ i ] == '-' ) continue;

		int rank_i = ecaf[ i ] - ( i < 2 ? 'A' : 'a' );
		assert( rank_i >= 0 && rank_i <= 7 );
		if( ( i < 2 && r1[ rank_i ] != 'R' ) || ( i >= 2 && r8[ rank_i ] != 'r' ) )
			return true; }
	
	return false;
}

static bool
x_rook_on_wrong_side_of_king( const char *r1, const char *r8, const char *ecaf )
{
	for( int i = 0; i < 4; i++ ) {
		if( ecaf[ i ] == '-' ) continue;
		char file_of_rook = tolower( ecaf[ i ] );
		assert( file_of_rook >= 'a' && file_of_rook <= 'h' );
		if( ( i % 2 && x_file_of_king( i == 1 ? r1 : r8 ) > file_of_rook ) ||
			( !( i % 2 ) && x_file_of_king( i == 0 ? r1 : r8 ) < file_of_rook )
		) return true; }

	return false;
}

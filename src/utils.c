#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <regex.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "utils.h"
#include "chester.h"
#include "move_gen.h"
#include "validation.h"

static const char *x_sq_navigator_kings_sqs(
	const char *sq_name, enum sq_dir dir );
static const char *x_sq_navigator_knights_sqs(
	const char *sq_name, enum sq_dir dir );
static const char *x_ALT_sq_navigator_kings_sqs(
	const char *sq_name, enum sq_dir dir );
static const char *x_ALT_sq_navigator_knights_sqs(
	const char *sq_name, enum sq_dir dir );
static Bitboard x_sq_rectangle( const char *ulc, const char *lrc );
static void x_compress_eppf_rank_dashes_to_digit(
	int *eri_ptr, int cri, const char *eppf_rank, char *compressed_rank );

/***********************
 **** External data ****
 ***********************/

bool srand_has_been_called = false;

/*************************************
 **** Chester interface functions ****
 *************************************/

/****************************
 **** External functions ****
 ****************************/

// Returns true if 'str' matches the extended regular expression 'pattern'
bool
str_matches_pattern( const char *str, const char *pattern )
{
	regex_t regex;
	int ret_val;

	// Compile regular expression
	ret_val = regcomp( &regex, pattern, REG_EXTENDED );
	assert( !ret_val ); // Something went wrong in compiling regex

	// Testing if str matches the regular expression in regex.
	// Note this: "You must match the regular expression with the same
	// set of current locales that were in effect when you compiled the
	// regular expression."
	// https://www.gnu.org/software/libc/manual/html_node/Matching-POSIX-Regexps.html
	ret_val = regexec( &regex, str, 0, NULL, 0 );

	if( ! ret_val ) { // str matched the regex
		return true;
	}
	// Making sure regexec() didn't run into problems
	assert( ret_val == REG_NOMATCH );

	return false;
}

// A FEN string such as "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
// consists of six fields. The following function returns the nth field of a FEN.
// It is assumed  that the size of the memory area pointed to by 'writable_mem_ptr'
// is FEN_STR_MAX_LENGTH + 1.
char *
nth_field_of_fen_str( const char *fen_str, char *writable_mem_ptr, int field_num )
{
	assert( field_num > 0 && field_num < 7 );

// Avoiding copy & paste code
#define STRCPY_AND_STRTOK( str_with_fields, field_sep ) \
	strcpy( writable_mem_ptr, str_with_fields ); \
	char *field = strtok( writable_mem_ptr, field_sep ); \
	for( int i = 2; i <= field_num; i++ ) { \
		field = strtok( NULL, field_sep ); \
	}

	STRCPY_AND_STRTOK( fen_str, " " );

	return field;
}

// A PPF such as "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR" consists of
// eight subfields that represent ranks. The following function returns the
// nth *rank* of a PPF. It is assumed  that the size of the memory area
// pointed to by 'writable_mem_ptr' is *at least* PPF_MAX_LENGTH + 1. When
// calling this function, keep in mind that rank 8 corresponds to subfield 1,
// rank 7 to subfield 2, etc.
char *
nth_rank_of_ppf( const char *ppf, char *writable_mem_ptr, int rank_num )
{
	assert( rank_num > 0 && rank_num < 9 );

	// The first subfield of a PPF corresponds to rank 8, the second
	// to rank 7, and so on.
	int field_num = 9 - rank_num;

	STRCPY_AND_STRTOK( ppf, "/" );

	return field;
}

// Replaces digits in a PPF rank string with a corresponding number of
// dashes. For example, "1p2p3" gets converted to "-p--p---". Note that
// the length of the original rank string is *at most* eight chars,
// whereas the length of the new string (the one with dashes) is always
// eight. It is therefore assumed that the writable array 'expanded_rank'
// is at least 8 + 1 bytes long.
void
expand_ppf_rank( const char *ppf_rank, char *expanded_rank )
{
	expanded_rank[ 8 ] = '\0';
	for( int i = 0; i < 8; i++ )
		expanded_rank[ i ] = '-'; // A dash means an empty square

	int i = 0, j = 0;
	while( ppf_rank[ i ] ) {
		if( !isdigit( ppf_rank[ i ] ) ) { // Copying non-digit chars into the empty rank
			expanded_rank[ j ] = ppf_rank[ i ];
			j++;
		}
		else j += ( ppf_rank[ i ] - 48 ); // '1' - 48 = 1, '8' - 48 = 8

		i++;
	}

	assert( j == 8 );
	assert( !expanded_rank[ j ] );
	assert( strlen( expanded_rank ) == 8 );
} // Review: 2018-01-05

// Does the opposite of expand_ppf_rank(). The length of a PPF rank is between one
// to eight chars, and thus the writable array 'compressed_rank' is assumed to be
// at least 8 + 1 chars long.
void
compress_eppf_rank( const char *eppf_rank, char *compressed_rank )
{
	int cri = 0; // cri, compressed rank index

	for( int eri = 0; eri < 8; eri++, cri++ ) { // eri, EPPF rank index
		char c = eppf_rank[ eri ];
		if( c == 'K' || c == 'Q' || c == 'R' || c == 'B' || c == 'N' || c == 'P' ||
			c == 'k' || c == 'q' || c == 'r' || c == 'b' || c == 'n' || c == 'p'
		) compressed_rank[ cri ] = c;
		else if( c == '-' ) x_compress_eppf_rank_dashes_to_digit(
			&eri, cri, eppf_rank, compressed_rank );
		else assert( false );
	}

	compressed_rank[ cri ] = '\0';

	assert( strlen( compressed_rank ) >= 1 && strlen( compressed_rank ) <= 8 );
	assert( str_matches_pattern( compressed_rank, "^[KQRBNPkqrbnp12345678]*$" ) );
}

// Sets or unsets the bits in 'bits' specified by 'BITMASK'. The bits are
// set ("assigned" the value of 1) if BIT equals true; otherwise the
// bits are unset.
void
set_or_unset_bits( uint64_t *bits, const uint64_t BITMASK, const bool BIT )
{
	*bits = BIT ? ( *bits | BITMASK ) : ( *bits & ~BITMASK );
}

// A slightly more convenient way to set bits with set_or_unset_bits()
void
set_bits( uint64_t *bits, const uint64_t BITMASK )
{
	set_or_unset_bits( bits, BITMASK, 1 );
}

// A slightly more convenient way to unset bits with set_or_unset_bits()
void
unset_bits( uint64_t *bits, const uint64_t BITMASK )
{
	set_or_unset_bits( bits, BITMASK, 0 );
}

// Returns the type of chessman on a particular square of a particular position
Chessman
occupant_of_sq( const Pos *p, const char *sq_name )
{
	Bitboard sq_bit = sq_name_to_sq_bit( sq_name );

	for( Chessman cm = WHITE_KING; cm <= BLACK_PAWN; cm++ )
		if( sq_bit & p->cm[ cm ] )
			return cm;

	return EMPTY_SQUARE;
}

// Converts a square name such as "c2" to a square bit such as 0x400
Bitboard
sq_name_to_sq_bit( const char *sq_name )
{
	assert( str_matches_pattern( sq_name, "^[abcdefgh][12345678]$" ) );
	int sq_bit_array_index = sq_name[ 0 ] - 'a' + ( sq_name[ 1 ] - '1' ) * 8;
	assert( sq_bit_array_index >= 0 && sq_bit_array_index <= 63 );
	return SBA[ sq_bit_array_index ];
}

// Converts a square bit to a square name
const char *
sq_bit_to_sq_name( Bitboard sq_bit )
{
	assert( bb_is_sq_bit( sq_bit ) );

	char file = file_of_sq( sq_bit ), rank = rank_of_sq( sq_bit );
	int sq_name_array_index = file - 'a' + ( rank - '1' ) * 8;

	assert( sq_name_array_index >= 0 && sq_name_array_index <= 63 );
	return SNA[ sq_name_array_index ];
}

// Returns the file of the square bit argument
char
file_of_sq( const Bitboard sq_bit )
{
	assert( bb_is_sq_bit( sq_bit ) );

	if( sq_bit & SS_FILE_A )
		return 'a';
	if( sq_bit & SS_FILE_B )
		return 'b';
	if( sq_bit & SS_FILE_C )
		return 'c';
	if( sq_bit & SS_FILE_D )
		return 'd';
	if( sq_bit & SS_FILE_E )
		return 'e';
	if( sq_bit & SS_FILE_F )
		return 'f';
	if( sq_bit & SS_FILE_G )
		return 'g';
	if( sq_bit & SS_FILE_H )
		return 'h';

	assert( false ); // Fail unconditionally
	return 0; // The null character
}

// Returns the rank of the square bit argument
char
rank_of_sq( const Bitboard sq_bit )
{
	assert( bb_is_sq_bit( sq_bit ) );

	if( sq_bit & SS_RANK_1 )
		return '1';
	if( sq_bit & SS_RANK_2 )
		return '2';
	if( sq_bit & SS_RANK_3 )
		return '3';
	if( sq_bit & SS_RANK_4 )
		return '4';
	if( sq_bit & SS_RANK_5 )
		return '5';
	if( sq_bit & SS_RANK_6 )
		return '6';
	if( sq_bit & SS_RANK_7 )
		return '7';
	if( sq_bit & SS_RANK_8 )
		return '8';

	assert( false ); // Fail unconditionally
	return 0; // The null character
}

// Returns true if the argument is a square bit
bool
bb_is_sq_bit( Bitboard bb ) {
	for( int i = 0; i < 64; i++ )
		if( bb == SBA[ i ] )
			return true;

	return false;
}

// Returns the square name of the EPTSF of a Pos var
const char *
epts_from_pos_var( const Pos *p )
{
	char file;

	if( p->info & BM_EPTS_FILE_A )
		file = 'a';
	else if( p->info & BM_EPTS_FILE_B )
		file = 'b';
	else if( p->info & BM_EPTS_FILE_C )
		file = 'c';
	else if( p->info & BM_EPTS_FILE_D )
		file = 'd';
	else if( p->info & BM_EPTS_FILE_E )
		file = 'e';
	else if( p->info & BM_EPTS_FILE_F )
		file = 'f';
	else if( p->info & BM_EPTS_FILE_G )
		file = 'g';
	else if( p->info & BM_EPTS_FILE_H )
		file = 'h';
	else
		return "-";

	char rank = whites_turn( p ) ? '6' : '3';

	char tmp_sq_name[ 3 ] = { 0 };
	tmp_sq_name[ 0 ] = file;
	tmp_sq_name[ 1 ] = rank;

	int sq_name_array_index = sq_bit_index( sq_name_to_sq_bit( tmp_sq_name ) );
	assert( sq_name_array_index >= 0 && sq_name_array_index <= 63 );

	return SNA[ sq_name_array_index ];
}

// Returns the bit index of square bit argument
int
sq_bit_index( Bitboard sq_bit )
{
	for( int i = 0; i < 64; i++ )
		if( sq_bit == SBA[ i ] )
			return i;

	assert( false );
	return -1;
} // Review: 2017-12-07

// Converts the argument into a "binary string", that is, a string
// consisting of 64 '0' and '1' chars
char *
uint64_to_bin_str( uint64_t uint64 )
{
	char *bin_str = (char *) malloc( 64 + 1 );

	*( bin_str + 64 ) = 0; // Null char at the end of string
	for( int i = 0; i < 64; i++ ) {
		*( bin_str + i ) = ( uint64 & SBA[ 63 - i ] ) ? '1' : '0';
	}

	assert( strlen( bin_str ) == 64 );

	return bin_str;
}

// REMOVE OR RELOCATE!
//
// Prints a Pos var to stdout with each uint64 represented as a binary string
void
print_pos_var( const Pos *p )
{
	char* bin_str;

	for( Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++ ) {
		bin_str = uint64_to_bin_str( p->cm[ cm ] );
		printf( "%s    %c\n", bin_str, FEN_PIECE_LETTERS[ cm ] );
		free( bin_str );
	}

	bin_str = uint64_to_bin_str( p->info );
	printf( "\n%s\n", bin_str );
	free( bin_str );
}

// Returns the name of the square that is in direction 'dir' of
// square 'sq_name'. For example, the call sq_navigator( "e4", NE ) would
// return "f5" and the call sq_navigator( "e4", ONE_OCLOCK ) would
// return "f6".
const char *
sq_navigator( const char *sq_name, enum sq_dir dir )
{
	assert( valid_sq_name( sq_name ) );

	if( dir >= NORTH && dir <= NORTHWEST )
		return x_sq_navigator_kings_sqs( sq_name, dir );

	return x_sq_navigator_knights_sqs( sq_name, dir );
}

// A different implementation of sq_navigator(). Used for testing purposes.
// Should produce exactly the same results as sq_navigator().
const char *
ALT_sq_navigator( const char *sq_name, enum sq_dir dir )
{
	assert( valid_sq_name( sq_name ) );

	if( dir >= NORTH && dir <= NORTHWEST )
		return x_ALT_sq_navigator_kings_sqs( sq_name, dir );

	return x_ALT_sq_navigator_knights_sqs( sq_name, dir );
}

// Returns the index 'i' of the square name argument such that SNA[ i ]
// equals 'sq_name'
int
sq_name_index( const char *sq_name )
{
	assert( valid_sq_name( sq_name ) );
	int ret_val = ( sq_name[ 1 ] - '1' ) * 8 + ( sq_name[ 0 ] - 'a' );

	assert( ret_val >= 0 && ret_val <= 63 );
	return ret_val;
}

// Returns the number of squares in a square set. For example, the
// number of squares in the square set { d4, e4, d5, e5 } is four.
// As usual, the square set is represented as a bitboard.
int
num_of_sqs_in_sq_set( Bitboard bb )
{
	int num_of_sqs = 0;
	const uint64_t bit = 1u;

	for( int i = 0; i < 64; i++ )
		if( bb & ( bit << i ) )
			++num_of_sqs;

	return num_of_sqs;
}

// Returns the square rectangle determined by the parameters. Any rectangle
// on the chessboard can be defined by its upper left and lower right corners.
// Here's an example of a square rectangle where the upper left and lower right
// corners are marked with square brackets:
// [c5]  d5   e5   f5
//  c4   d4   e4  [f4]
Bitboard
sq_rectangle( const Bitboard upper_left, const Bitboard lower_right )
{
	assert( bb_is_sq_bit( upper_left ) && bb_is_sq_bit( lower_right ) );

	if( upper_left == lower_right )
		return upper_left;

	const char file_ulc = file_of_sq( upper_left ),
		file_lrc = file_of_sq( lower_right ),
		rank_ulc = rank_of_sq( upper_left ),
		rank_lrc = rank_of_sq( lower_right );

	if( file_ulc <= file_lrc && rank_ulc >= rank_lrc ) {
		const char *ulc = file_and_rank_to_sq_name( file_ulc, rank_ulc ),
			*lrc = file_and_rank_to_sq_name( file_lrc, rank_lrc );
		return x_sq_rectangle( ulc, lrc );
	}

	return 0;
} // Review: 2018-01-03

// The call file_and_rank_to_sq_name( 'e', '4' ) would return "e4"
const char *
file_and_rank_to_sq_name( const char file, const char rank )
{
	assert( file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8' );

	char tmp_sq_name[ 3 ] = { 0 };
	tmp_sq_name[ 0 ] = file;
	tmp_sq_name[ 1 ] = rank;

	return SNA[ sq_name_index( tmp_sq_name ) ];
}

// Returns the diagonal the square parameter is on
Bitboard
diag_of_sq( Bitboard sq_bit )
{
	assert( bb_is_sq_bit( sq_bit ) );

	for( int i = 0; i < 15; i++ )
		if( sq_bit & sq_set_of_diag( i ) )
			return sq_set_of_diag( i );

	assert( false );
	return 0u;
}

// Returns the antidiagonal the square parameter is on
Bitboard
antidiag_of_sq( Bitboard sq_bit )
{
	assert( bb_is_sq_bit( sq_bit ) );

	for( int i = 0; i < 15; i++ )
		if( sq_bit & sq_set_of_antidiag( i ) )
			return sq_set_of_antidiag( i );

	assert( false );
	return 0u;
}

// Returns the next square in the square set parameter 'ss'. The square
// returned is removed from 'ss'. This makes it convenient to call
// next_sq_bit() repeatedly until 'ss' is empty.
Bitboard
next_sq_of_ss( Bitboard *ss )
{
	if( !( *ss ) )
		return 0;

	Bitboard sq = SB.a1;
	while( !( ( *ss ) & sq ) ) {
		sq <<= 1;
	}
	*ss ^= sq;

	assert( bb_is_sq_bit( sq ) );
	return sq;
}

// TODO: ...
char *
compress_expanded_ppf( const char *eppf )
{
	return NULL;
}

// TODO: ...
char *
expand_ppf( const char *ppf )
{
	assert( false );
	return (char *) ppf;
}

/**************************
 **** Static functions ****
 **************************/

#define FIND_SQUARE_IN_DIRECTION( dir_constant, ss_file_or_rank, int_constant ) \
if( dir == dir_constant ) { \
	if( sq_bit & ( ss_file_or_rank ) ) \
		return NULL; \
	return SNA[ sq_bit_index( sq_bit ) + ( int_constant ) ]; }

static const char *
x_sq_navigator_kings_sqs( const char *sq_name, enum sq_dir dir )
{
	Bitboard sq_bit = sq_name_to_sq_bit( sq_name );

	FIND_SQUARE_IN_DIRECTION( NORTH, SS_RANK_8, 8 )
	FIND_SQUARE_IN_DIRECTION( NORTHEAST, SS_FILE_H | SS_RANK_8, 9 )
	FIND_SQUARE_IN_DIRECTION( EAST, SS_FILE_H, 1 )
	FIND_SQUARE_IN_DIRECTION( SOUTHEAST, SS_FILE_H | SS_RANK_1, -7 )
	FIND_SQUARE_IN_DIRECTION( SOUTH, SS_RANK_1, -8 )
	FIND_SQUARE_IN_DIRECTION( SOUTHWEST, SS_FILE_A | SS_RANK_1, -9 )
	FIND_SQUARE_IN_DIRECTION( WEST, SS_FILE_A, -1 )
	FIND_SQUARE_IN_DIRECTION( NORTHWEST, SS_FILE_A | SS_RANK_8, 7 )

	assert( false );
	return "";
}

static const char *
x_sq_navigator_knights_sqs( const char *sq_name, enum sq_dir dir )
{
	Bitboard sq_bit = sq_name_to_sq_bit( sq_name );

	FIND_SQUARE_IN_DIRECTION( ONE_OCLOCK,
		SS_FILE_H | SS_RANK_7 | SS_RANK_8, 17 )
	FIND_SQUARE_IN_DIRECTION( TWO_OCLOCK,
		SS_FILE_G | SS_FILE_H | SS_RANK_8, 10 )
	FIND_SQUARE_IN_DIRECTION( FOUR_OCLOCK,
		SS_FILE_G | SS_FILE_H | SS_RANK_1, -6 )
	FIND_SQUARE_IN_DIRECTION( FIVE_OCLOCK,
		SS_FILE_H | SS_RANK_1 | SS_RANK_2, -15 )
	FIND_SQUARE_IN_DIRECTION( SEVEN_OCLOCK,
		SS_FILE_A | SS_RANK_1 | SS_RANK_2, -17 )
	FIND_SQUARE_IN_DIRECTION( EIGHT_OCLOCK,
		SS_FILE_A | SS_FILE_B | SS_RANK_1, -10 )
	FIND_SQUARE_IN_DIRECTION( TEN_OCLOCK,
		SS_FILE_A | SS_FILE_B | SS_RANK_8, 6 )
	FIND_SQUARE_IN_DIRECTION( ELEVEN_OCLOCK,
		SS_FILE_A | SS_RANK_7 | SS_RANK_8, 15 )

	assert( false );
	return "";
}

static const char *
x_ALT_sq_navigator_kings_sqs( const char *sq_name, enum sq_dir dir )
{
	char sq_name_copy[ 3 ] = { 0 };
	strcpy( sq_name_copy, sq_name );

	if( dir == NORTHEAST || dir == EAST || dir == SOUTHEAST )
		++sq_name_copy[ 0 ];
	else if( dir == NORTHWEST || dir == WEST || dir == SOUTHWEST )
		--sq_name_copy[ 0 ];

	if( dir == NORTHWEST || dir == NORTH || dir == NORTHEAST )
		++sq_name_copy[ 1 ];
	else if( dir == SOUTHWEST || dir == SOUTH || dir == SOUTHEAST )
		--sq_name_copy[ 1 ];

	return valid_sq_name( sq_name_copy ) ?
		SNA[ sq_name_index( sq_name_copy ) ] : NULL;
}

static const char *
x_ALT_sq_navigator_knights_sqs( const char *sq_name, enum sq_dir dir )
{
	char sq_name_copy[ 3 ] = { 0 };
	strcpy( sq_name_copy, sq_name );

	if( dir == TWO_OCLOCK || dir == FOUR_OCLOCK )
		sq_name_copy[ 0 ] += 2;
	else if( dir == ONE_OCLOCK || dir == FIVE_OCLOCK )
		++sq_name_copy[ 0 ];
	else if( dir == SEVEN_OCLOCK || dir == ELEVEN_OCLOCK )
		--sq_name_copy[ 0 ];
	else
		sq_name_copy[ 0 ] -= 2;

	if( dir == ONE_OCLOCK || dir == ELEVEN_OCLOCK )
		sq_name_copy[ 1 ] += 2;
	else if( dir == TWO_OCLOCK || dir == TEN_OCLOCK )
		++sq_name_copy[ 1 ];
	else if( dir == FOUR_OCLOCK || dir == EIGHT_OCLOCK )
		--sq_name_copy[ 1 ];
	else
		sq_name_copy[ 1 ] -= 2;

	return valid_sq_name( sq_name_copy ) ?
		SNA[ sq_name_index( sq_name_copy ) ] : NULL;
}

/*
The following is a square rectangle that has four rows and five columns.
	c6  d6  e6  f6  g6
	c5  d5  e5  f5  g5
	c4  d4  e4  f4  g4
	c3  d3  e3  f3  g3

x_sq_rectangle() begins its job knowing only the upper left corner and the
lower right corner:
	c6  ??  ??  ??  ??
	??  ??  ??  ??  ??
	??  ??  ??  ??  ??
	??  ??  ??  ??  g3
*/
static Bitboard // ulc: upper left corner, lrc: lower right corner
x_sq_rectangle( const char *ulc, const char *lrc )
{
	Bitboard bb = 0u;
	const char *first_sq_of_row = ulc;

	while( first_sq_of_row && ( first_sq_of_row[ 1 ] >= lrc[ 1 ] ) ) {
		const char *sq_of_current_row = first_sq_of_row;

		while( sq_of_current_row && ( sq_of_current_row[ 0 ] <= lrc[ 0 ] ) ) {
			bb |= sq_name_to_sq_bit( sq_of_current_row );
			sq_of_current_row = sq_navigator( sq_of_current_row, EAST );
		}

		first_sq_of_row = sq_navigator( first_sq_of_row, SOUTH );
	}

	return bb;
}

static void
x_compress_eppf_rank_dashes_to_digit(
	int *eri_ptr, int cri, const char *eppf_rank, char *compressed_rank )
{
	char digit = '1';
	while( eppf_rank[ *eri_ptr + 1 ] == '-' ) {
		*eri_ptr += 1;
		++digit;
	}
	compressed_rank[ cri ] = digit;
}

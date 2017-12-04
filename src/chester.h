#ifndef CHESTER_H
#define CHESTER_H

#include <stddef.h>

// The possible return values of the function che_validate_fen_str()
enum fen_str_error {
	FEN_STR_NO_ERRORS,
	FEN_STR_LENGTH_ERROR,
	FEN_STR_CHARS_ERROR,
	FEN_STR_FIELD_STRUCTURE_ERROR,
	FEN_STR_PPF_STRUCTURE_ERROR,
	FEN_STR_PPF_CONSECUTIVE_DIGITS_ERROR,
	FEN_STR_PPF_RANK_SUM_ERROR,
	FEN_STR_ACF_ERROR,
	FEN_STR_CAF_ERROR,
	FEN_STR_EPTSF_ERROR,
	FEN_STR_HMCF_ERROR,
	FEN_STR_FMNF_ERROR,
	FEN_STR_PPF_CONTRADICTS_CAF_ERROR,
	FEN_STR_EPTSF_CONTRADICTS_HMCF_ERROR,
	FEN_STR_EPTSF_CONTRADICTS_ACF_ERROR,
	FEN_STR_EPTSF_CONTRADICTS_PPF_ERROR,
	FEN_STR_WHITE_PAWN_ON_FIRST_RANK,
	FEN_STR_BLACK_PAWN_ON_FIRST_RANK, // In Black's case the first rank is rank 8
	FEN_STR_WHITE_PAWN_ON_LAST_RANK,
	FEN_STR_BLACK_PAWN_ON_LAST_RANK,
	FEN_STR_INVALID_NUMBER_OF_WHITE_KINGS,
	FEN_STR_INVALID_NUMBER_OF_BLACK_KINGS,
	FEN_STR_WHITE_KING_CAN_BE_CAPTURED,
	FEN_STR_BLACK_KING_CAN_BE_CAPTURED
};

// Chester's external representation of the square set. The main internal
// representation is Bitboard (defined in types.h).
typedef struct {
	char *sqs[ 64 ];
	size_t size;
} Squares;
// size_t che_Squares_add( /* var args */ ); // Returns size
// size_t che_Squares_init( /* var args */ );

enum fen_str_error che_validate_fen_str( const char *fen_str );

int che_move_gen( const char *fen, unsigned char ***moves, int *num_mv_cm );
/*
unsigned char **moves;
int num_mv_cm;
int num_moves = che_move_gen( STD_START_POS, &moves, &num_mv_cm );
After the function call moves should be like this:
0 'R' '-' 255
1 'N' '-'
2 'B' '-'
3 'Q' '-'
4 'K' '-'
5 'B' '-'
6 'N' '-'
7 'R' '-'
8 'P' '-'
9 'P' '-'
10 'P' '-'
11 'P' '-'
12 'P' '-'
13 'P' '-'
14 'P' '-'
15 'P' '-'
*/

#endif
// end CHESTER_H

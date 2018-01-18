#ifndef CHESTER_H
#define CHESTER_H

#include <stddef.h>
#include <stdint.h>

// The possible return values of the function che_fen_validator()
enum che_fen_error {
	FEN_NO_ERRORS,
	FEN_LENGTH_ERROR,
	FEN_CHARS_ERROR,
	FEN_FIELD_STRUCTURE_ERROR,
	FEN_PPF_STRUCTURE_ERROR,
	FEN_PPF_CONSECUTIVE_DIGITS_ERROR,
	FEN_PPF_RANK_SUM_ERROR,
	FEN_ACF_ERROR,
	FEN_CAF_ERROR,
	FEN_EPTSF_ERROR,
	FEN_HMCF_ERROR,
	FEN_FMNF_ERROR,
	FEN_KING_PLACEMENT_CONTRADICTS_CAF_ERROR,
	FEN_ROOK_PLACEMENT_CONTRADICTS_CAF_ERROR,
	FEN_EPTSF_CONTRADICTS_HMCF_ERROR,
	FEN_EPTSF_CONTRADICTS_ACF_ERROR,
	FEN_EPTSF_CONTRADICTS_PPF_ERROR,
	FEN_PAWN_ON_INVALID_RANK,
	FEN_INVALID_NUMBER_OF_KINGS,
	FEN_KING_CAN_BE_CAPTURED
};

// Function prototypes
enum che_fen_error che_fen_validator( const char *fen );
int che_move_gen( const char *fen, uint16_t ***moves, int *num_mv_cm );

#endif
// end CHESTER_H

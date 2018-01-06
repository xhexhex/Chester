#ifndef CHESTER_H
#define CHESTER_H

#include <stddef.h>
#include <stdint.h>

// The possible return values of the function che_fen_validator()
enum che_fen_error {
	FEN_STR_NO_ERRORS,
	FEN_LENGTH_ERROR,
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
	FEN_STR_BLACK_KING_CAN_BE_CAPTURED,
	FEN_STR_CHESS960_PPF_CONTRADICTS_CAF_ERROR
};

extern const uint64_t CFSV_BF_CHESS960;

// Function prototypes
enum che_fen_error che_fen_validator( const char *fen_str, const uint64_t settings );
int che_move_gen( const char *fen, uint16_t ***moves, int *num_mv_cm );

#endif
// end CHESTER_H

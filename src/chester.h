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
	FEN_PPF_CONTRADICTS_CAF_ERROR,
	FEN_EPTSF_CONTRADICTS_HMCF_ERROR,
	FEN_EPTSF_CONTRADICTS_ACF_ERROR,
	FEN_EPTSF_CONTRADICTS_PPF_ERROR,
	FEN_WHITE_PAWN_ON_FIRST_RANK,
	FEN_BLACK_PAWN_ON_FIRST_RANK, // In Black's case the first rank is rank 8
	FEN_WHITE_PAWN_ON_LAST_RANK,
	FEN_BLACK_PAWN_ON_LAST_RANK,
	FEN_INVALID_NUMBER_OF_WHITE_KINGS,
	FEN_INVALID_NUMBER_OF_BLACK_KINGS,
	FEN_WHITE_KING_CAN_BE_CAPTURED,
	FEN_BLACK_KING_CAN_BE_CAPTURED,
	FEN_CHESS960_PPF_CONTRADICTS_CAF_ERROR
};

extern const uint64_t CFSV_BF_CHESS960;

// Function prototypes
enum che_fen_error che_fen_validator( const char *fen, const uint64_t settings );
int che_move_gen( const char *fen, uint16_t ***moves, int *num_mv_cm );

#endif
// end CHESTER_H

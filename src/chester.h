#ifndef CHESTER_H
#define CHESTER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint16_t Rawcode;

// The FEN string minimum and maximum lengths
#define FEN_MIN_LENGTH 27
#define FEN_MAX_LENGTH 92

// The initial position of standard chess
#define INIT_POS \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// The possible return values of che_fen_validator()
// *** "FERR_" instead of "FEN_"? Or "CFE_"? ***
// FEN_NO_ERRORS => FERR_NONE
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
    FEN_PAWN_ON_INVALID_RANK_ERROR,
    FEN_INVALID_NUMBER_OF_KINGS_ERROR,
    FEN_KING_CAN_BE_CAPTURED_ERROR,
    FEN_HMCF_CONTRADICTS_FMNF_ERROR };

//
// The Chester interface
//
enum che_fen_error che_fen_validator( const char *fen );
bool che_is_san( const char *san );
char *che_make_moves( const char *fen, const char *sans );
char *che_move_gen( const char *fens );
char **che_fen_fields( const char *fen, bool expand_ppf, bool expand_caf );
void che_free_ff( void *ff );
long long che_perft( const char *fen, int depth, bool mt );

// enum che_movetext_error che_movetext_validator(fen, movetext);

#endif
// end CHESTER_H

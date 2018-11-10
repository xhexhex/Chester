#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "chester.h"

typedef uint64_t Bitboard;

struct single_instance_variable_SB {
    const Bitboard
        a1, b1, c1, d1, e1, f1, g1, h1,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a8, b8, c8, d8, e8, f8, g8, h8;
};

extern const struct single_instance_variable_SB SB;
extern const Bitboard SBA[], SQ_NAV[][16];
extern const size_t POSSIBLE_IRPF_VALUES_COUNT, SHREDDER_FEN_CAFS_COUNT;
extern const uint8_t POSSIBLE_IRPF_VALUES[];

extern const char PPF_CHESSMAN_LETTERS[], STD_FEN_CAF_REGEX[];
extern const char * const SNA[];
extern const char * const SHREDDER_FEN_CAFS[];

extern const char APM_DATA[];
#define APM_COUNT 1792

// The FEN string minimum and maximum lengths
#define FEN_MIN_LENGTH 27
#define FEN_MAX_LENGTH 92
// The minimum and maximum lengths for the PPF of a FEN string
#define PPF_MIN_LENGTH 17
#define PPF_MAX_LENGTH 71

// FEN strings of special significance
#define FEN_STD_START_POS \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FEN_PERSONALLY_SIGNIFICANT \
    "6k1/r1q1b2n/6QP/p3R3/1p3p2/1P6/1PP2P2/2K4R b - - 1 35"
#define FEN_GAME_OF_THE_CENTURY \
    "r2q1rk1/pp2ppbp/1np2np1/2Q3B1/3PP1b1/2N2N2/PP3PPP/3RKB1R b K - 6 11"
#define FEN_SUPERPOSITION_1 \
    "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1"
#define FEN_SUPERPOSITION_2 \
    "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1"

// The 'ppa' member of the Pos type should be indexed with these,
// e.g., p->ppa[WHITE_KING]
typedef enum {
    EMPTY_SQUARE,
    WHITE_KING, WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN,
    BLACK_KING, BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN
} Chessman;

// TODO: Doc
enum sq_dir {
    NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST,
    ONE_OCLOCK, TWO_OCLOCK, FOUR_OCLOCK, FIVE_OCLOCK,
    SEVEN_OCLOCK, EIGHT_OCLOCK, TEN_OCLOCK, ELEVEN_OCLOCK
};

// The Pos type or "Pos variable" is Chester's internal representation
// of chess positions. A Pos variable contains the same information as
// a FEN string. See doc/pos-spec.md for a comprehensive description of
// Pos variables.
typedef struct {
    Bitboard ppa[13];
    uint8_t turn_and_ca_flags, irp[2], epts_file;
    uint16_t hmc, fmn;
} Pos;

// The square sets for the 15 diagonals and 15 antidiagonals of the chessboard
extern const Bitboard
    SS_DIAG_H1H1, SS_DIAG_G1H2, SS_DIAG_F1H3, SS_DIAG_E1H4, SS_DIAG_D1H5,
    SS_DIAG_C1H6, SS_DIAG_B1H7, SS_DIAG_A1H8, SS_DIAG_A2G8, SS_DIAG_A3F8,
    SS_DIAG_A4E8, SS_DIAG_A5D8, SS_DIAG_A6C8, SS_DIAG_A7B8, SS_DIAG_A8A8,
    SS_ANTIDIAG_A1A1, SS_ANTIDIAG_B1A2, SS_ANTIDIAG_C1A3,
    SS_ANTIDIAG_D1A4, SS_ANTIDIAG_E1A5, SS_ANTIDIAG_F1A6,
    SS_ANTIDIAG_G1A7, SS_ANTIDIAG_H1A8, SS_ANTIDIAG_H2B8,
    SS_ANTIDIAG_H3C8, SS_ANTIDIAG_H4D8, SS_ANTIDIAG_H5E8,
    SS_ANTIDIAG_H6F8, SS_ANTIDIAG_H7G8, SS_ANTIDIAG_H8H8;

// The expression evaluates to true if it is White's turn to move.
#define whites_turn( pos_ptr ) ( pos_ptr->turn_and_ca_flags & 0x80U )

// Function prototypes
Pos *fen_to_pos( const char *fen );
char *pos_to_fen( const Pos *p );
Bitboard sq_set_of_diag( const int index );
Bitboard sq_set_of_antidiag( const int index );
Bitboard ss_white_army( const Pos *p );
Bitboard ss_black_army( const Pos *p );
Bitboard sq_nav( Bitboard sq, enum sq_dir dir );
bool has_castling_right( const Pos *p, const char *color, const char *side );
Bitboard epts( const Pos *p );
Rawcode rawcode( const char *rawmove );
void rawmove( Rawcode rawcode, char *writable );
void make_move( Pos *p, Rawcode code, char promotion );
void remove_castling_rights( Pos *p, const char *color, const char *side );
char *ecaf( const Pos *p );
void toggle_turn( Pos *p );
char *single_san_make_move( const char *fen, const char *san );

#endif
// End BASE_H

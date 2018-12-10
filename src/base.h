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
        a8, b8, c8, d8, e8, f8, g8, h8; };

extern const struct single_instance_variable_SB SB;
extern const Bitboard SBA[];
extern const char * const SNA[];
extern const Bitboard SQ_NAV[][16];
extern const int POSSIBLE_IRPF_VALUES_COUNT;
extern const uint8_t POSSIBLE_IRPF_VALUES[];
extern const int SHREDDER_FEN_CAFS_COUNT;
extern const char * const SHREDDER_FEN_CAFS[];
extern const char PPF_CHESSMAN_LETTERS[];
extern const char STD_FEN_CAF_REGEX[];
extern const Rawcode MIN_RAWCODE;
extern const Rawcode MAX_RAWCODE;
extern const char APM_DATA[];
extern const int8_t RC_ORIG_SQ_BINDEX[];
extern const int8_t RC_DEST_SQ_BINDEX[];
extern const Rawcode ORIG_DEST_RC[64][64];
extern const Bitboard ONE;
extern const Bitboard SQ_RAY[][8];
extern const Bitboard KNIGHT_SQS[];
extern const Bitboard KING_SQS[];
extern const Bitboard ROOK_SQS[];
extern const Bitboard BISHOP_SQS[];

// The minimum and maximum lengths for the PPF of a FEN string
#define PPF_MIN_LENGTH 17
#define PPF_MAX_LENGTH 71

// It is said that the maximum number of moves in any chess position
// is 218. An example of a position with 218 moves is
// "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1".
#define MAX_LEGAL_MOVE_COUNT 218

// The 'ppa' member of a Pos variable should be indexed with these,
// e.g., p->ppa[WHITE_KING].
typedef enum {
    EMPTY_SQUARE,
    WHITE_KING, WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN,
    BLACK_KING, BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN
} Chessman;

// The 16 square directions. The first eight directions correspond to the
// squares to which a king can move on a non-edge square. For example,
// if the king is on square e4, then it could move to e5 (NORTH), e3 (SOUTH)
// or d5 (NORTHWEST). Directions from ONE_OCLOCK to ELEVEN_OCLOCK correspond
// to the directions a knight can jump on a square such as e4. For example,
// from e4 a knight could jump to f6 (ONE_OCLOCK) or d2 (SEVEN_OCLOCK).
enum sq_dir {
    NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST,
    ONE_OCLOCK, TWO_OCLOCK, FOUR_OCLOCK, FIVE_OCLOCK,
    SEVEN_OCLOCK, EIGHT_OCLOCK, TEN_OCLOCK, ELEVEN_OCLOCK };

// The Pos type or "Pos variable" is Chester's internal representation
// of chess positions. A Pos variable contains the same information as
// a FEN string. See doc/pos-spec.md for a comprehensive description of
// Pos variables.
typedef struct {
    Bitboard ppa[13];
    uint8_t turn_and_ca_flags, irp[2], epts_file;
    uint16_t hmc, fmn;
} Pos;

// The expression evaluates to true iff it is White's turn to move.
#define whites_turn(pos_ptr) (pos_ptr->turn_and_ca_flags & 0x80U)

// Function prototypes
Pos *fen_to_pos( const char *fen );
char *pos_to_fen( const Pos *p );
Bitboard sq_nav( Bitboard sq, enum sq_dir dir );
bool has_castling_right( const Pos *p, const char *color, const char *side );
Bitboard get_epts( const Pos *p );
Rawcode rawcode( const char *rawmove );
void rawmove( Rawcode rawcode, char *writable );
void make_move( Pos *p, Rawcode rc, char promotion );
void remove_castling_rights( Pos *p, const char *color, const char *side );
char *get_ecaf( const Pos *p );
void toggle_turn( Pos *p );
char *single_san_make_move( const char *fen, const char *san );

#endif
// End BASE_H

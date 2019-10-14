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
// Rename, document >>
#define FEN_SUPERPOSITION_1 \
    "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1"
#define FEN_SUPERPOSITION_2 \
    "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1"
// << Rename, document
// TODO: doc
#define FEN_KIWIPETE \
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 12 34"
#define FEN_PERFT_POS_3 \
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 12 34"
#define FEN_PERFT_POS_4 \
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define FEN_PERFT_POS_4_MIRROR \
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"
#define FEN_PERFT_POS_5 \
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
#define FEN_PERFT_POS_6 \
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"

// TODO: doc
#define NO_BRAINER_MATE_IN_2 \
    "RK1kr1nN/P2p1pPp/1P3PRP/Bp4P1/1Pq5/8/8/8 w - - 0 1"

// TODO: doc
extern const long long EPR_INIT_POS[]; // epr, expected perft results
extern const long long EPR_KIWIPETE[];
extern const long long EPR_PERFT_POS_3[];
extern const long long EPR_PERFT_POS_4[];
extern const long long EPR_PERFT_POS_5[];
extern const long long EPR_PERFT_POS_6[];

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

// FEN game tree level offset (array) size
#define FGT_LO_SIZE 21

// TODO: doc
struct explicit_game_tree {
    char **ufen; // ufen, unique FENs
    uint8_t height, *cc; // cc, child count
    // nc, node count; lo, level offset
    uint32_t nc, num_ufen, lo[FGT_LO_SIZE], *parent, **children,
        *findex;
};

//
// The Chester interface
//
enum che_fen_error che_fen_validator( const char *fen );
bool che_is_san( const char *san );
char *che_make_moves( const char *fen, const char *sans );
char *che_move_gen( const char *fens );
// char **che_fen_fields( const char *fen, bool expand_ppf, bool expand_caf );
// void che_free_ff( void *ff );
long long che_perft( const char *fen, int depth, bool mt );
long long che_gt_node_count( const char *fen, uint8_t height );
struct explicit_game_tree che_build_explicit_gt( const char *fen,
    uint8_t height, bool set_highest_level_child_counts );
void che_free_explicit_gt( struct explicit_game_tree gt );
uint32_t che_explicit_gt_stats( struct explicit_game_tree gt, uint32_t *captures,
    uint32_t *en_passants, uint32_t *castles, uint32_t *proms, uint32_t *checks,
    uint32_t *checkmates );
char *che_children( const char *fen );
size_t che_remove_redundant_epts( char **fen, size_t count );

// enum che_movetext_error che_movetext_validator(fen, movetext);

#endif
// end CHESTER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "extra.h"
#include "utils.h"
#include "chester.h"
#include "validation.h"
#include "move_gen.h"

// Diagonals and antidiagonals
const Bitboard
    SS_DIAG_H1H1 = 0x80U, SS_DIAG_G1H2 = 0x8040U, SS_DIAG_F1H3 = 0x804020U,
    SS_DIAG_E1H4 = 0x80402010U, SS_DIAG_D1H5 = 0x8040201008U,
    SS_DIAG_C1H6 = 0x804020100804U, SS_DIAG_B1H7 = 0x80402010080402U,
    SS_DIAG_A1H8 = 0x8040201008040201U, SS_DIAG_A2G8 = 0x4020100804020100U,
    SS_DIAG_A3F8 = 0x2010080402010000U, SS_DIAG_A4E8 = 0x1008040201000000U,
    SS_DIAG_A5D8 = 0x804020100000000U, SS_DIAG_A6C8 = 0x402010000000000U,
    SS_DIAG_A7B8 = 0x201000000000000U, SS_DIAG_A8A8 = 0x100000000000000U,
    SS_ANTIDIAG_A1A1 = 0x1U, SS_ANTIDIAG_B1A2 = 0x102U,
    SS_ANTIDIAG_C1A3 = 0x10204U, SS_ANTIDIAG_D1A4 = 0x1020408U,
    SS_ANTIDIAG_E1A5 = 0x102040810U, SS_ANTIDIAG_F1A6 = 0x10204081020U,
    SS_ANTIDIAG_G1A7 = 0x1020408102040U, SS_ANTIDIAG_H1A8 = 0x102040810204080U,
    SS_ANTIDIAG_H2B8 = 0x204081020408000U, SS_ANTIDIAG_H3C8 = 0x408102040800000U,
    SS_ANTIDIAG_H4D8 = 0x810204080000000U, SS_ANTIDIAG_H5E8 = 0x1020408000000000U,
    SS_ANTIDIAG_H6F8 = 0x2040800000000000U, SS_ANTIDIAG_H7G8 = 0x4080000000000000U,
    SS_ANTIDIAG_H8H8 = 0x8000000000000000U;

//
// Static function prototypes
//
static Bitboard x_sq_set_of_diag( const int index );
static Bitboard x_sq_set_of_antidiag( const int index );
static void x_kerc_zero_one_or_two_sqs_in_dir( const Bitboard sq_bit,
    int *num_of_sqs_north, int *num_of_sqs_east,
    int *num_of_sqs_south, int *num_of_sqs_west );
static Bitboard x_kerc_find_upper_left_corner( const Bitboard sq_bit,
    const int num_of_sqs_north, const int num_of_sqs_west );
static Bitboard x_kerc_find_lower_right_corner( const Bitboard sq_bit,
    const int num_of_sqs_south, const int num_of_sqs_east );
static Bitboard x_kerc_find_upper_right_or_lower_left_corner(
    const Bitboard upper_left, const Bitboard lower_right,
    const bool find_upper_right );
static Bitboard x_kerc_unset_corner_bits( Bitboard sq_rect,
    const int num_of_sqs_north, const int num_of_sqs_east,
    const int num_of_sqs_south, const int num_of_sqs_west,
    const Bitboard upper_left, const Bitboard lower_right );

// The following function is purely for performance research purposes.
// It converts the tic-tac-toe (ttt) version of a piece placement array
// (PPA) into the corresponding piece placement field (PPF). For example,
// {174, 257, 80} => "o1x/1o1/x2".
//
// Note that the function is analogous to pos_to_fen(). The "dyn" in
// the function name refers to the fact that dynamic memory allocation
// is involved.
//
// +-----------+
// | 6 | 7 | 8 |
// +-----------+
// | 3 | 4 | 5 |    Bit indexes
// +-----------+
// | 0 | 1 | 2 |
// +-----------+
//
// +-----------+
// | o |   | x |
// +-----------+
// |   | o |   |    "o1x/1o1/x2"
// +-----------+
// | x |   |   |
// +-----------+
//
// The following is the PPA for the above board position.
//
//      8 7 6 5 4 3 2 1 0  | Bit indexes
// [0]: 0 1 0 1 0 1 1 1 0  | ppa[0], the empty squares
// [1]: 1 0 0 0 0 0 0 0 1  | ppa[1], the X's
// [2]: 0 0 1 0 1 0 0 0 0  | ppa[2], the O's

#define TTT_PPA_TO_PPF_CONV \
    const uint16_t one = 1; \
    const int bi[] = {6, 7, 8, 3, 4, 5, 0, 1, 2}; \
    int index = 0; \
\
    for(int i = 0; i < 9; i++) { \
        if(bi[i] == 3 || bi[i] == 0) s[index++] = '/'; \
        uint16_t bit = (one << bi[i]); \
        for(int j = 0; ; j++) { \
            if(bit & ppa[j]) { \
                if(j) s[index++] = (j == 1 ? 'x' : 'o'); \
                else if(bi[i] % 3 && \
                        (s[index - 1] == '1' || s[index - 1] == '2')) \
                    ++s[index - 1]; \
                else s[index++] = '1'; \
\
                break; \
            } \
        } \
    } \
\
    s[index] = '\0';

char *
dyn_ttt_ppa_to_ppf_conv( const uint16_t *ppa )
{
    char *s = malloc(11 + 1); // strlen("oxx/xoo/xoo") == 11

    TTT_PPA_TO_PPF_CONV

    return s;
}

// The static memory counterpart of dyn_ttt_ppa_to_ppf_conv(). The
// argument to 'ppa' should point to an array of three integers. The
// result of the conversion is saved in the character array pointed to
// by 's'. The character array should be at least 12 bytes in size.
// Note that unlike dyn_ttt_ppa_to_ppf_conv(), using
// sta_ttt_ppa_to_ppf_conv() doesn't involve any calls to malloc()
// and free().
void
sta_ttt_ppa_to_ppf_conv( const uint16_t *ppa, char *s )
{
    TTT_PPA_TO_PPF_CONV
}

// The means for testing the difference in performance between
// dyn_ttt_ppa_to_ppf_conv() and sta_ttt_ppa_to_ppf_conv(). Using the
// latter is (slightly) more efficient due to not having to call
// malloc() and free().
//
// $ uname -a
// Linux nterror 4.15.0-39-generic #42~16.04.1-Ubuntu SMP
// Wed Oct 24 17:09:54 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
//
// A PC desktop built in 2012 with the above specs gave a performance
// ratio of about 1.3. (See the printf() at the end of the function.)
void
comparative_ttt_ppa_to_ppf_conv_test( const int reps )
{
    const uint16_t nine_1s = 511, ppa1[] = {nine_1s, 0, 0},
        ppa2[] = {174, 257, 80};

    char *ppf_ptr;
    long long t1 = time_in_milliseconds(), t2, result_1, result_2;
    printf("Calling dyn_ttt_ppa_to_ppf_conv() %d times...\n", 2*reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000 * 1000) == 0) fprintf(stderr, "-");
        ppf_ptr = dyn_ttt_ppa_to_ppf_conv(ppa1);
        free(ppf_ptr);
        ppf_ptr = dyn_ttt_ppa_to_ppf_conv(ppa2);
        free(ppf_ptr); }
    t2 = time_in_milliseconds(), result_1 = t2 - t1;
    printf("\nThe first operation took %lld ms\n", result_1);

    char ppf_array[11 + 1];
    t1 = time_in_milliseconds();
    printf("Calling sta_ttt_ppa_to_ppf_conv() %d times...\n", 2*reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000 * 1000) == 0) fprintf(stderr, "+");
        sta_ttt_ppa_to_ppf_conv(ppa1, ppf_array);
        sta_ttt_ppa_to_ppf_conv(ppa2, ppf_array); }
    t2 = time_in_milliseconds(), result_2 = t2 - t1;
    printf("\nThe second operation took %lld ms\n", result_2);

    printf("Performance ratio: %.2f\n", (double) result_1 / result_2);
}

// An ad hoc tester function to evaluate the efficiency of move_move().
void
make_move_performance_test()
{
    const int REPS = 1 * 10000;

    const char *fen[] = {
        INIT_POS,
        "r1bqk2r/2ppbppp/p1n2n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 b kq - 1 7",
        "3b4/P3P3/8/8/8/8/8/K6k w - - 1 123",
        "b3k3/8/8/8/3P4/8/8/4K2R b K d3 0 9",
        "4k2r/8/8/8/6Pp/8/8/7K b k g3 0 75",
        NULL };

    const char *check[] = {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "r1bq1rk1/2ppbppp/p1n2n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 w - - 2 8",
        "3Q4/P7/8/8/8/8/8/K6k b - - 0 123",
        "4k3/8/8/8/3P4/8/8/4K2b w - - 0 10",
        "4k2r/8/8/8/8/6p1/8/7K w k - 0 76" };

    const Pos *p[5];
    Pos result[5];
    for(int i = 0; i < 5; i++) p[i] = fen_to_pos(fen[i]);
    copy_pos(p[0], &result[0]), make_move(&result[0], 936, '-');  // "e4"
    copy_pos(p[1], &result[1]), make_move(&result[1], 1140, '-'); // "O-O"
    copy_pos(p[2], &result[2]), make_move(&result[2], 1098, 'q'); // "exd8=Q"
    copy_pos(p[3], &result[3]), make_move(&result[3], 193, '-');  // "Bxh1"
    copy_pos(p[4], &result[4]), make_move(&result[4], 1685, '-'); // "hxg3+"

    for(int i = 0; i < 5; i++) {
        char *tmp = pos_to_fen(&result[i]);
        assert(!strcmp(tmp, check[i]));
        free(tmp); }

    Pos pos;
    long long t0 = time_in_milliseconds();
    // 50 * 1000 calls to make_move() is the standard
    for(int count = 1; count <= REPS; count++) {
        copy_pos(p[0], &pos), make_move(&pos,  936, '-');
        copy_pos(p[1], &pos), make_move(&pos, 1140, '-');
        copy_pos(p[2], &pos), make_move(&pos, 1098, 'q');
        copy_pos(p[3], &pos), make_move(&pos,  193, '-');
        copy_pos(p[4], &pos), make_move(&pos, 1685, '-'); }

    printf("%s: %d calls to make_move() took %lld ms\n",
        __func__, 5 * REPS, time_in_milliseconds() - t0);

    for(int i = 0; i < 5; i++) free((void *) p[i]);
}

// An ad hoc tester function to evaluate the efficiency of rawcodes().
void
rawcodes_performance_test()
{
    const int REPS = 10 * 1000;
    long long t0, dur, sum = 0;

    const char *fen[] = {
        // FEN_SUPERPOSITION_1,
        // FEN_SUPERPOSITION_2,

        INIT_POS,
        "k4r2/8/8/2p5/8/4K3/r7/8 w - c6 0 123",
        "k7/8/8/4K3/8/3N4/5r2/4r3 w - - 12 34",
        "k7/8/4N3/4K3/8/3N4/5r2/4r3 w - - 12 34",
        "4k2r/7p/7P/8/8/8/8/4K3 b k - 19 83",
        FEN_GAME_OF_THE_CENTURY,
        FEN_PERSONALLY_SIGNIFICANT,
        "1r2krbq/p1pp2bp/4p1p1/3nPp2/3P1P2/1N1N2P1/PPP4P/1R2KRBQ w FBfb f6 0 11",
        "r1bqk2r/2ppbppp/p1n2n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 b kq - 1 7",
        "4k3/8/8/8/3q4/8/8/4K3 b - - 13 37",
        NULL };

    int i = 0;
    for(; fen[i]; i++) {
        assert(!che_fen_validator(fen[i]));
        const Pos *p = fen_to_pos(fen[i]);
        t0 = time_in_milliseconds();
        for(int count = 1; count <= REPS; count++) {
            Rawcode *rc = rawcodes(p);
            free(rc); }
        dur = time_in_milliseconds() - t0;
        sum += dur;
        printf("[%5lld ms]  \"%s\"\n", dur, fen[i]);
        free((void *) p); }

    printf("Average: %.2f ms\n", (double) sum / i);
}

// Finds the origin and destination square bit indexes of the rawcode
// argument 'rc' and saves the result in 'orig' and 'dest'. For example,
// if 'rc' equals 1, then the bit index values 0 and 8 will be saved
// in 'orig' and 'dest', respectively.
//
// What comes to performance (problems), this function is the devil.
void
rawcode_bit_indexes( Rawcode rc, int *orig, int *dest )
{
    char move[4 + 1], orig_sq_name[2 + 1] = {0},
        dest_sq_name[2 + 1] = {0};

    rawmove( rc, move ); // 'rc' gets validated in rawmove()
    orig_sq_name[0] = move[0], orig_sq_name[1] = move[1];
    dest_sq_name[0] = move[2], dest_sq_name[1] = move[3];
    assert( is_sq_name( orig_sq_name ) );
    assert( is_sq_name( dest_sq_name ) );

    *orig = sq_name_to_bindex( orig_sq_name );
    *dest = sq_name_to_bindex( dest_sq_name );
}

// Returns the bit index of the square bit argument 'sq_bit'. The bit
// index (of a Bitboard) is an integer in the range [0,63]. Note that the
// bit index is also the exponent 'k' in sq_bit = 2^k.
//
// Note that the function does the same thing as bindex() and is probably
// as efficient as bindex().
int
sq_bit_index( Bitboard sq_bit )
{
    for( int i = 0; i < 64; i++ )
        if( sq_bit == SBA[i] )
            return i;

    assert(false);
    return -1;
}

// Returns the diagonal the square parameter is on
Bitboard
diag_of_sq( Bitboard sq_bit )
{
    assert( is_sq_bit( sq_bit ) );

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
    assert( is_sq_bit( sq_bit ) );

    for( int i = 0; i < 15; i++ )
        if( sq_bit & sq_set_of_antidiag( i ) )
            return sq_set_of_antidiag( i );

    assert( false );
    return 0u;
}

// Can be used to collectively access the SS_DIAG_* constants as if they
// were elements of the same array. The index of diagonal h1h1 is 0 and
// the index of a8a8 is 14.
Bitboard
sq_set_of_diag( const int index )
{
    return x_sq_set_of_diag( index );
}

// Same as above for the SS_ANTIDIAG_* constants. The index of antidiagonal
// a1a1 is 0 and the index of h8h8 14.
Bitboard
sq_set_of_antidiag( const int index )
{
    return x_sq_set_of_antidiag( index );
}

void
make_move_sanity_checks( const Pos *p, Rawcode rc, char promotion,
    int orig, int dest, Chessman mover, Chessman target )
{
    bool w = whites_turn(p);

    // 'promotion' should be one of the five valid character values.
    // If 'promotion' is not the char '-', then the move involved
    // should be a promotion.
    assert( promotion == '-' || promotion == 'q' || promotion == 'r' ||
        promotion == 'b' || promotion == 'n' );
    assert( ( promotion == '-' && !is_promotion(p, rc) ) ||
        ( (promotion == 'q' || promotion == 'r' || promotion == 'b' ||
            promotion == 'n') &&
        is_promotion(p, rc) ) );

    // On White's turn only white chessmen can move; the same for Black
    assert(
        (w && mover >= WHITE_KING && mover <= WHITE_PAWN) ||
        (!w && mover >= BLACK_KING && mover <= BLACK_PAWN));
    // A castling move is either O-O or O-O-O but not both
    assert( !(is_short_castle(p, rc) && is_long_castle(p, rc)) );
    // A pawn advance such as e2–e4 cannot involve a capture
    assert( !is_pawn_advance(p, rc) || target == EMPTY_SQUARE );
    // If a pawn moves a single square diagonally "upwards", it should
    // involve a capture
    assert( !( mover == (w ? WHITE_PAWN : BLACK_PAWN) &&
        ( SBA[dest] == sq_nav(SBA[orig], w ? NORTHWEST :
                SOUTHWEST) ||
            SBA[dest] == sq_nav(SBA[orig], w ? NORTHEAST :
                SOUTHEAST) ) ) ||
        is_capture(p, rc) );

    // It's OK for make_move() to execute moves that result in a position
    // where a king can be captured. However, the position should be legal
    // before the execution of the move.
    assert( !forsaken_king(p) );

    // The white king captures a white rook if and only if the move in
    // question is a castling move (according to is_castle()).
    assert(!w || (
        ((mover == WHITE_KING && target == WHITE_ROOK) && is_castle(p, rc)) ||
        (!(mover == WHITE_KING && target == WHITE_ROOK) && !is_castle(p, rc))));
    // The black king captures a black rook if and only if the move in
    // question is a castling move (according to is_castle()).
    assert(w || (
        ((mover == BLACK_KING && target == BLACK_ROOK) && is_castle(p, rc)) ||
        (!(mover == BLACK_KING && target == BLACK_ROOK) && !is_castle(p, rc))));

    Bitboard the_epts = get_epts(p); // En passant target square
    bool ep = (the_epts == SBA[dest] &&
        (mover == WHITE_PAWN || mover == BLACK_PAWN));

    // (ep == true) if and only if (is_en_passant_capture(p, rc) == true)
    assert( (ep && is_en_passant_capture(p, rc)) ||
        (!ep && !is_en_passant_capture(p, rc)) );

    // The EPTS is empty
    assert(!ep || ((the_epts & p->ppa[EMPTY_SQUARE]) && target == EMPTY_SQUARE));

    Bitboard double_advanced_pawn = the_epts;
    if(w) double_advanced_pawn >>= 8;
    else double_advanced_pawn <<= 8;
    // The square "after" the EPTS is occupied by a pawn of the
    // non-active color
    assert(!ep || ((double_advanced_pawn & p->ppa[w ? BLACK_PAWN : WHITE_PAWN])));

    bool pp = ( // pp, pawn promotion
        (mover == WHITE_PAWN && orig >= 48 && orig <= 55 ) ||
        (mover == BLACK_PAWN && orig >=  8 && orig <= 15));
    assert((pp && is_promotion(p, rc)) || (!pp && !is_promotion(p, rc)));

    bool capture = ( ep ||
        (mover >= WHITE_KING && mover <= WHITE_PAWN &&
            target >= BLACK_QUEEN && target <= BLACK_PAWN) ||
        (mover >= BLACK_KING && mover <= BLACK_PAWN &&
            target >= WHITE_QUEEN && target <= WHITE_PAWN) );

    assert((capture &&  is_capture(p, rc)) || (!capture && !is_capture(p, rc)));

    bool sspa = ( // sspa, single step pawn advance
        (mover == WHITE_PAWN && SBA[dest] == (SBA[orig] << 8)) ||
        (mover == BLACK_PAWN && SBA[dest] == (SBA[orig] >> 8)));
    bool dspa = ( // dspa, double step pawn advance
        (mover == WHITE_PAWN && SBA[dest] == (SBA[orig] << 16)) ||
        (mover == BLACK_PAWN && SBA[dest] == (SBA[orig] >> 16)));
    assert(( sspa &&  is_single_step_pawn_advance(p, rc)) ||
        (!sspa && !is_single_step_pawn_advance(p, rc)));
    assert(( dspa &&  is_double_step_pawn_advance(p, rc)) ||
        (!dspa && !is_double_step_pawn_advance(p, rc)));
}

// KERC, knight's effective range circle. The call kerc( SB.e4 ) would
// return 0x387c7c7c3800U. Here's a diagram of the bitboard:
//
//     -  -  -  -  -  -  -  -  8
//     -  -  -  -  -  -  -  -  7
//     -  -  -  #  #  #  -  -  6
//     -  -  #  #  #  #  #  -  5
//     -  -  #  #  #  #  #  -  4
//     -  -  #  #  #  #  #  -  3
//     -  -  -  #  #  #  -  -  2
//     -  -  -  -  -  -  -  -  1
//     a  b  c  d  e  f  g  h
//
Bitboard
kerc( const Bitboard sq_bit )
{
    assert( is_sq_bit( sq_bit ) );

    int num_of_sqs_north, num_of_sqs_east, num_of_sqs_south, num_of_sqs_west;
    x_kerc_zero_one_or_two_sqs_in_dir( sq_bit, &num_of_sqs_north,
        &num_of_sqs_east, &num_of_sqs_south, &num_of_sqs_west );

    const Bitboard upper_left = x_kerc_find_upper_left_corner( sq_bit,
        num_of_sqs_north, num_of_sqs_west ),
        lower_right = x_kerc_find_lower_right_corner( sq_bit,
        num_of_sqs_south, num_of_sqs_east );

    Bitboard sq_rect = rectangle_of_sqs(bindex(upper_left), bindex(lower_right));

    return x_kerc_unset_corner_bits( sq_rect, num_of_sqs_north, num_of_sqs_east,
        num_of_sqs_south, num_of_sqs_west, upper_left, lower_right );
}

// Returns the rectangle of squares indicated by the bit index parameters
// 'ulc' and 'lrc' (upper left/lower right corner). Any rectangular area
// on the chessboard can be defined by its upper left and lower right
// corner squares. The following is a square rectangle that has four rows
// and five columns.
//
//     c6  d6  e6  f6  g6
//     c5  d5  e5  f5  g5
//     c4  d4  e4  f4  g4
//     c3  d3  e3  f3  g3
//
// The function begins its job knowing only the upper left corner and the
// lower right corner:
//
//     c6  ??  ??  ??  ??
//     ??  ??  ??  ??  ??
//     ??  ??  ??  ??  ??
//     ??  ??  ??  ??  g3
//
// The function returns zero if the arguments don't make sense as an upper
// left and lower right corner. For example, rectangle_of_sqs(28, 35)
// returns zero.
Bitboard
rectangle_of_sqs( int ulc, int lrc )
{
    Bitboard rectangle = 0;

    int urc; // Upper right corner
    if(ulc == lrc || SQ_RAY[ulc][EAST] & ONE << lrc) urc = lrc;
    else if(SQ_RAY[ulc][SOUTH] & ONE << lrc) urc = ulc;
    else urc = bindex(SQ_RAY[ulc][EAST] & SQ_RAY[lrc][NORTH]);

    const Bitboard top_row = ONE << ulc | (SQ_RAY[ulc][EAST] &
        SQ_RAY[urc][WEST]) | ONE << urc;
    int height = 0;
    for(int bi = lrc; bi <= urc; bi += 8, ++height) {}
    for(int coef = 0; coef < height; ++coef)
        rectangle |= top_row >> coef * 8;

    return rectangle;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

static Bitboard
x_sq_set_of_diag( const int index )
{
    switch( index ) {
        case 0: return SS_DIAG_H1H1;
        case 1: return SS_DIAG_G1H2;
        case 2: return SS_DIAG_F1H3;
        case 3: return SS_DIAG_E1H4;
        case 4: return SS_DIAG_D1H5;
        case 5: return SS_DIAG_C1H6;
        case 6: return SS_DIAG_B1H7;
        case 7: return SS_DIAG_A1H8;
        case 8: return SS_DIAG_A2G8;
        case 9: return SS_DIAG_A3F8;
        case 10: return SS_DIAG_A4E8;
        case 11: return SS_DIAG_A5D8;
        case 12: return SS_DIAG_A6C8;
        case 13: return SS_DIAG_A7B8;
        case 14: return SS_DIAG_A8A8;

        default: assert( false ); return 0u;
    }
}

static Bitboard
x_sq_set_of_antidiag( const int index )
{
    switch( index ) {
        case 0: return SS_ANTIDIAG_A1A1;
        case 1: return SS_ANTIDIAG_B1A2;
        case 2: return SS_ANTIDIAG_C1A3;
        case 3: return SS_ANTIDIAG_D1A4;
        case 4: return SS_ANTIDIAG_E1A5;
        case 5: return SS_ANTIDIAG_F1A6;
        case 6: return SS_ANTIDIAG_G1A7;
        case 7: return SS_ANTIDIAG_H1A8;
        case 8: return SS_ANTIDIAG_H2B8;
        case 9: return SS_ANTIDIAG_H3C8;
        case 10: return SS_ANTIDIAG_H4D8;
        case 11: return SS_ANTIDIAG_H5E8;
        case 12: return SS_ANTIDIAG_H6F8;
        case 13: return SS_ANTIDIAG_H7G8;
        case 14: return SS_ANTIDIAG_H8H8;

        default: assert( false ); return 0u;
    }
}

static void
x_kerc_zero_one_or_two_sqs_in_dir( const Bitboard sq_bit,
    int *num_of_sqs_north, int *num_of_sqs_east,
    int *num_of_sqs_south, int *num_of_sqs_west )
{
    const char *sq = SQ_NAME[bindex(sq_bit)];

    int *eight_dirs[ 8 ] = { NULL };
    eight_dirs[ NORTH ] = num_of_sqs_north, eight_dirs[ EAST ] = num_of_sqs_east,
        eight_dirs[ SOUTH ] = num_of_sqs_south, eight_dirs[ WEST ] = num_of_sqs_west;
    *( eight_dirs[ NORTH ] ) = *( eight_dirs[ EAST ] ) =
        *( eight_dirs[ SOUTH ] ) = *( eight_dirs[ WEST ] ) = 0;

    for( enum sq_dir dir = NORTH; dir <= WEST; dir += 2 ) {
        if( sq_navigator( sq, dir ) )
            **( eight_dirs + dir ) += 1;
        else
            continue;

        if( sq_navigator( sq_navigator( sq, dir ), dir ) )
            **( eight_dirs + dir ) += 1;
    }

    assert(
        ( *num_of_sqs_north >= 0 && *num_of_sqs_north <= 2 ) &&
        ( *num_of_sqs_east  >= 0 && *num_of_sqs_east  <= 2 ) &&
        ( *num_of_sqs_south >= 0 && *num_of_sqs_south <= 2 ) &&
        ( *num_of_sqs_west  >= 0 && *num_of_sqs_west  <= 2 ) );
}

#define X_KERC_IF_ELSE_IF( north_or_south, east_or_west, \
        first_swift_op, second_shift_op ) \
    if( north_or_south == 0 && east_or_west == 0 ) \
        ret_val = sq_bit; \
    else if( north_or_south == 0 && east_or_west == 1 ) \
        ret_val = sq_bit first_swift_op 1; \
    else if( north_or_south == 0 && east_or_west == 2 ) \
        ret_val = sq_bit first_swift_op 2; \
    else if( north_or_south == 1 && east_or_west == 0 ) \
        ret_val = sq_bit second_shift_op 8; \
    else if( north_or_south == 1 && east_or_west == 1 ) \
        ret_val = sq_bit second_shift_op 7; \
    else if( north_or_south == 1 && east_or_west == 2 ) \
        ret_val = sq_bit second_shift_op 6; \
    else if( north_or_south == 2 && east_or_west == 0 ) \
        ret_val = sq_bit second_shift_op 16; \
    else if( north_or_south == 2 && east_or_west == 1 ) \
        ret_val = sq_bit second_shift_op 15; \
    else if( north_or_south == 2 && east_or_west == 2 ) \
        ret_val = sq_bit second_shift_op 14;

static Bitboard
x_kerc_find_upper_left_corner( const Bitboard sq_bit,
    const int num_of_sqs_north, const int num_of_sqs_west )
{
    Bitboard ret_val = 0u;

    X_KERC_IF_ELSE_IF( num_of_sqs_north, num_of_sqs_west, >>, << )

    assert( ret_val );
    return ret_val;
}

static Bitboard
x_kerc_find_lower_right_corner( const Bitboard sq_bit,
    const int num_of_sqs_south, const int num_of_sqs_east )
{
    Bitboard ret_val = 0u;

    X_KERC_IF_ELSE_IF( num_of_sqs_south, num_of_sqs_east, <<, >> )

    assert( ret_val );
    return ret_val;
}

static Bitboard
x_kerc_find_upper_right_or_lower_left_corner(
    const Bitboard upper_left, const Bitboard lower_right,
    const bool find_upper_right )
{
    const char *upper_left_sq_name = SQ_NAME[bindex(upper_left)],
        *lower_right_sq_name = SQ_NAME[bindex(lower_right)];
    char tmp_sq_name[ 3 ] = { 0 };

    tmp_sq_name[ 0 ] = find_upper_right ?
            lower_right_sq_name[ 0 ] : upper_left_sq_name[ 0 ],
        tmp_sq_name[ 1 ] = find_upper_right ?
            upper_left_sq_name[ 1 ] : lower_right_sq_name[ 1 ];

    return SBA[ sq_name_to_bindex( tmp_sq_name ) ];
}

static Bitboard
x_kerc_unset_corner_bits( Bitboard sq_rect,
    const int num_of_sqs_north, const int num_of_sqs_east,
    const int num_of_sqs_south, const int num_of_sqs_west,
    const Bitboard upper_left, const Bitboard lower_right )
{
    const Bitboard upper_right = x_kerc_find_upper_right_or_lower_left_corner(
        upper_left, lower_right, true ),
        lower_left = x_kerc_find_upper_right_or_lower_left_corner(
        upper_left, lower_right, false );

    if( num_of_sqs_north == 2 && num_of_sqs_east == 2 )
        sq_rect ^= upper_right;
    if( num_of_sqs_south == 2 && num_of_sqs_east == 2 )
        sq_rect ^= lower_right;
    if( num_of_sqs_south == 2 && num_of_sqs_west == 2 )
        sq_rect ^= lower_left;
    if( num_of_sqs_north == 2 && num_of_sqs_west == 2 )
        sq_rect ^= upper_left;

    return sq_rect;
}

#undef X_KERC_IF_ELSE_IF

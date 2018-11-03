#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "move_gen.h"
#include "utils.h"
#include "extra.h"
#include "validation.h"
#include "pgn.h"

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
static void x_attackers_init_attacker_type(
    bool *attacker_type, va_list arg_ptr, int num_arg );
static void x_attackers_find(
    Bitboard *the_attackers, bool *attacker_type,
        const Bitboard *ppa, Bitboard sq );
static Bitboard x_attackers_kings(
    const Bitboard *ppa, Bitboard sq, bool color_is_white );
static Bitboard x_attackers_rooks_or_queens(
    const Bitboard *ppa, Bitboard sq, bool color_is_white, bool cm_is_queen );
static Bitboard x_attackers_bishops_or_queens(
    const Bitboard *ppa, Bitboard sq, bool color_is_white, bool cm_is_queen );
static Bitboard x_attackers_knights(
    const Bitboard *ppa, Bitboard sq, bool color_is_white );
static Bitboard x_attackers_pawns(
    const Bitboard *ppa, Bitboard sq, bool color_is_white );
static bool x_castle_valid_castle_type( const char *castle_type );
static void x_castle_set_castling_king_and_rook( const Pos *p, bool kingside,
    Bitboard *castling_king, Bitboard *castling_rook );
static bool x_castle_king_in_check( const Pos *p );
static bool x_castle_kings_path_blocked( const Pos *p, bool kingside,
    Bitboard castling_king, Bitboard castling_rook );
static bool x_castle_kings_exclusive_path_in_check( const Pos *p,
    bool kingside, Bitboard castling_king );
static bool x_castle_rooks_path_blocked( const Pos *p, bool kingside,
    Bitboard castling_rook, Bitboard castling_king );
static Rawcode x_castle_rawcode(
    Bitboard castling_king, Bitboard castling_rook );
static void x_rawcodes_king_and_knight( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant, bool rook );
static void x_rawcodes_rook_and_bishop( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant, bool rook );
static void x_rawcodes_pawn_advance( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant );
static void x_rawcodes_pawn_capture( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant );
static void x_rawcodes_en_passant( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant );
static int x_qsort_rawcode_compare( const void *a, const void *b );

/***********************
 **** External data ****
 ***********************/

// Used for getting the "knight squares" of a particular square. For example,
// the knight squares of square b1 is the set { d2, c3, a3 }.
const Bitboard KNIGHT_SQS[] = {
    0x20400u, 0x50800u, 0xa1100u, 0x142200u, 0x284400u, 0x508800u, 0xa01000u,
    0x402000u, 0x2040004u, 0x5080008u, 0xa110011u, 0x14220022u, 0x28440044u,
    0x50880088u, 0xa0100010u, 0x40200020u, 0x204000402u, 0x508000805u,
    0xa1100110au, 0x1422002214u, 0x2844004428u, 0x5088008850u, 0xa0100010a0u,
    0x4020002040u, 0x20400040200u, 0x50800080500u, 0xa1100110a00u,
    0x142200221400u, 0x284400442800u, 0x508800885000u, 0xa0100010a000u,
    0x402000204000u, 0x2040004020000u, 0x5080008050000u, 0xa1100110a0000u,
    0x14220022140000u, 0x28440044280000u, 0x50880088500000u, 0xa0100010a00000u,
    0x40200020400000u, 0x204000402000000u, 0x508000805000000u, 0xa1100110a000000u,
    0x1422002214000000u, 0x2844004428000000u, 0x5088008850000000u,
    0xa0100010a0000000u, 0x4020002040000000u, 0x400040200000000u,
    0x800080500000000u, 0x1100110a00000000u, 0x2200221400000000u,
    0x4400442800000000u, 0x8800885000000000u, 0x100010a000000000u,
    0x2000204000000000u, 0x4020000000000u, 0x8050000000000u, 0x110a0000000000u,
    0x22140000000000u, 0x44280000000000u, 0x88500000000000u, 0x10a00000000000u,
    0x20400000000000u };

// The "king squares" of a given square. For example, KING_SQS[ 4 ] is the
// Bitboard 0x3828u (which is the square set { d1, f1, d2, e2, f2 }).
const Bitboard KING_SQS[] = {
    0x302U, 0x705U, 0xe0aU, 0x1c14U, 0x3828U, 0x7050U, 0xe0a0U, 0xc040U, 0x30203U,
    0x70507U, 0xe0a0eU, 0x1c141cU, 0x382838U, 0x705070U, 0xe0a0e0U, 0xc040c0U,
    0x3020300U, 0x7050700U, 0xe0a0e00U, 0x1c141c00U, 0x38283800U, 0x70507000U,
    0xe0a0e000U, 0xc040c000U, 0x302030000U, 0x705070000U, 0xe0a0e0000U,
    0x1c141c0000U, 0x3828380000U, 0x7050700000U, 0xe0a0e00000U, 0xc040c00000U,
    0x30203000000U, 0x70507000000U, 0xe0a0e000000U, 0x1c141c000000U,
    0x382838000000U, 0x705070000000U, 0xe0a0e0000000U, 0xc040c0000000U,
    0x3020300000000U, 0x7050700000000U, 0xe0a0e00000000U, 0x1c141c00000000U,
    0x38283800000000U, 0x70507000000000U, 0xe0a0e000000000U, 0xc040c000000000U,
    0x302030000000000U, 0x705070000000000U, 0xe0a0e0000000000U,
    0x1c141c0000000000U, 0x3828380000000000U, 0x7050700000000000U,
    0xe0a0e00000000000U, 0xc040c00000000000U, 0x203000000000000U,
    0x507000000000000U, 0xa0e000000000000U, 0x141c000000000000U,
    0x2838000000000000U, 0x5070000000000000U, 0xa0e0000000000000U,
    0x40c0000000000000U };

// Example: ROOK_SQS[ 28 ] is 0x10101010ef101010U. That is, the rook squares
// of the square e4 is the set { e1, e2, e3, a4, b4, c4, d4, f4, g4, h4, e5,
// e6, e7, e8 }.
const Bitboard ROOK_SQS[] = {
    0x1010101010101feU, 0x2020202020202fdU, 0x4040404040404fbU, 0x8080808080808f7U,
    0x10101010101010efU, 0x20202020202020dfU, 0x40404040404040bfU, 0x808080808080807fU,
    0x10101010101fe01U, 0x20202020202fd02U, 0x40404040404fb04U, 0x80808080808f708U,
    0x101010101010ef10U, 0x202020202020df20U, 0x404040404040bf40U, 0x8080808080807f80U,
    0x101010101fe0101U, 0x202020202fd0202U, 0x404040404fb0404U, 0x808080808f70808U,
    0x1010101010ef1010U, 0x2020202020df2020U, 0x4040404040bf4040U, 0x80808080807f8080U,
    0x1010101fe010101U, 0x2020202fd020202U, 0x4040404fb040404U, 0x8080808f7080808U,
    0x10101010ef101010U, 0x20202020df202020U, 0x40404040bf404040U, 0x808080807f808080U,
    0x10101fe01010101U, 0x20202fd02020202U, 0x40404fb04040404U, 0x80808f708080808U,
    0x101010ef10101010U, 0x202020df20202020U, 0x404040bf40404040U, 0x8080807f80808080U,
    0x101fe0101010101U, 0x202fd0202020202U, 0x404fb0404040404U, 0x808f70808080808U,
    0x1010ef1010101010U, 0x2020df2020202020U, 0x4040bf4040404040U, 0x80807f8080808080U,
    0x1fe010101010101U, 0x2fd020202020202U, 0x4fb040404040404U, 0x8f7080808080808U,
    0x10ef101010101010U, 0x20df202020202020U, 0x40bf404040404040U, 0x807f808080808080U,
    0xfe01010101010101U, 0xfd02020202020202U, 0xfb04040404040404U, 0xf708080808080808U,
    0xef10101010101010U, 0xdf20202020202020U, 0xbf40404040404040U, 0x7f80808080808080U };

// Example: BISHOP_SQS[ 28 ] is 0x182442800284482U. That is, the bishop squares
// of e4 is the set { b1, c2, d3, f5, g6, h7, h1, g2, f3, d5, c6, b7, a8 }.
const Bitboard BISHOP_SQS[] = {
    0x8040201008040200U, 0x80402010080500U, 0x804020110a00U, 0x8041221400U,
    0x182442800U, 0x10204885000U, 0x102040810a000U, 0x102040810204000U,
    0x4020100804020002U, 0x8040201008050005U, 0x804020110a000aU, 0x804122140014U,
    0x18244280028U, 0x1020488500050U, 0x102040810a000a0U, 0x204081020400040U,
    0x2010080402000204U, 0x4020100805000508U, 0x804020110a000a11U, 0x80412214001422U,
    0x1824428002844U, 0x102048850005088U, 0x2040810a000a010U, 0x408102040004020U,
    0x1008040200020408U, 0x2010080500050810U, 0x4020110a000a1120U, 0x8041221400142241U,
    0x182442800284482U, 0x204885000508804U, 0x40810a000a01008U, 0x810204000402010U,
    0x804020002040810U, 0x1008050005081020U, 0x20110a000a112040U, 0x4122140014224180U,
    0x8244280028448201U, 0x488500050880402U, 0x810a000a0100804U, 0x1020400040201008U,
    0x402000204081020U, 0x805000508102040U, 0x110a000a11204080U, 0x2214001422418000U,
    0x4428002844820100U, 0x8850005088040201U, 0x10a000a010080402U, 0x2040004020100804U,
    0x200020408102040U, 0x500050810204080U, 0xa000a1120408000U, 0x1400142241800000U,
    0x2800284482010000U, 0x5000508804020100U, 0xa000a01008040201U, 0x4000402010080402U,
    0x2040810204080U, 0x5081020408000U, 0xa112040800000U, 0x14224180000000U,
    0x28448201000000U, 0x50880402010000U, 0xa0100804020100U, 0x40201008040201U };

// Used by function castle() to store the outcome of castling move
// evaluation. The purpose of the global variable is to faciliate
// Check unit testing of castle().
enum castle_error_codes castle_error;

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// TODO: doc
char *
che_move_gen( const char *fens )
{
    assert(fens);
    /*
    char *unmod_ptr = malloc(strlen(fens) + 1), *fen_data = unmod_ptr, *fen;
    strcpy(fen_data, fens);

    const Pos *p; Rawcode *rc; int num_legal_moves;
    while((fen = next_line(&fen_data))) {
    }
    // https://www.lemoda.net/c/array-sort/

    free(unmod_ptr);
    */
    return NULL;
}

// TODO: doc
char *
single_fen_move_gen( const char *fen )
{
    assert(che_fen_validator(fen) == FEN_NO_ERRORS); // Remove!

    const Pos *p = fen_to_pos(fen);
    Rawcode *rc = rawcodes(p);
    int num_legal_moves = rc[0];
    char *str_data = malloc(
            num_legal_moves * (SAN_MAX_LENGTH + 1) * sizeof(char)),
        **san = malloc(num_legal_moves * sizeof(char *));

    for(int i = 1; i <= num_legal_moves; i++) {
        assert(!is_promotion(p, rc[i]));
        char *tmp_san = rawcode_to_san(p, rc[i], '-'),
            *target = str_data + (i - 1) * (SAN_MAX_LENGTH + 1) *
                sizeof(char);
        printf("%s ", tmp_san);
        strcpy(target, tmp_san), san[i - 1] = target;
        free(tmp_san);
    }
    printf("\n");

    string_sort(san, num_legal_moves);

    for(int i = 0; i < num_legal_moves; i++)
        printf("%s ", san[i]);
    printf("\n");

    int length = 0;
    for(int i = 0; i < num_legal_moves; i++)
        length += strlen(san[i]) + 1;
    char *legal_moves = malloc(length + 1);
    // legal_moves[0] = '\0';
    strcpy(legal_moves, "");

    for(int i = 0; i < num_legal_moves; i++)
        strcat(legal_moves, san[i]), strcat(legal_moves, " ");

    // legal_moves[length - 1] = '.', legal_moves[length] = '\0';
    legal_moves[length - 1] = '\0';

    free((void *) p), free(rc), free(str_data), free(san);

    return legal_moves;
}

// TODO: doc
Rawcode *
rawcodes( const Pos *p )
{
    // It is assumed that no chess position can have more than 500
    // pseudo-legal moves (according to some sources the maximum
    // known number of *legal* moves in a chess position is 218 as
    // is the case in
    // "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1").
    Rawcode pseudo[500]; // Array for pseudo-legal moves
    int vacant = 0; // Index of first vacant slot in 'pseudo'

    Rawcode O_O, O_O_O;
    if( (O_O = castle(p, "kingside")) )
        pseudo[vacant++] = O_O;
    if( (O_O_O = castle(p, "queenside")) )
        pseudo[vacant++] = O_O_O;

    for( int i = 0; i < 64; i++ ) {
        Bitboard bit = SBA[i];

        if( bit & p->ppa[whites_turn(p) ? WHITE_KING : BLACK_KING] ) {
            x_rawcodes_king_and_knight( p, sq_bit_index(bit),
                pseudo, &vacant, true );
        } else if( bit & p->ppa[whites_turn(p) ? WHITE_QUEEN : BLACK_QUEEN] ) {
            for( int i = 0; i <= 1; i++ )
                x_rawcodes_rook_and_bishop( p, sq_bit_index(bit),
                    pseudo, &vacant, i );
        } else if( bit & p->ppa[whites_turn(p) ? WHITE_ROOK : BLACK_ROOK] ) {
            x_rawcodes_rook_and_bishop( p, sq_bit_index(bit), pseudo,
                &vacant, true );
        } else if( bit & p->ppa[whites_turn(p) ? WHITE_BISHOP : BLACK_BISHOP] ) {
            x_rawcodes_rook_and_bishop( p, sq_bit_index(bit), pseudo,
                &vacant, false );
        } else if( bit & p->ppa[whites_turn(p) ? WHITE_KNIGHT : BLACK_KNIGHT] ) {
            x_rawcodes_king_and_knight( p, sq_bit_index(bit),
                pseudo, &vacant, false );
        } else if( bit & p->ppa[whites_turn(p) ? WHITE_PAWN : BLACK_PAWN] ) {
            x_rawcodes_pawn_advance( p, sq_bit_index(bit), pseudo, &vacant );
            x_rawcodes_pawn_capture( p, sq_bit_index(bit), pseudo, &vacant );
            x_rawcodes_en_passant( p, sq_bit_index(bit), pseudo, &vacant );
        }
    }

    int updated_vacant = vacant;
    for( int i = 0; i < vacant; i++ ) { // For each non-vacant slot in 'pseudo'
        Pos copy;
        copy_pos( p, &copy );

        make_move( &copy, pseudo[i], is_promotion(p, pseudo[i]) ? 'q' : '-' );
        if( king_can_be_captured( &copy ) ) pseudo[i] = 0, --updated_vacant;
    }

    Rawcode *codes =
        (Rawcode *) malloc( (updated_vacant + 1) * sizeof(Rawcode) );
    assert(codes);

    *codes = updated_vacant;

    int j = 0;
    for( int i = 0; i < vacant; i++ )
        // Illegal moves have previously been replaced with zero
        if( pseudo[i] )
            codes[++j] = pseudo[i];
    assert( j == updated_vacant );

    // Let's assume 'codes' contains the rawcodes 1, 2 and 3. The whole
    // dynamically allocated array should be the following after sorting:
    // {3,1,2,3}. Note that the first element of 'codes' always indicates
    // the remaining number of elements in the array, i.e., the number of
    // rawcodes the array contains (three in this case).
    qsort( codes + 1, *codes, sizeof(Rawcode), x_qsort_rawcode_compare );

    return codes;
}

// Checks whether a king can be captured in the given position. Note that
// being able to capture the enemy king is different from having it in
// check. The difference is in the active color.
bool
king_can_be_captured( const Pos *p )
{
    return whites_turn( p ) ?
        white_attackers( p->ppa, p->ppa[BLACK_KING] ) :
        black_attackers( p->ppa, p->ppa[WHITE_KING] );
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

    Bitboard sq_rect = sq_rectangle( upper_left, lower_right );

    return x_kerc_unset_corner_bits( sq_rect, num_of_sqs_north, num_of_sqs_east,
        num_of_sqs_south, num_of_sqs_west, upper_left, lower_right );
}

// Returns a bitboard of the chessmen attacking square 'sq'. The variable
// part of the argument list should consist of one or more Chessman
// constants. This list of chessmen identify the attackers. For example,
// the following call returns all the white and black pawns attacking
// square e4: attackers( ppa, SB.e4, 2, WHITE_PAWN, BLACK_PAWN ).
//
// It's important to realize that not all chessmen attacking a square can
// immediately move to that square. Consider the following position:
// "rk5b/8/8/1b6/8/8/BN6/K1Q5 w - - 10 50"
//
// In the above position square c4 is attacked by four chessmen, namely
// the black bishop as well as the white bishop, knight and queen. It's
// White's turn so obviously the black bishop cannot immediately move
// to square c4. Both the white knight and white bishop are in an
// absolute pin so they cannot legally make the move to c4 even though
// they are attacking the square. Only the white queen can immediately
// make the move to c4.
//
// A chessman is attacking a square if and only if it could capture an enemy
// chessman on that square. Here's another position to consider:
// "4k3/8/8/8/8/4p3/8/4K2R w K - 10 50"
//
// The black pawn is attacking squares d2 and f2 but not square e2. Therefore
// the white king can move to square e2 but not to either of the squares d2
// or f2. The white king can immediately castle kingside yet it doesn't follow
// that the white king is attacking square g1. Castling wouldn't be possible
// if there were an enemy chessman on square g1.
Bitboard
attackers( const Bitboard *ppa, Bitboard sq, int num_arg, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, num_arg );

    bool attacker_type[13] = {0};
    x_attackers_init_attacker_type( attacker_type, arg_ptr, num_arg );

    Bitboard the_attackers = 0;
    x_attackers_find( &the_attackers, attacker_type, ppa, sq );

    va_end(arg_ptr);
    return the_attackers;
} // Review: 2018-06-16

// A convenience function for attackers(). Returns a bitboard of all the
// white chessmen attacking square 'sq'.
Bitboard
white_attackers( const Bitboard *ppa, Bitboard sq )
{
    return attackers( ppa, sq, 6, WHITE_KING, WHITE_QUEEN, WHITE_ROOK,
        WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN );
} // Review: 2018-06-16

// A convenience function for attackers(). Returns a bitboard of all the
// black chessmen attacking square 'sq'.
Bitboard
black_attackers( const Bitboard *ppa, Bitboard sq )
{
    return attackers( ppa, sq, 6, BLACK_KING, BLACK_QUEEN, BLACK_ROOK,
        BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN );
} // Review: 2018-06-16

// Returns zero if the type of castling specified by 'castle_type' is
// not (immediately) available in position 'p'. The 'castle_type' parameter
// indicates either short or long castling (O-O or O-O-O) and should be
// one of the strings "kingside", "h-side", "queenside" or "a-side". If
// the specified type of castling is available as a move in position 'p',
// the function returns the rawcode corresponding to the castling move.
//
// In Chester, as far as coordinate notation is concerned, castling moves
// are encoded as the castling king capturing the castling rook. Consider
// the following made-up position: "4k3/8/8/8/8/8/8/4K2R w K - 0 1".
// White's kingside castling move O-O is represented as "e1h1" in
// coordinate notation which in turn translates to the rawcode 920.
Rawcode
castle( const Pos *p, const char *castle_type )
{
    assert( x_castle_valid_castle_type( castle_type ) );

    bool kingside = ( !strcmp( castle_type, "kingside" ) ||
        !strcmp( castle_type, "h-side" ) );

    Bitboard castling_king, castling_rook;
    x_castle_set_castling_king_and_rook(
        p, kingside, &castling_king, &castling_rook );

    // The global variable is for unit testing purposes
    castle_error = CASTLE_OK;

    if( !has_castling_right( p, whites_turn(p) ? "white" : "black",
            kingside ? "kingside" : "queenside" ) )
        castle_error = CASTLE_NO_CASTLING_RIGHT;
    else if( x_castle_king_in_check(p) )
        castle_error = CASTLE_KING_IN_CHECK;
    else if( x_castle_kings_path_blocked( p, kingside, castling_king,
            castling_rook ) )
        castle_error = CASTLE_KINGS_PATH_BLOCKED;
    else if( x_castle_kings_exclusive_path_in_check( p, kingside,
            castling_king ) )
        castle_error = CASTLE_KINGS_EXCLUSIVE_PATH_IN_CHECK;
    else if( x_castle_rooks_path_blocked( p, kingside, castling_rook,
            castling_king ) )
        castle_error = CASTLE_ROOKS_PATH_BLOCKED;

    if( castle_error ) return 0;

    return x_castle_rawcode( castling_king, castling_rook );
}

#define RETURN_STATEMENT(index) \
    return \
        ( \
            ( mover == WHITE_KING && target == WHITE_ROOK ) || \
            ( mover == BLACK_KING && target == BLACK_ROOK ) \
        ) && \
        ( \
            (  whites_turn(p) && (SBA[dest] & p->irp[index]) ) || \
            ( !whites_turn(p) && ((SBA[dest] >> 56) & p->irp[index]) ) \
        );

#define INIT_VARS \
    Chessman mover, target; \
    int orig, dest; \
    set_mover_target_orig_and_dest( p, move, &mover, &target, \
        &orig, &dest );

// Returns true if the move represented by 'move' is O-O in position 'p';
// otherwise returns false.
bool
is_short_castle( const Pos *p, Rawcode move )
{
    INIT_VARS
    RETURN_STATEMENT(1)
}

// Returns true if the move represented by 'move' is O-O-O in position 'p';
// otherwise returns false.
bool
is_long_castle( const Pos *p, Rawcode move )
{
    INIT_VARS
    RETURN_STATEMENT(0)
}

#undef RETURN_STATEMENT

// Returns true if the move represented by 'move' is a capture in
// position 'p'; otherwise returns false.
bool
is_capture( const Pos *p, Rawcode move )
{
    INIT_VARS

    assert( target != WHITE_KING && target != BLACK_KING );

    return
        is_en_passant_capture(p, move) ||
        (
            mover >= WHITE_KING && mover <= WHITE_PAWN &&
            target >= BLACK_QUEEN && target <= BLACK_PAWN
        ) ||
        (
            mover >= BLACK_KING && mover <= BLACK_PAWN &&
            target >= WHITE_QUEEN && target <= WHITE_PAWN
        );
}

// Returns true if the move represented by 'move' is a pawn advance
// in position 'p'; otherwise returns false.
bool
is_pawn_advance( const Pos *p, Rawcode move )
{
    INIT_VARS
    assert( !is_single_step_pawn_advance(p, move) ||
        !is_double_step_pawn_advance(p, move) );

    return is_single_step_pawn_advance(p, move) ||
        is_double_step_pawn_advance(p, move);
}

// Not tested
#define ASSERT_THAT_MOVER_NOT_PAWN_ON_FIRST_OR_LAST_RANK \
    assert( (mover != WHITE_PAWN && mover != BLACK_PAWN) || \
        ( !(SBA[orig] & rank('1')) && !(SBA[orig] & rank('8')) ) );

// Returns true if 'move' is a *single step* pawn advance in
// position 'p'; otherwise returns false.
bool
is_single_step_pawn_advance( const Pos *p, Rawcode move )
{
    INIT_VARS
    ASSERT_THAT_MOVER_NOT_PAWN_ON_FIRST_OR_LAST_RANK

    return (mover == WHITE_PAWN && SBA[dest] == (SBA[orig] << 8)) ||
        (mover == BLACK_PAWN && SBA[dest] == (SBA[orig] >> 8));
}

// Returns true if 'move' is a *double step* pawn advance in
// position 'p'; otherwise returns false.
bool
is_double_step_pawn_advance( const Pos *p, Rawcode move )
{
    INIT_VARS
    ASSERT_THAT_MOVER_NOT_PAWN_ON_FIRST_OR_LAST_RANK

    return
        (
            mover == WHITE_PAWN && (SBA[orig] & rank('2')) &&
                SBA[dest] == (SBA[orig] << 16)
        ) ||
        (
            mover == BLACK_PAWN && (SBA[orig] & rank('7')) &&
                SBA[dest] == (SBA[orig] >> 16)
        );
}

#undef ASSERT_THAT_MOVER_NOT_PAWN_ON_FIRST_OR_LAST_RANK

// Returns true if and only if 'move' is a pawn promotion move
// in position 'p'.
bool
is_promotion( const Pos *p, Rawcode move )
{
    INIT_VARS

    if( mover != WHITE_PAWN && mover != BLACK_PAWN ) return false;

    // pros_prom_sq: prospective promotion square
    Bitboard pawn = SBA[orig], pros_prom_sq = SBA[dest];

    assert( !(pawn & rank('1')) && (!(pawn & rank('8'))) );

    // Pawn advance as opposed to pawn capture
    assert( !is_pawn_advance(p, move) || target == EMPTY_SQUARE );

    if( mover == WHITE_PAWN )
        return ( (pawn & SB.a7) && ( pros_prom_sq & (SB.a8 | SB.b8) ) ) ||
            ( (pawn & SB.h7) && ( pros_prom_sq & (SB.g8 | SB.h8) ) ) ||
            ( ( pawn & (SB.b7 | SB.c7 | SB.d7 | SB.e7 | SB.f7 | SB.g7) ) &&
            ( dest >= orig + 7 && dest <= orig + 9 ) );

    return ( (pawn & SB.a2) && ( pros_prom_sq & (SB.a1 | SB.b1) ) ) ||
        ( (pawn & SB.h2) && ( pros_prom_sq & (SB.g1 | SB.h1) ) ) ||
        ( (pawn & (SB.b2 | SB.c2 | SB.d2 | SB.e2 | SB.f2 | SB.g2) ) &&
        (dest >= orig - 9 && dest <= orig - 7) );
}

// Returns true if and only if 'move' is an en passant capture
// in position 'p'.
bool
is_en_passant_capture( const Pos *p, Rawcode move )
{
    INIT_VARS

    Bitboard epts_bb = epts(p);

    if( !epts_bb || SBA[dest] != epts(p) ||
            (mover != WHITE_PAWN && mover != BLACK_PAWN) )
        return false;

    if( ( mover == WHITE_PAWN &&
            ( (orig == 32 && dest == 41) || (orig == 39 && dest == 46) ) ) ||
        ( mover == BLACK_PAWN &&
            ( (orig == 24 && dest == 17) || (orig == 31 && dest == 22) ) )
    ) return true;

    if( ( mover == WHITE_PAWN && (dest == orig + 7 || dest == orig + 9) ) ||
            ( mover == BLACK_PAWN && (dest == orig - 7 || dest == orig - 9) )
    ) return true;

    return false;
}

#undef INIT_VARS

// Returns true iff 'move' is a castling move (either O-O or O-O-O)
// in position 'p'.
bool
is_castle( const Pos *p, Rawcode move )
{
    assert( !( is_short_castle(p, move) && is_long_castle(p, move) ) );
    return is_short_castle(p, move) || is_long_castle(p, move);
}

// Returns true iff the king of the active color is in check in position 'p'.
bool
king_in_check( const Pos *p )
{
    Pos pos;
    copy_pos(p, &pos);
    toggle_turn(&pos);

    return king_can_be_captured(&pos);
}

// Returns true iff the position 'p' is a checkmate.
bool
checkmate( const Pos *p )
{
    if(!king_in_check(p)) return false;

    Rawcode *codes = rawcodes(p);
    bool moves_available = codes[0];
    free(codes);

    return !moves_available;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

static void
x_kerc_zero_one_or_two_sqs_in_dir( const Bitboard sq_bit,
    int *num_of_sqs_north, int *num_of_sqs_east,
    int *num_of_sqs_south, int *num_of_sqs_west )
{
    const char *sq = sq_bit_to_sq_name( sq_bit );

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
    const char *upper_left_sq_name = sq_bit_to_sq_name( upper_left ),
        *lower_right_sq_name = sq_bit_to_sq_name( lower_right );
    char tmp_sq_name[ 3 ] = { 0 };

    tmp_sq_name[ 0 ] = find_upper_right ?
            lower_right_sq_name[ 0 ] : upper_left_sq_name[ 0 ],
        tmp_sq_name[ 1 ] = find_upper_right ?
            upper_left_sq_name[ 1 ] : lower_right_sq_name[ 1 ];

    return SBA[ sq_name_index( tmp_sq_name ) ];
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

static void
x_attackers_init_attacker_type(
    bool *attacker_type, va_list arg_ptr, int num_arg )
{
    for( ; num_arg; num_arg-- ) {
        Chessman cm = va_arg( arg_ptr, Chessman );
        assert( cm >= 1 && cm <= 12 );
        attacker_type[cm] = true;
    }

    for( int i = 0; i < 13; i++ )
        assert( attacker_type[i] == false || attacker_type[i] == true );
}

static void
x_attackers_find( Bitboard *the_attackers, bool *attacker_type,
    const Bitboard *ppa, Bitboard sq )
{
    if( attacker_type[WHITE_KING] )
        *the_attackers |= x_attackers_kings( ppa, sq, true );
    if( attacker_type[WHITE_QUEEN] )
        *the_attackers |= x_attackers_rooks_or_queens(
            ppa, sq, true, true ),
        *the_attackers |= x_attackers_bishops_or_queens(
            ppa, sq, true, true );
    if( attacker_type[WHITE_ROOK] )
        *the_attackers |= x_attackers_rooks_or_queens(
            ppa, sq, true, false );
    if( attacker_type[WHITE_BISHOP] )
        *the_attackers |= x_attackers_bishops_or_queens(
            ppa, sq, true, false );
    if( attacker_type[WHITE_KNIGHT] )
        *the_attackers |= x_attackers_knights( ppa, sq, true );
    if( attacker_type[WHITE_PAWN] )
        *the_attackers |= x_attackers_pawns( ppa, sq, true );
    if( attacker_type[BLACK_KING] )
        *the_attackers |= x_attackers_kings( ppa, sq, false );
    if( attacker_type[BLACK_QUEEN] )
        *the_attackers |= x_attackers_rooks_or_queens(
            ppa, sq, false, true ),
        *the_attackers |= x_attackers_bishops_or_queens(
            ppa, sq, false, true );
    if( attacker_type[BLACK_ROOK] )
        *the_attackers |= x_attackers_rooks_or_queens(
            ppa, sq, false, false );
    if( attacker_type[BLACK_BISHOP] )
        *the_attackers |= x_attackers_bishops_or_queens(
            ppa, sq, false, false );
    if( attacker_type[BLACK_KNIGHT] )
        *the_attackers |= x_attackers_knights( ppa, sq, false );
    if( attacker_type[BLACK_PAWN] )
        *the_attackers |= x_attackers_pawns( ppa, sq, false );
}

static Bitboard
x_attackers_kings( const Bitboard *ppa, Bitboard sq, bool color_is_white )
{
    Bitboard king = ppa[color_is_white ? WHITE_KING : BLACK_KING];
    if( king & KING_SQS[sq_bit_index(sq)] )
        return king;

    return 0;
}

#define CHESSMEN_OF_INTEREST_SELECTOR( \
        chessmen_of_interest, lesser_chessman_type ) \
    if( cm_is_queen ) chessmen_of_interest = \
        ppa[color_is_white ? WHITE_QUEEN : BLACK_QUEEN]; \
    else chessmen_of_interest = \
        ppa[color_is_white ? \
            WHITE_ ## lesser_chessman_type : BLACK_ ## lesser_chessman_type];

#define FOUR_DIRS_FOR_LOOP( first_dir, fourth_dir, chessmen_of_interest ) \
    for( enum sq_dir dir = first_dir; dir <= fourth_dir; dir += 2 ) { \
        Bitboard sq_in_dir = sq; \
        while( ( sq_in_dir = sq_nav( sq_in_dir, dir ) ) ) { \
            if( sq_in_dir & chessmen_of_interest ) { \
                the_attackers |= sq_in_dir; \
                break; \
            } \
            else if( !( sq_in_dir & ppa[EMPTY_SQUARE] ) ) \
                break; \
        } \
    }

static Bitboard
x_attackers_rooks_or_queens(
    const Bitboard *ppa, Bitboard sq, bool color_is_white, bool cm_is_queen )
{
    Bitboard rooks_or_queens, the_attackers = 0;

    CHESSMEN_OF_INTEREST_SELECTOR( rooks_or_queens, ROOK )
    FOUR_DIRS_FOR_LOOP( NORTH, WEST, rooks_or_queens )

    return the_attackers;
}

static Bitboard
x_attackers_bishops_or_queens(
    const Bitboard *ppa, Bitboard sq, bool color_is_white, bool cm_is_queen )
{
    Bitboard bishops_or_queens, the_attackers = 0;

    CHESSMEN_OF_INTEREST_SELECTOR( bishops_or_queens, BISHOP )
    FOUR_DIRS_FOR_LOOP( NORTHEAST, NORTHWEST, bishops_or_queens )

    return the_attackers;
}

#undef CHESSMEN_OF_INTEREST_SELECTOR
#undef FOUR_DIRS_FOR_LOOP

static Bitboard
x_attackers_knights( const Bitboard *ppa, Bitboard sq, bool color_is_white )
{
    return ppa[color_is_white ? WHITE_KNIGHT : BLACK_KNIGHT]
        & KNIGHT_SQS[sq_bit_index(sq)];
}

static Bitboard
x_attackers_pawns( const Bitboard *ppa, Bitboard sq, bool color_is_white )
{
    return ppa[color_is_white ? WHITE_PAWN : BLACK_PAWN] &
        ( sq_nav( sq, color_is_white ? SOUTHEAST : NORTHEAST ) |
        sq_nav( sq, color_is_white ? SOUTHWEST : NORTHWEST ) );
}

static bool
x_castle_valid_castle_type( const char *castle_type )
{
    return castle_type && (
        !strcmp( castle_type, "kingside" ) || !strcmp( castle_type, "h-side" ) ||
        !strcmp( castle_type, "queenside" ) || !strcmp( castle_type, "a-side" ) );
}

static void
x_castle_set_castling_king_and_rook( const Pos *p, bool kingside,
    Bitboard *castling_king, Bitboard *castling_rook )
{
    *castling_king = p->ppa[whites_turn(p) ? WHITE_KING : BLACK_KING];
    *castling_rook = p->irp[kingside ? 1 : 0];
    if( !whites_turn(p) ) *castling_rook <<= 56;
}

static bool
x_castle_king_in_check( const Pos *p )
{
    return
        (  whites_turn(p) && black_attackers(p->ppa, p->ppa[WHITE_KING]) ) ||
        ( !whites_turn(p) && white_attackers(p->ppa, p->ppa[BLACK_KING]) );
}

#define INIT_VARS \
    Bitboard kings_dest = (kingside ? SB.g1 : SB.c1); \
    if( !whites_turn(p) ) kings_dest <<= 56; \
    Bitboard sqs_in_between = in_between( castling_king, kings_dest ); \
    assert( num_of_sqs_in_sq_set(sqs_in_between) >= 0 && \
        num_of_sqs_in_sq_set(sqs_in_between) <= 4 );

static bool
x_castle_kings_path_blocked( const Pos *p, bool kingside,
    Bitboard castling_king, Bitboard castling_rook )
{
    INIT_VARS

    return
        ( castling_king != kings_dest ) &&
        (
            ( sqs_in_between | kings_dest ) &
            ( ( ss_white_army(p) | ss_black_army(p) ) ^ castling_rook )
        );
}

static bool
x_castle_kings_exclusive_path_in_check( const Pos *p, bool kingside,
    Bitboard castling_king )
{
    INIT_VARS

    Bitboard bit = castling_king;

    // Examine each of the squares between the origin and destination
    // squares of the king
    while(true) {
        if( kingside ) bit <<= 1;
        else bit >>= 1;
        if( !(bit & sqs_in_between) ) break;

        if( ( whites_turn(p) && black_attackers(p->ppa, bit) ) ||
                ( !whites_turn(p) && white_attackers(p->ppa, bit) ) )
            return true; // Square under attack by enemy chessmen
    }

    return false;
}

#undef INIT_VARS

#define INIT_VARS \
    Bitboard rooks_dest = (kingside ? SB.f1 : SB.d1); \
    if( !whites_turn(p) ) rooks_dest <<= 56; \
    Bitboard sqs_in_between = in_between( castling_rook, rooks_dest ); \
    assert( num_of_sqs_in_sq_set(sqs_in_between) >= 0 && \
        num_of_sqs_in_sq_set(sqs_in_between) <= 2 );

static bool
x_castle_rooks_path_blocked( const Pos *p, bool kingside,
    Bitboard castling_rook, Bitboard castling_king )
{
    INIT_VARS

    return
        ( castling_rook != rooks_dest ) &&
        (
            ( sqs_in_between | rooks_dest ) &
            ( ( ss_white_army(p) | ss_black_army(p) ) ^ castling_king )
        );
}

#undef INIT_VARS

static Rawcode
x_castle_rawcode( Bitboard castling_king, Bitboard castling_rook )
{
    const char *orig_sq = SNA[sq_bit_index(castling_king)],
        *dest_sq = SNA[sq_bit_index(castling_rook)];
    char rawmove[4 + 1] = {0};

    rawmove[0] = orig_sq[0], rawmove[1] = orig_sq[1];
    rawmove[2] = dest_sq[0], rawmove[3] = dest_sq[1];

    return rawcode(rawmove);
}

#define ASSERT_THAT_MOVE_IS_VALID \
    assert( str_m_pat( move, "^[a-h][1-8][a-h][1-8]$" ) );

static void
x_rawcodes_king_and_knight( const Pos *p, int mover, Rawcode *pseudo,
    int *vacant, bool king )
{
    char move[4 + 1] = {0};
    move[0] = SNA[mover][0], move[1] = SNA[mover][1];

    Bitboard friendly_cm =
        whites_turn(p) ? ss_white_army(p) : ss_black_army(p);

    for( enum sq_dir d = king ? NORTH : ONE_OCLOCK;
        d <= ( king ? NORTHWEST : ELEVEN_OCLOCK ); d++ )
    {
        Bitboard bit = SBA[mover];
        if( (bit = sq_nav(bit, d)) && !(bit & friendly_cm) ) {
            move[2] = SNA[sq_bit_index(bit)][0];
            move[3] = SNA[sq_bit_index(bit)][1];
            ASSERT_THAT_MOVE_IS_VALID

            pseudo[(*vacant)++] = rawcode(move);
        }
    }
}

static void
x_rawcodes_rook_and_bishop( const Pos *p, int mover, Rawcode *pseudo,
    int *vacant, bool rook )
{
    char move[4 + 1] = {0};
    move[0] = SNA[mover][0], move[1] = SNA[mover][1];

    Bitboard
        friendly_cm =
            whites_turn(p) ? ss_white_army(p) : ss_black_army(p),
        enemy_cm =
            whites_turn(p) ? ss_black_army(p) : ss_white_army(p);

    for( enum sq_dir d = rook ? NORTH : NORTHEAST;
        d <= ( rook ? WEST : NORTHWEST ); d += 2 )
    {
        Bitboard bit = SBA[mover];
        while( (bit = sq_nav(bit, d)) && !(bit & friendly_cm) ) {
            move[2] = SNA[sq_bit_index(bit)][0];
            move[3] = SNA[sq_bit_index(bit)][1];
            ASSERT_THAT_MOVE_IS_VALID

            pseudo[(*vacant)++] = rawcode(move);
            if( bit & enemy_cm ) break;
        }
    }
}

static void
x_rawcodes_pawn_advance( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant )
{
    Bitboard sq_in_front =
        sq_nav( SBA[mover], whites_turn(p) ? NORTH : SOUTH );
    assert( sq_in_front );
    if( !(sq_in_front & p->ppa[EMPTY_SQUARE]) ) return;

    char move[4 + 1] = {0};
    move[0] = SNA[mover][0], move[1] = SNA[mover][1];
    move[2] = SNA[sq_bit_index(sq_in_front)][0];
    move[3] = SNA[sq_bit_index(sq_in_front)][1];
    ASSERT_THAT_MOVE_IS_VALID

    pseudo[(*vacant)++] = rawcode(move);

    sq_in_front = sq_nav( sq_in_front, whites_turn(p) ? NORTH : SOUTH );
    if( !(sq_in_front & p->ppa[EMPTY_SQUARE]) ) return;
    if( !(whites_turn(p) && (rank('2') & SBA[mover])) &&
        !(!whites_turn(p) && (rank('7') & SBA[mover])) ) return;
    move[0] = SNA[mover][0], move[1] = SNA[mover][1];
    move[2] = SNA[sq_bit_index(sq_in_front)][0];
    move[3] = SNA[sq_bit_index(sq_in_front)][1];
    ASSERT_THAT_MOVE_IS_VALID

    pseudo[(*vacant)++] = rawcode(move);
}

static void
x_rawcodes_pawn_capture( const Pos *p, int mover,
    Rawcode *pseudo, int *vacant )
{
    Bitboard
        w_capture_sq = // "w" for "western"
            sq_nav( SBA[mover], whites_turn(p) ? NORTHWEST : SOUTHWEST ),
        e_capture_sq = // "e" for "eastern"
            sq_nav( SBA[mover], whites_turn(p) ? NORTHEAST : SOUTHEAST ),
        enemy_cm =
            whites_turn(p) ? ss_black_army(p) : ss_white_army(p);
    assert( w_capture_sq || e_capture_sq );

    if( !( (w_capture_sq | e_capture_sq) & enemy_cm) ) return;

    char move[4 + 1] = {0};
    move[0] = SNA[mover][0], move[1] = SNA[mover][1];

    // Copy-paste coding
    if( w_capture_sq & enemy_cm ) {
        move[2] = SNA[sq_bit_index(w_capture_sq)][0];
        move[3] = SNA[sq_bit_index(w_capture_sq)][1];
        ASSERT_THAT_MOVE_IS_VALID
        pseudo[(*vacant)++] = rawcode(move);
    }
    if( e_capture_sq & enemy_cm ) {
        move[2] = SNA[sq_bit_index(e_capture_sq)][0];
        move[3] = SNA[sq_bit_index(e_capture_sq)][1];
        ASSERT_THAT_MOVE_IS_VALID
        pseudo[(*vacant)++] = rawcode(move);
    }
}

static void
x_rawcodes_en_passant( const Pos *p, int mover, Rawcode *pseudo, int *vacant )
{
    if( !epts(p) ) return;

    Chessman cm = occupant_of_sq(p, SBA[mover]);
    if( cm != WHITE_PAWN && cm != BLACK_PAWN ) return;

    if( epts(p) == sq_nav(SBA[mover], whites_turn(p) ? NORTHWEST : SOUTHWEST) ||
        epts(p) == sq_nav(SBA[mover], whites_turn(p) ? NORTHEAST : SOUTHEAST)
    ) {
        char move[4 + 1] = {0};
        move[0] = SNA[mover][0], move[1] = SNA[mover][1],
            move[2] = SNA[sq_bit_index(epts(p))][0],
            move[3] = SNA[sq_bit_index(epts(p))][1];
        ASSERT_THAT_MOVE_IS_VALID
        pseudo[(*vacant)++] = rawcode(move);
    }
}

#undef ASSERT_THAT_MOVE_IS_VALID

static int
x_qsort_rawcode_compare( const void *a, const void *b )
{
    Rawcode x = *( (Rawcode *) a ), y = *( (Rawcode *) b );

    if( x < y ) return -1;
    else if( x > y ) return 1;
    else return 0;
}

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "move_gen.h"
#include "utils.h"
#include "extra.h"

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
static Bitboard x_find_ac_army( const Pos *p );
static uint16_t *x_che_move_gen_move_list_for_cm(
    Bitboard origin_sq, Bitboard dest_sqs );
static Bitboard x_dest_sqs_king( const Pos *p );
// static void x_cm_attacking_sq_debug_print_attackers( int attackers[] );
static void x_cm_attacking_sq_prepare_attackers(
    int attackers[], va_list arg_ptr, int num_arg );
static Bitboard x_cm_attacking_sq_kings( const Pos *p, Bitboard sq,
    bool color_is_white );
static Bitboard x_cm_attacking_sq_rooks_or_queens( const Pos *p,
    Bitboard sq, bool color_is_white, bool cm_is_queen );
static Bitboard x_cm_attacking_sq_bishops_or_queens( const Pos *p,
    Bitboard sq, bool color_is_white, bool cm_is_queen );
static Bitboard x_cm_attacking_sq_knights( const Pos *p, Bitboard sq,
    bool color_is_white );
static Bitboard x_cm_attacking_sq_pawns( const Pos *p, Bitboard sq,
    bool color_is_white );
static bool x_castling_move_status_valid_castle_type( const char *castle_type );
static Bitboard x_castling_move_status_castling_king( const Pos *p );
static Bitboard x_castling_move_status_castling_rook( const Pos *p, bool kingside );
static bool x_castling_move_status_ca_bit_set( const Pos *p, bool kingside );
static bool x_castling_move_status_kings_path_cleared( const Pos *p, bool kingside );
static bool x_castling_move_status_rooks_path_cleared( const Pos *p, bool kingside );
static bool x_castling_move_status_kings_path_in_check( const Pos *p, bool kingside );

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

/*************************************
 **** Chester interface functions ****
 *************************************/

// ...
int
che_move_gen( const char *fen, uint16_t ***moves, int *num_mov_cm )
{
    const Pos *p = (const Pos *) fen_to_pos( fen );

    Bitboard ac_army = x_find_ac_army( p ); // TODO: Make reusable
    *num_mov_cm = num_of_sqs_in_sq_set( ac_army );

    uint16_t **ptr_array = (uint16_t **) malloc(
        ( *num_mov_cm ) * sizeof(uint16_t *) );
    assert( ptr_array );

    for( int bi = 0, i = 0; bi < 64; bi++ ) { // bi, bit index
        Bitboard cur_sq_bit = SBA[ bi ];
        if( ac_army & cur_sq_bit ) {
            ptr_array[ i ] = x_che_move_gen_move_list_for_cm(
                cur_sq_bit, dest_sqs( p, cur_sq_bit ) );
            ++i;
        }
    }

    free( (Pos *) p );

    *moves = ptr_array;
    return 0;
}

/****************************
 **** External functions ****
 ****************************/

// Checks whether a king can be captured in the given position. Note that
// being able to capture the enemy king is different from having it in
// check. The difference is in the active color.
bool
king_can_be_captured( const Pos *p )
{
    return whites_turn( p ) ?
        white_cm_attacking_sq( p, p->ppa[ BLACK_KING ] ) :
        black_cm_attacking_sq( p, p->ppa[ WHITE_KING ] );
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
    assert( bb_is_sq_bit( sq_bit ) );

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

// Returns the destination squares of the origin square parameter in the
// context of the position represented by 'p'. Another way of saying this
// is that dest_sqs() finds the moves for the chessman on the square
// indicated by 'origin_sq'. For example, if 'p' is the standard starting
// position and 'origin_sq' is 0x40U (square g1), then dest_sqs() returns
// the value 0xa00000U (squares f3 and h3).
Bitboard
dest_sqs( const Pos *p, Bitboard origin_sq )
{
    assert( bb_is_sq_bit( origin_sq ) );

    Bitboard dest_sqs = 0;
    Chessman cm = occupant_of_sq( p, sq_bit_to_sq_name( origin_sq ) );

    if( cm == WHITE_KING || cm == BLACK_KING )
        dest_sqs = x_dest_sqs_king( p );

    return dest_sqs;
}

// Returns a bitboard of the chessmen attacking square 'sq'. The variable
// part of the parameter list should consist of one or more Chessman enum
// constants. This list of chessmen identify the attackers. For example,
// the following call returns all the white and black pawns attacking
// square e4: cm_attacking_sq( p, SB.e4, 2, WHITE_PAWN, BLACK_PAWN )
Bitboard
cm_attacking_sq( const Pos *p, Bitboard sq, int num_arg, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, num_arg );

    int attackers[ 13 ] = { 0 };
    x_cm_attacking_sq_prepare_attackers( attackers, arg_ptr, num_arg );
    // x_cm_attacking_sq_debug_print_attackers( attackers );

    Bitboard attackers_bb = 0;

    if( attackers[ WHITE_KING ] )
        attackers_bb |= x_cm_attacking_sq_kings( p, sq, true );
    if( attackers[ WHITE_QUEEN ] ) {
        attackers_bb |= x_cm_attacking_sq_rooks_or_queens( p, sq, true, true );
        attackers_bb |= x_cm_attacking_sq_bishops_or_queens( p, sq, true, true ); }
    if( attackers[ WHITE_ROOK ] )
        attackers_bb |= x_cm_attacking_sq_rooks_or_queens( p, sq, true, false );
    if( attackers[ WHITE_BISHOP ] )
        attackers_bb |= x_cm_attacking_sq_bishops_or_queens( p, sq, true, false );
    if( attackers[ WHITE_KNIGHT ] )
        attackers_bb |= x_cm_attacking_sq_knights( p, sq, true );
    if( attackers[ WHITE_PAWN ] )
        attackers_bb |= x_cm_attacking_sq_pawns( p, sq, true );
    if( attackers[ BLACK_KING ] )
        attackers_bb |= x_cm_attacking_sq_kings( p, sq, false );
    if( attackers[ BLACK_QUEEN ] ) {
        attackers_bb |= x_cm_attacking_sq_rooks_or_queens( p, sq, false, true );
        attackers_bb |= x_cm_attacking_sq_bishops_or_queens( p, sq, false, true ); }
    if( attackers[ BLACK_ROOK ] )
        attackers_bb |= x_cm_attacking_sq_rooks_or_queens( p, sq, false, false );
    if( attackers[ BLACK_BISHOP ] )
        attackers_bb |= x_cm_attacking_sq_bishops_or_queens( p, sq, false, false );
    if( attackers[ BLACK_KNIGHT ] )
        attackers_bb |= x_cm_attacking_sq_knights( p, sq, false );
    if( attackers[ BLACK_PAWN ] )
        attackers_bb |= x_cm_attacking_sq_pawns( p, sq, false );

    va_end( arg_ptr );
    return attackers_bb;
}

// A convenience function for cm_attacking_sq(). Returns a bitboard of all the
// white chessmen attacking square 'sq'.
Bitboard
white_cm_attacking_sq( const Pos *p, Bitboard sq )
{
    return cm_attacking_sq( p, sq, 6, WHITE_KING, WHITE_QUEEN, WHITE_ROOK,
        WHITE_BISHOP, WHITE_KNIGHT, WHITE_PAWN );
}

// A convenience function for cm_attacking_sq(). Returns a bitboard of all the
// black chessmen attacking square 'sq'.
Bitboard
black_cm_attacking_sq( const Pos *p, Bitboard sq )
{
    return cm_attacking_sq( p, sq, 6, BLACK_KING, BLACK_QUEEN, BLACK_ROOK,
        BLACK_BISHOP, BLACK_KNIGHT, BLACK_PAWN );
}

// TODO: ...
enum cms
castling_move_status( const Pos *p, const char *castle_type )
{
    assert( x_castling_move_status_valid_castle_type( castle_type ) );

    bool kingside = ( !strcmp( castle_type, "kingside" ) ||
        !strcmp( castle_type, "h_side" ) );

    if( !x_castling_move_status_ca_bit_set( p, kingside ) )
        return CMS_CA_BIT_UNSET;
    if( !x_castling_move_status_kings_path_cleared( p, kingside ) )
        return CMS_KINGS_PATH_BLOCKED;
    if( !x_castling_move_status_rooks_path_cleared( p, kingside ) )
        return CMS_ROOKS_PATH_BLOCKED;
    if( x_castling_move_status_kings_path_in_check( p, kingside ) )
        return CMS_KINGS_PATH_IN_CHECK;
    if( false )
        return CMS_CASTLED_KING_IN_CHECK;

    return CMS_AVAILABLE;
}

/**************************
 **** Static functions ****
 **************************/

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

static Bitboard
x_find_ac_army( const Pos *p ) // ac, active color
{
    Bitboard ac_army = 0u;

    for( int i = 0; i < 64; i++ ) {
        for( Chessman cm = whites_turn( p ) ? WHITE_KING : BLACK_KING;
            ( whites_turn( p ) && cm <= WHITE_PAWN ) ||
                ( !whites_turn( p ) && cm <= BLACK_PAWN ); cm++ ) {
            if( p->ppa[ cm ] & SBA[ i ] ) {
                ac_army |= SBA[ i ];
                break;
            }
        }
    }

    return ac_army;
}

static uint16_t *
x_che_move_gen_move_list_for_cm( Bitboard origin_sq, Bitboard dest_sqs )
{
    uint16_t num_dest_sqs = (uint16_t) num_of_sqs_in_sq_set( dest_sqs );
    uint16_t *move_list = (uint16_t *) malloc(
        ( num_dest_sqs + 2 ) * sizeof(uint16_t) );

    move_list[ 0 ] = sq_bit_index( origin_sq );
    move_list[ 1 ] = num_dest_sqs;

    for( int i = 2; i < num_dest_sqs + 2; i++ ) {
        Bitboard cur_dest_sq = next_sq_of_ss( &dest_sqs );
        move_list[ i ] = sq_bit_index( cur_dest_sq );
    }
    assert( !dest_sqs );

    return move_list;
}

static Bitboard
x_dest_sqs_king( const Pos *p )
{
    Bitboard dest_sqs = 0;
    int king_bi = sq_bit_index(
        p->ppa[ whites_turn( p ) ? WHITE_KING : BLACK_KING ] );

    dest_sqs = KING_SQS[ king_bi ];

    // printf( "%s(): About to return %lx\n", __func__, dest_sqs );
    return dest_sqs;
}

/*
static void
x_cm_attacking_sq_debug_print_attackers( int attackers[] )
{
    printf( "%s():\n", __func__ );
    for( int i = 0; i < 13; i++ ) {
        printf( "\tattackers[ %2d ]: %d\n", i, attackers[ i ] );
    }
}
*/

static void
x_cm_attacking_sq_prepare_attackers(
    int attackers[], va_list arg_ptr, int num_arg )
{
    for( ; num_arg; num_arg-- ) {
        Chessman cm = va_arg( arg_ptr, Chessman );
        assert( cm >= 1 && cm <= 12 );
        ++attackers[ cm ];
    }

    for( int i = 0; i < 13; i++ )
        assert( attackers[ i ] == 0 || attackers[ i ] == 1 );
}

static Bitboard
x_cm_attacking_sq_kings( const Pos *p, Bitboard sq, bool color_is_white )
{
    Bitboard king = p->ppa[ color_is_white ? WHITE_KING : BLACK_KING ];
    if( king & KING_SQS[ sq_bit_index( sq ) ] )
        return king;

    return 0;
}

#define CHESSMEN_OF_INTEREST_SELECTOR( chessmen_of_interest, lesser_chessman_type ) \
if( cm_is_queen ) chessmen_of_interest = \
    p->ppa[ color_is_white ? WHITE_QUEEN : BLACK_QUEEN ]; \
else chessmen_of_interest = \
    p->ppa[ color_is_white ? \
        WHITE_ ## lesser_chessman_type : BLACK_ ## lesser_chessman_type ];

#define FOUR_DIRS_FOR_LOOP( first_dir, fourth_dir, chessmen_of_interest ) \
for( enum sq_dir dir = first_dir; dir <= fourth_dir; dir += 2 ) { \
    Bitboard sq_in_dir = sq; \
    while( ( sq_in_dir = sq_nav( sq_in_dir, dir ) ) ) { \
        if( sq_in_dir & chessmen_of_interest ) { \
            attackers |= sq_in_dir; \
            break; \
        } \
        else if( !( sq_in_dir & p->ppa[ EMPTY_SQUARE ] ) ) \
            break; \
    } \
}

static Bitboard
x_cm_attacking_sq_rooks_or_queens( const Pos *p, Bitboard sq,
    bool color_is_white, bool cm_is_queen )
{
    Bitboard rooks_or_queens, attackers = 0;

    CHESSMEN_OF_INTEREST_SELECTOR( rooks_or_queens, ROOK )
    FOUR_DIRS_FOR_LOOP( NORTH, WEST, rooks_or_queens )

    return attackers;
}

static Bitboard
x_cm_attacking_sq_bishops_or_queens( const Pos *p, Bitboard sq,
    bool color_is_white, bool cm_is_queen )
{
    Bitboard bishops_or_queens, attackers = 0;

    CHESSMEN_OF_INTEREST_SELECTOR( bishops_or_queens, BISHOP )
    FOUR_DIRS_FOR_LOOP( NORTHEAST, NORTHWEST, bishops_or_queens )

    return attackers;
}

#undef CHESSMEN_OF_INTEREST_SELECTOR
#undef FOUR_DIRS_FOR_LOOP

static Bitboard
x_cm_attacking_sq_knights( const Pos *p, Bitboard sq, bool color_is_white )
{
    return p->ppa[ color_is_white ? WHITE_KNIGHT : BLACK_KNIGHT ]
        & KNIGHT_SQS[ sq_bit_index( sq ) ];
}

static Bitboard
x_cm_attacking_sq_pawns( const Pos *p, Bitboard sq, bool color_is_white )
{
    return p->ppa[ color_is_white ? WHITE_PAWN : BLACK_PAWN ] &
        ( sq_nav( sq, color_is_white ? SOUTHEAST : NORTHEAST ) |
        sq_nav( sq, color_is_white ? SOUTHWEST : NORTHWEST ) );
}

static bool
x_castling_move_status_valid_castle_type( const char *castle_type )
{
    return castle_type && (
        !strcmp( castle_type, "kingside" ) || !strcmp( castle_type, "h-side" ) ||
        !strcmp( castle_type, "queenside" ) || !strcmp( castle_type, "a-side" ) );
}

static Bitboard
x_castling_move_status_castling_king( const Pos *p )
{
    return whites_turn( p ) ? p->ppa[ WHITE_KING ] : p->ppa[ BLACK_KING ];
}

static Bitboard
x_castling_move_status_castling_rook( const Pos *p, bool kingside )
{
    uint64_t irpf = ( p->irp[0] | p->irp[1] ), file_of_queenside_rook = 1,
        file_of_kingside_rook;
    while( !( irpf & file_of_queenside_rook ) )
        file_of_queenside_rook <<= 1;
    file_of_kingside_rook = irpf ^ file_of_queenside_rook;

    assert( bb_is_sq_bit( file_of_queenside_rook ) && file_of_queenside_rook >= 1 &&
        file_of_queenside_rook <= 32 );
    assert( bb_is_sq_bit( file_of_kingside_rook ) && file_of_kingside_rook >= 4 &&
        file_of_kingside_rook <= 128 );

    Bitboard castling_rook = kingside ? file_of_kingside_rook : file_of_queenside_rook;
    if( !whites_turn( p ) )
        castling_rook <<= 56;

    return castling_rook;
}

static bool
x_castling_move_status_ca_bit_set( const Pos *p, bool kingside )
{
    bool wt = whites_turn( p );

    return ( wt && kingside && has_castling_right( p, "white", "kingside" ) ) ||
        ( wt && !kingside && has_castling_right( p, "white", "queenside" ) ) ||
        ( !wt && kingside && has_castling_right( p, "black", "kingside" ) ) ||
        ( !wt && !kingside && has_castling_right( p, "black", "queenside" ) );
}

#define INIT_CASTLING_CMEN_AND_TARGETS( king_or_rook, right_target_sq, left_target_sq ) \
Bitboard castling_king = x_castling_move_status_castling_king( p ), \
    castling_rook = x_castling_move_status_castling_rook( p, kingside ), \
    king_or_rook ## s_target_sq = kingside ? right_target_sq : left_target_sq; \
if( !whites_turn( p ) ) king_or_rook ## s_target_sq <<= 56;

#define EXAMINE_CASTLING_CMANS_PATH_TO_TARGET_SQ( king_or_rook, other_castling_cman ) \
while( castling_ ## king_or_rook != king_or_rook ## s_target_sq ) { \
    castling_ ## king_or_rook = \
        ( castling_ ## king_or_rook < king_or_rook ## s_target_sq ) ? \
        ( castling_ ## king_or_rook << 1 ) : \
        ( castling_ ## king_or_rook >> 1 ); \
    if( !( castling_ ## king_or_rook & p->ppa[ EMPTY_SQUARE ] ) && \
        !( castling_ ## king_or_rook & other_castling_cman ) ) return false; }

static bool
x_castling_move_status_kings_path_cleared( const Pos *p, bool kingside )
{
    INIT_CASTLING_CMEN_AND_TARGETS( king, SB.g1, SB.c1 )
    EXAMINE_CASTLING_CMANS_PATH_TO_TARGET_SQ( king, castling_rook )

    return true;
}

static bool
x_castling_move_status_rooks_path_cleared( const Pos *p, bool kingside )
{
    INIT_CASTLING_CMEN_AND_TARGETS( rook, SB.f1, SB.d1 )
    EXAMINE_CASTLING_CMANS_PATH_TO_TARGET_SQ( rook, castling_king )

    return true;
}

#undef INIT_CASTLING_CMEN_AND_TARGETS
#undef EXAMINE_CASTLING_CMANS_PATH_TO_TARGET_SQ

static bool
x_castling_move_status_kings_path_in_check( const Pos *p, bool kingside )
{
    bool w = whites_turn( p );
    Bitboard kings_origin_sq = x_castling_move_status_castling_king( p ),
        kings_dest_sq = kingside ? ( w ? SB.g1 : SB.g8 ) : ( w ? SB.c1 : SB.c8 );
    bool origin_on_the_left = kings_origin_sq < kings_dest_sq;
    Bitboard kings_path = sq_rectangle(
        origin_on_the_left ? kings_origin_sq : kings_dest_sq,
        origin_on_the_left ? kings_dest_sq : kings_origin_sq );

    for( int i = 0; i < 64; i++ ) {
        Bitboard sq = SBA[ i ];
        if( kings_path & sq && ( ( w && black_cm_attacking_sq( p, sq ) ) ||
                ( !w && white_cm_attacking_sq( p, sq ) ) ) )
            return true;
    }

    return false;
}

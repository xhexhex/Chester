#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "move_gen.h"
#include "utils.h"

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

#define ALL_POSSIBLE_MOVES \
	"a1b1", "a1c1", "a1d1", "a1e1", "a1f1", "a1g1", "a1h1", "a1a2", "a1b2", \
	"a1c2", "a1a3", "a1b3", "a1c3", "a1a4", "a1d4", "a1a5", "a1e5", "a1a6", \
	"a1f6", "a1a7", "a1g7", "a1a8", "a1h8", "b1a1", "b1c1", "b1d1", "b1e1", \
	"b1f1", "b1g1", "b1h1", "b1a2", "b1b2", "b1c2", "b1d2", "b1a3", "b1b3", \
	"b1c3", "b1d3", "b1b4", "b1e4", "b1b5", "b1f5", "b1b6", "b1g6", "b1b7", \
	"b1h7", "b1b8", "c1a1", "c1b1", "c1d1", "c1e1", "c1f1", "c1g1", "c1h1", \
	"c1a2", "c1b2", "c1c2", "c1d2", "c1e2", "c1a3", "c1b3", "c1c3", "c1d3", \
	"c1e3", "c1c4", "c1f4", "c1c5", "c1g5", "c1c6", "c1h6", "c1c7", "c1c8", \
	"d1a1", "d1b1", "d1c1", "d1e1", "d1f1", "d1g1", "d1h1", "d1b2", "d1c2", \
	"d1d2", "d1e2", "d1f2", "d1b3", "d1c3", "d1d3", "d1e3", "d1f3", "d1a4", \
	"d1d4", "d1g4", "d1d5", "d1h5", "d1d6", "d1d7", "d1d8", "e1a1", "e1b1", \
	"e1c1", "e1d1", "e1f1", "e1g1", "e1h1", "e1c2", "e1d2", "e1e2", "e1f2", \
	"e1g2", "e1c3", "e1d3", "e1e3", "e1f3", "e1g3", "e1b4", "e1e4", "e1h4", \
	"e1a5", "e1e5", "e1e6", "e1e7", "e1e8", "f1a1", "f1b1", "f1c1", "f1d1", \
	"f1e1", "f1g1", "f1h1", "f1d2", "f1e2", "f1f2", "f1g2", "f1h2", "f1d3", \
	"f1e3", "f1f3", "f1g3", "f1h3", "f1c4", "f1f4", "f1b5", "f1f5", "f1a6", \
	"f1f6", "f1f7", "f1f8", "g1a1", "g1b1", "g1c1", "g1d1", "g1e1", "g1f1", \
	"g1h1", "g1e2", "g1f2", "g1g2", "g1h2", "g1e3", "g1f3", "g1g3", "g1h3", \
	"g1d4", "g1g4", "g1c5", "g1g5", "g1b6", "g1g6", "g1a7", "g1g7", "g1g8", \
	"h1a1", "h1b1", "h1c1", "h1d1", "h1e1", "h1f1", "h1g1", "h1f2", "h1g2", \
	"h1h2", "h1f3", "h1g3", "h1h3", "h1e4", "h1h4", "h1d5", "h1h5", "h1c6", \
	"h1h6", "h1b7", "h1h7", "h1a8", "h1h8", "a2a1", "a2b1", "a2c1", "a2b2", \
	"a2c2", "a2d2", "a2e2", "a2f2", "a2g2", "a2h2", "a2a3", "a2b3", "a2c3", \
	"a2a4", "a2b4", "a2c4", "a2a5", "a2d5", "a2a6", "a2e6", "a2a7", "a2f7", \
	"a2a8", "a2g8", "b2a1", "b2b1", "b2c1", "b2d1", "b2a2", "b2c2", "b2d2", \
	"b2e2", "b2f2", "b2g2", "b2h2", "b2a3", "b2b3", "b2c3", "b2d3", "b2a4", \
	"b2b4", "b2c4", "b2d4", "b2b5", "b2e5", "b2b6", "b2f6", "b2b7", "b2g7", \
	"b2b8", "b2h8", "c2a1", "c2b1", "c2c1", "c2d1", "c2e1", "c2a2", "c2b2", \
	"c2d2", "c2e2", "c2f2", "c2g2", "c2h2", "c2a3", "c2b3", "c2c3", "c2d3", \
	"c2e3", "c2a4", "c2b4", "c2c4", "c2d4", "c2e4", "c2c5", "c2f5", "c2c6", \
	"c2g6", "c2c7", "c2h7", "c2c8", "d2b1", "d2c1", "d2d1", "d2e1", "d2f1", \
	"d2a2", "d2b2", "d2c2", "d2e2", "d2f2", "d2g2", "d2h2", "d2b3", "d2c3", \
	"d2d3", "d2e3", "d2f3", "d2b4", "d2c4", "d2d4", "d2e4", "d2f4", "d2a5", \
	"d2d5", "d2g5", "d2d6", "d2h6", "d2d7", "d2d8", "e2c1", "e2d1", "e2e1", \
	"e2f1", "e2g1", "e2a2", "e2b2", "e2c2", "e2d2", "e2f2", "e2g2", "e2h2", \
	"e2c3", "e2d3", "e2e3", "e2f3", "e2g3", "e2c4", "e2d4", "e2e4", "e2f4", \
	"e2g4", "e2b5", "e2e5", "e2h5", "e2a6", "e2e6", "e2e7", "e2e8", "f2d1", \
	"f2e1", "f2f1", "f2g1", "f2h1", "f2a2", "f2b2", "f2c2", "f2d2", "f2e2", \
	"f2g2", "f2h2", "f2d3", "f2e3", "f2f3", "f2g3", "f2h3", "f2d4", "f2e4", \
	"f2f4", "f2g4", "f2h4", "f2c5", "f2f5", "f2b6", "f2f6", "f2a7", "f2f7", \
	"f2f8", "g2e1", "g2f1", "g2g1", "g2h1", "g2a2", "g2b2", "g2c2", "g2d2", \
	"g2e2", "g2f2", "g2h2", "g2e3", "g2f3", "g2g3", "g2h3", "g2e4", "g2f4", \
	"g2g4", "g2h4", "g2d5", "g2g5", "g2c6", "g2g6", "g2b7", "g2g7", "g2a8", \
	"g2g8", "h2f1", "h2g1", "h2h1", "h2a2", "h2b2", "h2c2", "h2d2", "h2e2", \
	"h2f2", "h2g2", "h2f3", "h2g3", "h2h3", "h2f4", "h2g4", "h2h4", "h2e5", \
	"h2h5", "h2d6", "h2h6", "h2c7", "h2h7", "h2b8", "h2h8", "a3a1", "a3b1", \
	"a3c1", "a3a2", "a3b2", "a3c2", "a3b3", "a3c3", "a3d3", "a3e3", "a3f3", \
	"a3g3", "a3h3", "a3a4", "a3b4", "a3c4", "a3a5", "a3b5", "a3c5", "a3a6", \
	"a3d6", "a3a7", "a3e7", "a3a8", "a3f8", "b3a1", "b3b1", "b3c1", "b3d1", \
	"b3a2", "b3b2", "b3c2", "b3d2", "b3a3", "b3c3", "b3d3", "b3e3", "b3f3", \
	"b3g3", "b3h3", "b3a4", "b3b4", "b3c4", "b3d4", "b3a5", "b3b5", "b3c5", \
	"b3d5", "b3b6", "b3e6", "b3b7", "b3f7", "b3b8", "b3g8", "c3a1", "c3b1", \
	"c3c1", "c3d1", "c3e1", "c3a2", "c3b2", "c3c2", "c3d2", "c3e2", "c3a3", \
	"c3b3", "c3d3", "c3e3", "c3f3", "c3g3", "c3h3", "c3a4", "c3b4", "c3c4", \
	"c3d4", "c3e4", "c3a5", "c3b5", "c3c5", "c3d5", "c3e5", "c3c6", "c3f6", \
	"c3c7", "c3g7", "c3c8", "c3h8", "d3b1", "d3c1", "d3d1", "d3e1", "d3f1", \
	"d3b2", "d3c2", "d3d2", "d3e2", "d3f2", "d3a3", "d3b3", "d3c3", "d3e3", \
	"d3f3", "d3g3", "d3h3", "d3b4", "d3c4", "d3d4", "d3e4", "d3f4", "d3b5", \
	"d3c5", "d3d5", "d3e5", "d3f5", "d3a6", "d3d6", "d3g6", "d3d7", "d3h7", \
	"d3d8", "e3c1", "e3d1", "e3e1", "e3f1", "e3g1", "e3c2", "e3d2", "e3e2", \
	"e3f2", "e3g2", "e3a3", "e3b3", "e3c3", "e3d3", "e3f3", "e3g3", "e3h3", \
	"e3c4", "e3d4", "e3e4", "e3f4", "e3g4", "e3c5", "e3d5", "e3e5", "e3f5", \
	"e3g5", "e3b6", "e3e6", "e3h6", "e3a7", "e3e7", "e3e8", "f3d1", "f3e1", \
	"f3f1", "f3g1", "f3h1", "f3d2", "f3e2", "f3f2", "f3g2", "f3h2", "f3a3", \
	"f3b3", "f3c3", "f3d3", "f3e3", "f3g3", "f3h3", "f3d4", "f3e4", "f3f4", \
	"f3g4", "f3h4", "f3d5", "f3e5", "f3f5", "f3g5", "f3h5", "f3c6", "f3f6", \
	"f3b7", "f3f7", "f3a8", "f3f8", "g3e1", "g3f1", "g3g1", "g3h1", "g3e2", \
	"g3f2", "g3g2", "g3h2", "g3a3", "g3b3", "g3c3", "g3d3", "g3e3", "g3f3", \
	"g3h3", "g3e4", "g3f4", "g3g4", "g3h4", "g3e5", "g3f5", "g3g5", "g3h5", \
	"g3d6", "g3g6", "g3c7", "g3g7", "g3b8", "g3g8", "h3f1", "h3g1", "h3h1", \
	"h3f2", "h3g2", "h3h2", "h3a3", "h3b3", "h3c3", "h3d3", "h3e3", "h3f3", \
	"h3g3", "h3f4", "h3g4", "h3h4", "h3f5", "h3g5", "h3h5", "h3e6", "h3h6", \
	"h3d7", "h3h7", "h3c8", "h3h8", "a4a1", "a4d1", "a4a2", "a4b2", "a4c2", \
	"a4a3", "a4b3", "a4c3", "a4b4", "a4c4", "a4d4", "a4e4", "a4f4", "a4g4", \
	"a4h4", "a4a5", "a4b5", "a4c5", "a4a6", "a4b6", "a4c6", "a4a7", "a4d7", \
	"a4a8", "a4e8", "b4b1", "b4e1", "b4a2", "b4b2", "b4c2", "b4d2", "b4a3", \
	"b4b3", "b4c3", "b4d3", "b4a4", "b4c4", "b4d4", "b4e4", "b4f4", "b4g4", \
	"b4h4", "b4a5", "b4b5", "b4c5", "b4d5", "b4a6", "b4b6", "b4c6", "b4d6", \
	"b4b7", "b4e7", "b4b8", "b4f8", "c4c1", "c4f1", "c4a2", "c4b2", "c4c2", \
	"c4d2", "c4e2", "c4a3", "c4b3", "c4c3", "c4d3", "c4e3", "c4a4", "c4b4", \
	"c4d4", "c4e4", "c4f4", "c4g4", "c4h4", "c4a5", "c4b5", "c4c5", "c4d5", \
	"c4e5", "c4a6", "c4b6", "c4c6", "c4d6", "c4e6", "c4c7", "c4f7", "c4c8", \
	"c4g8", "d4a1", "d4d1", "d4g1", "d4b2", "d4c2", "d4d2", "d4e2", "d4f2", \
	"d4b3", "d4c3", "d4d3", "d4e3", "d4f3", "d4a4", "d4b4", "d4c4", "d4e4", \
	"d4f4", "d4g4", "d4h4", "d4b5", "d4c5", "d4d5", "d4e5", "d4f5", "d4b6", \
	"d4c6", "d4d6", "d4e6", "d4f6", "d4a7", "d4d7", "d4g7", "d4d8", "d4h8", \
	"e4b1", "e4e1", "e4h1", "e4c2", "e4d2", "e4e2", "e4f2", "e4g2", "e4c3", \
	"e4d3", "e4e3", "e4f3", "e4g3", "e4a4", "e4b4", "e4c4", "e4d4", "e4f4", \
	"e4g4", "e4h4", "e4c5", "e4d5", "e4e5", "e4f5", "e4g5", "e4c6", "e4d6", \
	"e4e6", "e4f6", "e4g6", "e4b7", "e4e7", "e4h7", "e4a8", "e4e8", "f4c1", \
	"f4f1", "f4d2", "f4e2", "f4f2", "f4g2", "f4h2", "f4d3", "f4e3", "f4f3", \
	"f4g3", "f4h3", "f4a4", "f4b4", "f4c4", "f4d4", "f4e4", "f4g4", "f4h4", \
	"f4d5", "f4e5", "f4f5", "f4g5", "f4h5", "f4d6", "f4e6", "f4f6", "f4g6", \
	"f4h6", "f4c7", "f4f7", "f4b8", "f4f8", "g4d1", "g4g1", "g4e2", "g4f2", \
	"g4g2", "g4h2", "g4e3", "g4f3", "g4g3", "g4h3", "g4a4", "g4b4", "g4c4", \
	"g4d4", "g4e4", "g4f4", "g4h4", "g4e5", "g4f5", "g4g5", "g4h5", "g4e6", \
	"g4f6", "g4g6", "g4h6", "g4d7", "g4g7", "g4c8", "g4g8", "h4e1", "h4h1", \
	"h4f2", "h4g2", "h4h2", "h4f3", "h4g3", "h4h3", "h4a4", "h4b4", "h4c4", \
	"h4d4", "h4e4", "h4f4", "h4g4", "h4f5", "h4g5", "h4h5", "h4f6", "h4g6", \
	"h4h6", "h4e7", "h4h7", "h4d8", "h4h8", "a5a1", "a5e1", "a5a2", "a5d2", \
	"a5a3", "a5b3", "a5c3", "a5a4", "a5b4", "a5c4", "a5b5", "a5c5", "a5d5", \
	"a5e5", "a5f5", "a5g5", "a5h5", "a5a6", "a5b6", "a5c6", "a5a7", "a5b7", \
	"a5c7", "a5a8", "a5d8", "b5b1", "b5f1", "b5b2", "b5e2", "b5a3", "b5b3", \
	"b5c3", "b5d3", "b5a4", "b5b4", "b5c4", "b5d4", "b5a5", "b5c5", "b5d5", \
	"b5e5", "b5f5", "b5g5", "b5h5", "b5a6", "b5b6", "b5c6", "b5d6", "b5a7", \
	"b5b7", "b5c7", "b5d7", "b5b8", "b5e8", "c5c1", "c5g1", "c5c2", "c5f2", \
	"c5a3", "c5b3", "c5c3", "c5d3", "c5e3", "c5a4", "c5b4", "c5c4", "c5d4", \
	"c5e4", "c5a5", "c5b5", "c5d5", "c5e5", "c5f5", "c5g5", "c5h5", "c5a6", \
	"c5b6", "c5c6", "c5d6", "c5e6", "c5a7", "c5b7", "c5c7", "c5d7", "c5e7", \
	"c5c8", "c5f8", "d5d1", "d5h1", "d5a2", "d5d2", "d5g2", "d5b3", "d5c3", \
	"d5d3", "d5e3", "d5f3", "d5b4", "d5c4", "d5d4", "d5e4", "d5f4", "d5a5", \
	"d5b5", "d5c5", "d5e5", "d5f5", "d5g5", "d5h5", "d5b6", "d5c6", "d5d6", \
	"d5e6", "d5f6", "d5b7", "d5c7", "d5d7", "d5e7", "d5f7", "d5a8", "d5d8", \
	"d5g8", "e5a1", "e5e1", "e5b2", "e5e2", "e5h2", "e5c3", "e5d3", "e5e3", \
	"e5f3", "e5g3", "e5c4", "e5d4", "e5e4", "e5f4", "e5g4", "e5a5", "e5b5", \
	"e5c5", "e5d5", "e5f5", "e5g5", "e5h5", "e5c6", "e5d6", "e5e6", "e5f6", \
	"e5g6", "e5c7", "e5d7", "e5e7", "e5f7", "e5g7", "e5b8", "e5e8", "e5h8", \
	"f5b1", "f5f1", "f5c2", "f5f2", "f5d3", "f5e3", "f5f3", "f5g3", "f5h3", \
	"f5d4", "f5e4", "f5f4", "f5g4", "f5h4", "f5a5", "f5b5", "f5c5", "f5d5", \
	"f5e5", "f5g5", "f5h5", "f5d6", "f5e6", "f5f6", "f5g6", "f5h6", "f5d7", \
	"f5e7", "f5f7", "f5g7", "f5h7", "f5c8", "f5f8", "g5c1", "g5g1", "g5d2", \
	"g5g2", "g5e3", "g5f3", "g5g3", "g5h3", "g5e4", "g5f4", "g5g4", "g5h4", \
	"g5a5", "g5b5", "g5c5", "g5d5", "g5e5", "g5f5", "g5h5", "g5e6", "g5f6", \
	"g5g6", "g5h6", "g5e7", "g5f7", "g5g7", "g5h7", "g5d8", "g5g8", "h5d1", \
	"h5h1", "h5e2", "h5h2", "h5f3", "h5g3", "h5h3", "h5f4", "h5g4", "h5h4", \
	"h5a5", "h5b5", "h5c5", "h5d5", "h5e5", "h5f5", "h5g5", "h5f6", "h5g6", \
	"h5h6", "h5f7", "h5g7", "h5h7", "h5e8", "h5h8", "a6a1", "a6f1", "a6a2", \
	"a6e2", "a6a3", "a6d3", "a6a4", "a6b4", "a6c4", "a6a5", "a6b5", "a6c5", \
	"a6b6", "a6c6", "a6d6", "a6e6", "a6f6", "a6g6", "a6h6", "a6a7", "a6b7", \
	"a6c7", "a6a8", "a6b8", "a6c8", "b6b1", "b6g1", "b6b2", "b6f2", "b6b3", \
	"b6e3", "b6a4", "b6b4", "b6c4", "b6d4", "b6a5", "b6b5", "b6c5", "b6d5", \
	"b6a6", "b6c6", "b6d6", "b6e6", "b6f6", "b6g6", "b6h6", "b6a7", "b6b7", \
	"b6c7", "b6d7", "b6a8", "b6b8", "b6c8", "b6d8", "c6c1", "c6h1", "c6c2", \
	"c6g2", "c6c3", "c6f3", "c6a4", "c6b4", "c6c4", "c6d4", "c6e4", "c6a5", \
	"c6b5", "c6c5", "c6d5", "c6e5", "c6a6", "c6b6", "c6d6", "c6e6", "c6f6", \
	"c6g6", "c6h6", "c6a7", "c6b7", "c6c7", "c6d7", "c6e7", "c6a8", "c6b8", \
	"c6c8", "c6d8", "c6e8", "d6d1", "d6d2", "d6h2", "d6a3", "d6d3", "d6g3", \
	"d6b4", "d6c4", "d6d4", "d6e4", "d6f4", "d6b5", "d6c5", "d6d5", "d6e5", \
	"d6f5", "d6a6", "d6b6", "d6c6", "d6e6", "d6f6", "d6g6", "d6h6", "d6b7", \
	"d6c7", "d6d7", "d6e7", "d6f7", "d6b8", "d6c8", "d6d8", "d6e8", "d6f8", \
	"e6e1", "e6a2", "e6e2", "e6b3", "e6e3", "e6h3", "e6c4", "e6d4", "e6e4", \
	"e6f4", "e6g4", "e6c5", "e6d5", "e6e5", "e6f5", "e6g5", "e6a6", "e6b6", \
	"e6c6", "e6d6", "e6f6", "e6g6", "e6h6", "e6c7", "e6d7", "e6e7", "e6f7", \
	"e6g7", "e6c8", "e6d8", "e6e8", "e6f8", "e6g8", "f6a1", "f6f1", "f6b2", \
	"f6f2", "f6c3", "f6f3", "f6d4", "f6e4", "f6f4", "f6g4", "f6h4", "f6d5", \
	"f6e5", "f6f5", "f6g5", "f6h5", "f6a6", "f6b6", "f6c6", "f6d6", "f6e6", \
	"f6g6", "f6h6", "f6d7", "f6e7", "f6f7", "f6g7", "f6h7", "f6d8", "f6e8", \
	"f6f8", "f6g8", "f6h8", "g6b1", "g6g1", "g6c2", "g6g2", "g6d3", "g6g3", \
	"g6e4", "g6f4", "g6g4", "g6h4", "g6e5", "g6f5", "g6g5", "g6h5", "g6a6", \
	"g6b6", "g6c6", "g6d6", "g6e6", "g6f6", "g6h6", "g6e7", "g6f7", "g6g7", \
	"g6h7", "g6e8", "g6f8", "g6g8", "g6h8", "h6c1", "h6h1", "h6d2", "h6h2", \
	"h6e3", "h6h3", "h6f4", "h6g4", "h6h4", "h6f5", "h6g5", "h6h5", "h6a6", \
	"h6b6", "h6c6", "h6d6", "h6e6", "h6f6", "h6g6", "h6f7", "h6g7", "h6h7", \
	"h6f8", "h6g8", "h6h8", "a7a1", "a7g1", "a7a2", "a7f2", "a7a3", "a7e3", \
	"a7a4", "a7d4", "a7a5", "a7b5", "a7c5", "a7a6", "a7b6", "a7c6", "a7b7", \
	"a7c7", "a7d7", "a7e7", "a7f7", "a7g7", "a7h7", "a7a8", "a7b8", "a7c8", \
	"b7b1", "b7h1", "b7b2", "b7g2", "b7b3", "b7f3", "b7b4", "b7e4", "b7a5", \
	"b7b5", "b7c5", "b7d5", "b7a6", "b7b6", "b7c6", "b7d6", "b7a7", "b7c7", \
	"b7d7", "b7e7", "b7f7", "b7g7", "b7h7", "b7a8", "b7b8", "b7c8", "b7d8", \
	"c7c1", "c7c2", "c7h2", "c7c3", "c7g3", "c7c4", "c7f4", "c7a5", "c7b5", \
	"c7c5", "c7d5", "c7e5", "c7a6", "c7b6", "c7c6", "c7d6", "c7e6", "c7a7", \
	"c7b7", "c7d7", "c7e7", "c7f7", "c7g7", "c7h7", "c7a8", "c7b8", "c7c8", \
	"c7d8", "c7e8", "d7d1", "d7d2", "d7d3", "d7h3", "d7a4", "d7d4", "d7g4", \
	"d7b5", "d7c5", "d7d5", "d7e5", "d7f5", "d7b6", "d7c6", "d7d6", "d7e6", \
	"d7f6", "d7a7", "d7b7", "d7c7", "d7e7", "d7f7", "d7g7", "d7h7", "d7b8", \
	"d7c8", "d7d8", "d7e8", "d7f8", "e7e1", "e7e2", "e7a3", "e7e3", "e7b4", \
	"e7e4", "e7h4", "e7c5", "e7d5", "e7e5", "e7f5", "e7g5", "e7c6", "e7d6", \
	"e7e6", "e7f6", "e7g6", "e7a7", "e7b7", "e7c7", "e7d7", "e7f7", "e7g7", \
	"e7h7", "e7c8", "e7d8", "e7e8", "e7f8", "e7g8", "f7f1", "f7a2", "f7f2", \
	"f7b3", "f7f3", "f7c4", "f7f4", "f7d5", "f7e5", "f7f5", "f7g5", "f7h5", \
	"f7d6", "f7e6", "f7f6", "f7g6", "f7h6", "f7a7", "f7b7", "f7c7", "f7d7", \
	"f7e7", "f7g7", "f7h7", "f7d8", "f7e8", "f7f8", "f7g8", "f7h8", "g7a1", \
	"g7g1", "g7b2", "g7g2", "g7c3", "g7g3", "g7d4", "g7g4", "g7e5", "g7f5", \
	"g7g5", "g7h5", "g7e6", "g7f6", "g7g6", "g7h6", "g7a7", "g7b7", "g7c7", \
	"g7d7", "g7e7", "g7f7", "g7h7", "g7e8", "g7f8", "g7g8", "g7h8", "h7b1", \
	"h7h1", "h7c2", "h7h2", "h7d3", "h7h3", "h7e4", "h7h4", "h7f5", "h7g5", \
	"h7h5", "h7f6", "h7g6", "h7h6", "h7a7", "h7b7", "h7c7", "h7d7", "h7e7", \
	"h7f7", "h7g7", "h7f8", "h7g8", "h7h8", "a8a1", "a8h1", "a8a2", "a8g2", \
	"a8a3", "a8f3", "a8a4", "a8e4", "a8a5", "a8d5", "a8a6", "a8b6", "a8c6", \
	"a8a7", "a8b7", "a8c7", "a8b8", "a8c8", "a8d8", "a8e8", "a8f8", "a8g8", \
	"a8h8", "b8b1", "b8b2", "b8h2", "b8b3", "b8g3", "b8b4", "b8f4", "b8b5", \
	"b8e5", "b8a6", "b8b6", "b8c6", "b8d6", "b8a7", "b8b7", "b8c7", "b8d7", \
	"b8a8", "b8c8", "b8d8", "b8e8", "b8f8", "b8g8", "b8h8", "c8c1", "c8c2", \
	"c8c3", "c8h3", "c8c4", "c8g4", "c8c5", "c8f5", "c8a6", "c8b6", "c8c6", \
	"c8d6", "c8e6", "c8a7", "c8b7", "c8c7", "c8d7", "c8e7", "c8a8", "c8b8", \
	"c8d8", "c8e8", "c8f8", "c8g8", "c8h8", "d8d1", "d8d2", "d8d3", "d8d4", \
	"d8h4", "d8a5", "d8d5", "d8g5", "d8b6", "d8c6", "d8d6", "d8e6", "d8f6", \
	"d8b7", "d8c7", "d8d7", "d8e7", "d8f7", "d8a8", "d8b8", "d8c8", "d8e8", \
	"d8f8", "d8g8", "d8h8", "e8e1", "e8e2", "e8e3", "e8a4", "e8e4", "e8b5", \
	"e8e5", "e8h5", "e8c6", "e8d6", "e8e6", "e8f6", "e8g6", "e8c7", "e8d7", \
	"e8e7", "e8f7", "e8g7", "e8a8", "e8b8", "e8c8", "e8d8", "e8f8", "e8g8", \
	"e8h8", "f8f1", "f8f2", "f8a3", "f8f3", "f8b4", "f8f4", "f8c5", "f8f5", \
	"f8d6", "f8e6", "f8f6", "f8g6", "f8h6", "f8d7", "f8e7", "f8f7", "f8g7", \
	"f8h7", "f8a8", "f8b8", "f8c8", "f8d8", "f8e8", "f8g8", "f8h8", "g8g1", \
	"g8a2", "g8g2", "g8b3", "g8g3", "g8c4", "g8g4", "g8d5", "g8g5", "g8e6", \
	"g8f6", "g8g6", "g8h6", "g8e7", "g8f7", "g8g7", "g8h7", "g8a8", "g8b8", \
	"g8c8", "g8d8", "g8e8", "g8f8", "g8h8", "h8a1", "h8h1", "h8b2", "h8h2", \
	"h8c3", "h8h3", "h8d4", "h8h4", "h8e5", "h8h5", "h8f6", "h8g6", "h8h6", \
	"h8f7", "h8g7", "h8h7", "h8a8", "h8b8", "h8c8", "h8d8", "h8e8", "h8f8", \
	"h8g8"

// APM, all possible moves. A move in this context means a valid source and
// destination square such as "a1d1" or "e4f6" but not "d4d4", "h1g4" or "x1b2".
const char *APM[] = {
	ALL_POSSIBLE_MOVES, NULL
};

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
		white_cm_attacking_sq( p, p->pieces[ BLACK_KING ] ) :
		black_cm_attacking_sq( p, p->pieces[ WHITE_KING ] );
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

	bool kingside =
		( !strcmp( castle_type, "kingside" ) || !strcmp( castle_type, "h_side" ) );

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
			if( p->pieces[ cm ] & SBA[ i ] ) {
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
		p->pieces[ whites_turn( p ) ? WHITE_KING : BLACK_KING ] );

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
	Bitboard king = p->pieces[ color_is_white ? WHITE_KING : BLACK_KING ];
	if( king & KING_SQS[ sq_bit_index( sq ) ] )
		return king;

	return 0;
}

#define CHESSMEN_OF_INTEREST_SELECTOR( chessmen_of_interest, lesser_chessman_type ) \
if( cm_is_queen ) chessmen_of_interest = \
	p->pieces[ color_is_white ? WHITE_QUEEN : BLACK_QUEEN ]; \
else chessmen_of_interest = \
	p->pieces[ color_is_white ? \
		WHITE_ ## lesser_chessman_type : BLACK_ ## lesser_chessman_type ];

#define FOUR_DIRS_FOR_LOOP( first_dir, fourth_dir, chessmen_of_interest ) \
for( enum sq_dir dir = first_dir; dir <= fourth_dir; dir += 2 ) { \
	Bitboard sq_in_dir = sq; \
	while( ( sq_in_dir = sq_nav( sq_in_dir, dir ) ) ) { \
		if( sq_in_dir & chessmen_of_interest ) { \
			attackers |= sq_in_dir; \
			break; \
		} \
		else if( !( sq_in_dir & p->pieces[ EMPTY_SQUARE ] ) ) \
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
	return p->pieces[ color_is_white ? WHITE_KNIGHT : BLACK_KNIGHT ]
		& KNIGHT_SQS[ sq_bit_index( sq ) ];
}

static Bitboard
x_cm_attacking_sq_pawns( const Pos *p, Bitboard sq, bool color_is_white )
{
	return p->pieces[ color_is_white ? WHITE_PAWN : BLACK_PAWN ] &
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
	return whites_turn( p ) ? p->pieces[ WHITE_KING ] : p->pieces[ BLACK_KING ];
}

static Bitboard
x_castling_move_status_castling_rook( const Pos *p, bool kingside )
{
	uint64_t irpf = value_BM_C960IRPF( p ), file_of_queenside_rook = 1,
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

	return ( wt && kingside && white_has_h_side_castling_right( p ) ) ||
		( wt && !kingside && white_has_a_side_castling_right( p ) ) ||
		( !wt && kingside && black_has_h_side_castling_right( p ) ) ||
		( !wt && !kingside && black_has_a_side_castling_right( p ) );
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
	if( !( castling_ ## king_or_rook & p->pieces[ EMPTY_SQUARE ] ) && \
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

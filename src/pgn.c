#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "chester.h"
#include "base.h"
#include "pgn.h"
#include "utils.h"
#include "move_gen.h"
#include "validation.h"

static void x_san_to_rawcode_find_dest_sq( const Pos *p,
    const char *san, char *dest );
static void x_san_to_rawcode_find_piece_move_orig_sq( const Pos *p,
    char piece, char disambiguator, const char *dest_sq, char *orig_sq );

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// Converts the standard algebratic notation parameter 'san' into the
// corresponding rawcode. The Pos variable pointer 'p' provides the
// necessary context for the conversion to take place. For example,
// if 'p' represents the standard starting position and the value of
// 'san' = "Nf3", then the function returns rawcode("g1f3").
//
// Move 'san' is assumed to be a legal move in position 'p'.
Rawcode
san_to_rawcode( const Pos *p, const char *san )
{
    char orig[2 + 1] = {0}, dest[2 + 1], move[4 + 1] = {0};

    x_san_to_rawcode_find_dest_sq(p, san, dest);

    if( san[0] == 'O' ) {
        strcpy( orig, sq_bit_to_sq_name( p->ppa[whites_turn(p) ?
            WHITE_KING : BLACK_KING] ) );
    } else if( str_m_pat(san, "^[a-h][1-8]") ) {
        Bitboard pawn = sq_name_to_sq_bit(dest);
        while( !( ( pawn = sq_nav(pawn, whites_turn(p) ? SOUTH : NORTH) ) &
            p->ppa[whites_turn(p) ? WHITE_PAWN : BLACK_PAWN] ) );
        strcpy( orig, sq_bit_to_sq_name(pawn) );
    } else if( str_m_pat(san, "^[a-h]x") ) {
        orig[0] = san[0], orig[1] = dest[1] + ( whites_turn(p) ? -1 : 1 );
    } else if( str_m_pat(san, "^[KQRBN]x?[a-h][1-8][+#]?$") ) {
        x_san_to_rawcode_find_piece_move_orig_sq(p, san[0], '-', dest, orig);
    } else if( str_m_pat(san, "^[QRBN][a-h1-8]x?[a-h][1-8][+#]?$") ) {
        x_san_to_rawcode_find_piece_move_orig_sq(p, san[0], san[1], dest, orig);
    } else if( str_m_pat(san, "^[QRBN][a-h][1-8]x?[a-h][1-8][+#]?$") ) {
        orig[0] = san[1], orig[1] = san[2];
    }

    move[0] = orig[0], move[1] = orig[1],
        move[2] = dest[0], move[3] = dest[1];
    assert( str_m_pat(move, "^[a-h][1-8][a-h][1-8]$") );

    return rawcode(move);
}

// TODO: doc
char *
rawcode_to_san( const Pos *p, Rawcode rc )
{
    Chessman mover, target; int orig, dest;
    set_mover_target_orig_and_dest(p, rc, &mover, &target, &orig, &dest);
    assert(mover != EMPTY_SQUARE);
    assert( target == EMPTY_SQUARE ||
        (mover >= WHITE_KING && mover <= WHITE_PAWN &&
            target >= BLACK_QUEEN && target <= BLACK_PAWN) ||
        (mover >= BLACK_KING && mover <= BLACK_PAWN &&
            target >= WHITE_QUEEN && target <= WHITE_PAWN));

    int san_length = 2; // Min length of a SAN, e.g., "e4"

    char piece_letter[2] = {'\0'};
    if(mover != WHITE_PAWN && mover != BLACK_PAWN)
        piece_letter[0] = toupper(PPF_CHESSMAN_LETTERS[mover]);
    else if(target != EMPTY_SQUARE || is_en_passant_capture(p, rc))
        piece_letter[0] = SNA[orig][0];

    san_length += strlen(piece_letter);

    char capture[2] = {'\0'};
    if(target != EMPTY_SQUARE || islower(piece_letter[0]))
        capture[0] = 'x';
    san_length += strlen(capture);

    char check_or_mate[2] = {'\0'};
    Pos after_move;
    copy_pos(p, &after_move);
    assert(!is_promotion(p, rc));
    make_move(&after_move, rc,  '-');
    if(king_in_check(&after_move)) {
        check_or_mate[0] = '+';
        if(checkmate(&after_move)) check_or_mate[0] = '#'; }
    san_length += strlen(check_or_mate);

    char *san = (char *) malloc(san_length + 1);
    strcpy(san, piece_letter), strcat(san, capture), strcat(san, SNA[dest]),
        strcat(san, check_or_mate);

    printf("san = \"%s\", san_length = %d\n", san, san_length);
    assert(che_is_san(san));
    return san;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

static void
x_san_to_rawcode_find_dest_sq( const Pos *p, const char *san, char *dest )
{
    if( san[0] == 'O' ) {
        Bitboard castling_rook_bb = p->irp[strlen(san) < 5 ? 1 : 0];
        castling_rook_bb <<= (whites_turn(p) ? 0 : 56);
        const char *castling_rook = sq_bit_to_sq_name(castling_rook_bb);
        strcpy( dest, castling_rook );
        return; }

    const char *c = &san[strlen(san) - 1];
    while( !( *c >= '1' && *c <= '8' &&
        *(c - 1) >= 'a' && *(c - 1) <= 'h' ) ) --c;

    dest[0] = *(c - 1), dest[1] = *c, dest[2] = '\0';
}

#define REMOVE_MOVER_CANDIDATES_WITH_BLOCKED_PATHS \
    for( int bi = 0; bi < 64; bi++ ) { \
        Bitboard bit = SBA[bi]; \
        if( !(bit & orig_sq_bb) ) continue; \
        Bitboard sqs_in_between = in_between(bit, SBA[dest_bi]); \
        if( !sqs_in_between ) continue; \
        if( (sqs_in_between & p->ppa[EMPTY_SQUARE]) != sqs_in_between ) \
            orig_sq_bb ^= bit; }

#define REMOVE_MOVER_CANDIDATES_IN_ABSOLUTE_PIN \
    for( int bi = 0; bi < 64; bi++ ) { \
        Bitboard bit = SBA[bi]; \
        if( !(bit & orig_sq_bb) ) continue; \
        char move[4 + 1] = {0}; \
        move[0] = file_of_sq(bit), move[1] = rank_of_sq(bit), \
            move[2] = dest_sq[0], move[3] = dest_sq[1]; \
        Pos pos; \
        copy_pos(p, &pos); \
        make_move( &pos, rawcode(move), '-' ); \
        if( king_can_be_captured( &pos ) ) \
            orig_sq_bb ^= bit; }

#define APPLY_DISAMBIGUATOR { \
    if( disambiguator >= 'a' && disambiguator <= 'h' ) { \
        orig_sq_bb &= file(disambiguator); \
    } else if( disambiguator >= '1' && disambiguator <= '8' ) { \
        orig_sq_bb &= rank(disambiguator); \
    } else assert(false); }

static void
x_san_to_rawcode_find_piece_move_orig_sq( const Pos *p, char piece,
    char disambiguator, const char *dest_sq, char *orig_sq )
{
    bool w = whites_turn(p);
    int dest_bi = sq_bit_index( sq_name_to_sq_bit(dest_sq) );
    Bitboard orig_sq_bb;

    switch( piece ) {
        case 'K':
            orig_sq_bb = p->ppa[w ? WHITE_KING : BLACK_KING];
            break;
        case 'Q':
            orig_sq_bb = ( p->ppa[w ? WHITE_QUEEN : BLACK_QUEEN] &
                ( ROOK_SQS[dest_bi] | BISHOP_SQS[dest_bi] ) );
            break;
        case 'R':
            orig_sq_bb = ( p->ppa[w ? WHITE_ROOK : BLACK_ROOK] &
                ROOK_SQS[dest_bi] );
            break;
        case 'B':
            orig_sq_bb = ( p->ppa[w ? WHITE_BISHOP : BLACK_BISHOP] &
                BISHOP_SQS[dest_bi] );
            break;
        case 'N':
            orig_sq_bb = ( p->ppa[w ? WHITE_KNIGHT : BLACK_KNIGHT] &
                KNIGHT_SQS[dest_bi] );
            break;
        default:
            assert(false);
    }

    if( disambiguator != '-' )
        APPLY_DISAMBIGUATOR
    if( !is_sq_bit(orig_sq_bb) )
        REMOVE_MOVER_CANDIDATES_WITH_BLOCKED_PATHS
    if( !is_sq_bit(orig_sq_bb) )
        REMOVE_MOVER_CANDIDATES_IN_ABSOLUTE_PIN

    assert( is_sq_bit(orig_sq_bb) );

    strcpy( orig_sq, sq_bit_to_sq_name(orig_sq_bb) );
}

#undef REMOVE_MOVER_CANDIDATES_WITH_BLOCKED_PATHS
#undef REMOVE_MOVER_CANDIDATES_IN_ABSOLUTE_PIN
#undef APPLY_DISAMBIGUATOR

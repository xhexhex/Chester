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
static void x_rawcode_to_san_set_disambiguator( const Pos *p, Chessman mover,
    int orig, int dest, char *disambiguator, int *san_length );

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
rawcode_to_san( const Pos *p, Rawcode rc, char promotion )
{
    assert((!is_promotion(p, rc) && promotion == '-') || (is_promotion(p, rc) && (
        toupper(promotion) == 'Q' || toupper(promotion) == 'R' ||
        toupper(promotion) == 'B' || toupper(promotion) == 'N' )));

    Chessman mover, target; int orig, dest;
    set_mover_target_orig_and_dest(p, rc, &mover, &target, &orig, &dest);
    assert(mover != EMPTY_SQUARE);
    assert( target == EMPTY_SQUARE ||
        (mover >= WHITE_KING && mover <= WHITE_PAWN &&
            target >= BLACK_QUEEN && target <= BLACK_PAWN) ||
        (mover >= BLACK_KING && mover <= BLACK_PAWN &&
            target >= WHITE_QUEEN && target <= WHITE_PAWN));
    assert((whites_turn(p) && mover >= WHITE_KING && mover <= WHITE_PAWN) ||
        (!whites_turn(p) && mover >= BLACK_KING && mover <= BLACK_PAWN));

    int san_length = 2; // Min length of a SAN, e.g., "e4"

    char piece_letter[2] = {'\0'};
    if(mover != WHITE_PAWN && mover != BLACK_PAWN)
        piece_letter[0] = toupper(PPF_CHESSMAN_LETTERS[mover]);
    else if(target != EMPTY_SQUARE || is_en_passant_capture(p, rc))
        piece_letter[0] = SNA[orig][0];
    san_length += strlen(piece_letter);

    char disambiguator[3] = {'\0'};
    x_rawcode_to_san_set_disambiguator(p, mover, orig, dest, disambiguator,
        &san_length);

    char capture[2] = {'\0'};
    if(target != EMPTY_SQUARE || islower(piece_letter[0]))
        capture[0] = 'x';
    san_length += strlen(capture);

    char promotion_indicator[3] = {'\0'};
    if(is_promotion(p, rc))
        promotion_indicator[0] = '=',
            promotion_indicator[1] = toupper(promotion);
    san_length += strlen(promotion_indicator);

    char check_or_mate[2] = {'\0'};
    Pos after_move;
    copy_pos(p, &after_move);
    make_move(&after_move, rc, tolower(promotion));
    if(king_in_check(&after_move)) {
        check_or_mate[0] = '+';
        if(checkmate(&after_move)) check_or_mate[0] = '#'; }
    san_length += strlen(check_or_mate);

    char *san = (char *) malloc(san_length + 1);
    strcpy(san, piece_letter), strcat(san, disambiguator), strcat(san, capture),
        strcat(san, SNA[dest]), strcat(san, promotion_indicator),
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

#define LINE_MOVING_PIECE_FINDER(starting_dir, piece_type, piece_type_plural, \
        line_type) \
    for(enum sq_dir dir = starting_dir; dir <= starting_dir + 4; dir += 4) { \
        Bitboard sq = SBA[dest]; \
        while((sq = sq_nav(sq, dir)) && ( \
            (w && !(sq & p->ppa[WHITE_ ## piece_type])) || \
            (!w && !(sq & p->ppa[BLACK_ ## piece_type])))); \
    piece_type_plural ## _on_ ## line_type |= sq; } \
    num_ ## piece_type_plural ## _on_ ## line_type = num_of_sqs_in_sq_set( \
        piece_type_plural ## _on_ ## line_type); \
    assert(num_ ## piece_type_plural ## _on_ ## line_type >= 0 && \
        num_ ## piece_type_plural ## _on_ ## line_type <= 2);

static void
x_rawcode_to_san_set_disambiguator( const Pos *p, Chessman mover, int orig,
    int dest, char *disambiguator, int *san_length )
{
    if(mover == WHITE_KING || mover == BLACK_KING ||
            mover == WHITE_PAWN || mover == BLACK_PAWN)
        return;

    bool w = whites_turn(p), // od, orig and dest
        // od_on_same_file = (file_of_sq(SBA[orig]) == file_of_sq(SBA[dest])),
        od_on_same_rank = (rank_of_sq(SBA[orig]) == rank_of_sq(SBA[dest]));
        // od_on_same_diag = 0,
        // od_on_same_antidiag = 0;
    Bitboard queens_on_file = 0, queens_on_rank = 0, queens_on_diag = 0,
        queens_on_antidiag = 0, rooks_on_file = 0, rooks_on_rank = 0;
    int num_queens_on_file = 0, num_queens_on_rank = 0, num_queens_on_diag = 0,
        num_queens_on_antidiag = 0, num_rooks_on_file = 0, num_rooks_on_rank = 0;

    if(mover == WHITE_QUEEN || mover == BLACK_QUEEN) {
        LINE_MOVING_PIECE_FINDER(NORTH, QUEEN, queens, file)
        LINE_MOVING_PIECE_FINDER(EAST, QUEEN, queens, rank)
        LINE_MOVING_PIECE_FINDER(NORTHEAST, QUEEN, queens, diag)
        LINE_MOVING_PIECE_FINDER(SOUTHEAST, QUEEN, queens, antidiag)

        assert(SBA[orig] & (queens_on_file | queens_on_rank |
            queens_on_diag | queens_on_antidiag));
        assert(num_queens_on_file || num_queens_on_rank || num_queens_on_diag ||
            num_queens_on_antidiag);

        int queens_with_access = num_queens_on_file + num_queens_on_rank +
            num_queens_on_diag + num_queens_on_antidiag;

        if(queens_with_access == 1) return;
        else if(queens_with_access == 2) {
            printf("Yo yo, sir!\n");
        }
        else assert(false);
    } else if(mover == WHITE_ROOK || mover == BLACK_ROOK) {
        LINE_MOVING_PIECE_FINDER(NORTH, ROOK, rooks, file)
        LINE_MOVING_PIECE_FINDER(EAST, ROOK, rooks, rank)

        assert(SBA[orig] & (rooks_on_file | rooks_on_rank));
        assert(num_rooks_on_file || num_rooks_on_rank);

        int rooks_with_access = num_rooks_on_file + num_rooks_on_rank;
        assert(rooks_with_access <= 4);

        if(rooks_with_access == 1) return;

        disambiguator[0] = (od_on_same_rank || num_rooks_on_rank == 1) ?
            file_of_sq(SBA[orig]) : rank_of_sq(SBA[orig]);
        *san_length += 1;
    } else if(mover == WHITE_BISHOP || mover == BLACK_BISHOP) {

    } else if(mover == WHITE_KNIGHT || mover == BLACK_KNIGHT) {

    } else assert(false);
}

#undef LINE_MOVING_PIECE_FINDER

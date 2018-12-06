#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "base.h"
#include "utils.h"
#include "validation.h"
#include "move_gen.h"
#include "pgn.h"
#include "extra.h"

// Are these useful?
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
static void x_fen_to_pos_init_turn_and_ca_flags( Pos *p, const char *acf,
    const char *caf, const char *fen );
static void x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen );
static void x_fen_to_pos_init_epts_file( Pos *p, const char eptsf[] );
static bool x_rawcode_preliminary_checks( const char *rawmove );
static void x_make_monster_sanity_checks( const Pos *p, Rawcode rc,
    char promotion, int orig, int dest, Chessman mover, Chessman target );
static int x_ppa_to_ppf( const Bitboard *ppa, char *ppf );
static void x_conditional_shredder_ecaf_to_std_ecaf_conv( char *the_ecaf,
    const Pos *p );

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// To understand che_make_moves() it's best to first study
// single_san_make_move() as the former is basically a multiple-items-
// allowed-in-single-call version of the latter.
//
// Parameter 'fen' receives a single FEN string such as
// "8/8/8/8/8/7k/8/N6K w - - 12 34". If 'fen' is a null pointer,
// it is taken to mean the standard starting position. The 'sans'
// parameter is a null-terminated string that is interpreted as a
// sequence of SANs separated either by spaces or newlines. Assuming
// that 'sans' is not a null pointer, 'fen' is considered the starting
// position for the sequence of moves specified in 'sans'. Each of the
// resulting FENs are appended to the string data returned by the function.
// For example, the call che_make_moves("k7/8/8/8/8/8/8/K7 w - - 0 99",
// "Kb2 Ka7") would return a pointer to the string
// "k7/8/8/8/8/8/1K6/8 b - - 1 99\n8/k7/8/8/8/8/1K6/8 w - - 2 100\n".
// In general each of the FENs in the string data returned by the
// function is terminated with a newline.
//
// The behavior of the function changes somewhat if 'sans' is passed
// a null pointer. In this case the list of FENs returned correspond
// to the positions that can be reached from 'fen' with a single move.
// That is, if 'fen' is interpreted as a node in the game tree of
// chess, a call with 'sans' receiving null returns all of the child
// nodes of 'fen'.
//
// Any call to the function should involve a subsequent call to free().
char *
che_make_moves( const char *fen, const char *sans )
{
    if(!fen) fen = FEN_STD_START_POS;

    bool fc = !sans; // fc, find children
    if(fc) {
        sans = che_move_gen(fen);
        assert(sans && strlen(sans) >= 1);
        if(sans[0] == '-') {
            char *empty_str = malloc(1 * sizeof(char));
            assert(empty_str), *empty_str = '\0';
            free((void *) sans);
            return empty_str; } }

    int num_alloc_bytes = 8 * 1024, iter_count = 0, len_fens = 0;
    char *unmod_ptr = malloc(strlen(sans) + 1), *san_data = unmod_ptr,
        *san, *fens = malloc(num_alloc_bytes),
        *next_fen = malloc(strlen(fen) + 1);
    strcpy(san_data, sans), strcpy(fens, ""), strcpy(next_fen, fen);
    for(int i = 0; san_data[i]; i++)
        if(san_data[i] == ' ') san_data[i] = '\n';

    while((san = next_line(&san_data))) {
        ++iter_count;
        char *tmp = next_fen;
        next_fen = single_san_make_move(fc ? fen : next_fen, san),
            free(tmp);
        len_fens += strlen(next_fen) + 1;
        while(len_fens >= num_alloc_bytes) {
            num_alloc_bytes *= 2;
            assert((fens = realloc(fens, num_alloc_bytes))); }
        strcat(fens, next_fen), strcat(fens, "\n"); }

    assert(iter_count);
    assert(len_fens == (int) strlen(fens));
    assert(len_fens >= FEN_MIN_LENGTH + 1);
    assert(fens[len_fens - 1] == '\n');

    free(unmod_ptr), free(next_fen);
    if(fc) free((void *) sans);
    return fens;
}

// Returns the result of making the move 'san' in position 'fen'. For
// example, the call
// single_san_make_move("5k2/8/8/4p3/8/8/8/R3K2R w KQ e6 0 99", "O-O-O")
// returns a pointer to the dynamically allocated string
// "5k2/8/8/4p3/8/8/8/2KR3R b - - 1 99".
char *
single_san_make_move( const char *fen, const char *san )
{
    Pos *p = fen_to_pos(fen);
    Rawcode move = san_to_rawcode(p, san);

    char promotion = '-'; int index;
    if( str_m_pat(san, "^.*=[QRBN][+#]?$") && (index = strlen(san) - 1) )
        do { promotion = san[index--]; } while(!isupper(promotion));

    make_monster( p, move, tolower(promotion) );
    char *new_fen = pos_to_fen(p);
    free(p);

    return new_fen;
}

// Converts the FEN string argument 'fen' to a Pos variable. The Pos
// variable is allocated dynamically so free() should be called when
// the variable is no longer needed.
Pos *
fen_to_pos( const char *fen )
{
    Pos *p;
    assert( (p = malloc(sizeof(Pos))) );

    char copy[FEN_MAX_LENGTH + 1], *ptr_1 = copy, *ptr_2;
    copy[strlen(fen)] = '\0';
    for(int i = 0; fen[i]; i++)
        if(fen[i] == ' ') copy[i] = '\0';
        else copy[i] = fen[i];

    const int first_in_rank[] = {56, 48, 40, 32, 24, 16, 8, 0};
    int *bi_ptr = (int *) first_in_rank, bi = *bi_ptr;
    for(Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++) p->ppa[cm] = 0;

    for(char *c = ptr_1; *c; c++) {
        if(*c == '/') {
            ++bi_ptr, bi = *bi_ptr;
        } else if(*c >= '1' && *c <= '8') {
            for(char count = '1'; count <= *c; count++, bi++)
                p->ppa[EMPTY_SQUARE] |= SBA[bi];
        } else {
            switch(*c) {
                case 'K': p->ppa[WHITE_KING  ] |= SBA[bi]; break;
                case 'Q': p->ppa[WHITE_QUEEN ] |= SBA[bi]; break;
                case 'R': p->ppa[WHITE_ROOK  ] |= SBA[bi]; break;
                case 'B': p->ppa[WHITE_BISHOP] |= SBA[bi]; break;
                case 'N': p->ppa[WHITE_KNIGHT] |= SBA[bi]; break;
                case 'P': p->ppa[WHITE_PAWN  ] |= SBA[bi]; break;
                case 'k': p->ppa[BLACK_KING  ] |= SBA[bi]; break;
                case 'q': p->ppa[BLACK_QUEEN ] |= SBA[bi]; break;
                case 'r': p->ppa[BLACK_ROOK  ] |= SBA[bi]; break;
                case 'b': p->ppa[BLACK_BISHOP] |= SBA[bi]; break;
                case 'n': p->ppa[BLACK_KNIGHT] |= SBA[bi]; break;
                case 'p': p->ppa[BLACK_PAWN  ] |= SBA[bi]; break;
                default: assert(false); }
            ++bi; }
    } // End for

    while(*ptr_1++); ptr_2 = ptr_1; while(*ptr_2++);
    x_fen_to_pos_init_turn_and_ca_flags(p, ptr_1, ptr_2, fen);
    x_fen_to_pos_init_irp(p, ptr_2, fen);

    while(*ptr_2++);
    x_fen_to_pos_init_epts_file(p, ptr_2);

    while(*ptr_2++);
    p->hmc = atoi(ptr_2);

    while(*ptr_2++);
    p->fmn = atoi(ptr_2);

    return p;
}

// Converts the Pos variable pointed to by 'p' to the corresponding FEN
// string. The FEN string is allocated dynamically so free() should be
// called when the string is no longer needed.
char *
pos_to_fen( const Pos *p )
{
    char *fen = malloc(FEN_MAX_LENGTH + 1);
    int index = x_ppa_to_ppf(p->ppa, fen);

    bool w = whites_turn(p);
    fen[index++] = ' ', fen[index++] = (w ? 'w' : 'b'),
        fen[index++] = ' ';

    char *the_ecaf = ecaf(p);
    if(!strcmp(the_ecaf, "----"))
        fen[index++] = '-';
    else {
        x_conditional_shredder_ecaf_to_std_ecaf_conv(the_ecaf, p);
        for(int i = 0; i < 4; i++)
            if(the_ecaf[i] != '-') fen[index++] = the_ecaf[i]; }
    fen[index++] = ' ';

    free(the_ecaf);

    Bitboard the_epts = epts(p);
    if(!the_epts) fen[index++] = '-';
    else fen[index++] = file_of_sq(the_epts),
        fen[index++] = rank_of_sq(the_epts);
    fen[index++] = ' ';

    char hmcf[5 + 2], fmnf[5 + 1];
    sprintf(hmcf, "%d ", p->hmc), sprintf(fmnf, "%d", p->fmn);
    for(int i = 0; hmcf[i]; i++) fen[index++] = hmcf[i];
    for(int i = 0; fmnf[i]; i++) fen[index++] = fmnf[i];

    fen[index] = '\0';
    assert((int) strlen(fen) == index);

    return fen;
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

// Returns the square set of the white army. That means all the squares
// that contain a white chessman.
Bitboard
white_army( const Pos *p )
{
    return p->ppa[ WHITE_KING ] | p->ppa[ WHITE_QUEEN ] |
        p->ppa[ WHITE_ROOK ] | p->ppa[ WHITE_BISHOP ] |
        p->ppa[ WHITE_KNIGHT ] | p->ppa[ WHITE_PAWN ];
}

// Returns the square set of the black army. That means all the squares
// that contain a black chessman.
Bitboard
black_army( const Pos *p )
{
    return p->ppa[ BLACK_KING ] | p->ppa[ BLACK_QUEEN ] |
        p->ppa[ BLACK_ROOK ] | p->ppa[ BLACK_BISHOP ] |
        p->ppa[ BLACK_KNIGHT ] | p->ppa[ BLACK_PAWN ];
}

// The Square Navigator returns the square bit that is in direction 'dir'
// of square bit 'sq'. For example, the call sq_nav( SB.e4, NORTHEAST )
// would return SB.f5 and the call sq_nav( SB.e4, ONE_OCLOCK ) would
// return SB.f6.
Bitboard
sq_nav( Bitboard sq, enum sq_dir dir )
{
    return SQ_NAV[bindex(sq)][dir];
}

// Examines the Pos variable 'p' to see if there's a castling
// right/availability for color 'color' on side 'side'. For example,
// if the call has_castling_right( p, "white", "kingside" ) returns true,
// White has the kingside castling right. In a standard FEN CAF this would
// translate to the presence of the character 'K'. It's important to
// remember that having a castling right is different from being able to
// castle in the given position. For example, all the castling rights are
// present in the standard starting position, yet "O-O" or "O-O-O" are not
// available as moves.
bool
has_castling_right( const Pos *p, const char *color, const char *side )
{
    /*
    assert( !strcmp( color, "white" ) || !strcmp( color, "black" ) );
    assert( !strcmp( side, "queenside" ) || !strcmp( side, "kingside" ) ||
        !strcmp( side, "a-side" ) || !strcmp( side, "h-side" ) );
    */

    bool w = (color[0] == 'w'),
        kingside = (side[0] == 'k' || side[0] == 'h');

    uint8_t bit = ( w ? 8 : 2 );
    if(!kingside) bit >>= 1;
    // assert(bindex(bit));

    return bit & p->turn_and_ca_flags;
}

// Returns the square bit corresponding to the en passant target square
// of the Pos variable 'p' or zero if there is no EPTS set.
Bitboard
epts( const Pos *p )
{
    if(!p->epts_file) return 0;

    // Bitboard bb = p->epts_file;
    // bb <<= (whites_turn(p) ? 40 : 16);

    // assert(is_sq_bit(bb));
    return (Bitboard) p->epts_file << (whites_turn(p) ? 40 : 16);
}

// Returns the rawcode corresponding to the 'rawmove' argument. If
// 'rawmove' is not valid, then zero is returned.
Rawcode
rawcode( const char *rawmove )
{
    if( !x_rawcode_preliminary_checks(rawmove) )
        return 0;

    char current[4 + 1] = {'\0'};
    // const int N = sizeof(APM_DATA)/4;
    const int N = strlen(APM_DATA)/4;
    int left = 0, right = N - 1;

    // Binary search algorithm
    while( left <= right ) {
        int middle = (left + right) / 2;
        assert( middle >= 0 && middle < N );

        for( int i = 0; i < 4; i++ )
            current[i] = APM_DATA[4*middle + i];

        if( strcmp( current, rawmove ) < 0 ) {
            left = middle + 1;
            continue;
        } else if( strcmp( current, rawmove ) > 0 ) {
            right = middle - 1;
            continue;
        }

        return middle + 1;
    }

    return 0;
}

// Converts 'rawcode' into the corresponding rawmove and saves the
// result in 'writable' which is assumed to point to an array of
// at least five bytes.
void
rawmove( Rawcode rawcode, char *writable )
{
    assert( rawcode >= 1 );
    assert( rawcode <= 1792 );

    writable[4] = '\0';

    const char *ptr = &APM_DATA[4*(rawcode - 1)];
    for( int i = 0; i < 4; i++ )
        writable[i] = ptr[i];
}

// Chester's core "make move" function
//
// Makes the move 'rc' in position 'p'. This invariably involves
// modifying or updating 'p'. If the move made is a pawn promotion,
// parameter 'promotion' indicates the piece the pawn gets promoted
// to ('q' for queen, 'r' for rook, 'b' for bishop and 'n' for knight).
// If the move is not a pawn promotion, the value passed to the
// 'promotion' parameter should be the character '-'.
//
// The function was originally called "make_move". The name was changed
// to "make_monster" due to the function's monolithic implementation.
void
make_monster( Pos *p, Rawcode rc, char promotion )
{
    bool w = whites_turn(p);

    int sg_orig = RC_ORIG_SQ_BINDEX[rc], sg_dest = RC_DEST_SQ_BINDEX[rc];
    Chessman sg_mover, sg_target;

    const Bitboard ONE = 1;
    Bitboard sq_bit = (ONE << sg_orig);
    for(Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++)
        if(sq_bit & p->ppa[cm]) { sg_mover = cm; break; }
    sq_bit = (ONE << sg_dest);
    for(Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++)
        if(sq_bit & p->ppa[cm]) { sg_target = cm; break; }

    if(false) x_make_monster_sanity_checks(p, rc, promotion,
        sg_orig, sg_dest, sg_mover, sg_target); // Expensive!

    Bitboard the_epts = (p->epts_file ?
        (Bitboard) p->epts_file << (w ? 40 : 16) : 0);
    bool ep = (the_epts == SBA[sg_dest] && (sg_mover == WHITE_PAWN ||
        sg_mover == BLACK_PAWN));

    if((sg_mover == WHITE_KING && sg_target == WHITE_ROOK) ||
            (sg_mover == BLACK_KING && sg_target == BLACK_ROOK)) {
        Bitboard castling_rook = (SBA[sg_dest] &
                (p->ppa[WHITE_ROOK] | p->ppa[BLACK_ROOK])),
            tmp = castling_rook;
        if(tmp > SB.h1) tmp >>= 56;

        uint8_t file_of_castling_rook = tmp;
        bool kingside = (file_of_castling_rook & p->irp[1]);

        p->ppa[EMPTY_SQUARE] ^= p->ppa[w ? WHITE_KING : BLACK_KING];
        p->ppa[w ? WHITE_KING : BLACK_KING] = 0;
        p->ppa[EMPTY_SQUARE] ^= castling_rook;
        p->ppa[w ? WHITE_ROOK : BLACK_ROOK] ^= castling_rook;

        if(kingside && w) {
            p->ppa[WHITE_KING] = SB.g1, p->ppa[EMPTY_SQUARE] ^= SB.g1;
            p->ppa[WHITE_ROOK] |= SB.f1, p->ppa[EMPTY_SQUARE] ^= SB.f1;
        } else if(kingside && !w) {
            p->ppa[BLACK_KING] = SB.g8, p->ppa[EMPTY_SQUARE] ^= SB.g8;
            p->ppa[BLACK_ROOK] |= SB.f8, p->ppa[EMPTY_SQUARE] ^= SB.f8;
        } else if(w) { // queenside
            p->ppa[WHITE_KING] = SB.c1, p->ppa[EMPTY_SQUARE] ^= SB.c1;
            p->ppa[WHITE_ROOK] |= SB.d1, p->ppa[EMPTY_SQUARE] ^= SB.d1;
        } else if(!w) { // queenside
            p->ppa[BLACK_KING] = SB.c8, p->ppa[EMPTY_SQUARE] ^= SB.c8;
            p->ppa[BLACK_ROOK] |= SB.d8, p->ppa[EMPTY_SQUARE] ^= SB.d8;
        } else assert(false); // Should be impossible

        if(w) {
            if(p->turn_and_ca_flags & 8) p->turn_and_ca_flags ^= 8;
            if(p->turn_and_ca_flags & 4) p->turn_and_ca_flags ^= 4;
        } else {
            if(p->turn_and_ca_flags & 2) p->turn_and_ca_flags ^= 2;
            if(p->turn_and_ca_flags & 1) p->turn_and_ca_flags ^= 1; }
    } // End if
    else {
        if(ep) {
            Bitboard double_advanced_pawn = the_epts;
            if(w) double_advanced_pawn >>= 8;
            else double_advanced_pawn <<= 8;

            // Make the pawn that has made a double-step advance go
            // backwards a single step
            p->ppa[EMPTY_SQUARE] ^= the_epts;
            p->ppa[w ? BLACK_PAWN : WHITE_PAWN] |= the_epts;
            p->ppa[EMPTY_SQUARE] |= double_advanced_pawn;
            p->ppa[w ? BLACK_PAWN : WHITE_PAWN] ^= double_advanced_pawn;

            sg_target = w ? BLACK_PAWN : WHITE_PAWN; }

        // Make the origin square vacant
        p->ppa[sg_mover] ^= SBA[sg_orig], p->ppa[EMPTY_SQUARE] |= SBA[sg_orig];
        // Make the moving chessman "reappear" in the destination square
        p->ppa[sg_mover] |= SBA[sg_dest], p->ppa[sg_target] ^= SBA[sg_dest];

        if(sg_mover == WHITE_KING) {
            if(p->turn_and_ca_flags & 8) p->turn_and_ca_flags ^= 8;
            if(p->turn_and_ca_flags & 4) p->turn_and_ca_flags ^= 4;
        } else if(sg_mover == BLACK_KING) {
            if(p->turn_and_ca_flags & 2) p->turn_and_ca_flags ^= 2;
            if(p->turn_and_ca_flags & 1) p->turn_and_ca_flags ^= 1; }

        if((sg_mover == WHITE_ROOK && SBA[sg_orig] == p->irp[1]) ||
                (sg_target == WHITE_ROOK && SBA[sg_dest] == p->irp[1])) {
            if(p->turn_and_ca_flags & 8) p->turn_and_ca_flags ^= 8;
        } else if((sg_mover == WHITE_ROOK && SBA[sg_orig] == p->irp[0]) ||
                (sg_target == WHITE_ROOK && SBA[sg_dest] == p->irp[0])) {
            if(p->turn_and_ca_flags & 4) p->turn_and_ca_flags ^= 4; }

        if((sg_mover == BLACK_ROOK && SBA[sg_orig] ==
                ((Bitboard) p->irp[1] << 56)) ||
                (sg_target == BLACK_ROOK && SBA[sg_dest] ==
                ((Bitboard) p->irp[1] << 56))) {
            if(p->turn_and_ca_flags & 2) p->turn_and_ca_flags ^= 2;
        } else if((sg_mover == BLACK_ROOK && SBA[sg_orig] ==
                ((Bitboard) p->irp[0] << 56)) ||
                (sg_target == BLACK_ROOK && SBA[sg_dest] ==
                ((Bitboard) p->irp[0] << 56))) {
            if(p->turn_and_ca_flags & 1) p->turn_and_ca_flags ^= 1; }
    } // End else

    bool pp = ( // pp, pawn promotion
        (sg_mover == WHITE_PAWN && sg_orig >= 48 && sg_orig <= 55 ) ||
        (sg_mover == BLACK_PAWN && sg_orig >=  8 && sg_orig <= 15));

    /*
    if(pp) {
        const Bitboard ranks_1_and_8 = 0xff000000000000ff;
        Bitboard exactly_one_pawn = (p->ppa[w ? WHITE_PAWN : BLACK_PAWN] &
            ranks_1_and_8);
        assert( num_of_sqs_in_sq_set(exactly_one_pawn) == 1 );
        assert( SBA[sg_dest] == exactly_one_pawn ); }
    */

    if(pp) {
        Bitboard pawn = SBA[sg_dest];
        p->ppa[w ? WHITE_PAWN : BLACK_PAWN] ^= pawn;

        Chessman promote_to = WHITE_QUEEN;
        if(promotion == 'r') promote_to = WHITE_ROOK;
        else if(promotion == 'b') promote_to = WHITE_BISHOP;
        else if(promotion == 'n') promote_to = WHITE_KNIGHT;
        if(!w) promote_to += 6;

        p->ppa[promote_to] |= pawn; }

    // Toggle turn
    if(w) p->turn_and_ca_flags ^= (1 << 7);
    else p->turn_and_ca_flags |= (1 << 7);
    w = !w;

    bool capture = ( ep ||
        (sg_mover >= WHITE_KING && sg_mover <= WHITE_PAWN &&
            sg_target >= BLACK_QUEEN && sg_target <= BLACK_PAWN) ||
        (sg_mover >= BLACK_KING && sg_mover <= BLACK_PAWN &&
            sg_target >= WHITE_QUEEN && sg_target <= WHITE_PAWN) );
    bool sspa = ( // sspa, single step pawn advance
        (sg_mover == WHITE_PAWN && SBA[sg_dest] == (SBA[sg_orig] << 8)) ||
        (sg_mover == BLACK_PAWN && SBA[sg_dest] == (SBA[sg_orig] >> 8)));
    bool dspa = ( // dspa, double step pawn advance
        (sg_mover == WHITE_PAWN && SBA[sg_dest] == (SBA[sg_orig] << 16)) ||
        (sg_mover == BLACK_PAWN && SBA[sg_dest] == (SBA[sg_orig] >> 16)));

    if(capture || sspa || dspa) p->hmc = 0;
    else p->hmc++;

    if(dspa) p->epts_file = (SBA[sg_orig] >> (w ? 48 : 8));
    else p->epts_file = 0;

    if(w) p->fmn++;

    if(false) assert(!ppa_integrity_check(p->ppa)); // Expensive!
}

// Removes the castling right(s) specified by the 'color' and 'side'
// parameters from position 'p'. The valid values for 'color' are "white"
// and "black"; for 'side' the valid values are "kingside", "h-side",
// "queenside", "a-side" and "both".
//
// For example, if 'p' corresponds to the standard starting position,
// then the call remove_castling_rights(p, "black", "both") would result
// in the Pos variable equilent to the FEN
// "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1".
void
remove_castling_rights( Pos *p, const char *color, const char *side )
{
    assert( !strcmp(color, "white") || !strcmp(color, "black") );
    assert( !strcmp(side, "both") || !strcmp(side, "kingside") ||
        !strcmp(side, "h-side") || !strcmp(side, "queenside") ||
        !strcmp(side, "a-side") );

    bool white = !strcmp(color, "white") ? true : false;

    if( !strcmp(side, "both") || !strcmp(side, "kingside") ||
            !strcmp(side, "h-side") ) {
        if( p->turn_and_ca_flags & (white ? 8 : 2) ) {
            p->turn_and_ca_flags ^= (white ? 8 : 2);
        }
    }
    if( !strcmp(side, "both") || !strcmp(side, "queenside") ||
            !strcmp(side, "a-side") ) {
        if( p->turn_and_ca_flags & (white ? 4 : 1) ) {
            p->turn_and_ca_flags ^= (white ? 4 : 1);
        }
    }
}

// Examines the four castling availability indicators in 'p' and returns
// the corresponding *expanded* Shredder-FEN CAF. For example, if 'p' equals
// fen_to_pos("7k/8/8/8/8/2n5/1B6/R3K3 w Q - 10 100"), then the call
// ecaf(p) returns "-A--". For the standard starting position the
// returned expanded CAF would be "HAha".
char *
ecaf( const Pos *p )
{
    char *the_ecaf = (char *) malloc(4 + 1), *color[] = {"white", "black"},
        *side[] = {"kingside", "queenside"};
    assert(the_ecaf);
    the_ecaf[4] = '\0';
    for(int i = 0; i < 4; i++) the_ecaf[i] = '-';

    for(int i = 0, index = 0; i < 2; i++) // Color
        for(int j = 0; j < 2; j++, index++) { // Side
            if( !has_castling_right(p, color[i], side[j]) ) continue;

            uint8_t bit = 1; char file = (!i ? 'A' : 'a');
            while( !(bit & p->irp[1 - j]) ) bit <<= 1, file++;
            the_ecaf[index] = file; }

    return the_ecaf;
}

// Toggles the binary value of the turn indicator in position 'p'.
void
toggle_turn( Pos *p )
{
    if( whites_turn(p) )
        p->turn_and_ca_flags ^= (1 << 7);
    else
        p->turn_and_ca_flags |= (1 << 7);
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

// turn_and_ca_flags:
//  7   6   5   4   3   2   1   0   <= Bit indexes
// [x] [ ] [ ] [ ] [x] [ ] [ ] [x]  <= Bit values
//  t               K   Q   k   q   <= Meaning
//
// From the above we learn that it is White's turn and that the remaining
// castling rights in the Pos variable 'p' are white kingside (K) and
// black queenside (q).
static void
x_fen_to_pos_init_turn_and_ca_flags( Pos *p, const char *acf,
    const char *caf, const char *fen )
{
    p->turn_and_ca_flags = 0;
    if(!strcmp(acf, "w")) p->turn_and_ca_flags |= (1 << 7);
    if(!strcmp(caf, "-")) return;

    bool std_caf = true;
    for(int i = 0; i < (int) strlen(caf); i++) {
        char c = caf[i];
        if(c != 'K' && c != 'Q' && c != 'k' && c != 'q') {
            std_caf = false;
            break; } }

    if(std_caf) {
        bool K = true, Q = true, k = true, q = true;

        if(!strcmp(caf, "KQkq"));
        else if(!strcmp(caf, "Qkq")) K = false;
        else if(!strcmp(caf, "Kkq")) Q = false;
        else if(!strcmp(caf, "KQq")) k = false;
        else if(!strcmp(caf, "KQk")) q = false;
        else if(!strcmp(caf, "KQ")) k = false, q = false; // [K][Q][ ][ ]
        else if(!strcmp(caf, "Kk")) Q = false, q = false; // [K][ ][k][ ]
        else if(!strcmp(caf, "Kq")) Q = false, k = false; // [K][ ][ ][q]
        else if(!strcmp(caf, "Qk")) K = false, q = false; // [ ][Q][k][ ]
        else if(!strcmp(caf, "Qq")) K = false, k = false; // [ ][Q][ ][q]
        else if(!strcmp(caf, "kq")) K = false, Q = false; // [ ][ ][k][q]
        else if(!strcmp(caf, "K")) Q = false, k = false, q = false;
        else if(!strcmp(caf, "Q")) K = false, k = false, q = false;
        else if(!strcmp(caf, "k")) K = false, Q = false, q = false;
        else if(!strcmp(caf, "q")) K = false, Q = false, k = false;
        else assert(false);

        if(K) p->turn_and_ca_flags |= 8; // K
        if(Q) p->turn_and_ca_flags |= 4; // Q
        if(k) p->turn_and_ca_flags |= 2; // k
        if(q) p->turn_and_ca_flags |= 1; // q

        return; }

    assert(!str_m_pat(caf, STD_FEN_CAF_REGEX));
    char ecaf[9 + 1];
    EXPAND_CAF(caf, ecaf, fen)
    assert(!ecaf[4]);

    if(ecaf[1] != '-') p->turn_and_ca_flags |= 8; // K
    if(ecaf[0] != '-') p->turn_and_ca_flags |= 4; // Q
    if(ecaf[3] != '-') p->turn_and_ca_flags |= 2; // k
    if(ecaf[2] != '-') p->turn_and_ca_flags |= 1; // q
}

// irp[0] and irp[1]:
//  7   6   5   4   3   2   1   0   <= Bit indexes
// [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]  <= Bit values
//  h   g   f   e   d   c   b   a   <= Meaning
//
// The default queenside and kingside bits are those at indexes 0 and 7,
// respectively. This means that if the bit of the queenside or kingside
// IRP cannot be determined from the CAF, the default is used. Therefore
// irp[0] and irp[1] have always exactly one bit set.
static void
x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen )
{
    char ecaf[9 + 1];
    EXPAND_CAF(caf, ecaf, fen)
    assert(!ecaf[4]);

    p->irp[0] = 0, p->irp[1] = 0;

    if(ecaf[0] != '-') p->irp[0] |= (1 << (ecaf[0] - 'A'));
    else if(ecaf[2] != '-') p->irp[0] |= (1 << (ecaf[2] - 'a'));
    else p->irp[0] |= 1;

    if(ecaf[1] != '-') p->irp[1] |= (1 << (ecaf[1] - 'A'));
    else if(ecaf[3] != '-') p->irp[1] |= (1 << (ecaf[3] - 'a'));
    else p->irp[1] |= 0x80;

    assert(is_sq_bit(p->irp[0]));
    assert(is_sq_bit(p->irp[1]));
    assert(p->irp[0] < p->irp[1]);
}

static void
x_fen_to_pos_init_epts_file( Pos *p, const char eptsf[] )
{
    p->epts_file = 0;
    if(!strcmp(eptsf, "-")) return;

    char file = eptsf[0];
    assert(file >= 'a' && file <= 'h');
    p->epts_file |= (1 << (file - 'a'));
}

static bool
x_rawcode_preliminary_checks( const char *rawmove )
{
    if( !rawmove || strlen(rawmove) != 4 )
        return false;

    char src[2 + 1] = {'\0'}, dst[2 + 1] = {'\0'};
    src[0] = rawmove[0], src[1] = rawmove[1];
    dst[0] = rawmove[2], dst[1] = rawmove[3];

    if( !is_sq_name(src) || !is_sq_name(dst) )
        return false;

    return true;
}

static void
x_make_monster_sanity_checks( const Pos *p, Rawcode rc, char promotion,
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

    // It's OK for make_monster() to execute moves that result in a position
    // where a king can be captured. However, the position should be legal
    // before the execution of the move.
    assert( !king_can_be_captured(p) );

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

    Bitboard the_epts = epts(p); // En passant target square
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

static int
x_ppa_to_ppf( const Bitboard *ppa, char *ppf )
{
    const int bi[] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7 };
    int index = 0;
    const uint64_t one = 1;

    for(int i = 0; i < 64; i++) {
        if(bi[i] % 8 == 0 && bi[i] < 56) ppf[index++] = '/';
        uint64_t bit = (one << bi[i]);
        for(int j = 0; ; j++) {
            if(bit & ppa[j]) {
                if(j) ppf[index++] = PPF_CHESSMAN_LETTERS[j];
                else if(bi[i] % 8 &&
                        (ppf[index - 1] >= '1' && ppf[index - 1] <= '7'))
                    ++ppf[index - 1];
                else ppf[index++] = '1';

                break;
            }
            assert(j < 13);
        } // End inner for
    }

    ppf[index] = '\0';
    return index;
}

static void
x_conditional_shredder_ecaf_to_std_ecaf_conv( char *the_ecaf, const Pos *p )
{
    assert(strcmp(the_ecaf, "----"));

    const char shredder_caf[] = "HAha";
    for(int i = 0; i < 4; i++) {
        if(the_ecaf[i] == '-') continue;
        if(the_ecaf[i] != shredder_caf[i]) return; }

    if((the_ecaf[0] != '-' || the_ecaf[1] != '-') &&
            !(SB.e1 & p->ppa[WHITE_KING]))
        return;
    if((the_ecaf[2] != '-' || the_ecaf[3] != '-') &&
            !(SB.e8 & p->ppa[BLACK_KING]))
        return;

    const char std_caf[] = "KQkq";
    for(int i = 0; i < 4; i++)
        if(the_ecaf[i] != '-') the_ecaf[i] = std_caf[i];
}

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

static void x_fen_to_pos_init_turn_and_ca_flags( Pos *p, const char *acf,
    const char *caf, const char *fen );
static void x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen );
static void x_fen_to_pos_init_epts_file( Pos *p, const char eptsf[] );
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
    if(!fen) fen = INIT_POS;

    bool fc = !sans; // fc, find children
    if(fc) {
        sans = che_move_gen(fen); // This seems to be the root of all evil
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

// TODO: doc
struct fen_game_tree
che_build_fen_gt( const char *fen, uint8_t height )
{
    // What if 'fen' is a stalemate position?

    assert(height < FGT_LO_SIZE);
    if(!fen) fen = INIT_POS;

    struct fen_game_tree gt;

    gt.nc = 0, gt.lo[0] = 1, gt.height = height;
    for(int level = 1; level <= height; level++) {
        gt.nc += che_perft(fen, level - 1, true);
        gt.lo[level] = gt.nc + 1; }
    gt.nc += che_perft(fen, height, true);

    // BHNC, below height node count
    const uint32_t BHNC = gt.lo[height] - 1;
    assert((gt.cc = malloc((BHNC + 1) * sizeof(uint8_t))));
    for(uint32_t id = 1; id <= BHNC; id++) gt.cc[id] = 0;
    assert((gt.children = malloc((BHNC + 1) * sizeof(void *))));
    const int SLOT_COUNT = 50;
    uint8_t *num_alloc_slots = malloc((BHNC + 1) * sizeof(uint8_t));
    for(uint32_t id = 1; id <= BHNC; id++) {
        assert((gt.children[id] = malloc(SLOT_COUNT * sizeof(uint32_t))));
        num_alloc_slots[id] = SLOT_COUNT; }

    assert((gt.parent = malloc((gt.nc + 1) * sizeof(uint32_t))));
    gt.parent[1] = 0;

    char **tmp_fen, **sorted_tmp_fen; // Free!
    assert((tmp_fen = malloc((gt.nc + 1) * sizeof(void *))));
    assert((tmp_fen[1] = malloc(strlen(fen) + 1)));
    strcpy(tmp_fen[1], fen);

    uint32_t cur = 1, vac = 1; // current, vacant
    for(; cur < gt.lo[height]; cur++) {
        char *unmod_ptr = che_children(tmp_fen[cur]),
            *children = unmod_ptr, *child;

        while((child = next_line(&children))) {
            // 'vac' is a child of 'cur'
            assert((tmp_fen[++vac] = malloc(strlen(child) + 1)));
            strcpy(tmp_fen[vac], child);

            if(gt.cc[cur] == num_alloc_slots[cur]) {
                num_alloc_slots[cur] += SLOT_COUNT;
                gt.children[cur] = realloc(gt.children[cur],
                    num_alloc_slots[cur] * sizeof(uint32_t)); }

            gt.parent[vac] = cur, gt.children[cur][gt.cc[cur]] = vac;
            ++gt.cc[cur]; }

        free(unmod_ptr);
    } // End for

    /*
    printf("BEGIN TMP_FEN >>\n");
    for(uint32_t id = 1; id <= gt.nc; id++) printf("%s\n", tmp_fen[id]);
    printf("<< END TMP_FEN\n");
    */

    assert((sorted_tmp_fen = malloc((gt.nc + 1) * sizeof(void *))));
    sorted_tmp_fen[0] = "";
    for(uint32_t id = 1; id <= gt.nc; ++id) {
        sorted_tmp_fen[id] = malloc((strlen(tmp_fen[id]) + 1) * sizeof(char));
        strcpy(sorted_tmp_fen[id], tmp_fen[id]); }
    string_sort(sorted_tmp_fen + 1, gt.nc);
    // gt.num_ufen = gt.nc + 1;
    size_t size = gt.nc + 1;
    gt.ufen = unique_strings(sorted_tmp_fen, &size);
    gt.num_ufen = size - 1;
    // --gt.num_ufen;
    // gt.num_ufen = unique(&gt.ufen, gt.nc + 1, true) - 1;
    assert(!strcmp("", gt.ufen[0]));

    for(uint32_t id = 1; id <= BHNC; id++)
        gt.children[id] = realloc(gt.children[id],
            gt.cc[id] * sizeof(uint32_t));
    free(num_alloc_slots);

    return gt;
}

// TODO: doc
void
che_free_fen_gt( struct fen_game_tree gt )
{
    for(uint32_t id = 1; id <= gt.nc; id++)
        free(gt.ufen[id]);
    free(gt.ufen), free(gt.cc), free(gt.parent);

    for(uint32_t id = 1; id < gt.lo[gt.height]; id++)
        free(gt.children[id]);
    free(gt.children);
}

// TODO: doc
char *che_children( const char *fen )
{
    const Pos *p = fen_to_pos(fen);
    Rawcode *rc = rawcodes(p);
    const int num_rc = *rc;
    int index = 0, prom_count = 0;
    const char piece[] = "-qrbn";
    Pos pos_child;
    char *tmp_children[MAX_LEGAL_MOVE_COUNT + 1];

    for(int i = 1; i <= num_rc; i++) {
        bool prom = is_promotion(p, rc[i]);
        if(prom) ++prom_count;

        for(int j = prom ? 1 : 0; j < (prom ? 5 : 1); j++) {
            copy_pos(p, &pos_child);
            make_move(&pos_child, rc[i], piece[j]);
            tmp_children[++index] = pos_to_fen(&pos_child);
        }
    }

    assert(index == num_rc + 3 * prom_count);

    int alloc_bytes_count = 1; // One byte for terminating null char
    for(int i = 1; i <= index; i++)
        alloc_bytes_count += strlen(tmp_children[i]) + 1; // FEN plus '\n'

    char *children = malloc(alloc_bytes_count * sizeof(char));
    strcpy(children, "");
    for(int i = 1; i <= index; i++)
        strcat(children, tmp_children[i]), strcat(children, "\n");
    assert((int) strlen(children) == alloc_bytes_count - 1);

    for(int i = 1; i <= index; i++) free(tmp_children[i]);
    free((void *) p), free(rc);

    return children;
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

    make_move( p, move, tolower(promotion) );
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
                p->ppa[EMPTY_SQUARE] |= ONE << bi;
        } else {
            switch(*c) {
                case 'K': p->ppa[WHITE_KING  ] |= ONE << bi; break;
                case 'Q': p->ppa[WHITE_QUEEN ] |= ONE << bi; break;
                case 'R': p->ppa[WHITE_ROOK  ] |= ONE << bi; break;
                case 'B': p->ppa[WHITE_BISHOP] |= ONE << bi; break;
                case 'N': p->ppa[WHITE_KNIGHT] |= ONE << bi; break;
                case 'P': p->ppa[WHITE_PAWN  ] |= ONE << bi; break;
                case 'k': p->ppa[BLACK_KING  ] |= ONE << bi; break;
                case 'q': p->ppa[BLACK_QUEEN ] |= ONE << bi; break;
                case 'r': p->ppa[BLACK_ROOK  ] |= ONE << bi; break;
                case 'b': p->ppa[BLACK_BISHOP] |= ONE << bi; break;
                case 'n': p->ppa[BLACK_KNIGHT] |= ONE << bi; break;
                case 'p': p->ppa[BLACK_PAWN  ] |= ONE << bi; break;
                default: assert(false); }
            ++bi; }
    } // End for

    while(*ptr_1++);
    ptr_2 = ptr_1;
    while(*ptr_2++);
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

    char *the_ecaf = get_ecaf(p);
    if(!strcmp(the_ecaf, "----"))
        fen[index++] = '-';
    else {
        x_conditional_shredder_ecaf_to_std_ecaf_conv(the_ecaf, p);
        for(int i = 0; i < 4; i++)
            if(the_ecaf[i] != '-') fen[index++] = the_ecaf[i]; }
    fen[index++] = ' ';

    free(the_ecaf);

    Bitboard the_epts = get_epts(p);
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

    return bit & p->turn_and_ca_flags;
}

// Returns the square bit corresponding to the en passant target square
// of the Pos variable 'p' or zero if there is no EPTS set.
Bitboard
get_epts( const Pos *p )
{
    if(!p->epts_file) return 0;
    return (Bitboard) p->epts_file << (whites_turn(p) ? 40 : 16);
}

// Returns the rawcode corresponding to the 'rawmove' argument which is
// assumed to be a valid rawmove such as "e2e4".
Rawcode
rawcode( const char *rawmove )
{
    char current[4 + 1] = {'\0'};
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
    assert( rawcode >= MIN_RAWCODE );
    assert( rawcode <= MAX_RAWCODE );

    writable[4] = '\0';

    const char *ptr = &APM_DATA[4*(rawcode - 1)];
    for( int i = 0; i < 4; i++ )
        writable[i] = ptr[i];
}

// Makes the move 'rc' in position 'p'. This invariably involves
// modifying or updating 'p'. If the move made is a pawn promotion,
// parameter 'promotion' indicates the piece the pawn gets promoted
// to ('q' for queen, 'r' for rook, 'b' for bishop and 'n' for knight).
// If the move is not a pawn promotion, the value passed to the
// 'promotion' parameter should be the character '-'.
void
make_move( Pos *p, Rawcode rc, char promotion )
{
    bool w = whites_turn(p);

    int orig = RC_ORIG_SQ_BINDEX[rc], dest = RC_DEST_SQ_BINDEX[rc];
    Chessman mover, target;

    const Bitboard ONE = 1;
    Bitboard sq_bit = (ONE << orig);
    for(Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++)
        if(sq_bit & p->ppa[cm]) { mover = cm; break; }
    sq_bit = (ONE << dest);
    for(Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++)
        if(sq_bit & p->ppa[cm]) { target = cm; break; }

    Bitboard the_epts = (p->epts_file ?
        (Bitboard) p->epts_file << (w ? 40 : 16) : 0);
    bool ep = (the_epts == ONE << dest && (mover == WHITE_PAWN ||
        mover == BLACK_PAWN));

    if((mover == WHITE_KING && target == WHITE_ROOK) ||
            (mover == BLACK_KING && target == BLACK_ROOK)) {
        Bitboard castling_rook = (ONE << dest &
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
        } else assert(false);

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

            target = w ? BLACK_PAWN : WHITE_PAWN; }

        // Make the origin square vacant
        p->ppa[mover] ^= ONE << orig, p->ppa[EMPTY_SQUARE] |= ONE << orig;
        // Make the moving chessman "reappear" in the destination square
        p->ppa[mover] |= ONE << dest, p->ppa[target] ^= ONE << dest;

        if(mover == WHITE_KING) {
            if(p->turn_and_ca_flags & 8) p->turn_and_ca_flags ^= 8;
            if(p->turn_and_ca_flags & 4) p->turn_and_ca_flags ^= 4;
        } else if(mover == BLACK_KING) {
            if(p->turn_and_ca_flags & 2) p->turn_and_ca_flags ^= 2;
            if(p->turn_and_ca_flags & 1) p->turn_and_ca_flags ^= 1; }

        if((mover == WHITE_ROOK && ONE << orig == p->irp[1]) ||
                (target == WHITE_ROOK && ONE << dest == p->irp[1])) {
            if(p->turn_and_ca_flags & 8) p->turn_and_ca_flags ^= 8;
        } else if((mover == WHITE_ROOK && ONE << orig == p->irp[0]) ||
                (target == WHITE_ROOK && ONE << dest == p->irp[0])) {
            if(p->turn_and_ca_flags & 4) p->turn_and_ca_flags ^= 4; }

        if((mover == BLACK_ROOK && ONE << orig ==
                ((Bitboard) p->irp[1] << 56)) ||
                (target == BLACK_ROOK && ONE << dest ==
                ((Bitboard) p->irp[1] << 56))) {
            if(p->turn_and_ca_flags & 2) p->turn_and_ca_flags ^= 2;
        } else if((mover == BLACK_ROOK && ONE << orig ==
                ((Bitboard) p->irp[0] << 56)) ||
                (target == BLACK_ROOK && ONE << dest ==
                ((Bitboard) p->irp[0] << 56))) {
            if(p->turn_and_ca_flags & 1) p->turn_and_ca_flags ^= 1; }
    } // End else

    bool pp = ( // pp, pawn promotion
        (mover == WHITE_PAWN && orig >= 48 && orig <= 55 ) ||
        (mover == BLACK_PAWN && orig >=  8 && orig <= 15));

    if(pp) {
        Bitboard pawn = ONE << dest;
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
        (mover >= WHITE_KING && mover <= WHITE_PAWN &&
            target >= BLACK_QUEEN && target <= BLACK_PAWN) ||
        (mover >= BLACK_KING && mover <= BLACK_PAWN &&
            target >= WHITE_QUEEN && target <= WHITE_PAWN) );
    bool sspa = ( // sspa, single step pawn advance
        (mover == WHITE_PAWN && ONE << dest == (ONE << orig << 8)) ||
        (mover == BLACK_PAWN && ONE << dest == (ONE << orig >> 8)));
    bool dspa = ( // dspa, double step pawn advance
        (mover == WHITE_PAWN && ONE << dest == (ONE << orig << 16)) ||
        (mover == BLACK_PAWN && ONE << dest == (ONE << orig >> 16)));

    if(capture || sspa || dspa) p->hmc = 0;
    else p->hmc++;

    if(dspa) p->epts_file = (ONE << orig >> (w ? 48 : 8));
    else p->epts_file = 0;

    if(w) p->fmn++;
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
get_ecaf( const Pos *p )
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

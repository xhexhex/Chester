#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "extra.h"
#include "utils.h"

//
// Static function prototypes
//
static void x_shredder_to_std_fen_conv_handle_len_4_caf( char *fen,
    const char *ecaf, int index );
static void x_shredder_to_std_fen_conv_handle_len_3_caf( char *fen,
    char *ecaf, int index );
static void x_shredder_to_std_fen_conv_handle_len_2_caf( char *fen,
    char *ecaf, int index );
static void x_shredder_to_std_fen_conv_handle_len_1_caf( char *fen,
    const char *ecaf, int index );

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

// Converts the Pos variable pointed to by 'p' to the corresponding FEN
// string. The FEN string is allocated dynamically so free() should be
// called when the string is no longer needed.
//
// This is the old version of pos_to_fen(). The current pos_to_fen()
// is much more efficient.
char *
slow_pos_to_fen( const Pos *p )
{
    char *fen, *fen_field[6];

    char eppf[PPF_MAX_LENGTH + 1];
    ppa_to_eppf( p->ppa, eppf ), fen_field[0] = compress_eppf(eppf);

    fen_field[1] = whites_turn(p) ? "w" : "b";

    char caf[4 + 1] = {'\0'}, *expanded_caf = ecaf(p);
    caf[0] = '-';
    for(int i = 0, j = 0; i < 4; i++)
        if(expanded_caf[i] != '-')
            caf[j++] = expanded_caf[i];
    free(expanded_caf), fen_field[2] = caf;

    fen_field[3] = epts(p) ? (char *) sq_bit_to_sq_name(epts(p)) : "-";

    char hmcf[5 + 1], fmnf[5 + 1];
    sprintf( hmcf, "%d", p->hmc ), sprintf( fmnf, "%d", p->fmn );
    fen_field[4] = hmcf, fen_field[5] = fmnf;

    int fen_length = 5; // The five field-separating spaces
    for(int i = 0; i < 6; i++) fen_length += (int) strlen(fen_field[i]);
    fen = (char *) malloc(fen_length + 1);
    fen[fen_length] = '\0';

    int fen_index = 0;
    for(int i = 0; ; i++) {
        for(int j = 0; j < (int) strlen(fen_field[i]); j++)
            fen[fen_index++] = fen_field[i][j];

        if( i == 5 ) break;
        fen[fen_index++] = ' '; }
    assert(fen_index == fen_length);

    free(fen_field[0]);

    shredder_to_std_fen_conv(fen);
    return fen;
}

// Converts a Shredder-FEN to a standard FEN in a "soft" or conditional
// manner. This means that a conversion only takes place when a king
// with castling rights is on square e1 (e8) and the rooks involved
// are on the squares a1 (a8) or h1 (h8). For example, consider the
// Shredder-FEN "r3kr2/8/8/8/8/8/8/1R2K2R w Ha - 0 123". Both sides have
// castling rights and the kings are on the squares e1 and e8 implying
// the kings are on their original squares. The rooks involved with
// White's kingside and Black's queenside castling are on the squares
// h1 and a8, respectively. The placement of the kings and rooks relevant
// to castling implies that the Shredder-FEN is really a standard FEN
// "in disguise".
//
// The main use of shredder_to_std_fen_conv() is in slow_pos_to_fen(). It
// seems intuitively correct that some_fen == pos_to_fen( fen_to_pos(some_fen) ).
// Among other things this implies that if the input to fen_to_pos() is
// a standard FEN, the output of pos_to_fen() should also be a standard
// FEN.
//
// When considering Chester and standard FENs vs Shredder-FENs, it's good
// to keep in mind that Chester considers the CAF "KQkq" to be a synonym
// for either "HAha" or "AHah".
void
shredder_to_std_fen_conv( char *fen )
{
    char caf[4 + 1] = {'\0'}, ecaf[9 + 1];
    int index = 0, encountered_spaces = 0, caf_length = 0,
        castling_rights_count = 0;

    while( encountered_spaces < 2 )
        if(fen[index++] == ' ') ++encountered_spaces;
    for(int i = 0; fen[index + i] != ' '; i++) ++caf_length;

    for(int i = index; i < index + caf_length; i++)
        caf[i - index] = fen[i];
    assert( strlen(caf) > 0 );
    if( !strcmp(caf, "-") || str_m_pat(caf, "^K?Q?k?q?$") ) return;
    assert( str_m_pat(caf, "^[A-H]?[A-H]?[a-h]?[a-h]?$") );

    EXPAND_CAF(caf, ecaf, fen)
    assert( str_m_pat(ecaf, "^[-A-H][-A-H][-a-h][-a-h]$") );
    for(int i = 0; i < 4; i++) if(ecaf[i] != '-') ++castling_rights_count;

    switch(castling_rights_count) {
        case 4: x_shredder_to_std_fen_conv_handle_len_4_caf(fen, ecaf, index);
            break;
        case 3: x_shredder_to_std_fen_conv_handle_len_3_caf(fen, ecaf, index);
            break;
        case 2: x_shredder_to_std_fen_conv_handle_len_2_caf(fen, ecaf, index);
            break;
        case 1: x_shredder_to_std_fen_conv_handle_len_1_caf(fen, ecaf, index);
            break;
        default: assert(false); }
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

#define SET_EPPF \
    char **ff = fen_fields(fen), eppf[PPF_MAX_LENGTH + 1]; \
    expand_ppf(ff[0], eppf); \
    free_fen_fields(ff);

static bool
x_len_2_caf_needs_to_be_modified( const char *fen, const char *ecaf )
{
    char cr[2 + 1] = {'\0'}; // cr, castling rights
    int crc = 1; // crc, castling rights count
    if( !(ecaf[0] != '-' && ecaf[2] != '-') &&
            !(ecaf[1] != '-' && ecaf[3] != '-') )
        ++crc;

    if(crc == 1) {
        for(int i = 0; ; i++)
            if(ecaf[i] != '-') { cr[0] = tolower(ecaf[i]); break; }
    } else if(crc == 2) {
        for(int i = 0, j = -1; ; i++)
            if(ecaf[i] != '-') {
                cr[++j] = tolower(ecaf[i]);
                if(j == 1) break; }
    } else assert(false);

    if(crc == 2 && cr[0] < cr[1]) swap(cr[0], cr[1], char);

    if( ( crc == 1 && (cr[0] != 'a' && cr[0] != 'h') ) ||
            ( crc == 2 && (cr[0] != 'h' || cr[1] != 'a') ) )
        return false;

    bool white_has_castling_rights = (isupper(ecaf[0]) || isupper(ecaf[1]));

    SET_EPPF
    return (white_has_castling_rights && eppf[67] == 'K') || eppf[4] == 'k';
}

static bool
x_len_3_caf_needs_to_be_modified( const char *fen, const char *ecaf )
{
    char castling_rights[2 + 1] = {'\0'};
    bool use_uppercase_half = (ecaf[0] != '-' && ecaf[1] != '-');

    castling_rights[0] = ecaf[use_uppercase_half ? 0 : 2];
    castling_rights[1] = ecaf[use_uppercase_half ? 1 : 3];

    if(castling_rights[0] < castling_rights[1])
        swap(castling_rights[0], castling_rights[1], char);

    if( tolower(castling_rights[0]) != 'h' ||
            tolower(castling_rights[1]) != 'a' )
        return false;

    SET_EPPF
    return eppf[67] == 'K';
}

static bool
x_len_4_caf_needs_to_be_modified( const char *fen, const char *ecaf )
{
    bool in_alphabetical_order = (ecaf[0] < ecaf[1]);
    char queenside_rook = ecaf[in_alphabetical_order ? 0 : 1],
        kingside_rook = ecaf[in_alphabetical_order ? 1 : 0];

    if( kingside_rook != 'H' || queenside_rook != 'A' ) return false;

    SET_EPPF
    return eppf[67] == 'K';
}

static void
x_shredder_to_std_fen_conv_handle_len_1_caf( char *fen, const char *ecaf, int index )
{
    char cr; // cr, castling right
    for(int i = 0; ; i++) if(ecaf[i] != '-') { cr = ecaf[i]; break; }
    if( tolower(cr) != 'a' && tolower(cr) != 'h' ) return;

    SET_EPPF
    if( (isupper(cr) && eppf[67] != 'K') || (islower(cr) && eppf[4] != 'k') ) return;

    switch(cr) {
        case 'H': fen[index] = 'K'; break;
        case 'A': fen[index] = 'Q'; break;
        case 'h': fen[index] = 'k'; break;
        case 'a': fen[index] = 'q'; break;
        default: assert(false); }
}

#undef SET_EPPF

static void
x_shredder_to_std_fen_conv_handle_len_2_caf( char *fen, char *ecaf, int index )
{
    if( !x_len_2_caf_needs_to_be_modified(fen, ecaf) ) return;

    char caf[2 + 1] = {'\0'};
    int j = -1;
    for(int i = 0; i < 4; i++) if(ecaf[i] != '-') caf[++j] = ecaf[i];
    assert(j == 1), assert(strlen(caf) == 2);

    if( ( str_m_pat(caf, "^[A-H]{2}$") || str_m_pat(caf, "^[a-h]{2}$") ) &&
            caf[0] < caf[1] )
        swap( caf[0], caf[1], char );

    for(int i = 0; i < 2; i++)
        switch(caf[i]) {
            case 'H': caf[i] = 'K'; break;
            case 'A': caf[i] = 'Q'; break;
            case 'h': caf[i] = 'k'; break;
            case 'a': caf[i] = 'q'; break;
            default: assert(false); }

    fen[index] = caf[0], fen[index + 1] = caf[1];
}

static void
x_shredder_to_std_fen_conv_handle_len_3_caf( char *fen, char *ecaf, int index )
{
    if( !x_len_3_caf_needs_to_be_modified(fen, ecaf) ) return;

    bool alphabet_order = (
        (ecaf[0] != '-' && ecaf[1] != '-' && ecaf[0] < ecaf[1]) ||
        (ecaf[2] != '-' && ecaf[3] != '-' && ecaf[2] < ecaf[3]) );
    if(alphabet_order) {
        swap(ecaf[0], ecaf[1], char);
        swap(ecaf[2], ecaf[3], char); }

    const char full_std_caf[] = "KQkq";
    for(int i = 0; i < 4; i++) if(ecaf[i] != '-') ecaf[i] = full_std_caf[i];

    int j = -1;
    for(int i = 0; i < 4; i++ ) {
        if(ecaf[i] == '-') continue;
        ++j; fen[index + j] = ecaf[i]; }
    assert(j == 2);
}

static void
x_shredder_to_std_fen_conv_handle_len_4_caf( char *fen, const char *ecaf, int index )
{
    if( !x_len_4_caf_needs_to_be_modified(fen, ecaf) ) return;

    fen[index] = 'K', fen[index + 1] = 'Q',
        fen[index + 2] = 'k', fen[index + 3] = 'q';
}

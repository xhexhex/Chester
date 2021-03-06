#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <regex.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>

#include "utils.h"
#include "chester.h"
#include "move_gen.h"
#include "validation.h"
#include "extra.h"

static Pos x_state_pos;

static const char *x_sq_navigator_kings_sqs(
    const char *sq_name, enum sq_dir dir );
static const char *x_sq_navigator_knights_sqs(
    const char *sq_name, enum sq_dir dir );
static const char *x_ALT_sq_navigator_kings_sqs(
    const char *sq_name, enum sq_dir dir );
static const char *x_ALT_sq_navigator_knights_sqs(
    const char *sq_name, enum sq_dir dir );
static void x_compress_eppf_rank_dashes_to_digit(
    int *eri_ptr, int cri, const char *eppf_rank, char *compressed_rank );
static void x_expand_caf_convert_std_caf_to_shredder_caf( char *caf );
static void x_expand_caf_sort_2_char_ecaf( char *caf );
static void x_expand_caf_sort_3_char_ecaf( char *caf );
static void x_expand_caf_sort_4_char_ecaf( const char *caf, char *ecaf );
static int x_expand_caf_find_missing_char_index( const char *caf );
static void x_expand_caf_handle_2_char_caf_cases(
    char *ecaf, const char *first, const char *second );
static void x_expand_caf_handle_1_char_caf_cases( char *ecaf, const char *cptr );
static bool x_king_in_dir(
    const char *fen, enum sq_dir dir, bool color_is_white, char rooks_file );
static int x_qsort_string_compare( const void *a, const void *b );
static long long x_gt_node_count( uint8_t depth );
static int x_che_remove_redundant_epts_set_index( char *single_fen );
static void x_che_remove_redundant_epts_set_bit_indices( int *epts_bi,
    int *western_pawn_bi, int *eastern_pawn_bi, const Pos *p, const char *eptsf );
static void x_che_remove_redundant_epts_remove_epts( char *fen, int index,
    size_t *rfc );
static bool x_che_remove_redundant_epts_king_not_forsaken( int pawn_bindex,
    int epts_bindex, const Pos *p );
static struct naive_bst_node *x_recursive_insert_into_naive_bst(
    struct naive_bst_node *node, const char *key );

/******************************
 ****                      ****
 ****  External functions  ****
 ****                      ****
 ******************************/

// Returns the number of nodes/positions/states in the game tree with
// the root 'fen' and height 'height'. If 'fen' is null, then the initial
// position is assumed.
//
// A few initial position game tree sizes:
// * che_gt_node_count(NULL, 0) == 1
// * che_gt_node_count(NULL, 1) == 1 + 20
// * che_gt_node_count(NULL, 2) == 1 + 20 + 400
// * che_gt_node_count(NULL, 3) == 1 + 20 + 400 + 8902
//                              == perft(0) + perft(1) + perft(2) + perft(3)
long long
che_gt_node_count( const char *fen, uint8_t height )
{
    if(!fen) fen = INIT_POS;

    const Pos *p = fen_to_pos(fen);
    copy_pos(p, &x_state_pos);
    free((void *) p);

    long long nc = 1; // Node count
    nc += x_gt_node_count(height);

    return nc;
}

// Used to remove a redundant en passant target square of a FEN string.
// In Chester the EPTS is set whenever a pawn moves two squares forward.
// It's therefore possible for the EPTS to be set in a position where an
// en passant move isn't available. Here's a couple of examples of such
// positions:
// "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
// "b3kr2/8/8/3PpP2/8/5K2/8/8 w - e6 0 2"
//
// As neither position has a pawn that can (legally) do an en passant
// capture, the positions are equivalent to
// "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"  and
// "b3kr2/8/8/3PpP2/8/5K2/8/8 w - - 0 2".
//
// The parameter 'fen' is a string array that contains 'count' FEN strings.
// The function returns the number of removed en passant target squares.
// Note that removing an EPTS means replacing a square name such as "e3"
// with the dash character '-'.
//
// The string elements of 'fen' are assumed to be in writable memory. Any
// string element of 'fen' that contains a redundant EPTS will get modified
// during the call to the function.
size_t
che_remove_redundant_epts( char **fen, const size_t count )
{
    size_t rfc = 0; // rfc, removed (EPTS) field count
    char eptsf[3] = {'\0'};

    for(size_t i = 1; i <= count; ++i, ++fen) {
        // Determine the index of the first character of the EPTS field.
        int index = x_che_remove_redundant_epts_set_index(*fen);
        if((*fen)[index] == '-') continue;

        eptsf[0] = (*fen)[index], eptsf[1] = (*fen)[index + 1];
        int epts_bi, western_pawn_bi, eastern_pawn_bi;
        Pos *p = fen_to_pos(*fen);
        x_che_remove_redundant_epts_set_bit_indices(
            &epts_bi, &western_pawn_bi, &eastern_pawn_bi, p, eptsf);

        if(western_pawn_bi == -1 && eastern_pawn_bi == -1) {
            x_che_remove_redundant_epts_remove_epts(*fen, index, &rfc);
            free(p);
            continue; }

        if(western_pawn_bi > -1 &&
            x_che_remove_redundant_epts_king_not_forsaken(
                western_pawn_bi, epts_bi, p)) { free(p); continue; }
        if(eastern_pawn_bi > -1 &&
            x_che_remove_redundant_epts_king_not_forsaken(
                eastern_pawn_bi, epts_bi, p)) { free(p); continue; }

        x_che_remove_redundant_epts_remove_epts(*fen, index, &rfc);

        free(p);
    } // End for

    return rfc;
}

// Allocate and initialize an instance of struct naive_binary_search_tree
// that corresponds to an empty "naive" binary search tree. The new tree
// can hold a maximum of 'node_limit' nodes.
struct naive_binary_search_tree *
init_naive_bst( uint32_t node_limit )
{
    struct naive_binary_search_tree *nbst =
        malloc(sizeof(struct naive_binary_search_tree));

    nbst->root = NULL;
    nbst->node_ptr = malloc(node_limit * sizeof(void *));
    nbst->height = nbst->node_count = 0;
    nbst->node_limit = node_limit;

    return nbst;
}

// Free the memory that was dynamically allocated for the 'nbst' parameter.
void
destroy_naive_bst( struct naive_binary_search_tree *nbst )
{
    for(uint32_t i = 0; i < nbst->node_count; ++i) {
        free(nbst->node_ptr[i]->key);
        free(nbst->node_ptr[i]->data);
        free(nbst->node_ptr[i]); }

    free(nbst->node_ptr), free(nbst);
}

// The parameter 'root' is (on the first call of the function) assumed
// to be the root of a "naive" binary search tree. The function returns
// a pointer to the node in the tree that has a key equal to the 'key'
// parameter. If no such key is found the function returns NULL.
struct naive_bst_node *
search_naive_bst( struct naive_bst_node *root, const char *key )
{
    // Base cases: the current node 'root' is null or the search key
    // 'key' is present at 'root'.
    if(root == NULL || !strcmp(key, root->key))
        return root;
    // The search key 'key' is greater than the key stored in
    // the node 'root'.
    if(strcmp(key, root->key) > 0)
        return search_naive_bst(root->right, key);
    // The search key 'key' is less than the key stored in
    // the node 'root'.
    return search_naive_bst(root->left, key);
}

static uint8_t x_current_level_in_tree, x_tree_height;

// The means of inserting nodes into a "naive" binary search tree. A new
// node is inserted into the tree 'nbst' with the values specified by the
// 'key' and 'data' parameters. It is assumed that the tree does not already
// contain a node that has a key equal to 'key'.
void
insert_into_naive_bst( struct naive_binary_search_tree *nbst,
    const char *key, const char *data )
{
    x_current_level_in_tree = UINT8_MAX, x_tree_height = nbst->height;
    struct naive_bst_node *root_node =
        x_recursive_insert_into_naive_bst(nbst->root, key);
    nbst->height = x_tree_height;

    if(!nbst->root) nbst->root = root_node;

    struct naive_bst_node *inserted_node =
        search_naive_bst(nbst->root, key);
    inserted_node->data = malloc(strlen(data) + 1);
    strcpy(inserted_node->data, data);

    nbst->node_ptr[nbst->node_count++] = inserted_node;
}

// str_m_pat as in "string matches pattern". Returns true if 'str' matches the
// extended regular expression 'pat'.
bool
str_m_pat( const char *str, const char *pat )
{
    regex_t regex;

    // Compile the regular expression
    int result = regcomp( &regex, pat, REG_EXTENDED );
    assert( !result ); // Something went wrong in compiling regex

    // Testing if 'str' matches the regular expression in regex.
    // Note this: "You must match the regular expression with the same
    // set of current locales that were in effect when you compiled the
    // regular expression."
    // https://www.gnu.org/software/libc/manual/html_node/Matching-POSIX-Regexps.html
    result = regexec( &regex, str, 0, NULL, 0 );
    regfree(&regex);

    // Making sure regexec() didn't run into problems
    assert( !result || result == REG_NOMATCH );

    return !result;
} // Review: 2018-03-30

// Replaces digits in a PPF rank string with a corresponding number of
// dashes. For example, "1p2p3" gets converted to "-p--p---". Note that
// the length of the original rank string is *at most* eight chars,
// whereas the length of the new string (the one with dashes) is always
// eight. It is therefore assumed that the writable array 'expanded_rank'
// is at least 8 + 1 bytes long.
void
expand_ppf_rank( const char *ppf_rank, char *expanded_rank )
{
    expanded_rank[ 8 ] = '\0';
    for( int i = 0; i < 8; i++ )
        expanded_rank[ i ] = '-'; // A dash means an empty square

    int i = 0, j = 0;
    while( ppf_rank[ i ] ) {
        if( !isdigit( ppf_rank[ i ] ) ) { // Copying non-digit chars into the empty rank
            expanded_rank[ j ] = ppf_rank[ i ];
            j++;
        }
        else j += ( ppf_rank[ i ] - 48 ); // '1' - 48 = 1, '8' - 48 = 8

        i++;
    }

    assert( j == 8 );
    assert( !expanded_rank[ j ] );
    assert( strlen( expanded_rank ) == 8 );
} // Review: 2018-01-05

// Does the opposite of expand_ppf_rank(). The length of a PPF rank is between one
// to eight chars, and thus the writable array 'compressed_rank' is assumed to be
// at least 8 + 1 chars long.
void
compress_eppf_rank( const char *eppf_rank, char *compressed_rank )
{
    int cri = 0; // cri, compressed rank index

    for( int eri = 0; eri < 8; eri++, cri++ ) { // eri, EPPF rank index
        char c = eppf_rank[ eri ];
        if( c == 'K' || c == 'Q' || c == 'R' || c == 'B' || c == 'N' || c == 'P' ||
            c == 'k' || c == 'q' || c == 'r' || c == 'b' || c == 'n' || c == 'p'
        ) compressed_rank[ cri ] = c;
        else if( c == '-' ) x_compress_eppf_rank_dashes_to_digit(
            &eri, cri, eppf_rank, compressed_rank );
        else assert( false );
    }

    compressed_rank[ cri ] = '\0';

    assert( strlen( compressed_rank ) >= 1 && strlen( compressed_rank ) <= 8 );
    assert( str_m_pat( compressed_rank, "^[KQRBNPkqrbnp12345678]*$" ) );
}

// Returns the type of chessman on square 'sq_bit' in position 'p'. For
// example, if we assume 'p' represents the standard starting position,
// then the call occupant_of_sq(p, SB.e1) would return WHITE_KING.
Chessman
occupant_of_sq( const Pos *p, Bitboard sq_bit )
{
    for( Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++ )
        if( sq_bit & p->ppa[cm] )
            return cm;

    assert(false);
    return -1;
} // Review: 2018-06-20

// Returns the chessboard file of the square bit argument 'sq_bit'.
// For example, file_of_sq(SB.e4) returns 'e'.
char
file_of_sq( Bitboard sq_bit )
{
    // assert( is_sq_bit(sq_bit) );

    for( char file_letter = 'a'; file_letter <= 'h'; file_letter++ )
        if( sq_bit & file(file_letter) )
            return file_letter;

    assert(false);
    return 0;
}

// Returns the chessboard rank of the square bit argument 'sq_bit'.
// For example, rank_of_sq(SB.e4) returns '4'.
char
rank_of_sq( Bitboard sq_bit )
{
    // assert( is_sq_bit(sq_bit) );

    for( char rank_number = '1'; rank_number <= '8'; rank_number++ )
        if( sq_bit & rank(rank_number) )
            return rank_number;

    assert(false);
    return 0;
}

// Returns the bit index of the bit in the square bit argument 'bit'.
// The bit index (of a Bitboard) is an integer in the range [0,63]. Note
// that the bit index is also the exponent 'k' in "bit = 2^k". It is
// assumed that exactly one bit is set in 'bit'.
int
bindex(Bitboard bit)
{
    int bit_index = -1;
    while(bit) bit >>= 1, ++bit_index;
    return bit_index;
}

// Converts the argument into a "binary string", that is, a string
// consisting of 64 '0' and '1' chars
char *
uint64_to_bin_str( uint64_t uint64 )
{
    char *bin_str = (char *) malloc( 64 + 1 );

    *( bin_str + 64 ) = 0; // Null char at the end of string
    for( int i = 0; i < 64; i++ ) {
        *( bin_str + i ) = ( uint64 & SBA[ 63 - i ] ) ? '1' : '0';
    }

    assert( strlen( bin_str ) == 64 );

    return bin_str;
}

// REMOVE OR RELOCATE!
//
// Prints a Pos var to stdout with each uint64 represented as a binary string
void
print_pos_var( const Pos *p )
{
    char* bin_str;

    for( Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++ ) {
        bin_str = uint64_to_bin_str( p->ppa[ cm ] );
        printf( "%s    %c\n", bin_str, PPF_CHESSMAN_LETTERS[cm] );
        free( bin_str );
    }

    // bin_str = uint64_to_bin_str( p->info );
    // printf( "\n%s\n", bin_str );
    // free( bin_str );
}

// Returns the name of the square that is in direction 'dir' of
// square 'sq_name'. For example, the call sq_navigator( "e4", NE ) would
// return "f5" and the call sq_navigator( "e4", ONE_OCLOCK ) would
// return "f6".
const char *
sq_navigator( const char *sq_name, enum sq_dir dir )
{
    assert( is_sq_name( sq_name ) );

    if( dir >= NORTH && dir <= NORTHWEST )
        return x_sq_navigator_kings_sqs( sq_name, dir );

    return x_sq_navigator_knights_sqs( sq_name, dir );
}

// A different implementation of sq_navigator(). Used for testing purposes.
// Should produce exactly the same results as sq_navigator().
const char *
ALT_sq_navigator( const char *sq_name, enum sq_dir dir )
{
    assert( is_sq_name( sq_name ) );

    if( dir >= NORTH && dir <= NORTHWEST )
        return x_ALT_sq_navigator_kings_sqs( sq_name, dir );

    return x_ALT_sq_navigator_knights_sqs( sq_name, dir );
}

// Returns the number of set bits in 'bb'.
int
bit_count( Bitboard bb )
{
    int num_of_set_bits = 0;
    for(int i = 0; i < 64; ++i) if(bb & ONE << i) ++num_of_set_bits;
    return num_of_set_bits;
}

// Does the opposite of expand_ppf(). The function returns a pointer to a dynamically
// allocated string which is the result of "compressing" the 'eppf' argument. It's
// the caller's responsibility to release the allocated memory with free().
char *
compress_eppf( const char *eppf )
{
    assert( strlen( eppf ) == (size_t) PPF_MAX_LENGTH );
    char *ppf = (char *) malloc( PPF_MAX_LENGTH + 1 );
    assert( ppf );

    char eppf_rank[ 8 + 1 ], ppf_rank[ 8 + 1 ];

    int pi = 0, eri = 0; // Indexes for 'ppf', 'eppf_rank'
    for( int i = 0; i <= (int) PPF_MAX_LENGTH; i++ ) {
        char c = eppf[ i ];
        if( !c || c == '/' ) {
            eppf_rank[ eri ] = '\0';
            assert( eri == 8 && strlen( eppf_rank ) == 8 );
            eri = 0;
            compress_eppf_rank( eppf_rank, ppf_rank );
            assert( strlen( ppf_rank ) >= 1 && strlen( ppf_rank ) <= 8 );
            for( int i = 0; ppf_rank[ i ]; i++ ) ppf[ pi++ ] = ppf_rank[ i ];
            ppf[ pi++ ] = c ? '/' : '\0';
        } else {
            eppf_rank[ eri ] = c;
            ++eri;
        }
    }

    size_t ppf_size = strlen( ppf );
    if( !( ppf = realloc( ppf, ppf_size + 1 ) ) ) assert( false );
    assert( ppf_size >= PPF_MIN_LENGTH && ppf_size <= PPF_MAX_LENGTH );
    return ppf;
}

// Expands the 'ppf' argument and stores the result in 'eppf'. The size of an
// expanded PPF is always PPF_MAX_LENGTH, and thus 'eppf' is assumed to point
// to a writable array of at least PPF_MAX_LENGTH + 1 bytes. Expanding a PPF
// means replacing each digit with the corresponding number of dashes. The following
// is an example of a PPF in both standard ("compressed") and expanded form.
//
// rn2kb1r/ppp1qppp/5n2/4p3/2B1P3/1Q6/PPP2PPP/RNB1K2R
// rn--kb-r/ppp-qppp/-----n--/----p---/--B-P---/-Q------/PPP--PPP/RNB-K--R
void
expand_ppf( const char *ppf, char *eppf )
{
    eppf[ PPF_MAX_LENGTH ] = '\0';
    for( int i = 8; i <= 62; i += 9 ) eppf[ i ] = '/';

    char ppf_rank[ 8 + 1 ], eppf_rank[ 8 + 1 ];
    const char *ppf_ptr = ppf;
    int ei = 0, pri = 0; // Indexes for 'eppf' and 'ppf_rank'
    do {
        char c = *ppf_ptr;
        if( !c || c == '/' ) {
            ppf_rank[ pri ] = '\0';
            pri = 0;
            assert( strlen( ppf_rank ) >= 1 && strlen( ppf_rank ) <= 8 );
            expand_ppf_rank( ppf_rank, eppf_rank );
            assert( strlen( eppf_rank ) == 8 );
            for( int i = 0; i < 8; i++ ) eppf[ ei++ ] = eppf_rank[ i ];
            ++ei;
        } else {
            ppf_rank[ pri ] = c;
            ++pri;
        }
    } while( *ppf_ptr++ );

    assert( strlen( eppf ) == (size_t) PPF_MAX_LENGTH );
}

// Convert piece placement data from PPA (piece placement array, the 'ppa'
// member of the Pos type) to expanded PPF format. The following is an
// example of a PPF in both standard ("compressed") and expanded form.
//
// rn2kb1r/ppp1qppp/5n2/4p3/2B1P3/1Q6/PPP2PPP/RNB1K2R
// rn--kb-r/ppp-qppp/-----n--/----p---/--B-P---/-Q------/PPP--PPP/RNB-K--R
//
// It is assumed that parameter 'eppf' is a writable array with a size of
// at least PPF_MAX_LENGTH + 1 bytes.
void
ppa_to_eppf( const Bitboard ppa[], char *eppf )
{
    int eppf_index = -1;

    for( int i = 56; i >= 0; i -= 8 ) {
        for( int j = 0; j <= 7; j++ ) {
            int bi = i + j;
            Bitboard sq_bit = SBA[bi];
            eppf_index += ( !(bi % 8) && bi != 56 ) ? 2 : 1;

            if(      sq_bit & ppa[EMPTY_SQUARE] ) eppf[eppf_index] = '-';
            else if( sq_bit & ppa[WHITE_KING]   ) eppf[eppf_index] = 'K';
            else if( sq_bit & ppa[WHITE_QUEEN]  ) eppf[eppf_index] = 'Q';
            else if( sq_bit & ppa[WHITE_ROOK]   ) eppf[eppf_index] = 'R';
            else if( sq_bit & ppa[WHITE_BISHOP] ) eppf[eppf_index] = 'B';
            else if( sq_bit & ppa[WHITE_KNIGHT] ) eppf[eppf_index] = 'N';
            else if( sq_bit & ppa[WHITE_PAWN]   ) eppf[eppf_index] = 'P';
            else if( sq_bit & ppa[BLACK_KING]   ) eppf[eppf_index] = 'k';
            else if( sq_bit & ppa[BLACK_QUEEN]  ) eppf[eppf_index] = 'q';
            else if( sq_bit & ppa[BLACK_ROOK]   ) eppf[eppf_index] = 'r';
            else if( sq_bit & ppa[BLACK_BISHOP] ) eppf[eppf_index] = 'b';
            else if( sq_bit & ppa[BLACK_KNIGHT] ) eppf[eppf_index] = 'n';
            else if( sq_bit & ppa[BLACK_PAWN]   ) eppf[eppf_index] = 'p';
            else assert( false );
        }
    }

    for( int i = 8; i <= 62; i += 9 ) eppf[i] = '/';
    eppf[PPF_MAX_LENGTH] = '\0';
} // Review: 2018-06-04

// The inverse of ppa_to_eppf(). Keep in mind that the writable Bitboard
// array 'ppa' should have space for at least 13 elements.
void
eppf_to_ppa( const char eppf[], Bitboard *ppa )
{
    for( int i = 0; i < 13; i++ ) ppa[i] = 0;

    for( int i = 63, bi = 0; i >= 0; i -= 9 ) {
        for( int j = 0; j <= 7; j++, bi++ ) {
            int eppf_index = i + j;
            Bitboard sq_bit = SBA[bi];

            if(      eppf[eppf_index] == '-' ) ppa[EMPTY_SQUARE] |= sq_bit;
            else if( eppf[eppf_index] == 'K' ) ppa[WHITE_KING]   |= sq_bit;
            else if( eppf[eppf_index] == 'Q' ) ppa[WHITE_QUEEN]  |= sq_bit;
            else if( eppf[eppf_index] == 'R' ) ppa[WHITE_ROOK]   |= sq_bit;
            else if( eppf[eppf_index] == 'B' ) ppa[WHITE_BISHOP] |= sq_bit;
            else if( eppf[eppf_index] == 'N' ) ppa[WHITE_KNIGHT] |= sq_bit;
            else if( eppf[eppf_index] == 'P' ) ppa[WHITE_PAWN]   |= sq_bit;
            else if( eppf[eppf_index] == 'k' ) ppa[BLACK_KING]   |= sq_bit;
            else if( eppf[eppf_index] == 'q' ) ppa[BLACK_QUEEN]  |= sq_bit;
            else if( eppf[eppf_index] == 'r' ) ppa[BLACK_ROOK]   |= sq_bit;
            else if( eppf[eppf_index] == 'b' ) ppa[BLACK_BISHOP] |= sq_bit;
            else if( eppf[eppf_index] == 'n' ) ppa[BLACK_KNIGHT] |= sq_bit;
            else if( eppf[eppf_index] == 'p' ) ppa[BLACK_PAWN]   |= sq_bit;
            else assert( false );
        }
    }
} // Review: 2018-06-04

// Returns the six fields of the 'fen' parameter as an array of strings.
// The first element of the array is the PPF and the last the FMNF.
// Remember to call free_fen_fields() after the string array returned by
// fen_fields() is no longer needed.
char **
fen_fields( const char *fen )
{
    char **ff = (char **) malloc( 6 * sizeof( char * ) ),
        *ppf = (char *) malloc( PPF_MAX_LENGTH + 1 ),
        *acf = (char *) malloc( 1 + 1 ), *caf   = (char *) malloc( 4 + 1 ),
        *eptsf = (char *) malloc( 2 + 1 ), *hmcf  = (char *) malloc( 5 + 1 ),
        *fmnf  = (char *) malloc( 5 + 1 );
    assert( ff && ppf && acf && caf && eptsf && hmcf && fmnf );
    ff[0] = ppf, ff[1] = acf, ff[2] = caf,
        ff[3] = eptsf, ff[4] = hmcf, ff[5] = fmnf;

    for(int i = 0, j = 0, fen_index = 0; i < 6; i++) {
        char c;
        while( true ) {
            c = fen[ fen_index++ ];
            if( c && c != ' ' ) ff[ i ][ j++ ] = c;
            else break;
        }

        ff[i][j] = '\0', j = 0;
        ff[i] = realloc( ff[i], strlen( ff[i] ) + 1 );
        assert( ff[i] );
    }

    size_t l1 = strlen( ff[0] ), l2 = strlen( ff[1] ), l3 = strlen( ff[2] ),
        l4 = strlen( ff[3] ), l5 = strlen( ff[4] ), l6 = strlen( ff[5] );
    assert( l1 >= PPF_MIN_LENGTH && l1 <= PPF_MAX_LENGTH && l2 == 1 &&
        l3 >= 1 && l3 <= 4 && l4 >= 1 && l4 <= 2 && l5 >= 1 && l5 <= 5 &&
        l6 >= 1 && l6 <= 5 );

    return ff;
} // Review: 2018-05-31

// Releases the memory reserved for the dynamically allocated string
// array 'ff'. Should be called when the string array returned by
// fen_fields() is no longer needed.
void
free_fen_fields( char **ff )
{
    for( int i = 0; i < 6; i++ ) free( ff[ i ] );

    free( ff );
} // Review: 2018-05-31

// Expands 'caf' and stores the result in 'ecaf' (which is assumed to be a writable
// array of at least ten bytes). Expanding a CAF means adding dashes where there
// are "missing" characters and arranging the letters so that uppercase comes
// before lowercase and that letters of the same case are in alphabetical order.
// A few examples:
//
// "HAh" --> "AH-h"      "Hah" --> "-Hah"      "bd" --> "--bd"
// "Eg"  --> "E--g"      "CA"  --> "AC--"      "Hb" --> "-Hb-"
//
// In some cases the meaning of 'caf' is ambiguous. For example, if 'caf' is "E",
// it's not clear whether it means a-side or h-side castling availability for White.
// If it were known that "E" meant a-side CA for White, the conversion would be
// "E" --> "E---", and if it were known that "E" meant h-side CA for White,
// the conversion would be "E" --> "-E--". The solution to the ambiguity is to
// include both of the conversions in 'ecaf' with the a-side version placed before
// the h-side version. A few examples:
//
// "E" --> "E--- -E--"      "c" --> "--c- ---c"      "Dd" --> "D-d- -D-d"
//
// The caller can tell if the result is ambiguous by inspecting the length of
// 'ecaf'. The function resolve_ambiguous_ecaf() can be used to resolve the
// ambiguity. The most convenient means of CAF expansion is the macro EXPAND_CAF
// which includes a call to both expand_caf() and resolve_ambiguous_ecaf().
//
// If 'caf' is a standard FEN CAF such as "KQkq" or "k", it is converted into
// a Shredder-FEN before expansion. (Remember that Chester considers "KQkq" to be
// a synonym for "AHah" or "HAha".)
//
// 'caf' is assumed to be a *valid* standard FEN or Shredder-FEN CAF.
void
expand_caf( const char *caf, char *ecaf )
{
    for( int i = 0; i < 10; i++ ) ecaf[ i ] = '\0';
    char caf_c[ 4 + 1 ]; // 'caf' copy
    strcpy( caf_c, caf );
    x_expand_caf_convert_std_caf_to_shredder_caf( caf_c );

    size_t len = strlen( caf_c );
    assert( len >= 1 && len <= 4 );
    if( len == 4 ) {
        x_expand_caf_sort_4_char_ecaf( caf_c, ecaf );
    } else if( len == 3 ) {
        x_expand_caf_sort_3_char_ecaf( caf_c );
        int mci = x_expand_caf_find_missing_char_index( caf_c );
        ecaf[ mci ] = '-';
        int i = 0, j = -1;
        while( i < 3 ) if( ++j != mci ) ecaf[ j ] = caf_c[ i++ ];
    } else if( len == 2 ) {
        x_expand_caf_sort_2_char_ecaf( caf_c );
        char *first = &caf_c[ 0 ], *second = &caf_c[ 1 ];
        x_expand_caf_handle_2_char_caf_cases( ecaf, first, second );
    } else {
        char *cptr = &caf_c[ 0 ];
        x_expand_caf_handle_1_char_caf_cases( ecaf, cptr ); }
}

// Resolves any ambiguous expanded CAF produced by expand_caf(). For example, if
// 'ecaf' is "D-d- -D-d" and 'fen' is "2kr4/8/8/8/8/8/8/2KR4 b Dd - 0 60", the result
// (the updated content of 'ecaf') will be "-D-d". If the ambiguity cannot be resolved
// due to an invalid value in 'fen', 'ecaf' (containing an ambiguous expanded CAF) will
// remain unchanged by the function. See expand_caf() documentation for more details.
void
resolve_ambiguous_ecaf( char *ecaf, const char *fen )
{
    // The string length of an unambiguous expanded CAF is four
    if( strlen( ecaf ) != 9 ) return;

    char rooks_file;
    for( int i = 0; i < 4; i++ ) if( ecaf[ i ] != '-' ) {
        rooks_file = tolower( ecaf[ i ] );
        break; }

    bool ecaf_modified = true;
    if( ecaf[ 0 ] != '-' || ecaf[ 1 ] != '-' ) {
        if( x_king_in_dir( fen, EAST, true, rooks_file ) ) {
            ecaf[ 4 ] = '\0';
        } else if( x_king_in_dir( fen, WEST, true, rooks_file ) ) {
            for( int i = 0; i < 5; i++ ) ecaf[ i ] = ecaf[ i + 5 ];
        } else ecaf_modified = false;
    } else if( ecaf[ 2 ] != '-' || ecaf[ 3 ] != '-' ) {
        if( x_king_in_dir( fen, EAST, false, rooks_file ) ) {
            ecaf[ 4 ] = '\0';
        } else if( x_king_in_dir( fen, WEST, false, rooks_file ) ) {
            for( int i = 0; i < 5; i++ ) ecaf[ i ] = ecaf[ i + 5 ];
        } else ecaf_modified = false;
    }

    assert( !ecaf_modified || str_m_pat( ecaf, "^[-ABCDEFGHabcdefgh]{4}$" ) );
}

// Examines the PPF of 'fen' and returns the letter symbol of the chessman
// on square 'sq'. If the square is empty, then '-' is returned. For example,
// the call occupant_of_sq_fen_v( INIT_POS, "e1" ) would return 'K'.
// Note that occupant_of_sq_fen_v() is the FEN string equivalent (or version)
// of occupant_of_sq().
char
occupant_of_sq_fen_v( const char *fen, const char *sq )
{
    assert( is_sq_name( sq ) );
    char **ff = fen_fields(fen);
    assert(ff);

    int index = 0;
    char eppf[PPF_MAX_LENGTH + 1], file = sq[0], rank = sq[1];
    index += 9 * ('8' - rank);
    index += file - 'a';

    expand_ppf( ff[0], eppf );
    char occupant = eppf[index];

    bool valid_occupant = false;
    for( int i = 0; PPF_CHESSMAN_LETTERS[i]; i++ ) {
        if( occupant == PPF_CHESSMAN_LETTERS[i] ) {
            valid_occupant = true;
            break;
        }
    }
    assert( valid_occupant );

    free_fen_fields(ff);
    return occupant;
}

// Returns a bitboard of the squares between squares 'sq_1' and 'sq_2'.
// For example, the calls in_between(SB.c3,SB.f6) or in_between(SB.f6,SB.c3)
// return (SB.d4 | SB.e5) and the calls in_between(SB.d1,SB.f1) or
// in_between(SB.f1,SB.d1) return SB.e1. If the squares 'sq_1' and 'sq_2'
// are either the same or adjacent, then the function returns zero. If
// 'sq_1' and 'sq_2' are not on the same file, rank or (anti)diagonal,
// then the function returns a bitboard with all the 64 bits set.
Bitboard
in_between( Bitboard sq_1, Bitboard sq_2 )
{
    // assert( is_sq_bit(sq_1) );
    // assert( is_sq_bit(sq_2) );

    if( sq_1 == sq_2 ) return 0;

    for(enum sq_dir dir = NORTH; dir <= NORTHWEST; dir++) {
        Bitboard sq = sq_1, bb = 0;
        while((sq = SQ_NAV[bindex(sq)][dir])) {
            if(sq == sq_2) return bb;
            bb |= sq;
        }
    }

    return UINT64_MAX;
}

// Copies the Pos variable 'p' into 'copy'. After the operation all the
// fields of 'p' and 'copy' should be the same.
void
copy_pos( const Pos *p, Pos *copy )
{
    for( Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++ )
        copy->ppa[cm] = p->ppa[cm];

    copy->turn_and_ca_flags = p->turn_and_ca_flags;
    copy->irp[0] = p->irp[0];
    copy->irp[1] = p->irp[1];
    copy->epts_file = p->epts_file;
    copy->hmc = p->hmc;
    copy->fmn = p->fmn;
}

// Returns the bitboard corresponding to the chessboard file indicated
// by 'letter'. For example, the call file('e') returns a bitboard that
// has each of the eight squares of the e-file set. The 'letter' argument
// should be a lowercase character between 'a' and 'h'.
Bitboard
file( char letter )
{
    /*
    assert( letter >= 'a' && letter <= 'h' );
    Bitboard file_bb =
        (SB.a1 | SB.a2 | SB.a3 | SB.a4 | SB.a5 | SB.a6 | SB.a7 | SB.a8);
    */

    Bitboard file_bb = 0x101010101010101ULL;
    return file_bb << (letter - 'a');
}

// Analogous to function file(). Argument 'number' (the indicator of the
// chessboard rank) should be a character between '1' and '8'.
Bitboard
rank( char number )
{
    // assert( number >= '1' && number <= '8' );

    Bitboard rank_bb =
        (SB.a1 | SB.b1 | SB.c1 | SB.d1 | SB.e1 | SB.f1 | SB.g1 | SB.h1);
    return rank_bb << ( 8*(number - '1') );
}

// Uses the information in 'p' and 'move' to set the values of the four
// pointer arguments 'mover', 'target', 'orig' and 'dest'. For example,
// if 'p' corresponds to the standard starting position and 'move' to
// the move 1.e4, then the four pointer arguments will be set as follows:
// *mover = WHITE_PAWN, *target = EMPTY_SQUARE, *orig = 12, *dest = 28.
// The 'orig' and 'dest' arguments mean the bit indexes of the square bits
// that indicate the origin and destination squares of the 'move' argument.
void
set_mover_target_orig_and_dest( const Pos *p, Rawcode move, Chessman *mover,
    Chessman *target, int *orig, int *dest )
{
    *orig = RC_ORIG_SQ_BINDEX[move], *dest = RC_DEST_SQ_BINDEX[move];
    *mover = occupant_of_sq( p, SBA[*orig] );
    *target = occupant_of_sq( p, SBA[*dest] );
}

// Understanding the purpose of the function and the way it works is best
// illustrated with an example.
//
// After the following three lines of code the char pointer 'lines' points
// to the first character of the string "this\nis a\ntest\n" in writable
// memory.
// >> char str[] = "this\nis a\ntest\n";
// >> char *unmod_ptr = (char *) malloc(strlen(str) + 1), *lines = unmod_ptr;
// >> strcpy(lines, str);
//
// The following assignments and assertions should give an idea of what's
// going on.
// >> line = next_line(&lines); // FIRST call
// >> ck_assert(!strcmp(line, "this") && !strcmp(lines, "is a\ntest\n"));
// >> line = next_line(&lines); // SECOND call
// >> ck_assert(!strcmp(line, "is a") && !strcmp(lines, "test\n"));
// >> line = next_line(&lines); // THIRD call
// >> ck_assert(!strcmp(line, "test") && !strcmp(lines, ""));
// >> line = next_line(&lines); // FOURTH call
// >> ck_assert(line == NULL && !strcmp(lines, ""));
// >> free(unmod_ptr);
//
// So the purpose of the function is to extract one line at a time from
// a sequence of lines. It makes no difference whether or not the last
// line ends in a newline. It's important to remember that calls to the
// function are "destructive" in two ways, what comes to the 'lines'
// argument: (1) The caller's char pointer argument gets modified
// (note the pass-by-reference) and (2) on each call to the function a
// newline is replaced with a null character in the text pointed to by
// the pointer obtained by dereferencing 'lines'.
//
// It is assumed that the sequence of lines contain no empty lines. In
// other words the assumption is that there are no consecutive newlines
// in the text.
char *
next_line( char **lines )
{
    assert(lines), assert(*lines);
    if(!strcmp(*lines, "")) return NULL;

    char *line = *lines, *c_ptr = *lines;
    assert(*c_ptr != '\n');

    while(*++c_ptr != '\0' && *c_ptr != '\n');
    *lines = c_ptr;
    if(*c_ptr == '\n') {*c_ptr = '\0'; ++*lines;}

    return line;
}

// Sorts the string array 's' in alphabetical order. The parameter
// 'count' indicates the number of elements in 's'.
void string_sort( char *s[], int count )
{
    qsort(s, (size_t) count, sizeof(char *), x_qsort_string_compare);
}

// Works similarly to the Unix command uniq. The string array 's' with 'size'
// string elements is assumed to be sorted. The function stores all the
// unique strings in 's' in the returned string array. For example, if the
// value of 's' is the 4-element string array {"Alice", "Bob", "Bob", "Eve"},
// then the function returns {"Alice", "Bob", "Eve"}. Note that like 's',
// the returned string array is in sorted order.
char **
unique_strings( char **s, size_t *size )
{
    const size_t orig_size = *size;
    bool *duplicate;
    assert((duplicate = calloc(orig_size, sizeof(bool))));

    for(size_t i = 0; i < orig_size - 1; ++i) {
        if(duplicate[i]) continue;
        size_t j = i + 1;
        while(j < orig_size && !strcmp(s[i], s[j])) {
            duplicate[j] = true, --*size, ++j; } }

    char **unique;
    assert((unique = malloc((*size) * sizeof(void *))));

    for(size_t i = 0, j = 0; i < orig_size; ++i) {
        if(duplicate[i]) continue;
        assert((unique[j] = malloc((strlen(s[i]) + 1) * sizeof(char))));
        strcpy(unique[j], s[i]);
        ++j; }

    free(duplicate);
    return unique;
}

// Returns the current time in milliseconds.
long long
time_in_milliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000*tv.tv_sec + tv.tv_usec/1000;
}

// Returns the current time in microseconds.
long long
time_in_microseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000*tv.tv_sec + tv.tv_usec;
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

// Extracts the rank specified by 'rank_number' from the PPF of 'fen'
// and copies it into 'nine_bytes'. For example, after the call
//
// nth_ppf_rank(INIT_POS, 1, ca)
//
// the character array 'ca' would contain the string "RNBQKBNR".
void
nth_ppf_rank( const char *fen, int rank_number, char *nine_bytes )
{
    int index = 0, slash_count = 0;
    const int target_slash_count = 8 - rank_number;

    while(slash_count < target_slash_count) {
        while(fen[index++] != '/');
        ++slash_count; }

    int cai = 0; // character array index
    while(fen[index] != '/' && fen[index] != ' ')
        nine_bytes[cai++] = fen[index++];

    nine_bytes[cai] = 0;
}

// TODO: doc
char *
clipped_fen( const char *fen )
{
    int null_index = strlen(fen);

    bool first_space_encountered = false;
    while(true) {
        char c = fen[--null_index];
        if(c == ' ' && !first_space_encountered)
            first_space_encountered = true;
        else if(c == ' ')
            break; }

    char *clipped = malloc((size_t) null_index + 1);
    for(int i = 0; i < null_index; ++i) clipped[i] = fen[i];
    clipped[null_index] = '\0';

    return clipped;
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

#define FIND_SQUARE_IN_DIRECTION( dir_constant, ss_file_or_rank, int_constant ) \
if( dir == dir_constant ) { \
    if( sq_bit & ( ss_file_or_rank ) ) \
        return NULL; \
    return SQ_NAME[ sq_bit_index( sq_bit ) + ( int_constant ) ]; }

static const char *
x_sq_navigator_kings_sqs( const char *sq_name, enum sq_dir dir )
{
    Bitboard sq_bit = ONE << sq_name_to_bindex(sq_name);

    FIND_SQUARE_IN_DIRECTION( NORTH, rank('8'), 8 )
    FIND_SQUARE_IN_DIRECTION( NORTHEAST, file('h') | rank('8'), 9 )
    FIND_SQUARE_IN_DIRECTION( EAST, file('h'), 1 )
    FIND_SQUARE_IN_DIRECTION( SOUTHEAST, file('h') | rank('1'), -7 )
    FIND_SQUARE_IN_DIRECTION( SOUTH, rank('1'), -8 )
    FIND_SQUARE_IN_DIRECTION( SOUTHWEST, file('a') | rank('1'), -9 )
    FIND_SQUARE_IN_DIRECTION( WEST, file('a'), -1 )
    FIND_SQUARE_IN_DIRECTION( NORTHWEST, file('a') | rank('8'), 7 )

    assert( false );
    return "";
}

static const char *
x_sq_navigator_knights_sqs( const char *sq_name, enum sq_dir dir )
{
    Bitboard sq_bit = ONE << sq_name_to_bindex(sq_name);

    FIND_SQUARE_IN_DIRECTION( ONE_OCLOCK,    file('h') | rank('7') | rank('8'),  17 )
    FIND_SQUARE_IN_DIRECTION( TWO_OCLOCK,    file('g') | file('h') | rank('8'),  10 )
    FIND_SQUARE_IN_DIRECTION( FOUR_OCLOCK,   file('g') | file('h') | rank('1'),  -6 )
    FIND_SQUARE_IN_DIRECTION( FIVE_OCLOCK,   file('h') | rank('1') | rank('2'), -15 )
    FIND_SQUARE_IN_DIRECTION( SEVEN_OCLOCK,  file('a') | rank('1') | rank('2'), -17 )
    FIND_SQUARE_IN_DIRECTION( EIGHT_OCLOCK,  file('a') | file('b') | rank('1'), -10 )
    FIND_SQUARE_IN_DIRECTION( TEN_OCLOCK,    file('a') | file('b') | rank('8'),   6 )
    FIND_SQUARE_IN_DIRECTION( ELEVEN_OCLOCK, file('a') | rank('7') | rank('8'),  15 )

    assert( false );
    return "";
}

#undef FIND_SQUARE_IN_DIRECTION

static const char *
x_ALT_sq_navigator_kings_sqs( const char *sq_name, enum sq_dir dir )
{
    char sq_name_copy[ 3 ] = { 0 };
    strcpy( sq_name_copy, sq_name );

    if( dir == NORTHEAST || dir == EAST || dir == SOUTHEAST ) ++sq_name_copy[ 0 ];
    else if( dir == NORTHWEST || dir == WEST || dir == SOUTHWEST ) --sq_name_copy[ 0 ];

    if( dir == NORTHWEST || dir == NORTH || dir == NORTHEAST ) ++sq_name_copy[ 1 ];
    else if( dir == SOUTHWEST || dir == SOUTH || dir == SOUTHEAST ) --sq_name_copy[ 1 ];

    return is_sq_name( sq_name_copy ) ? SQ_NAME[ sq_name_to_bindex( sq_name_copy ) ] : NULL;
}

static const char *
x_ALT_sq_navigator_knights_sqs( const char *sq_name, enum sq_dir dir )
{
    char sq_name_copy[ 3 ] = { 0 };
    strcpy( sq_name_copy, sq_name );

    if( dir == TWO_OCLOCK || dir == FOUR_OCLOCK ) sq_name_copy[ 0 ] += 2;
    else if( dir == ONE_OCLOCK || dir == FIVE_OCLOCK ) ++sq_name_copy[ 0 ];
    else if( dir == SEVEN_OCLOCK || dir == ELEVEN_OCLOCK ) --sq_name_copy[ 0 ];
    else sq_name_copy[ 0 ] -= 2;

    if( dir == ONE_OCLOCK || dir == ELEVEN_OCLOCK ) sq_name_copy[ 1 ] += 2;
    else if( dir == TWO_OCLOCK || dir == TEN_OCLOCK ) ++sq_name_copy[ 1 ];
    else if( dir == FOUR_OCLOCK || dir == EIGHT_OCLOCK ) --sq_name_copy[ 1 ];
    else sq_name_copy[ 1 ] -= 2;

    return is_sq_name( sq_name_copy ) ?
        SQ_NAME[ sq_name_to_bindex( sq_name_copy ) ] : NULL;
}

static void
x_compress_eppf_rank_dashes_to_digit(
    int *eri_ptr, int cri, const char *eppf_rank, char *compressed_rank )
{
    char digit = '1';
    while( eppf_rank[ *eri_ptr + 1 ] == '-' ) {
        *eri_ptr += 1;
        ++digit; }
    compressed_rank[ cri ] = digit;
}

static void
x_expand_caf_handle_1_char_caf_cases( char *ecaf, const char *cptr )
{
    strcpy( ecaf, "---- ----" );
    if( *cptr == '-' ) {
        ecaf[ 4 ] = '\0';
    } else if( toupper( *cptr ) == 'A' || toupper( *cptr ) == 'B' ) {
        ecaf[ isupper( *cptr ) ? 0 : 2 ] = *cptr;
        ecaf[ 4 ] = '\0';
    } else if( toupper( *cptr ) == 'G' || toupper( *cptr ) == 'H' ) {
        ecaf[ isupper( *cptr ) ? 1 : 3 ] = *cptr;
        ecaf[ 4 ] = '\0';
    } else {
        ecaf[ isupper( *cptr ) ? 0 : 2 ] = *cptr;
        ecaf[ isupper( *cptr ) ? 6 : 8 ] = *cptr; }
}

static void
x_expand_caf_handle_2_char_caf_cases(
    char *ecaf, const char *first, const char *second )
{
    if( isupper( *first ) && isupper( *second ) ) {
        ecaf[ 0 ] = *first;
        ecaf[ 1 ] = *second;
        for( int i = 2; i <= 3; i++ ) ecaf[ i ] = '-';
    } else if( islower( *first ) && islower( *second ) ) {
        for( int i = 0; i <= 1; i++ ) ecaf[ i ] = '-';
        ecaf[ 2 ] = *first;
        ecaf[ 3 ] = *second;
    } else if( tolower( *first ) < *second ) {
        ecaf[ 0 ] = *first;
        for( int i = 1; i <= 2; i++ ) ecaf[ i ] = '-';
        ecaf[ 3 ] = *second;
    } else if( tolower( *first ) > *second ) {
        for( int i = 0; i <= 3; i += 3 ) ecaf[ i ] = '-';
        ecaf[ 1 ] = *first;
        ecaf[ 2 ] = *second;
    } else if( *first == 'A' || *first == 'B' ) {
        ecaf[ 0 ] = *first;
        ecaf[ 2 ] = *second;
        for( int i = 1; i <= 3; i += 2 ) ecaf[ i ] = '-';
    } else if( *first == 'G' || *first == 'H' ) {
        for( int i = 0; i <= 2; i += 2 ) ecaf[ i ] = '-';
        ecaf[ 1 ] = *first;
        ecaf[ 3 ] = *second;
    } else { // The ambiguous 2-character CAF case
        strcpy( ecaf, "?-?- -?-?" );
        for( int i = 0; i <= 6; i += 6 ) {
            ecaf[ i ] = *first;
            ecaf[ i + 2 ] = *second; } }
}

#define SWAP_CHARS( first, second ) { \
char tmp = first; \
first = second; \
second = tmp; }

static void
x_expand_caf_sort_2_char_ecaf( char *caf )
{
    char *c1 = &caf[ 0 ], *c2 = &caf[ 1 ];
    if( ( isupper( *c1 ) && isupper( *c2 ) ) ||
        ( islower( *c1 ) && islower( *c2 ) ) )
        if( *c1 > *c2 ) SWAP_CHARS( *c1, *c2 )
}

static void
x_expand_caf_sort_3_char_ecaf( char *caf_c )
{
    if( isupper( caf_c[ 1 ] ) && caf_c[ 0 ] > caf_c[ 1 ] )
        SWAP_CHARS( caf_c[ 0 ], caf_c[ 1 ] )
    else if( islower( caf_c[ 1 ] ) && caf_c[ 1 ] > caf_c[ 2 ] )
        SWAP_CHARS( caf_c[ 1 ], caf_c[ 2 ] )

    assert( strlen( caf_c ) == 3 );
}

#undef SWAP_CHARS

static void
x_expand_caf_sort_4_char_ecaf( const char *caf_c, char *ecaf )
{
    bool flip_letters = caf_c[ 0 ] > caf_c[ 1 ];
    ecaf[ 0 ] = caf_c[ flip_letters ? 1 : 0 ];
    ecaf[ 1 ] = caf_c[ flip_letters ? 0 : 1 ];
    ecaf[ 2 ] = caf_c[ flip_letters ? 3 : 2 ];
    ecaf[ 3 ] = caf_c[ flip_letters ? 2 : 3 ];

    assert( strlen( ecaf ) == 4 );
}

static int
x_expand_caf_find_missing_char_index( const char *caf )
{
    if( isupper( caf[ 1 ] ) ) return ( tolower( caf[ 0 ] ) == caf[ 2 ] ) ? 3 : 2;
    return ( tolower( caf[ 0 ] ) == caf[ 1 ] ) ? 1 : 0;
}

static void
x_expand_caf_convert_std_caf_to_shredder_caf( char *caf )
{
    for( int i = 0; caf[i]; i++ )
        if( caf[ i ] == 'K' ) caf[ i ] = 'H';
        else if( caf[ i ] == 'Q' ) caf[ i ] = 'A';
        else if( caf[ i ] == 'k' ) caf[ i ] = 'h';
        else if( caf[ i ] == 'q' ) caf[ i ] = 'a';

    assert(strlen(caf) <= 4);
}

static bool
x_king_in_dir( const char *fen, enum sq_dir dir, bool color_is_white,
    char rooks_file )
{
    assert( dir == EAST || dir == WEST );
    assert( rooks_file >= 'a' && rooks_file <= 'h' );

    char r1[ 8 + 1 ] = { '\0' }, r8[ 8 + 1 ] = { '\0' }, eppf[ PPF_MAX_LENGTH + 1 ];
    char **ff = fen_fields( fen );
    expand_ppf( ff[ 0 ], eppf );
    free_fen_fields( ff );

    for( int i = PPF_MAX_LENGTH - 1, j = 7; eppf[ i ] != '/'; i--, j-- )
        r1[ j ] = eppf[ i ];
    for( int i = 0; i < 8; i++ ) r8[ i ] = eppf[ i ];
    assert( strlen( r1 ) == 8 && strlen( r8 ) == 8 );

    char *rank = ( color_is_white ? r1 : r8 );
    for( int i = rooks_file - 'a' + ( dir == EAST ? 1 : -1 ); i >= 0 && i <= 7;
            i += ( dir == EAST ? 1 : -1 ) ) {
        assert( i >= 0 && i <= 7 );
        if( rank[ i ] == ( color_is_white ? 'K' : 'k' ) ) return true; }

    return false;
}

static int
x_qsort_string_compare( const void *a, const void *b )
{
    return strcmp(*(const char **) a, *(const char **) b);
}

static long long
x_gt_node_count( const uint8_t depth )
{
    if(depth == 0) return 0;

    Pos orig_state;
    copy_pos(&x_state_pos, &orig_state);

    Rawcode *rc = rawcodes(&x_state_pos);
    long long nc = *rc;

    for(int i = 1; i <= *rc; i++) {
        bool prom = is_promotion(&x_state_pos, rc[i]);
        if(prom) nc += 3;

        for(int j = 0; j < (prom ? 4 : 1); j++) {
            const char piece[] = "qrbn";
            make_move(&x_state_pos, rc[i], prom ? piece[j] : '-');
            nc += x_gt_node_count(depth - 1);
            copy_pos(&orig_state, &x_state_pos);
        }
    }

    free(rc);
    return nc;
}

static void
x_che_remove_redundant_epts_remove_epts( char *fen, int index, size_t *rfc )
{
    ++*rfc;
    for(int i = index; i < (int) strlen(fen); ++i)
        fen[i] = fen[i + 1];
    fen[index] = '-';
}

static void
x_che_remove_redundant_epts_set_bit_indices( int *epts_bi,
    int *western_pawn_bi, int *eastern_pawn_bi, const Pos *p,
    const char *eptsf )
{
    *epts_bi = sq_name_to_bindex(eptsf), *western_pawn_bi = -1,
        *eastern_pawn_bi = -1;

    if(whites_turn(p) && SQ_NAV[*epts_bi][SOUTHWEST] & p->ppa[WHITE_PAWN])
        *western_pawn_bi = *epts_bi - 9;
    if(whites_turn(p) && SQ_NAV[*epts_bi][SOUTHEAST] & p->ppa[WHITE_PAWN])
        *eastern_pawn_bi = *epts_bi - 7;
    if(!whites_turn(p) && SQ_NAV[*epts_bi][NORTHWEST] & p->ppa[BLACK_PAWN])
        *western_pawn_bi = *epts_bi + 7;
    if(!whites_turn(p) && SQ_NAV[*epts_bi][NORTHEAST] & p->ppa[BLACK_PAWN])
        *eastern_pawn_bi = *epts_bi + 9;
}

static int
x_che_remove_redundant_epts_set_index( char *single_fen )
{
    int space_count = 0, index = 15;
    while(space_count < 3) if(single_fen[index++] == ' ') ++space_count;
    return index;
}

static bool
x_che_remove_redundant_epts_king_not_forsaken( int pawn_bindex,
    int epts_bindex, const Pos *p )
{
    Pos p2;
    Rawcode rc = ORIG_DEST_RC[pawn_bindex][epts_bindex];
    copy_pos(p, &p2), make_move(&p2, rc, '-');

    return !forsaken_king(&p2);
}

static struct naive_bst_node *
x_recursive_insert_into_naive_bst( struct naive_bst_node *node,
    const char *key )
{
    ++x_current_level_in_tree;

    if(x_tree_height < x_current_level_in_tree)
        x_tree_height = x_current_level_in_tree;

    if(!node) {
        struct naive_bst_node *temp_node =
            malloc(1 * sizeof(struct naive_binary_search_tree));
        temp_node->key = malloc(strlen(key) + 1);
        strcpy(temp_node->key, key);
        temp_node->left = temp_node->right = NULL;
        return temp_node; }

    if(strcmp(key, node->key) < 0)
        node->left  = x_recursive_insert_into_naive_bst(node->left, key);
    else if(strcmp(key, node->key) > 0)
        node->right = x_recursive_insert_into_naive_bst(node->right, key);
    else assert(false);

    --x_current_level_in_tree;
    return node;
}

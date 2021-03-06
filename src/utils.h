#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#include "chester.h"
#include "base.h"

// Expands 'caf', stores the result in 'ecaf' and resolves ambiguous results
// using 'fen'. If the ambiguity cannot be resolved due to an invalid FEN,
// 'ecaf' will remain unchanged by resolve_ambiguous_ecaf(). 'ecaf' is
// assumed to be a writable array of at least ten bytes. For more details
// on CAF expansion, view the documention of expand_caf().
#define EXPAND_CAF( caf, ecaf, fen ) \
    expand_caf( caf, ecaf ); \
    resolve_ambiguous_ecaf( ecaf, fen );

// Swaps the values of the variables 'x' and 'y'. The two variables are of
// type 'T'. For example, swap( i, j, int ) swaps the values of the int
// variables 'i' and 'j'.
#define swap( x, y, T ) \
    do { T tmp = x; x = y; y = tmp; } while( false )

// Calculates the bit index corresponding to 'sq_name'. For example, if
// 'sq_name' contains the square name string "e4", the result of the
// calculation is 28.
#define sq_name_to_bindex(sq_name) \
    (sq_name[0] - 'a' + 8 * (sq_name[1] - '1'))

// A naive/simple binary search tree. 'root' points to the root node of
// a tree of 'node_count' nodes. The maximum number of nodes the tree
// can contain is indicated by 'node_limit'. The pointer array 'node_ptr'
// contains a pointer to each of the tree's 'node_count' nodes. The main
// use of 'node_ptr' has to do with freeing the memory allocated for
// the tree structure (as in free(node_ptr[i])).
struct naive_binary_search_tree {
    struct naive_bst_node *root, **node_ptr;
    uint8_t height;
    uint32_t node_count, node_limit;
};

struct naive_bst_node {
    char *key, *data;
    struct naive_bst_node *left, *right;
};

bool str_m_pat( const char *str, const char *pat );
Chessman occupant_of_sq( const Pos *p, Bitboard sq_bit );
char file_of_sq( Bitboard sq_bit );
char rank_of_sq( Bitboard sq_bit );
char *uint64_to_bin_str( uint64_t uint64 );
void print_pos_var( const Pos *p );
const char *sq_navigator( const char *sq_name, enum sq_dir dir );
const char *ALT_sq_navigator( const char *sq_name, enum sq_dir dir );
int bit_count( Bitboard bb );
void expand_ppf_rank( const char *ppf_rank, char *expanded_rank );
void compress_eppf_rank( const char *eppf_rank, char *compressed_rank );
void expand_ppf( const char *ppf, char *eppf );
char *compress_eppf( const char *eppf );
void ppa_to_eppf( const Bitboard ppa[], char *eppf );
void eppf_to_ppa( const char eppf[], Bitboard *ppa );
char **fen_fields( const char *fen );
void free_fen_fields( char **ff );
void expand_caf( const char *caf, char *ecaf );
void resolve_ambiguous_ecaf( char *ecaf, const char *fen );
char occupant_of_sq_fen_v( const char *fen, const char *sq );
Bitboard in_between( Bitboard sq_1, Bitboard sq_2 );
void copy_pos( const Pos *p, Pos *copy );
Bitboard file( char letter );
Bitboard rank( char number );
void set_mover_target_orig_and_dest( const Pos *p, Rawcode move,
    Chessman *mover, Chessman *target, int *orig, int *dest );
char *next_line( char **lines );
void string_sort( char *s[], int count );
// int unique( char ***s, int count, bool dyn_alloc );
char **unique_strings( char **s, size_t *size );
long long time_in_milliseconds();
long long time_in_microseconds();
int bindex(Bitboard bit);
Bitboard white_army( const Pos *p );
Bitboard black_army( const Pos *p );
void nth_ppf_rank( const char *fen, int rank_number, char *nine_bytes );
char *clipped_fen( const char *fen );
struct naive_binary_search_tree *init_naive_bst( uint32_t node_limit );
void destroy_naive_bst( struct naive_binary_search_tree *nbst );
struct naive_bst_node *search_naive_bst( struct naive_bst_node *root,
    const char *key);
void insert_into_naive_bst( struct naive_binary_search_tree *nbst,
    const char *key, const char *data );

#endif
// end UTILS_H

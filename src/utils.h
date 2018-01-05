#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#include "base.h"

#define SRAND( seed ) \
if( !srand_has_been_called ) { \
	srand_has_been_called = true; \
	srand( seed ); }

extern bool srand_has_been_called;

bool str_matches_pattern( const char *str, const char *pattern );
char *nth_field_of_fen_str( const char *fen_str, char *writable_mem, int field_num );
char *nth_rank_of_ppf( const char *ppf, char *writable_mem, int rank_num );
void set_or_unset_bits( uint64_t *bits, const uint64_t BITMASK, const bool BIT );
void set_bits( uint64_t *bits, const uint64_t BITMASK );
void unset_bits( uint64_t *bits, const uint64_t BITMASK );
Chessman occupant_of_sq( const Pos *p, const char *sq_name );
Bitboard sq_name_to_sq_bit( const char *sq_name );
const char *sq_bit_to_sq_name( Bitboard sq_bit );
char file_of_sq( const Bitboard sq_bit );
char rank_of_sq( const Bitboard sq_bit );
bool bb_is_sq_bit( Bitboard bb );
const char *epts_from_pos_var( const Pos *p );
int sq_bit_index( Bitboard sq_bit );
char *uint64_to_bin_str( uint64_t uint64 );
void print_pos_var( const Pos *p );
const char *sq_navigator( const char *sq_name, enum sq_dir dir );
const char *ALT_sq_navigator( const char *sq_name, enum sq_dir dir );
int sq_name_index( const char *sq_name );
int num_of_sqs_in_sq_set( Bitboard bb );
Bitboard sq_rectangle( Bitboard upper_left, Bitboard lower_right );
const char *file_and_rank_to_sq_name( const char file, const char rank );
Bitboard diag_of_sq( Bitboard sq_bit );
Bitboard antidiag_of_sq( Bitboard sq_bit );
Bitboard next_sq_of_ss( Bitboard *ss );
void expand_ppf_rank( const char *ppf_rank, char *expanded_rank );
void compress_eppf_rank( const char *eppf_rank, char *compressed_rank );
void expand_ppf( const char *ppf, char *eppf );
char *compress_eppf( const char *eppf );

#endif
// end UTILS_H

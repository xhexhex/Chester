#ifndef EXTRA_H
#define EXTRA_H

#include "base.h"

char *dyn_ttt_ppa_to_ppf_conv( const uint16_t *ppa );
void sta_ttt_ppa_to_ppf_conv( const uint16_t *ppa, char *s );
void comparative_ttt_ppa_to_ppf_conv_test( const int reps );
char *slow_pos_to_fen( const Pos *p );
void shredder_to_std_fen_conv( char *fen );
void make_move_performance_test();

#endif
// end EXTRA_H

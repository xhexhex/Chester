#ifndef CT_CHE_MAKE_MOVES_H
#define CT_CHE_MAKE_MOVES_H

#include "../../src/base.h"

void che_make_moves_tested_with_single_moves();
void che_make_moves_tested_with_short_game();
void che_make_moves_tested_with_long_game_1();
void che_make_moves_tested_with_long_game_2();
void che_make_moves_tested_with_long_game_3();
void ct_fen_based_perft( const char *root, int depth, int expected_nc,
    bool progress );
void ct_perft_v2( const Pos *root, int depth, long long expected_nc, bool progress );

#endif
// end CT_CHE_MAKE_MOVES_H

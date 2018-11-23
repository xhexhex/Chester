#ifndef CT_CHE_MAKE_MOVES_H
#define CT_CHE_MAKE_MOVES_H

void che_make_moves_tested_with_pawn_promotions();
void che_make_moves_tested_with_short_game();
void che_make_moves_tested_with_long_game_1();
void che_make_moves_tested_with_long_game_2();
void che_make_moves_tested_with_long_game_3();
void ct_perft_v1( const char *root, int depth, int expected_nc, bool progress );

#endif
// end CT_CHE_MAKE_MOVES_H

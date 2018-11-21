#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chester.h"
#include "test_che_make_moves.h"

extern int test_count, error_count;

static void x_check_input( const char *input[][2], const char *caller_name );
static void x_check_expected_output( const char *expected_output[],
    const char *caller_name );

#define FAIL_MSG "%s: FAIL: i = %d\n", __func__, i

void
test_CHE_MAKE_MOVES_with_pawn_promotions()
{
    const char *input[][2] = {
        {"k7/4P2R/8/8/8/8/8/4K3 w - - 19 75", "e8=Q#"},
        {NULL, NULL} };
    const char *expected[] = {
        "k3Q3/7R/8/8/8/8/8/4K3 b - - 0 75\n",
        NULL };

    x_check_input(input, __func__);
    x_check_expected_output(expected, __func__);

    for(int i = 0; input[i][0]; i++) {
        ++test_count;
        char *actual_output = che_make_moves(input[i][0], input[i][1]);
        if( strcmp(expected[i], actual_output) ) {
            printf(FAIL_MSG);
            ++error_count; }
        free(actual_output); }
}

void
test_CHE_MAKE_MOVES_with_short_game()
{
    const char *fen_and_san[][2] = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e4"},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", "e5"},
        {"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", "Nf3"},
        {"rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2", "Nc6"},
        {"r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3", "Bc4"},
        {"r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", "Nd4"},
        {"r1bqkbnr/pppp1ppp/8/4p3/2BnP3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", "Nxe5"},
        {"r1bqkbnr/pppp1ppp/8/4N3/2BnP3/8/PPPP1PPP/RNBQK2R b KQkq - 0 4", "Qg5"},
        {"r1b1kbnr/pppp1ppp/8/4N1q1/2BnP3/8/PPPP1PPP/RNBQK2R w KQkq - 1 5", "Nxf7"},
        {"r1b1kbnr/pppp1Npp/8/6q1/2BnP3/8/PPPP1PPP/RNBQK2R b KQkq - 0 5", "Qxg2"},
        {"r1b1kbnr/pppp1Npp/8/8/2BnP3/8/PPPP1PqP/RNBQK2R w KQkq - 0 6", "Rf1"},
        {"r1b1kbnr/pppp1Npp/8/8/2BnP3/8/PPPP1PqP/RNBQKR2 b Qkq - 1 6", "Qxe4+"},
        {"r1b1kbnr/pppp1Npp/8/8/2Bnq3/8/PPPP1P1P/RNBQKR2 w Qkq - 0 7", "Be2"},
        {"r1b1kbnr/pppp1Npp/8/8/3nq3/8/PPPPBP1P/RNBQKR2 b Qkq - 1 7", "Nf3#"},
        {"r1b1kbnr/pppp1Npp/8/8/4q3/5n2/PPPPBP1P/RNBQKR2 w Qkq - 2 8", NULL},
        {NULL, NULL} };

    x_check_input(fen_and_san, __func__);

    for(int i = 0; fen_and_san[i + 1][0]; i++) {
        ++test_count;
        char *actual = che_make_moves(fen_and_san[i][0], fen_and_san[i][1]);
        if(strcmp(actual, fen_and_san[i + 1][0])) {
            printf(FAIL_MSG);
            ++error_count; }
        free(actual); }
}

#undef FAIL_MSG

//
// Static functions
//

static void
x_check_input( const char *input[][2], const char *caller_name )
{
    for(int i = 0; input[i][0]; i++) {
        if(che_fen_validator(input[i][0]) != FEN_NO_ERRORS) {
            printf("%s: invalid input FEN: \"%s\"\n", caller_name, input[i][0]);
            exit(1);
        } else if(input[i][1] && !che_is_san(input[i][1])) {
            printf("%s: invalid input SAN: \"%s\"\n", caller_name, input[i][1]);
            exit(1);
        }
    }
}

static void
x_check_expected_output( const char *expected_output[], const char *caller_name )
{
    for(int i = 0; expected_output[i]; i++) {
        if( che_fen_validator(expected_output[i]) != FEN_NO_ERRORS ) {
            printf( "%s: invalid expected output FEN: \"%s\"\n", caller_name,
                expected_output[i] );
            exit(1);
        }
    }
}

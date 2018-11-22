#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "chester.h"
#include "test_che_make_moves.h"

extern int test_count, error_count;

static void x_check_input_fen_and_san( const char *input[][2],
    const char *caller_name );
static void x_check_expected_output_fen( const char *expected_output[],
    const char *caller_name );

#define FAIL_MSG "%s: FAIL: i = %d\n", __func__, i

void
test_CHE_MAKE_MOVES_with_pawn_promotions()
{
    const char *input[][2] = {
        {"k7/4P2R/8/8/8/8/8/4K3 w - - 19 75", "e8=Q#"},
        {"4k3/8/8/8/8/8/2p5/3RK3 b D - 12 34", "cxd1=R+"},
        {NULL, NULL} };
    const char *expected[] = {
        "k3Q3/7R/8/8/8/8/8/4K3 b - - 0 75\n",
        "4k3/8/8/8/8/8/8/3rK3 w - - 0 35\n",
        NULL };

    x_check_input_fen_and_san(input, __func__);
    x_check_expected_output_fen(expected, __func__);

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

    x_check_input_fen_and_san(fen_and_san, __func__);

    for(int i = 0; fen_and_san[i + 1][0]; i++) {
        ++test_count;
        char *actual = che_make_moves(fen_and_san[i][0], fen_and_san[i][1]);
        actual[strlen(actual) - 1] = '\0';
        if(strcmp(actual, fen_and_san[i + 1][0])) {
            printf(FAIL_MSG);
            ++error_count; }
        free(actual); }
}

void
test_CHE_MAKE_MOVES_with_long_game_1()
{
    ++test_count;

    const char start_pos[] =
        "qnbrkbnr/pppppppp/8/8/8/8/PPPPPPPP/QNBRKBNR w DHdh - 0 1";
    assert(che_fen_validator(start_pos) == FEN_NO_ERRORS);
    char *fens = che_make_moves(
        start_pos,
        "e4 d6 Nf3 e5 Bb5+ c6 Ba4 b5 Bb3 Ne7 Ng5 f5 Nf7 d5 Nxh8 dxe4 Nf7 a5 "
        "Nxd8 Bd7 Ne6 Nd5 Bxd5 Na6 Nxf8 Kxf8 Bxe4 Nc7 Bf3 a4 d4 Qa5+ Bd2 Qb6 "
        "Be3 Nd5 Rd3 f4 dxe5 Qb7 Bc5+ Ke8 Bxd5 g5 Bf3 b4 b3 g4 Be4 axb3 axb3 "
        "h6 Nd2 Be6 Rd6 Kf7 Nc4 Bd5 Bxd5+ cxd5 e6+ Kg6 e7+ Kf7 Rd8 d4 e8=Q+ "
        "Kf6 Qxd4+ Kg5 Qee5+ Kh4 Qf6+ Kh5 Qde5#" );

    // Get last line
    char *last_fen = &fens[strlen(fens) - 1];
    assert(*last_fen == '\n');
    *last_fen = '\0';
    while(*(--last_fen) != '\n');
    assert(*last_fen == '\n');
    ++last_fen;

    if(strcmp(last_fen,
            "3R4/1q6/5Q1p/2B1Q2k/1pN2pp1/1P6/2P2PPP/4K2R b K - 6 38")) {
        int i = 0; // For the macro
        printf(FAIL_MSG);
        ++error_count; }

    free(fens);
}

#undef FAIL_MSG

//
// Static functions
//

static void
x_check_input_fen_and_san( const char *input[][2], const char *caller_name )
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
x_check_expected_output_fen( const char *expected_output[], const char *caller_name )
{
    char writable_copy[8 * 1024];

    for(int i = 0; expected_output[i]; i++) {
        strcpy(writable_copy, expected_output[i]);
        if(writable_copy[strlen(expected_output[i]) - 1] == '\n')
            writable_copy[strlen(expected_output[i]) - 1] = '\0';
        // printf("\"%s\"\n", writable_copy);
        if( che_fen_validator(writable_copy) != FEN_NO_ERRORS ) {
            printf( "%s: invalid expected output FEN: \"%s\"\n", caller_name,
                expected_output[i] );
            exit(1);
        }
    }
}

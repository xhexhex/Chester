#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chester.h"
#include "test_che_make_move.h"

static void x_check_input( const char *input[][2], const char *caller_name );
static void x_check_expected_output( const char *expected_output[],
    const char *caller_name );

void
test_CHE_MAKE_MOVE_with_pawn_promotions()
{
    const char *input[][2] = {
        { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e4" },
        { "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", "c5" },
        { NULL, NULL } };
    const char *expected_output[] = {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
        NULL
    };

    x_check_input(input, __func__);
    x_check_expected_output(expected_output, __func__);

    for(int i = 0; input[i][0]; i++) {
        // ++call_counter
        char *actual_output = che_make_move(input[i][0], input[i][1]);
        if( strcmp(expected_output[i], actual_output) ) {
            printf("%s: FAIL: i = %d\n", __func__, i);
        }
        free(actual_output);
    }
}

static void
x_check_input( const char *input[][2], const char *caller_name )
{
    for(int i = 0; input[i][0]; i++) {
        if( che_fen_validator(input[i][0]) != FEN_NO_ERRORS ) {
            printf("%s: invalid input FEN: \"%s\"\n", caller_name, input[i][0]);
            exit(1);
        } else if( !che_is_san(input[i][1]) ) {
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "chester.h"
#include "ct_che_make_moves.h"
#include "../../src/utils.h"
#include "../../src/move_gen.h"

extern int test_count, error_count;

static char state_fen[FEN_MAX_LENGTH + 1];
static Pos state_pos;
static long long leaf_count;
static bool show_progress;

static void x_check_input_fen_and_san( const char *input[][2],
    const char *caller_name );
static void x_check_expected_output_fen( const char *expected_output[],
    const char *caller_name );
static char *x_first_char_of_last_line(const char *fens);
static int x_recursive_perft_v1( int depth );
static long long x_recursive_perft_v2( int depth );

#define FAIL_MSG "%s: FAIL: i = %d\n", __func__, i

void
che_make_moves_tested_with_single_moves()
{
    ++test_count;

    const char *input[][2] = {
        {INIT_POS, "e4"},
        {"k7/4P2R/8/8/8/8/8/4K3 w - - 19 75", "e8=Q#"},
        {"4k3/8/8/8/8/8/2p5/3RK3 b D - 12 34", "cxd1=R+"},
        {NULL, NULL} };
    const char *expected[] = {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1\n",
        "k3Q3/7R/8/8/8/8/8/4K3 b - - 0 75\n",
        "4k3/8/8/8/8/8/8/3rK3 w - - 0 35\n",
        NULL };

    x_check_input_fen_and_san(input, __func__);
    x_check_expected_output_fen(expected, __func__);

    for(int i = 0; input[i][0]; i++) {
        char *actual_output = che_make_moves(input[i][0], input[i][1]);
        if( strcmp(expected[i], actual_output) ) {
            printf(FAIL_MSG);
            ++error_count; }
        free(actual_output); }
}

void
che_make_moves_tested_with_short_game()
{
    ++test_count;

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
        char *actual = che_make_moves(fen_and_san[i][0], fen_and_san[i][1]);
        actual[strlen(actual) - 1] = '\0';
        if(strcmp(actual, fen_and_san[i + 1][0])) {
            printf(FAIL_MSG);
            ++error_count; }
        free(actual); }
}

#define FUNCTION_MAKER(ordinal, start_pos, move_seq, expected_fen) \
    void che_make_moves_tested_with_long_game_ ## ordinal() { \
        ++test_count; \
        assert(!start_pos || che_fen_validator(start_pos) == FEN_NO_ERRORS); \
        char *fens = che_make_moves( start_pos, move_seq); \
        char *last_fen = x_first_char_of_last_line(fens); \
        if(strcmp(last_fen, expected_fen)) { \
            int i = -1; \
            printf(FAIL_MSG); \
            ++error_count; } \
        free(fens); }

FUNCTION_MAKER(1, "qnbrkbnr/pppppppp/8/8/8/8/PPPPPPPP/QNBRKBNR w DHdh - 0 1"
    ,
    "e4 d6 Nf3 e5 Bb5+ c6 Ba4 b5 Bb3 Ne7 Ng5 f5 Nf7 d5 Nxh8 dxe4 Nf7 a5 "
    "Nxd8 Bd7 Ne6 Nd5 Bxd5 Na6 Nxf8 Kxf8 Bxe4 Nc7 Bf3 a4 d4 Qa5+ Bd2 Qb6 "
    "Be3 Nd5 Rd3 f4 dxe5 Qb7 Bc5+ Ke8 Bxd5 g5 Bf3 b4 b3 g4 Be4 axb3 axb3 "
    "h6 Nd2 Be6 Rd6 Kf7 Nc4 Bd5 Bxd5+ cxd5 e6+ Kg6 e7+ Kf7 Rd8 d4 e8=Q+ "
    "Kf6 Qxd4+ Kg5 Qee5+ Kh4 Qf6+ Kh5 Qde5#"
    ,
    "3R4/1q6/5Q1p/2B1Q2k/1pN2pp1/1P6/2P2PPP/4K2R b K - 6 38\n" )

FUNCTION_MAKER(2, "rbnqbnkr/pppppppp/8/8/8/8/PPPPPPPP/RBNQBNKR w HAha - 0 1"
    ,
    "c4 d5 Ne3 dxc4 Nxc4 Nd6 Qc2 Bb5 Ne5 Bxe2 Nxe2 Nd7 Nxd7 Qxd7 h4 Qg4 Nc3 "
    "c6 a4 Bc7 Rh2 Nf5 Kh1 Bxh2 f3 Qf4 Ne2 Qe5 Qe4 Qxb2 Qxf5 Qxa1 Qe4 Be5 h5 "
    "Bf6 Kg1 Rd8 Qb4 b6 Kf1 Rd5 Nf4 Rd4 Qb3 Rxf4 Bg3 Rd4 Kf2 Rxd2+ Kg1 Rb2 "
    "Qd3 Rxb1+ Kf2 Bd4+ Ke2 Qb2+ Qd2 Qxd2+ Kxd2 Rb2+ Kd1 Rxg2 Bh4 Bf6 Be1 Rg1 "
    "Kd2 Bc3+ Kc2 Bxe1 Kb3 Bh4 Kb2 Rg3 Ka3 Rxf3+ Kb2 Bg5 a5 e5 Kb1 Rf2 axb6 "
    "axb6 Ka1 e4 Kb1 e3 Ka1 e2 Ka2 e1=Q+ Ka3 Qb1 Ka4 Ra2#"
    ,
    "6kr/5ppp/1pp5/6bP/K7/8/r7/1q6 w h - 4 50\n")

FUNCTION_MAKER(3, NULL
    ,
    "e4 d6 d4 h5 Nf3 Bg4 Be2 c6 O-O Nf6 Nc3 Nbd7 Bg5 b5 h3 b4 Bxf6 exf6 Na4 "
    "Be6 d5 cxd5 exd5 Bg4 hxg4 hxg4 Nh2 Ne5 Nxg4 a6 Nxe5 Be7 Nc6 Qc7 Nxe7 "
    "Qxe7 Nb6 Rd8 Bxa6 Kf8 Re1 Qa7 Qe2 Qc7 Bb5 Qxb6 Qe7+ Kg8 Bd3 Rc8 Qe8+ "
    "Rxe8 Rxe8#"
    ,
    "4R1kr/5pp1/1q1p1p2/3P4/1p6/3B4/PPP2PP1/R5K1 b - - 0 27\n")

#undef FUNCTION_MAKER
#undef FAIL_MSG

/*
    ct_perft_v1(INIT_POS, 4, 197281, false)
    ---------------------------------------
    real    9m59.843s
    real    8m45.880s
    real    8m8.033s
    real    0m27.833s
    real    0m25.051s
    real    0m24.610s
    real    0m23.751s
    real    0m21.429s
    real    0m18.995s
    real    0m16.784s
*/
void
ct_perft_v1( const char *root, int depth, int expected_nc, bool progress )
{
    ++test_count, strcpy(state_fen, root), leaf_count = 0,
        show_progress = progress;

    if(show_progress)
        printf("Progress: %s: perft(%d): ", __func__, depth), fflush(stdout);
    int node_count = x_recursive_perft_v1(depth);
    if(show_progress) printf("\n");

    if(node_count != expected_nc) {
        printf("FAIL: %s(\"%s\", %d, ...): Invalid node count: %d\n",
            __func__, root, depth, node_count);
        ++error_count; }
}

/*
    ct_perft_v2(p, 5, 4865609, false)
    ---------------------------------
    real    0m5.888s
    real    0m5.685s
    real    0m4.935s
 */
void
ct_perft_v2( const Pos *root, int depth, long long expected_nc, bool progress )
{
    ++test_count, copy_pos(root, &state_pos), leaf_count = 0,
        show_progress = progress;

    if(show_progress)
        printf("Progress: %s: perft(%d): ", __func__, depth), fflush(stdout);
    long long node_count = x_recursive_perft_v2(depth);
    if(show_progress) printf("\n");

    if(node_count != expected_nc) {
        printf("FAIL: %s(..., %d, ...): Invalid node count: %lld\n",
            __func__, depth, node_count);
        ++error_count; }
}

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
    char writable_copy[1024];
    for(int i = 0; i < 1024; i++) writable_copy[i] = '\0';

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

static char *
x_first_char_of_last_line(const char *fens)
{
    const char *first = (const char *) &fens[strlen(fens) - 1];
    assert(*first == '\n');
    while(*(--first) != '\n');
    assert(*first == '\n');
    ++first;
    return (char *) first;
}

static int
x_recursive_perft_v1( int depth )
{
    if(!depth) {
        if(show_progress && ++leaf_count % (1000 * 1000) == 0)
            printf("M"), fflush(stdout);
        return 1; }

    int nodes = 0;
    char *unmod_ptr = che_make_moves(state_fen, NULL), *children = unmod_ptr,
        *the_end = children + strlen(children), *child = children,
        orig_state[FEN_MAX_LENGTH + 1];
    strcpy(orig_state, state_fen);
    for(char *ptr = children; ptr < the_end; ++ptr)
        if(*ptr == '\n') *ptr = '\0';

    while(child < the_end) {
        strcpy(state_fen, child);
        nodes += x_recursive_perft_v1(depth - 1);
        strcpy(state_fen, orig_state);

        while(*++child);
        ++child; }

    free(unmod_ptr);
    return nodes;
}

static long long
x_recursive_perft_v2( int depth )
{
    if(!depth) {
        if(show_progress && ++leaf_count % (10 * 1000 * 1000) == 0) {
            printf("X"), fflush(stdout);
            if(show_progress && leaf_count % (100 * 1000 * 1000) == 0) {
                printf("\n");
            }
        }
        return 1;
    }

    Pos orig_state;
    copy_pos(&state_pos, &orig_state);

    long long nodes = 0;
    Rawcode *move_list = rawcodes(&state_pos), n_moves = move_list[0];
    for(int i = 1; i <= (int) n_moves; i++) {
        Rawcode rc = move_list[i];
        bool promotion = is_promotion(&state_pos, rc);
        assert(!promotion);
        make_move(&state_pos, rc, '-');
        nodes += x_recursive_perft_v2(depth - 1);
        copy_pos(&orig_state, &state_pos); }

    free(move_list);

    return nodes;
}

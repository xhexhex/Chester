#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "chester.h"
#include "external_perft.h"

extern int test_count, error_count;

char state[FEN_MAX_LENGTH + 1];
int call_counter;

static ll x_recursive_ex_perft( int depth );
static void x_set_state( const char *fen );

// exp_nc, expected node count
void
ex_perft( const char *root, int depth, ll exp_nc )
{
    ++test_count;

    x_set_state(root);
    ll final_node_count = x_recursive_ex_perft(depth);
    if( final_node_count != exp_nc ) {
        printf("FAIL: %s(\"%s\", %d, %lld)\n",
            __func__, root, depth, exp_nc);
        ++error_count; }
}

//
// Static functions
//

static void
x_set_state( const char *fen )
{
    strcpy(state, fen);
    assert(che_fen_validator(state) == FEN_NO_ERRORS);
}

static ll
x_recursive_ex_perft( int depth )
{
    // ++call_counter, printf("%d ", call_counter);

    if(!depth) return 1;

    ll nodes = 0;
    char *unmod_ptr = che_make_moves(state, NULL), *children = unmod_ptr,
        *the_end = children + strlen(children), *child = children,
        orig_state[FEN_MAX_LENGTH + 1];
    strcpy(orig_state, state);
    for(char *ptr = children; ptr < the_end; ++ptr)
        if(*ptr == '\n') *ptr = '\0';

    while(child < the_end) {
        x_set_state(child);
        nodes += x_recursive_ex_perft(depth - 1);
        x_set_state(orig_state);

        while(*++child);
        ++child; }

    free(unmod_ptr);
    return nodes;
}

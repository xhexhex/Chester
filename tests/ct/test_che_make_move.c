#include <stdio.h>
#include <stdlib.h>

#include "chester.h"
#include "test_che_make_move.h"

void
xyz()
{
    const char orig_fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    char *new_fen = che_make_move(orig_fen, "e4");

    printf("%s() says \"%s\"\n", __func__, new_fen);

    free(new_fen);
}

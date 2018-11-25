#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// The following function is purely for performance research purposes.
// It converts the tic-tac-toe (ttt) version of a piece placement array
// (PPA) into the corresponding piece placement field (PPF). For example,
// {174, 257, 80} => "o1x/1o1/x2".
//
// Note that the function is analogous to pos_to_fen(). The "dyn" in
// the function name refers to the fact that dynamic memory allocation
// is involved.
//
// +-----------+
// | 6 | 7 | 8 |
// +-----------+
// | 3 | 4 | 5 |    Bit indexes
// +-----------+
// | 0 | 1 | 2 |
// +-----------+
//
// +-----------+
// | o |   | x |
// +-----------+
// |   | o |   |    "o1x/1o1/x2"
// +-----------+
// | x |   |   |
// +-----------+
//
// The following is the PPA for the above board position.
//
//      8 7 6 5 4 3 2 1 0  | Bit indexes
// [0]: 0 1 0 1 0 1 1 1 0  | ppa[0], the empty squares
// [1]: 1 0 0 0 0 0 0 0 1  | ppa[1], the X's
// [2]: 0 0 1 0 1 0 0 0 0  | ppa[2], the O's
char *
dyn_ttt_ppa_to_ppf_conv( const uint16_t *ppa )
{
    const uint16_t one = 1;
    const int bi[] = {6, 7, 8, 3, 4, 5, 0, 1, 2};
    char *s = malloc(11 + 1); // strlen("oxx/xoo/xoo") == 11
    int index = 0;

    for(int i = 0; i < 9; i++) {
        if(bi[i] == 3 || bi[i] == 0) s[index++] = '/';
        uint16_t bit = (one << bi[i]);
        for(int j = 0; ; j++) {
            if(bit & ppa[j]) {
                if(j) s[index++] = (j == 1 ? 'x' : 'o');
                else if(bi[i] % 3 &&
                        (s[index - 1] == '1' || s[index - 1] == '2'))
                    ++s[index - 1];
                else s[index++] = '1';

                break;
            }
        }
    }

    s[index] = '\0';
    return s;
}

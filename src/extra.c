#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "utils.h"

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

#define TTT_PPA_TO_PPF_CONV \
    const uint16_t one = 1; \
    const int bi[] = {6, 7, 8, 3, 4, 5, 0, 1, 2}; \
    int index = 0; \
\
    for(int i = 0; i < 9; i++) { \
        if(bi[i] == 3 || bi[i] == 0) s[index++] = '/'; \
        uint16_t bit = (one << bi[i]); \
        for(int j = 0; ; j++) { \
            if(bit & ppa[j]) { \
                if(j) s[index++] = (j == 1 ? 'x' : 'o'); \
                else if(bi[i] % 3 && \
                        (s[index - 1] == '1' || s[index - 1] == '2')) \
                    ++s[index - 1]; \
                else s[index++] = '1'; \
\
                break; \
            } \
        } \
    } \
\
    s[index] = '\0';

char *
dyn_ttt_ppa_to_ppf_conv( const uint16_t *ppa )
{
    char *s = malloc(11 + 1); // strlen("oxx/xoo/xoo") == 11

    TTT_PPA_TO_PPF_CONV

    return s;
}

// The static memory counterpart of dyn_ttt_ppa_to_ppf_conv(). The
// argument to 'ppa' should point to an array of three integers. The
// result of the conversion is saved in the character array pointed to
// by 's'. The character array should be at least 12 bytes in size.
// Note that unlike dyn_ttt_ppa_to_ppf_conv(), using
// sta_ttt_ppa_to_ppf_conv() doesn't involve any calls to malloc()
// and free().
void
sta_ttt_ppa_to_ppf_conv( const uint16_t *ppa, char *s )
{
    TTT_PPA_TO_PPF_CONV
}

// The means for testing the difference in performance between
// dyn_ttt_ppa_to_ppf_conv() and sta_ttt_ppa_to_ppf_conv(). Using the
// latter is (slightly) more efficient due to not having to call
// malloc() and free().
//
// $ uname -a
// Linux nterror 4.15.0-39-generic #42~16.04.1-Ubuntu SMP
// Wed Oct 24 17:09:54 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
//
// A PC desktop built in 2012 with the above specs gave a performance
// ratio of about 1.3. (See the printf() at the end of the function.)
void
comparative_ttt_ppa_to_ppf_conv_test( const int reps )
{
    const uint16_t nine_1s = 511, ppa1[] = {nine_1s, 0, 0},
        ppa2[] = {174, 257, 80};

    char *ppf_ptr;
    long long t1 = time_in_milliseconds(), t2, result_1, result_2;
    printf("Calling dyn_ttt_ppa_to_ppf_conv() %d times...\n", 2*reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000 * 1000) == 0) fprintf(stderr, "-");
        ppf_ptr = dyn_ttt_ppa_to_ppf_conv(ppa1);
        free(ppf_ptr);
        ppf_ptr = dyn_ttt_ppa_to_ppf_conv(ppa2);
        free(ppf_ptr); }
    t2 = time_in_milliseconds(), result_1 = t2 - t1;
    printf("\nThe first operation took %lld ms\n", result_1);

    char ppf_array[11 + 1];
    t1 = time_in_milliseconds();
    printf("Calling sta_ttt_ppa_to_ppf_conv() %d times...\n", 2*reps);
    for(int count = 1; count <= reps; count++) {
        if(count % (1000 * 1000) == 0) fprintf(stderr, "+");
        sta_ttt_ppa_to_ppf_conv(ppa1, ppf_array);
        sta_ttt_ppa_to_ppf_conv(ppa2, ppf_array); }
    t2 = time_in_milliseconds(), result_2 = t2 - t1;
    printf("\nThe second operation took %lld ms\n", result_2);

    printf("Performance ratio: %.2f\n", (double) result_1 / result_2);
}

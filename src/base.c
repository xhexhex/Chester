#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "base.h"
#include "utils.h"
#include "validation.h"
#include "move_gen.h"

/*************************
 ****                 ****
 ****  External data  ****
 ****                 ****
 *************************/

#define ALL_SQ_BIT_VALUES \
    0x1u, 0x2u, 0x4u, 0x8u, 0x10u, 0x20u, 0x40u, 0x80u, 0x100u, 0x200u, 0x400u, \
    0x800u, 0x1000u, 0x2000u, 0x4000u, 0x8000u, 0x10000u, 0x20000u, 0x40000u, \
    0x80000u, 0x100000u, 0x200000u, 0x400000u, 0x800000u, 0x1000000u, 0x2000000u, \
    0x4000000u, 0x8000000u, 0x10000000u, 0x20000000u, 0x40000000u, 0x80000000u, \
    0x100000000u, 0x200000000u, 0x400000000u, 0x800000000u, 0x1000000000u, \
    0x2000000000u, 0x4000000000u, 0x8000000000u, 0x10000000000u, 0x20000000000u, \
    0x40000000000u, 0x80000000000u, 0x100000000000u, 0x200000000000u, \
    0x400000000000u, 0x800000000000u, 0x1000000000000u, 0x2000000000000u, \
    0x4000000000000u, 0x8000000000000u, 0x10000000000000u, 0x20000000000000u, \
    0x40000000000000u, 0x80000000000000u, 0x100000000000000u, 0x200000000000000u, \
    0x400000000000000u, 0x800000000000000u, 0x1000000000000000u, 0x2000000000000000u, \
    0x4000000000000000u, 0x8000000000000000u

// The following should be the only variable of type struct
// single_instance_variable_SB. The variable 'SB' serves as a mapping
// between square names (e.g., SB.e4) and square bits (e.g., 0x10000000u).
const struct single_instance_variable_SB SB = {
    ALL_SQ_BIT_VALUES
};

// The 64 members of the 'SB' variable as an array. SBA[0] is SB.a1
// and SBA[63] is SB.h8.
const Bitboard SBA[] = {
    ALL_SQ_BIT_VALUES
};

#undef ALL_SQ_BIT_VALUES

// SNA, square names array. SNA[0] is "a1", SNA[1] is "b1", ...,
// SNA[63] is "h8".
const char * const SNA[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

// The 12 letters that identify the different chessmen in the PPF
// of a FEN string. The first element of the array is '-' and as such
// isn't one of the 12 letters. The order of the letters correspond
// to the Chessman enum type.
const char PPF_CHESSMAN_LETTERS[] = "-KQRBNPkqrbnp";

// Square navigation or direction data for each of the 64 squares of the
// chessboard. A square either has or doesn't have another square near it it
// a particular direction. For example, there's a square NORTH of e4 (namely
// the square e5) but there's no square SOUTH of e1. The directions are
// defined by enum sq_dir.
const Bitboard SQ_NAV[][ 16 ] = {
    { 0x100U, 0x200U, 0x2U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x20000U, 0x400U, 0x0U,
    0x0U, 0x0U, 0x0U, 0x0U, 0x0U }, { 0x200U, 0x400U, 0x4U, 0x0U, 0x0U, 0x0U, 0x1U,
    0x100U, 0x40000U, 0x800U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x10000U }, { 0x400U,
    0x800U, 0x8U, 0x0U, 0x0U, 0x0U, 0x2U, 0x200U, 0x80000U, 0x1000U, 0x0U, 0x0U,
    0x0U, 0x0U, 0x100U, 0x20000U }, { 0x800U, 0x1000U, 0x10U, 0x0U, 0x0U, 0x0U,
    0x4U, 0x400U, 0x100000U, 0x2000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x200U, 0x40000U
    }, { 0x1000U, 0x2000U, 0x20U, 0x0U, 0x0U, 0x0U, 0x8U, 0x800U, 0x200000U,
    0x4000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x400U, 0x80000U }, { 0x2000U, 0x4000U,
    0x40U, 0x0U, 0x0U, 0x0U, 0x10U, 0x1000U, 0x400000U, 0x8000U, 0x0U, 0x0U,
    0x0U, 0x0U, 0x800U, 0x100000U }, { 0x4000U, 0x8000U, 0x80U, 0x0U, 0x0U,
    0x0U, 0x20U, 0x2000U, 0x800000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x1000U,
    0x200000U }, { 0x8000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x40U, 0x4000U, 0x0U,
    0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x2000U, 0x400000U }, { 0x10000U, 0x20000U,
    0x200U, 0x2U, 0x1U, 0x0U, 0x0U, 0x0U, 0x2000000U, 0x40000U, 0x4U, 0x0U, 0x0U,
    0x0U, 0x0U, 0x0U }, { 0x20000U, 0x40000U, 0x400U, 0x4U, 0x2U, 0x1U, 0x100U,
    0x10000U, 0x4000000U, 0x80000U, 0x8U, 0x0U, 0x0U, 0x0U, 0x0U, 0x1000000U }, {
    0x40000U, 0x80000U, 0x800U, 0x8U, 0x4U, 0x2U, 0x200U, 0x20000U, 0x8000000U,
    0x100000U, 0x10U, 0x0U, 0x0U, 0x1U, 0x10000U, 0x2000000U }, { 0x80000U,
    0x100000U, 0x1000U, 0x10U, 0x8U, 0x4U, 0x400U, 0x40000U, 0x10000000U,
    0x200000U, 0x20U, 0x0U, 0x0U, 0x2U, 0x20000U, 0x4000000U }, { 0x100000U,
    0x200000U, 0x2000U, 0x20U, 0x10U, 0x8U, 0x800U, 0x80000U, 0x20000000U,
    0x400000U, 0x40U, 0x0U, 0x0U, 0x4U, 0x40000U, 0x8000000U }, { 0x200000U,
    0x400000U, 0x4000U, 0x40U, 0x20U, 0x10U, 0x1000U, 0x100000U, 0x40000000U,
    0x800000U, 0x80U, 0x0U, 0x0U, 0x8U, 0x80000U, 0x10000000U }, { 0x400000U,
    0x800000U, 0x8000U, 0x80U, 0x40U, 0x20U, 0x2000U, 0x200000U, 0x80000000U,
    0x0U, 0x0U, 0x0U, 0x0U, 0x10U, 0x100000U, 0x20000000U }, { 0x800000U, 0x0U,
    0x0U, 0x0U, 0x80U, 0x40U, 0x4000U, 0x400000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,
    0x20U, 0x200000U, 0x40000000U }, { 0x1000000U, 0x2000000U, 0x20000U, 0x200U,
    0x100U, 0x0U, 0x0U, 0x0U, 0x200000000U, 0x4000000U, 0x400U, 0x2U, 0x0U, 0x0U,
    0x0U, 0x0U }, { 0x2000000U, 0x4000000U, 0x40000U, 0x400U, 0x200U, 0x100U,
    0x10000U, 0x1000000U, 0x400000000U, 0x8000000U, 0x800U, 0x4U, 0x1U, 0x0U, 0x0U,
    0x100000000U }, { 0x4000000U, 0x8000000U, 0x80000U, 0x800U, 0x400U, 0x200U,
    0x20000U, 0x2000000U, 0x800000000U, 0x10000000U, 0x1000U, 0x8U, 0x2U, 0x100U,
    0x1000000U, 0x200000000U }, { 0x8000000U, 0x10000000U, 0x100000U, 0x1000U,
    0x800U, 0x400U, 0x40000U, 0x4000000U, 0x1000000000U, 0x20000000U, 0x2000U,
    0x10U, 0x4U, 0x200U, 0x2000000U, 0x400000000U }, { 0x10000000U, 0x20000000U,
    0x200000U, 0x2000U, 0x1000U, 0x800U, 0x80000U, 0x8000000U, 0x2000000000U,
    0x40000000U, 0x4000U, 0x20U, 0x8U, 0x400U, 0x4000000U, 0x800000000U }, {
    0x20000000U, 0x40000000U, 0x400000U, 0x4000U, 0x2000U, 0x1000U, 0x100000U,
    0x10000000U, 0x4000000000U, 0x80000000U, 0x8000U, 0x40U, 0x10U, 0x800U,
    0x8000000U, 0x1000000000U }, { 0x40000000U, 0x80000000U, 0x800000U, 0x8000U,
    0x4000U, 0x2000U, 0x200000U, 0x20000000U, 0x8000000000U, 0x0U, 0x0U, 0x80U,
    0x20U, 0x1000U, 0x10000000U, 0x2000000000U }, { 0x80000000U, 0x0U, 0x0U,
    0x0U, 0x8000U, 0x4000U, 0x400000U, 0x40000000U, 0x0U, 0x0U, 0x0U, 0x0U,
    0x40U, 0x2000U, 0x20000000U, 0x4000000000U }, { 0x100000000U, 0x200000000U,
    0x2000000U, 0x20000U, 0x10000U, 0x0U, 0x0U, 0x0U, 0x20000000000U, 0x400000000U,
    0x40000U, 0x200U, 0x0U, 0x0U, 0x0U, 0x0U }, { 0x200000000U, 0x400000000U,
    0x4000000U, 0x40000U, 0x20000U, 0x10000U, 0x1000000U, 0x100000000U,
    0x40000000000U, 0x800000000U, 0x80000U, 0x400U, 0x100U, 0x0U, 0x0U,
    0x10000000000U }, { 0x400000000U, 0x800000000U, 0x8000000U, 0x80000U, 0x40000U,
    0x20000U, 0x2000000U, 0x200000000U, 0x80000000000U, 0x1000000000U, 0x100000U,
    0x800U, 0x200U, 0x10000U, 0x100000000U, 0x20000000000U }, { 0x800000000U,
    0x1000000000U, 0x10000000U, 0x100000U, 0x80000U, 0x40000U, 0x4000000U,
    0x400000000U, 0x100000000000U, 0x2000000000U, 0x200000U, 0x1000U, 0x400U,
    0x20000U, 0x200000000U, 0x40000000000U }, { 0x1000000000U, 0x2000000000U,
    0x20000000U, 0x200000U, 0x100000U, 0x80000U, 0x8000000U, 0x800000000U,
    0x200000000000U, 0x4000000000U, 0x400000U, 0x2000U, 0x800U, 0x40000U,
    0x400000000U, 0x80000000000U }, { 0x2000000000U, 0x4000000000U, 0x40000000U,
    0x400000U, 0x200000U, 0x100000U, 0x10000000U, 0x1000000000U, 0x400000000000U,
    0x8000000000U, 0x800000U, 0x4000U, 0x1000U, 0x80000U, 0x800000000U,
    0x100000000000U }, { 0x4000000000U, 0x8000000000U, 0x80000000U, 0x800000U,
    0x400000U, 0x200000U, 0x20000000U, 0x2000000000U, 0x800000000000U, 0x0U, 0x0U,
    0x8000U, 0x2000U, 0x100000U, 0x1000000000U, 0x200000000000U }, { 0x8000000000U,
    0x0U, 0x0U, 0x0U, 0x800000U, 0x400000U, 0x40000000U, 0x4000000000U, 0x0U,
    0x0U, 0x0U, 0x0U, 0x4000U, 0x200000U, 0x2000000000U, 0x400000000000U }, {
    0x10000000000U, 0x20000000000U, 0x200000000U, 0x2000000U, 0x1000000U, 0x0U,
    0x0U, 0x0U, 0x2000000000000U, 0x40000000000U, 0x4000000U, 0x20000U, 0x0U, 0x0U,
    0x0U, 0x0U }, { 0x20000000000U, 0x40000000000U, 0x400000000U, 0x4000000U,
    0x2000000U, 0x1000000U, 0x100000000U, 0x10000000000U, 0x4000000000000U,
    0x80000000000U, 0x8000000U, 0x40000U, 0x10000U, 0x0U, 0x0U, 0x1000000000000U
    }, { 0x40000000000U, 0x80000000000U, 0x800000000U, 0x8000000U, 0x4000000U,
    0x2000000U, 0x200000000U, 0x20000000000U, 0x8000000000000U, 0x100000000000U,
    0x10000000U, 0x80000U, 0x20000U, 0x1000000U, 0x10000000000U, 0x2000000000000U
    }, { 0x80000000000U, 0x100000000000U, 0x1000000000U, 0x10000000U, 0x8000000U,
    0x4000000U, 0x400000000U, 0x40000000000U, 0x10000000000000U, 0x200000000000U,
    0x20000000U, 0x100000U, 0x40000U, 0x2000000U, 0x20000000000U, 0x4000000000000U
    }, { 0x100000000000U, 0x200000000000U, 0x2000000000U, 0x20000000U, 0x10000000U,
    0x8000000U, 0x800000000U, 0x80000000000U, 0x20000000000000U, 0x400000000000U,
    0x40000000U, 0x200000U, 0x80000U, 0x4000000U, 0x40000000000U, 0x8000000000000U
    }, { 0x200000000000U, 0x400000000000U, 0x4000000000U, 0x40000000U,
    0x20000000U, 0x10000000U, 0x1000000000U, 0x100000000000U, 0x40000000000000U,
    0x800000000000U, 0x80000000U, 0x400000U, 0x100000U, 0x8000000U, 0x80000000000U,
    0x10000000000000U }, { 0x400000000000U, 0x800000000000U, 0x8000000000U,
    0x80000000U, 0x40000000U, 0x20000000U, 0x2000000000U, 0x200000000000U,
    0x80000000000000U, 0x0U, 0x0U, 0x800000U, 0x200000U, 0x10000000U,
    0x100000000000U, 0x20000000000000U }, { 0x800000000000U, 0x0U, 0x0U, 0x0U,
    0x80000000U, 0x40000000U, 0x4000000000U, 0x400000000000U, 0x0U, 0x0U,
    0x0U, 0x0U, 0x400000U, 0x20000000U, 0x200000000000U, 0x40000000000000U }, {
    0x1000000000000U, 0x2000000000000U, 0x20000000000U, 0x200000000U, 0x100000000U,
    0x0U, 0x0U, 0x0U, 0x200000000000000U, 0x4000000000000U, 0x400000000U,
    0x2000000U, 0x0U, 0x0U, 0x0U, 0x0U }, { 0x2000000000000U, 0x4000000000000U,
    0x40000000000U, 0x400000000U, 0x200000000U, 0x100000000U, 0x10000000000U,
    0x1000000000000U, 0x400000000000000U, 0x8000000000000U, 0x800000000U,
    0x4000000U, 0x1000000U, 0x0U, 0x0U, 0x100000000000000U }, { 0x4000000000000U,
    0x8000000000000U, 0x80000000000U, 0x800000000U, 0x400000000U, 0x200000000U,
    0x20000000000U, 0x2000000000000U, 0x800000000000000U, 0x10000000000000U,
    0x1000000000U, 0x8000000U, 0x2000000U, 0x100000000U, 0x1000000000000U,
    0x200000000000000U }, { 0x8000000000000U, 0x10000000000000U, 0x100000000000U,
    0x1000000000U, 0x800000000U, 0x400000000U, 0x40000000000U, 0x4000000000000U,
    0x1000000000000000U, 0x20000000000000U, 0x2000000000U, 0x10000000U, 0x4000000U,
    0x200000000U, 0x2000000000000U, 0x400000000000000U }, { 0x10000000000000U,
    0x20000000000000U, 0x200000000000U, 0x2000000000U, 0x1000000000U,
    0x800000000U, 0x80000000000U, 0x8000000000000U, 0x2000000000000000U,
    0x40000000000000U, 0x4000000000U, 0x20000000U, 0x8000000U, 0x400000000U,
    0x4000000000000U, 0x800000000000000U }, { 0x20000000000000U, 0x40000000000000U,
    0x400000000000U, 0x4000000000U, 0x2000000000U, 0x1000000000U, 0x100000000000U,
    0x10000000000000U, 0x4000000000000000U, 0x80000000000000U, 0x8000000000U,
    0x40000000U, 0x10000000U, 0x800000000U, 0x8000000000000U, 0x1000000000000000U
    }, { 0x40000000000000U, 0x80000000000000U, 0x800000000000U, 0x8000000000U,
    0x4000000000U, 0x2000000000U, 0x200000000000U, 0x20000000000000U,
    0x8000000000000000U, 0x0U, 0x0U, 0x80000000U, 0x20000000U, 0x1000000000U,
    0x10000000000000U, 0x2000000000000000U }, { 0x80000000000000U, 0x0U, 0x0U,
    0x0U, 0x8000000000U, 0x4000000000U, 0x400000000000U, 0x40000000000000U,
    0x0U, 0x0U, 0x0U, 0x0U, 0x40000000U, 0x2000000000U, 0x20000000000000U,
    0x4000000000000000U }, { 0x100000000000000U, 0x200000000000000U,
    0x2000000000000U, 0x20000000000U, 0x10000000000U, 0x0U, 0x0U, 0x0U, 0x0U,
    0x400000000000000U, 0x40000000000U, 0x200000000U, 0x0U, 0x0U, 0x0U, 0x0U },
    { 0x200000000000000U, 0x400000000000000U, 0x4000000000000U, 0x40000000000U,
    0x20000000000U, 0x10000000000U, 0x1000000000000U, 0x100000000000000U,
    0x0U, 0x800000000000000U, 0x80000000000U, 0x400000000U, 0x100000000U, 0x0U,
    0x0U, 0x0U }, { 0x400000000000000U, 0x800000000000000U, 0x8000000000000U,
    0x80000000000U, 0x40000000000U, 0x20000000000U, 0x2000000000000U,
    0x200000000000000U, 0x0U, 0x1000000000000000U, 0x100000000000U,
    0x800000000U, 0x200000000U, 0x10000000000U, 0x100000000000000U, 0x0U
    }, { 0x800000000000000U, 0x1000000000000000U, 0x10000000000000U,
    0x100000000000U, 0x80000000000U, 0x40000000000U, 0x4000000000000U,
    0x400000000000000U, 0x0U, 0x2000000000000000U, 0x200000000000U,
    0x1000000000U, 0x400000000U, 0x20000000000U, 0x200000000000000U, 0x0U
    }, { 0x1000000000000000U, 0x2000000000000000U, 0x20000000000000U,
    0x200000000000U, 0x100000000000U, 0x80000000000U, 0x8000000000000U,
    0x800000000000000U, 0x0U, 0x4000000000000000U, 0x400000000000U,
    0x2000000000U, 0x800000000U, 0x40000000000U, 0x400000000000000U, 0x0U }, {
    0x2000000000000000U, 0x4000000000000000U, 0x40000000000000U, 0x400000000000U,
    0x200000000000U, 0x100000000000U, 0x10000000000000U, 0x1000000000000000U,
    0x0U, 0x8000000000000000U, 0x800000000000U, 0x4000000000U, 0x1000000000U,
    0x80000000000U, 0x800000000000000U, 0x0U }, { 0x4000000000000000U,
    0x8000000000000000U, 0x80000000000000U, 0x800000000000U, 0x400000000000U,
    0x200000000000U, 0x20000000000000U, 0x2000000000000000U, 0x0U, 0x0U, 0x0U,
    0x8000000000U, 0x2000000000U, 0x100000000000U, 0x1000000000000000U,
    0x0U }, { 0x8000000000000000U, 0x0U, 0x0U, 0x0U, 0x800000000000U,
    0x400000000000U, 0x40000000000000U, 0x4000000000000000U, 0x0U, 0x0U, 0x0U,
    0x0U, 0x4000000000U, 0x200000000000U, 0x2000000000000000U, 0x0U }, { 0x0U,
    0x0U, 0x200000000000000U, 0x2000000000000U, 0x1000000000000U, 0x0U, 0x0U,
    0x0U, 0x0U, 0x0U, 0x4000000000000U, 0x20000000000U, 0x0U, 0x0U, 0x0U, 0x0U
    }, { 0x0U, 0x0U, 0x400000000000000U, 0x4000000000000U, 0x2000000000000U,
    0x1000000000000U, 0x100000000000000U, 0x0U, 0x0U, 0x0U, 0x8000000000000U,
    0x40000000000U, 0x10000000000U, 0x0U, 0x0U, 0x0U }, { 0x0U, 0x0U,
    0x800000000000000U, 0x8000000000000U, 0x4000000000000U, 0x2000000000000U,
    0x200000000000000U, 0x0U, 0x0U, 0x0U, 0x10000000000000U, 0x80000000000U,
    0x20000000000U, 0x1000000000000U, 0x0U, 0x0U }, { 0x0U, 0x0U,
    0x1000000000000000U, 0x10000000000000U, 0x8000000000000U,
    0x4000000000000U, 0x400000000000000U, 0x0U, 0x0U, 0x0U, 0x20000000000000U,
    0x100000000000U, 0x40000000000U, 0x2000000000000U, 0x0U, 0x0U }, {
    0x0U, 0x0U, 0x2000000000000000U, 0x20000000000000U, 0x10000000000000U,
    0x8000000000000U, 0x800000000000000U, 0x0U, 0x0U, 0x0U, 0x40000000000000U,
    0x200000000000U, 0x80000000000U, 0x4000000000000U, 0x0U, 0x0U }, {
    0x0U, 0x0U, 0x4000000000000000U, 0x40000000000000U, 0x20000000000000U,
    0x10000000000000U, 0x1000000000000000U, 0x0U, 0x0U, 0x0U, 0x80000000000000U,
    0x400000000000U, 0x100000000000U, 0x8000000000000U, 0x0U, 0x0U }, { 0x0U, 0x0U,
    0x8000000000000000U, 0x80000000000000U, 0x40000000000000U, 0x20000000000000U,
    0x2000000000000000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x800000000000U, 0x200000000000U,
    0x10000000000000U, 0x0U, 0x0U }, { 0x0U, 0x0U, 0x0U, 0x0U, 0x80000000000000U,
    0x40000000000000U, 0x4000000000000000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,
    0x400000000000U, 0x20000000000000U, 0x0U, 0x0U } };

const Bitboard
    SS_FILE_A = 0x101010101010101u, SS_FILE_B = 0x202020202020202u,
    SS_FILE_C = 0x404040404040404u, SS_FILE_D = 0x808080808080808u,
    SS_FILE_E = 0x1010101010101010u, SS_FILE_F = 0x2020202020202020u,
    SS_FILE_G = 0x4040404040404040u, SS_FILE_H = 0x8080808080808080u,
    SS_RANK_1 = 0xffu, SS_RANK_2 = 0xff00u, SS_RANK_3 = 0xff0000u,
    SS_RANK_4 = 0xff000000u, SS_RANK_5 = 0xff00000000u,
    SS_RANK_6 = 0xff0000000000u, SS_RANK_7 = 0xff000000000000u,
    SS_RANK_8 = 0xff00000000000000u;

const Bitboard
    SS_DIAG_H1H1 = 0x80U, SS_DIAG_G1H2 = 0x8040U, SS_DIAG_F1H3 = 0x804020U,
    SS_DIAG_E1H4 = 0x80402010U, SS_DIAG_D1H5 = 0x8040201008U,
    SS_DIAG_C1H6 = 0x804020100804U, SS_DIAG_B1H7 = 0x80402010080402U,
    SS_DIAG_A1H8 = 0x8040201008040201U, SS_DIAG_A2G8 = 0x4020100804020100U,
    SS_DIAG_A3F8 = 0x2010080402010000U, SS_DIAG_A4E8 = 0x1008040201000000U,
    SS_DIAG_A5D8 = 0x804020100000000U, SS_DIAG_A6C8 = 0x402010000000000U,
    SS_DIAG_A7B8 = 0x201000000000000U, SS_DIAG_A8A8 = 0x100000000000000U,
    SS_ANTIDIAG_A1A1 = 0x1U, SS_ANTIDIAG_B1A2 = 0x102U,
    SS_ANTIDIAG_C1A3 = 0x10204U, SS_ANTIDIAG_D1A4 = 0x1020408U,
    SS_ANTIDIAG_E1A5 = 0x102040810U, SS_ANTIDIAG_F1A6 = 0x10204081020U,
    SS_ANTIDIAG_G1A7 = 0x1020408102040U, SS_ANTIDIAG_H1A8 = 0x102040810204080U,
    SS_ANTIDIAG_H2B8 = 0x204081020408000U, SS_ANTIDIAG_H3C8 = 0x408102040800000U,
    SS_ANTIDIAG_H4D8 = 0x810204080000000U, SS_ANTIDIAG_H5E8 = 0x1020408000000000U,
    SS_ANTIDIAG_H6F8 = 0x2040800000000000U, SS_ANTIDIAG_H7G8 = 0x4080000000000000U,
    SS_ANTIDIAG_H8H8 = 0x8000000000000000U;

//const size_t
    // The shortest possible FEN is something like "k7/8/8/8/8/8/8/K7 w - - 0 1"
    // which is 27 characters long. The following FEN is 90 chars long:
    // "rrrrkrrr/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/RRRRKRRR w KQkq - 65535 65535"
    // Allowing FENs longer than this wouldn't seem to make sense.
    // FEN_MIN_LENGTH = 27, FEN_MAX_LENGTH = 92;

// The 21 ways rooks can be placed in a Chess960 starting position. Each of the values
// is an 8-bit unsigned integer with exactly two bits set. The two set bits correspond
// to the files on which the rooks are located. The LSB is file a and the MSB is
// file h. As an example, here's the diagram for the value 2 + 16:
//
//   h    g    f    e    d    c    b    a
// [   ][   ][   ][ x ][   ][   ][ x ][   ]
//                                     LSB
//
const size_t POSSIBLE_IRPF_VALUES_COUNT = 21;
const uint8_t POSSIBLE_IRPF_VALUES[] = {
    1 + 4, 1 + 8, 1 + 16, 1 + 32, 1 + 64, 1 + 128,
    2 + 8, 2 + 16, 2 + 32, 2 + 64, 2 + 128,
    4 + 16, 4 + 32, 4 + 64, 4 + 128,
    8 + 32, 8 + 64, 8 + 128,
    16 + 64, 16 + 128,
    32 + 128 };

// All the possible Shredder-FEN castling availability fields along with their number
const size_t SHREDDER_FEN_CAFS_COUNT = 361;
const char * const SHREDDER_FEN_CAFS[] = {
    "-", "a", "A", "Aa", "Aac", "Aad", "Aae", "Aaf", "Aag", "Aah", "ac", "Ac", "AC",
    "Aca", "ACa", "ACac", "ACc", "ad", "Ad", "AD", "Ada", "ADa", "ADad", "ADd", "ae",
    "Ae", "AE", "Aea", "AEa", "AEae", "AEe", "af", "Af", "AF", "Afa", "AFa", "AFaf",
    "AFf", "ag", "Ag", "AG", "Aga", "AGa", "AGag", "AGg", "ah", "Ah", "AH", "Aha",
    "AHa", "AHah", "AHh", "b", "B", "Bb", "Bbd", "Bbe", "Bbf", "Bbg", "Bbh", "bd",
    "Bd", "BD", "Bdb", "BDb", "BDbd", "BDd", "be", "Be", "BE", "Beb", "BEb", "BEbe",
    "BEe", "bf", "Bf", "BF", "Bfb", "BFb", "BFbf", "BFf", "bg", "Bg", "BG", "Bgb",
    "BGb", "BGbg", "BGg", "bh", "Bh", "BH", "Bhb", "BHb", "BHbh", "BHh", "c", "C",
    "ca", "Ca", "CA", "CAa", "Cac", "CAc", "CAca", "Cc", "Cca", "Cce", "Ccf", "Ccg",
    "Cch", "ce", "Ce", "CE", "Cec", "CEc", "CEce", "CEe", "cf", "Cf", "CF", "Cfc",
    "CFc", "CFcf", "CFf", "cg", "Cg", "CG", "Cgc", "CGc", "CGcg", "CGg", "ch", "Ch",
    "CH", "Chc", "CHc", "CHch", "CHh", "d", "D", "da", "Da", "DA", "DAa", "Dad", "DAd",
    "DAda", "db", "Db", "DB", "DBb", "Dbd", "DBd", "DBdb", "Dd", "Dda", "Ddb", "Ddf",
    "Ddg", "Ddh", "df", "Df", "DF", "Dfd", "DFd", "DFdf", "DFf", "dg", "Dg", "DG",
    "Dgd", "DGd", "DGdg", "DGg", "dh", "Dh", "DH", "Dhd", "DHd", "DHdh", "DHh", "e",
    "E", "ea", "Ea", "EA", "EAa", "Eae", "EAe", "EAea", "eb", "Eb", "EB", "EBb", "Ebe",
    "EBe", "EBeb", "ec", "Ec", "EC", "ECc", "Ece", "ECe", "ECec", "Ee", "Eea", "Eeb",
    "Eec", "Eeg", "Eeh", "eg", "Eg", "EG", "Ege", "EGe", "EGeg", "EGg", "eh", "Eh",
    "EH", "Ehe", "EHe", "EHeh", "EHh", "f", "F", "fa", "Fa", "FA", "FAa", "Faf",
    "FAf", "FAfa", "fb", "Fb", "FB", "FBb", "Fbf", "FBf", "FBfb", "fc", "Fc", "FC",
    "FCc", "Fcf", "FCf", "FCfc", "fd", "Fd", "FD", "FDd", "Fdf", "FDf", "FDfd", "Ff",
    "Ffa", "Ffb", "Ffc", "Ffd", "Ffh", "fh", "Fh", "FH", "Fhf", "FHf", "FHfh", "FHh",
    "g", "G", "ga", "Ga", "GA", "GAa", "Gag", "GAg", "GAga", "gb", "Gb", "GB", "GBb",
    "Gbg", "GBg", "GBgb", "gc", "Gc", "GC", "GCc", "Gcg", "GCg", "GCgc", "gd", "Gd",
    "GD", "GDd", "Gdg", "GDg", "GDgd", "ge", "Ge", "GE", "GEe", "Geg", "GEg", "GEge",
    "Gg", "Gga", "Ggb", "Ggc", "Ggd", "Gge", "h", "H", "ha", "Ha", "HA", "HAa", "Hah",
    "HAh", "HAha", "hb", "Hb", "HB", "HBb", "Hbh", "HBh", "HBhb", "hc", "Hc", "HC",
    "HCc", "Hch", "HCh", "HChc", "hd", "Hd", "HD", "HDd", "Hdh", "HDh", "HDhd", "he",
    "He", "HE", "HEe", "Heh", "HEh", "HEhe", "hf", "Hf", "HF", "HFf", "Hfh", "HFh",
    "HFhf", "Hh", "Hha", "Hhb", "Hhc", "Hhd", "Hhe", "Hhf", NULL };

// Matches any of the 16 standard FEN castling availability fields. Note that the
// regex also matches the empty string so be sure to check that the string length
// is greater than zero when using the regex.
const char STD_FEN_CAF_REGEX[] = "^(-|K?Q?k?q?)$";

// APM stands for "All Possible (raw)Moves". The following string should
// logically be understood as the string array {"a1a2", "a1a3", ..., "h8h7"}
// with 1792 elements. The rawmove "g1f3" is included in APM_DATA because
// there exists a chessman that can move from g1 to f3. For the same reason
// there is no such rawmove as "e1d4".
const char APM_DATA[] =
    "a1a2a1a3a1a4a1a5a1a6a1a7a1a8a1b1a1b2a1b3a1c1a1c2a1c3a1d1a1d4a1e1"
    "a1e5a1f1a1f6a1g1a1g7a1h1a1h8a2a1a2a3a2a4a2a5a2a6a2a7a2a8a2b1a2b2"
    "a2b3a2b4a2c1a2c2a2c3a2c4a2d2a2d5a2e2a2e6a2f2a2f7a2g2a2g8a2h2a3a1"
    "a3a2a3a4a3a5a3a6a3a7a3a8a3b1a3b2a3b3a3b4a3b5a3c1a3c2a3c3a3c4a3c5"
    "a3d3a3d6a3e3a3e7a3f3a3f8a3g3a3h3a4a1a4a2a4a3a4a5a4a6a4a7a4a8a4b2"
    "a4b3a4b4a4b5a4b6a4c2a4c3a4c4a4c5a4c6a4d1a4d4a4d7a4e4a4e8a4f4a4g4"
    "a4h4a5a1a5a2a5a3a5a4a5a6a5a7a5a8a5b3a5b4a5b5a5b6a5b7a5c3a5c4a5c5"
    "a5c6a5c7a5d2a5d5a5d8a5e1a5e5a5f5a5g5a5h5a6a1a6a2a6a3a6a4a6a5a6a7"
    "a6a8a6b4a6b5a6b6a6b7a6b8a6c4a6c5a6c6a6c7a6c8a6d3a6d6a6e2a6e6a6f1"
    "a6f6a6g6a6h6a7a1a7a2a7a3a7a4a7a5a7a6a7a8a7b5a7b6a7b7a7b8a7c5a7c6"
    "a7c7a7c8a7d4a7d7a7e3a7e7a7f2a7f7a7g1a7g7a7h7a8a1a8a2a8a3a8a4a8a5"
    "a8a6a8a7a8b6a8b7a8b8a8c6a8c7a8c8a8d5a8d8a8e4a8e8a8f3a8f8a8g2a8g8"
    "a8h1a8h8b1a1b1a2b1a3b1b2b1b3b1b4b1b5b1b6b1b7b1b8b1c1b1c2b1c3b1d1"
    "b1d2b1d3b1e1b1e4b1f1b1f5b1g1b1g6b1h1b1h7b2a1b2a2b2a3b2a4b2b1b2b3"
    "b2b4b2b5b2b6b2b7b2b8b2c1b2c2b2c3b2c4b2d1b2d2b2d3b2d4b2e2b2e5b2f2"
    "b2f6b2g2b2g7b2h2b2h8b3a1b3a2b3a3b3a4b3a5b3b1b3b2b3b4b3b5b3b6b3b7"
    "b3b8b3c1b3c2b3c3b3c4b3c5b3d1b3d2b3d3b3d4b3d5b3e3b3e6b3f3b3f7b3g3"
    "b3g8b3h3b4a2b4a3b4a4b4a5b4a6b4b1b4b2b4b3b4b5b4b6b4b7b4b8b4c2b4c3"
    "b4c4b4c5b4c6b4d2b4d3b4d4b4d5b4d6b4e1b4e4b4e7b4f4b4f8b4g4b4h4b5a3"
    "b5a4b5a5b5a6b5a7b5b1b5b2b5b3b5b4b5b6b5b7b5b8b5c3b5c4b5c5b5c6b5c7"
    "b5d3b5d4b5d5b5d6b5d7b5e2b5e5b5e8b5f1b5f5b5g5b5h5b6a4b6a5b6a6b6a7"
    "b6a8b6b1b6b2b6b3b6b4b6b5b6b7b6b8b6c4b6c5b6c6b6c7b6c8b6d4b6d5b6d6"
    "b6d7b6d8b6e3b6e6b6f2b6f6b6g1b6g6b6h6b7a5b7a6b7a7b7a8b7b1b7b2b7b3"
    "b7b4b7b5b7b6b7b8b7c5b7c6b7c7b7c8b7d5b7d6b7d7b7d8b7e4b7e7b7f3b7f7"
    "b7g2b7g7b7h1b7h7b8a6b8a7b8a8b8b1b8b2b8b3b8b4b8b5b8b6b8b7b8c6b8c7"
    "b8c8b8d6b8d7b8d8b8e5b8e8b8f4b8f8b8g3b8g8b8h2b8h8c1a1c1a2c1a3c1b1"
    "c1b2c1b3c1c2c1c3c1c4c1c5c1c6c1c7c1c8c1d1c1d2c1d3c1e1c1e2c1e3c1f1"
    "c1f4c1g1c1g5c1h1c1h6c2a1c2a2c2a3c2a4c2b1c2b2c2b3c2b4c2c1c2c3c2c4"
    "c2c5c2c6c2c7c2c8c2d1c2d2c2d3c2d4c2e1c2e2c2e3c2e4c2f2c2f5c2g2c2g6"
    "c2h2c2h7c3a1c3a2c3a3c3a4c3a5c3b1c3b2c3b3c3b4c3b5c3c1c3c2c3c4c3c5"
    "c3c6c3c7c3c8c3d1c3d2c3d3c3d4c3d5c3e1c3e2c3e3c3e4c3e5c3f3c3f6c3g3"
    "c3g7c3h3c3h8c4a2c4a3c4a4c4a5c4a6c4b2c4b3c4b4c4b5c4b6c4c1c4c2c4c3"
    "c4c5c4c6c4c7c4c8c4d2c4d3c4d4c4d5c4d6c4e2c4e3c4e4c4e5c4e6c4f1c4f4"
    "c4f7c4g4c4g8c4h4c5a3c5a4c5a5c5a6c5a7c5b3c5b4c5b5c5b6c5b7c5c1c5c2"
    "c5c3c5c4c5c6c5c7c5c8c5d3c5d4c5d5c5d6c5d7c5e3c5e4c5e5c5e6c5e7c5f2"
    "c5f5c5f8c5g1c5g5c5h5c6a4c6a5c6a6c6a7c6a8c6b4c6b5c6b6c6b7c6b8c6c1"
    "c6c2c6c3c6c4c6c5c6c7c6c8c6d4c6d5c6d6c6d7c6d8c6e4c6e5c6e6c6e7c6e8"
    "c6f3c6f6c6g2c6g6c6h1c6h6c7a5c7a6c7a7c7a8c7b5c7b6c7b7c7b8c7c1c7c2"
    "c7c3c7c4c7c5c7c6c7c8c7d5c7d6c7d7c7d8c7e5c7e6c7e7c7e8c7f4c7f7c7g3"
    "c7g7c7h2c7h7c8a6c8a7c8a8c8b6c8b7c8b8c8c1c8c2c8c3c8c4c8c5c8c6c8c7"
    "c8d6c8d7c8d8c8e6c8e7c8e8c8f5c8f8c8g4c8g8c8h3c8h8d1a1d1a4d1b1d1b2"
    "d1b3d1c1d1c2d1c3d1d2d1d3d1d4d1d5d1d6d1d7d1d8d1e1d1e2d1e3d1f1d1f2"
    "d1f3d1g1d1g4d1h1d1h5d2a2d2a5d2b1d2b2d2b3d2b4d2c1d2c2d2c3d2c4d2d1"
    "d2d3d2d4d2d5d2d6d2d7d2d8d2e1d2e2d2e3d2e4d2f1d2f2d2f3d2f4d2g2d2g5"
    "d2h2d2h6d3a3d3a6d3b1d3b2d3b3d3b4d3b5d3c1d3c2d3c3d3c4d3c5d3d1d3d2"
    "d3d4d3d5d3d6d3d7d3d8d3e1d3e2d3e3d3e4d3e5d3f1d3f2d3f3d3f4d3f5d3g3"
    "d3g6d3h3d3h7d4a1d4a4d4a7d4b2d4b3d4b4d4b5d4b6d4c2d4c3d4c4d4c5d4c6"
    "d4d1d4d2d4d3d4d5d4d6d4d7d4d8d4e2d4e3d4e4d4e5d4e6d4f2d4f3d4f4d4f5"
    "d4f6d4g1d4g4d4g7d4h4d4h8d5a2d5a5d5a8d5b3d5b4d5b5d5b6d5b7d5c3d5c4"
    "d5c5d5c6d5c7d5d1d5d2d5d3d5d4d5d6d5d7d5d8d5e3d5e4d5e5d5e6d5e7d5f3"
    "d5f4d5f5d5f6d5f7d5g2d5g5d5g8d5h1d5h5d6a3d6a6d6b4d6b5d6b6d6b7d6b8"
    "d6c4d6c5d6c6d6c7d6c8d6d1d6d2d6d3d6d4d6d5d6d7d6d8d6e4d6e5d6e6d6e7"
    "d6e8d6f4d6f5d6f6d6f7d6f8d6g3d6g6d6h2d6h6d7a4d7a7d7b5d7b6d7b7d7b8"
    "d7c5d7c6d7c7d7c8d7d1d7d2d7d3d7d4d7d5d7d6d7d8d7e5d7e6d7e7d7e8d7f5"
    "d7f6d7f7d7f8d7g4d7g7d7h3d7h7d8a5d8a8d8b6d8b7d8b8d8c6d8c7d8c8d8d1"
    "d8d2d8d3d8d4d8d5d8d6d8d7d8e6d8e7d8e8d8f6d8f7d8f8d8g5d8g8d8h4d8h8"
    "e1a1e1a5e1b1e1b4e1c1e1c2e1c3e1d1e1d2e1d3e1e2e1e3e1e4e1e5e1e6e1e7"
    "e1e8e1f1e1f2e1f3e1g1e1g2e1g3e1h1e1h4e2a2e2a6e2b2e2b5e2c1e2c2e2c3"
    "e2c4e2d1e2d2e2d3e2d4e2e1e2e3e2e4e2e5e2e6e2e7e2e8e2f1e2f2e2f3e2f4"
    "e2g1e2g2e2g3e2g4e2h2e2h5e3a3e3a7e3b3e3b6e3c1e3c2e3c3e3c4e3c5e3d1"
    "e3d2e3d3e3d4e3d5e3e1e3e2e3e4e3e5e3e6e3e7e3e8e3f1e3f2e3f3e3f4e3f5"
    "e3g1e3g2e3g3e3g4e3g5e3h3e3h6e4a4e4a8e4b1e4b4e4b7e4c2e4c3e4c4e4c5"
    "e4c6e4d2e4d3e4d4e4d5e4d6e4e1e4e2e4e3e4e5e4e6e4e7e4e8e4f2e4f3e4f4"
    "e4f5e4f6e4g2e4g3e4g4e4g5e4g6e4h1e4h4e4h7e5a1e5a5e5b2e5b5e5b8e5c3"
    "e5c4e5c5e5c6e5c7e5d3e5d4e5d5e5d6e5d7e5e1e5e2e5e3e5e4e5e6e5e7e5e8"
    "e5f3e5f4e5f5e5f6e5f7e5g3e5g4e5g5e5g6e5g7e5h2e5h5e5h8e6a2e6a6e6b3"
    "e6b6e6c4e6c5e6c6e6c7e6c8e6d4e6d5e6d6e6d7e6d8e6e1e6e2e6e3e6e4e6e5"
    "e6e7e6e8e6f4e6f5e6f6e6f7e6f8e6g4e6g5e6g6e6g7e6g8e6h3e6h6e7a3e7a7"
    "e7b4e7b7e7c5e7c6e7c7e7c8e7d5e7d6e7d7e7d8e7e1e7e2e7e3e7e4e7e5e7e6"
    "e7e8e7f5e7f6e7f7e7f8e7g5e7g6e7g7e7g8e7h4e7h7e8a4e8a8e8b5e8b8e8c6"
    "e8c7e8c8e8d6e8d7e8d8e8e1e8e2e8e3e8e4e8e5e8e6e8e7e8f6e8f7e8f8e8g6"
    "e8g7e8g8e8h5e8h8f1a1f1a6f1b1f1b5f1c1f1c4f1d1f1d2f1d3f1e1f1e2f1e3"
    "f1f2f1f3f1f4f1f5f1f6f1f7f1f8f1g1f1g2f1g3f1h1f1h2f1h3f2a2f2a7f2b2"
    "f2b6f2c2f2c5f2d1f2d2f2d3f2d4f2e1f2e2f2e3f2e4f2f1f2f3f2f4f2f5f2f6"
    "f2f7f2f8f2g1f2g2f2g3f2g4f2h1f2h2f2h3f2h4f3a3f3a8f3b3f3b7f3c3f3c6"
    "f3d1f3d2f3d3f3d4f3d5f3e1f3e2f3e3f3e4f3e5f3f1f3f2f3f4f3f5f3f6f3f7"
    "f3f8f3g1f3g2f3g3f3g4f3g5f3h1f3h2f3h3f3h4f3h5f4a4f4b4f4b8f4c1f4c4"
    "f4c7f4d2f4d3f4d4f4d5f4d6f4e2f4e3f4e4f4e5f4e6f4f1f4f2f4f3f4f5f4f6"
    "f4f7f4f8f4g2f4g3f4g4f4g5f4g6f4h2f4h3f4h4f4h5f4h6f5a5f5b1f5b5f5c2"
    "f5c5f5c8f5d3f5d4f5d5f5d6f5d7f5e3f5e4f5e5f5e6f5e7f5f1f5f2f5f3f5f4"
    "f5f6f5f7f5f8f5g3f5g4f5g5f5g6f5g7f5h3f5h4f5h5f5h6f5h7f6a1f6a6f6b2"
    "f6b6f6c3f6c6f6d4f6d5f6d6f6d7f6d8f6e4f6e5f6e6f6e7f6e8f6f1f6f2f6f3"
    "f6f4f6f5f6f7f6f8f6g4f6g5f6g6f6g7f6g8f6h4f6h5f6h6f6h7f6h8f7a2f7a7"
    "f7b3f7b7f7c4f7c7f7d5f7d6f7d7f7d8f7e5f7e6f7e7f7e8f7f1f7f2f7f3f7f4"
    "f7f5f7f6f7f8f7g5f7g6f7g7f7g8f7h5f7h6f7h7f7h8f8a3f8a8f8b4f8b8f8c5"
    "f8c8f8d6f8d7f8d8f8e6f8e7f8e8f8f1f8f2f8f3f8f4f8f5f8f6f8f7f8g6f8g7"
    "f8g8f8h6f8h7f8h8g1a1g1a7g1b1g1b6g1c1g1c5g1d1g1d4g1e1g1e2g1e3g1f1"
    "g1f2g1f3g1g2g1g3g1g4g1g5g1g6g1g7g1g8g1h1g1h2g1h3g2a2g2a8g2b2g2b7"
    "g2c2g2c6g2d2g2d5g2e1g2e2g2e3g2e4g2f1g2f2g2f3g2f4g2g1g2g3g2g4g2g5"
    "g2g6g2g7g2g8g2h1g2h2g2h3g2h4g3a3g3b3g3b8g3c3g3c7g3d3g3d6g3e1g3e2"
    "g3e3g3e4g3e5g3f1g3f2g3f3g3f4g3f5g3g1g3g2g3g4g3g5g3g6g3g7g3g8g3h1"
    "g3h2g3h3g3h4g3h5g4a4g4b4g4c4g4c8g4d1g4d4g4d7g4e2g4e3g4e4g4e5g4e6"
    "g4f2g4f3g4f4g4f5g4f6g4g1g4g2g4g3g4g5g4g6g4g7g4g8g4h2g4h3g4h4g4h5"
    "g4h6g5a5g5b5g5c1g5c5g5d2g5d5g5d8g5e3g5e4g5e5g5e6g5e7g5f3g5f4g5f5"
    "g5f6g5f7g5g1g5g2g5g3g5g4g5g6g5g7g5g8g5h3g5h4g5h5g5h6g5h7g6a6g6b1"
    "g6b6g6c2g6c6g6d3g6d6g6e4g6e5g6e6g6e7g6e8g6f4g6f5g6f6g6f7g6f8g6g1"
    "g6g2g6g3g6g4g6g5g6g7g6g8g6h4g6h5g6h6g6h7g6h8g7a1g7a7g7b2g7b7g7c3"
    "g7c7g7d4g7d7g7e5g7e6g7e7g7e8g7f5g7f6g7f7g7f8g7g1g7g2g7g3g7g4g7g5"
    "g7g6g7g8g7h5g7h6g7h7g7h8g8a2g8a8g8b3g8b8g8c4g8c8g8d5g8d8g8e6g8e7"
    "g8e8g8f6g8f7g8f8g8g1g8g2g8g3g8g4g8g5g8g6g8g7g8h6g8h7g8h8h1a1h1a8"
    "h1b1h1b7h1c1h1c6h1d1h1d5h1e1h1e4h1f1h1f2h1f3h1g1h1g2h1g3h1h2h1h3"
    "h1h4h1h5h1h6h1h7h1h8h2a2h2b2h2b8h2c2h2c7h2d2h2d6h2e2h2e5h2f1h2f2"
    "h2f3h2f4h2g1h2g2h2g3h2g4h2h1h2h3h2h4h2h5h2h6h2h7h2h8h3a3h3b3h3c3"
    "h3c8h3d3h3d7h3e3h3e6h3f1h3f2h3f3h3f4h3f5h3g1h3g2h3g3h3g4h3g5h3h1"
    "h3h2h3h4h3h5h3h6h3h7h3h8h4a4h4b4h4c4h4d4h4d8h4e1h4e4h4e7h4f2h4f3"
    "h4f4h4f5h4f6h4g2h4g3h4g4h4g5h4g6h4h1h4h2h4h3h4h5h4h6h4h7h4h8h5a5"
    "h5b5h5c5h5d1h5d5h5e2h5e5h5e8h5f3h5f4h5f5h5f6h5f7h5g3h5g4h5g5h5g6"
    "h5g7h5h1h5h2h5h3h5h4h5h6h5h7h5h8h6a6h6b6h6c1h6c6h6d2h6d6h6e3h6e6"
    "h6f4h6f5h6f6h6f7h6f8h6g4h6g5h6g6h6g7h6g8h6h1h6h2h6h3h6h4h6h5h6h7"
    "h6h8h7a7h7b1h7b7h7c2h7c7h7d3h7d7h7e4h7e7h7f5h7f6h7f7h7f8h7g5h7g6"
    "h7g7h7g8h7h1h7h2h7h3h7h4h7h5h7h6h7h8h8a1h8a8h8b2h8b8h8c3h8c8h8d4"
    "h8d8h8e5h8e8h8f6h8f7h8f8h8g6h8g7h8g8h8h1h8h2h8h3h8h4h8h5h8h6h8h7";

/************************************
 **** Static function prototypes ****
 ************************************/

static Bitboard x_sq_set_of_diag( const int index );
static Bitboard x_sq_set_of_antidiag( const int index );
static void x_fen_to_pos_init_ppa( Pos *p, const char ppf[] );
static void x_fen_to_pos_init_turn_and_ca_flags( Pos *p, const char *acf,
    const char *caf, const char *fen );
static void x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen );
static void x_fen_to_pos_init_epts_file( Pos *p, const char eptsf[] );
static bool x_rawcode_preliminary_checks( const char *rawmove );
static void x_make_move_toggle_turn( Pos *p );
static void x_make_move_castle( Pos *p, Rawcode code );
static void x_make_move_remove_castling_rights( Pos *p, const char *color,
    const char *side );
static void x_make_move_regular( Pos *p, Rawcode code );

/****************************
 **** External functions ****
 ****************************/

// Converts the FEN string argument 'fen' to a Pos variable. The Pos
// variable is allocated dynamically so free() should be called when
// the variable is no longer needed.
Pos *
fen_to_pos( const char *fen )
{
    Pos *p = (Pos *) malloc( sizeof(Pos) );
    assert(p);
    char **ff = fen_fields(fen);
    assert(ff);

    x_fen_to_pos_init_ppa( p, ff[0] );
    x_fen_to_pos_init_turn_and_ca_flags( p, ff[1], ff[2], fen );
    x_fen_to_pos_init_irp( p, ff[2], fen );
    x_fen_to_pos_init_epts_file( p, ff[3] );
    // Should the values of the HMCF and the FMNF be checked before the
    // following assignments? Using che_fen_validator() to check the 'fen'
    // argument before the fen_to_pos() call is problematic because
    // fen_to_pos() is called also during the FEN string validation process.
    p->hmc = atoi( ff[4] ), p->fmn = atoi( ff[5] );

    free_fen_fields(ff);
    assert( !ppa_integrity_check( p->ppa ) );
    return p;
}

// TODO: ...
const char *
pos_to_fen( /* const Pos *p */ )
{
    // 1. Create expanded PPF from p->ppa[] --> x_create_expanded_ppf()
    // void ppa_to_eppf( const Bitboard *pp, char *eppf );
    // void eppf_to_ppa( const char *eppf, Bitboard *pp );
    // char eppf[ PPF_MAX_LENGTH + 1 ];
    // x_create_expanded_ppf( p->pp, eppf );
    // 2. Use compress_eppf()

    return NULL;
}

// The call sq_set_of_file( 'a' ) would return the constant SS_FILE_A.
// The main purpose of the function is to not have to define an array
// version of the SS_FILE_? constants.
Bitboard
sq_set_of_file( const char file )
{
    switch( file ) {
        case 'a': return SS_FILE_A;
        case 'b': return SS_FILE_B;
        case 'c': return SS_FILE_C;
        case 'd': return SS_FILE_D;
        case 'e': return SS_FILE_E;
        case 'f': return SS_FILE_F;
        case 'g': return SS_FILE_G;
        case 'h': return SS_FILE_H;

        default: assert( false ); return 0;
    }
}

// The call sq_set_of_rank( '1' ) would return the constant SS_RANK_1.
// The main purpose of the function is the same as in sq_set_of_file().
Bitboard
sq_set_of_rank( const char rank )
{
    switch( rank ) {
        case '1': return SS_RANK_1;
        case '2': return SS_RANK_2;
        case '3': return SS_RANK_3;
        case '4': return SS_RANK_4;
        case '5': return SS_RANK_5;
        case '6': return SS_RANK_6;
        case '7': return SS_RANK_7;
        case '8': return SS_RANK_8;

        default: assert( false ); return 0;
    }
}

// Can be used to collectively access the SS_DIAG_* constants as if they
// were elements of the same array. The index of diagonal h1h1 is 0 and
// the index of a8a8 is 14.
Bitboard
sq_set_of_diag( const int index )
{
    return x_sq_set_of_diag( index );
}

// Same as above for the SS_ANTIDIAG_* constants. The index of antidiagonal
// a1a1 is 0 and the index of h8h8 14.
Bitboard
sq_set_of_antidiag( const int index )
{
    return x_sq_set_of_antidiag( index );
}

// Returns the square set of the white army. That means all the squares
// that contain a white chessman.
Bitboard
ss_white_army( const Pos *p )
{
    return p->ppa[ WHITE_KING ] | p->ppa[ WHITE_QUEEN ] |
        p->ppa[ WHITE_ROOK ] | p->ppa[ WHITE_BISHOP ] |
        p->ppa[ WHITE_KNIGHT ] | p->ppa[ WHITE_PAWN ];
}

// Returns the square set of the black army. That means all the squares
// that contain a black chessman.
Bitboard
ss_black_army( const Pos *p )
{
    return p->ppa[ BLACK_KING ] | p->ppa[ BLACK_QUEEN ] |
        p->ppa[ BLACK_ROOK ] | p->ppa[ BLACK_BISHOP ] |
        p->ppa[ BLACK_KNIGHT ] | p->ppa[ BLACK_PAWN ];
}

// The Square Navigator returns the square bit that is in direction 'dir'
// of square bit 'sq'. For example, the call sq_nav( SB.e4, NORTHEAST )
// would return SB.f5 and the call sq_nav( SB.e4, ONE_OCLOCK ) would
// return SB.f6.
Bitboard
sq_nav( Bitboard sq, enum sq_dir dir )
{
    return SQ_NAV[ sq_bit_index( sq ) ][ dir ];
}

// Examines the Pos variable 'p' to see if there's a castling
// right/availability for color 'color' on side 'side'. For example,
// if the call has_castling_right( p, "white", "kingside" ) returns true,
// White has the kingside castling right. In a standard FEN CAF this would
// translate to the presence of the character 'K'. It's important to
// remember that having a castling right is different from being able to
// castle in the given position. For example, all the castling rights are
// present in the standard starting position, yet "O-O" or "O-O-O" are not
// available as moves.
bool
has_castling_right( const Pos *p, const char *color, const char *side )
{
    assert( !strcmp( color, "white" ) || !strcmp( color, "black" ) );
    assert( !strcmp( side, "queenside" ) || !strcmp( side, "kingside" ) ||
        !strcmp( side, "a-side" ) || !strcmp( side, "h-side" ) );

    bool white = !strcmp( color, "white" ), kingside =
        ( !strcmp( side, "kingside" ) || !strcmp( side, "h-side" ) );

    uint8_t bit = ( white ? 8 : 2 );
    if( !kingside ) bit >>= 1;
    assert( is_sq_bit( bit ) );

    return bit & p->turn_and_ca_flags;
}

// Returns the square bit corresponding to the en passant target square
// of the Pos variable 'p' or zero if there is no EPTS set.
Bitboard
epts( const Pos *p )
{
    if( !p->epts_file ) return 0;

    Bitboard bb = p->epts_file;
    bb <<= ( whites_turn(p) ? 40 : 16 );

    assert( is_sq_bit(bb) );
    return bb;
}

// Returns the rawcode corresponding to the 'rawmove' argument. If
// 'rawmove' is not valid, then zero is returned.
Rawcode
rawcode( const char *rawmove )
{
    if( !x_rawcode_preliminary_checks(rawmove) )
        return 0;

    char current[4 + 1] = {'\0'};
    const int N = sizeof(APM_DATA)/4;
    int left = 0, right = N - 1;

    // Binary search algorithm
    while( left <= right ) {
        int middle = (left + right) / 2;
        assert( middle >= 0 && middle < N );

        for( int i = 0; i < 4; i++ )
            current[i] = APM_DATA[4*middle + i];

        if( strcmp( current, rawmove ) < 0 ) {
            left = middle + 1;
            continue;
        } else if( strcmp( current, rawmove ) > 0 ) {
            right = middle - 1;
            continue;
        }

        return middle + 1;
    }

    return 0;
}

// Converts 'rawcode' into the corresponding rawmove and saves the
// result in 'writable' which is assumed to point to an array of
// at least five bytes.
void
rawmove( Rawcode rawcode, char *writable )
{
    assert( rawcode >= 1 );
    assert( rawcode <= 1792 );

    writable[4] = '\0';

    const char *ptr = &APM_DATA[4*(rawcode - 1)];
    for( int i = 0; i < 4; i++ )
        writable[i] = ptr[i];
}

// TODO: doc
void
make_move( Pos *p, Rawcode code, char promotion )
{
    promotion = 0;
    if(promotion) assert(false);

    uint32_t info_bits = move_info( p, code );

    p->epts_file = 0;

    // bool short_castle( const Pos *p, Rawcode code );
    // bool long_castle( const Pos *p, Rawcode code );
    // if( short_castle(p, code) || long_castle(p, code) )
    if( info_bits & MIB_CASTLE )
        x_make_move_castle( p, code );
    else
        x_make_move_regular( p, code );

    x_make_move_toggle_turn(p);

    if( true ) p->hmc++;
    if( whites_turn(p) ) p->fmn++;

    assert( !ppa_integrity_check( p->ppa ) );
}

/****************************
 ****                    ****
 ****  Static functions  ****
 ****                    ****
 ****************************/

static Bitboard
x_sq_set_of_diag( const int index )
{
    switch( index ) {
        case 0: return SS_DIAG_H1H1;
        case 1: return SS_DIAG_G1H2;
        case 2: return SS_DIAG_F1H3;
        case 3: return SS_DIAG_E1H4;
        case 4: return SS_DIAG_D1H5;
        case 5: return SS_DIAG_C1H6;
        case 6: return SS_DIAG_B1H7;
        case 7: return SS_DIAG_A1H8;
        case 8: return SS_DIAG_A2G8;
        case 9: return SS_DIAG_A3F8;
        case 10: return SS_DIAG_A4E8;
        case 11: return SS_DIAG_A5D8;
        case 12: return SS_DIAG_A6C8;
        case 13: return SS_DIAG_A7B8;
        case 14: return SS_DIAG_A8A8;

        default: assert( false ); return 0u;
    }
}

static Bitboard
x_sq_set_of_antidiag( const int index )
{
    switch( index ) {
        case 0: return SS_ANTIDIAG_A1A1;
        case 1: return SS_ANTIDIAG_B1A2;
        case 2: return SS_ANTIDIAG_C1A3;
        case 3: return SS_ANTIDIAG_D1A4;
        case 4: return SS_ANTIDIAG_E1A5;
        case 5: return SS_ANTIDIAG_F1A6;
        case 6: return SS_ANTIDIAG_G1A7;
        case 7: return SS_ANTIDIAG_H1A8;
        case 8: return SS_ANTIDIAG_H2B8;
        case 9: return SS_ANTIDIAG_H3C8;
        case 10: return SS_ANTIDIAG_H4D8;
        case 11: return SS_ANTIDIAG_H5E8;
        case 12: return SS_ANTIDIAG_H6F8;
        case 13: return SS_ANTIDIAG_H7G8;
        case 14: return SS_ANTIDIAG_H8H8;

        default: assert( false ); return 0u;
    }
}

static void
x_fen_to_pos_init_ppa( Pos *p, const char ppf[] )
{
    char eppf[PPF_MAX_LENGTH + 1];
    expand_ppf( ppf, eppf ), eppf_to_ppa( eppf, p->ppa );
}

// turn_and_ca_flags:
//  7   6   5   4   3   2   1   0   <= Bit indexes
// [x] [ ] [ ] [ ] [x] [ ] [ ] [x]  <= Bit values
//  t               K   Q   k   q   <= Meaning
//
// From the above we learn that it is White's turn and that the remaining
// castling rights in the Pos variable 'p' are white kingside (K) and
// black queenside (q).
static void
x_fen_to_pos_init_turn_and_ca_flags( Pos *p, const char *acf,
    const char *caf, const char *fen )
{
    char ecaf[9 + 1];
    EXPAND_CAF( caf, ecaf, fen )
    assert( strlen( ecaf ) == 4 );

    p->turn_and_ca_flags = 0;
    if( !strcmp( acf, "w" ) ) p->turn_and_ca_flags |= ( 1 << 7 );

    if( ecaf[1] != '-' ) p->turn_and_ca_flags |= 8; // K
    if( ecaf[0] != '-' ) p->turn_and_ca_flags |= 4; // Q
    if( ecaf[3] != '-' ) p->turn_and_ca_flags |= 2; // k
    if( ecaf[2] != '-' ) p->turn_and_ca_flags |= 1; // q
}

// irp[0] and irp[1]:
//  7   6   5   4   3   2   1   0   <= Bit indexes
// [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]  <= Bit values
//  h   g   f   e   d   c   b   a   <= Meaning
//
// The default queenside and kingside bits are those at indexes 0 and 7,
// respectively. This means that if the bit of the queenside or kingside
// IRP cannot be determined from the CAF, the default is used. Therefore
// irp[0] and irp[1] have always exactly one bit set.
static void
x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen )
{
    char ecaf[9 + 1];
    EXPAND_CAF( caf, ecaf, fen )
    assert( strlen( ecaf ) == 4 );

    p->irp[0] = 0, p->irp[1] = 0;

    if( ecaf[0] != '-' ) p->irp[0] |= ( 1 << ( ecaf[0] - 'A' ) );
    else if( ecaf[2] != '-' ) p->irp[0] |= ( 1 << ( ecaf[2] - 'a' ) );
    else p->irp[0] |= 1;

    if( ecaf[1] != '-' ) p->irp[1] |= ( 1 << ( ecaf[1] - 'A' ) );
    else if( ecaf[3] != '-' ) p->irp[1] |= ( 1 << ( ecaf[3] - 'a' ) );
    else p->irp[1] |= 0x80;

    assert( is_sq_bit( p->irp[0] ) );
    assert( is_sq_bit( p->irp[1] ) );
    assert( p->irp[0] < p->irp[1] );
}

static void
x_fen_to_pos_init_epts_file( Pos *p, const char eptsf[] )
{
    p->epts_file = 0;
    if( !strcmp( eptsf, "-" ) ) return;

    char file = eptsf[0];
    assert( file >= 'a' && file <= 'h' );
    p->epts_file |= ( 1 << ( file - 'a' ) );
}

static bool
x_rawcode_preliminary_checks( const char *rawmove )
{
    if( !rawmove || strlen(rawmove) != 4 )
        return false;

    char src[2 + 1] = {'\0'}, dst[2 + 1] = {'\0'};
    src[0] = rawmove[0], src[1] = rawmove[1];
    dst[0] = rawmove[2], dst[1] = rawmove[3];

    if( !is_sq_name(src) || !is_sq_name(dst) )
        return false;

    return true;
}

static void
x_make_move_remove_castling_rights( Pos *p, const char *color,
    const char *side )
{
    assert( !strcmp(color, "white") || !strcmp(color, "black") );
    assert( !strcmp(side, "both") || !strcmp(side, "kingside") ||
        !strcmp(side, "h-side") || !strcmp(side, "queenside") ||
        !strcmp(side, "a-side") );

    bool white = !strcmp(color, "white") ? true : false;

    if( !strcmp(side, "both") || !strcmp(side, "kingside") ||
            !strcmp(side, "h-side") ) {
        if( p->turn_and_ca_flags & (white ? 8 : 2) ) {
            p->turn_and_ca_flags ^= (white ? 8 : 2);
        }
    }
    if( !strcmp(side, "both") || !strcmp(side, "queenside") ||
            !strcmp(side, "a-side") ) {
        if( p->turn_and_ca_flags & (white ? 4 : 1) ) {
            p->turn_and_ca_flags ^= (white ? 4 : 1);
        }
    }
}

static void
x_make_move_toggle_turn( Pos *p )
{
    if( whites_turn(p) )
        p->turn_and_ca_flags ^= (1 << 7);
    else
        p->turn_and_ca_flags |= (1 << 7);
}

static void
x_make_move_castle( Pos *p, Rawcode code )
{
    int orig, dest;
    rawcode_bit_indexes( code, &orig, &dest );

    Bitboard castling_rook = ( SBA[dest] &
        (p->ppa[WHITE_ROOK] | p->ppa[BLACK_ROOK]) ),
        tmp = castling_rook;
    if( tmp > SB.h1 ) tmp >>= 56;

    uint8_t file_of_castling_rook = tmp;
    bool kingside = (file_of_castling_rook & p->irp[1]);

    p->ppa[EMPTY_SQUARE] ^= p->ppa[whites_turn(p) ? WHITE_KING : BLACK_KING];
    p->ppa[whites_turn(p) ? WHITE_KING : BLACK_KING] = 0;
    p->ppa[EMPTY_SQUARE] ^= castling_rook;
    p->ppa[whites_turn(p) ? WHITE_ROOK : BLACK_ROOK] ^= castling_rook;

    if( kingside && whites_turn(p) ) {
        p->ppa[WHITE_KING] = SB.g1, p->ppa[EMPTY_SQUARE] ^= SB.g1;
        p->ppa[WHITE_ROOK] |= SB.f1, p->ppa[EMPTY_SQUARE] ^= SB.f1;
    }
    else if( kingside && !whites_turn(p) ) {
        p->ppa[BLACK_KING] = SB.g8, p->ppa[EMPTY_SQUARE] ^= SB.g8;
        p->ppa[BLACK_ROOK] |= SB.f8, p->ppa[EMPTY_SQUARE] ^= SB.f8;
    }
    else if( whites_turn(p) ) { // queenside
        p->ppa[WHITE_KING] = SB.c1, p->ppa[EMPTY_SQUARE] ^= SB.c1;
        p->ppa[WHITE_ROOK] |= SB.d1, p->ppa[EMPTY_SQUARE] ^= SB.d1;
    }
    else if( !whites_turn(p) ) { // queenside
        p->ppa[BLACK_KING] = SB.c8, p->ppa[EMPTY_SQUARE] ^= SB.c8;
        p->ppa[BLACK_ROOK] |= SB.d8, p->ppa[EMPTY_SQUARE] ^= SB.d8;
    }
    else assert(false); // Should be impossible

    x_make_move_remove_castling_rights(
        p, whites_turn(p) ? "white" : "black", "both" );
}

static void
x_make_move_regular( Pos *p, Rawcode code )
{
    int orig, dest;
    rawcode_bit_indexes( code, &orig, &dest );
    Chessman mover = occupant_of_sq( p, SBA[orig] ),
        target = occupant_of_sq( p, SBA[dest] );
    assert( mover != EMPTY_SQUARE && mover != target );

    // Make the origin square vacant
    p->ppa[mover] ^= SBA[orig], p->ppa[EMPTY_SQUARE] |= SBA[orig];
    // Make the moving chessman "reappear" in the destination square
    p->ppa[mover] |= SBA[dest], p->ppa[target] ^= SBA[dest];
}

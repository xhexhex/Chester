#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "base.h"
#include "utils.h"
#include "validation.h"

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

// The following should be the only variable of type struct const_single_var_struct_SB.
// The variable 'SB' serves as a mapping between square names and square bits. This
// is mainly for the sake of convenience. For example, it is more convenient to refer
// to the square e8 with SB.e8 rather than 0x1000000000000000U.
const struct const_single_var_struct_SB SB = {
    ALL_SQ_BIT_VALUES
};

// The 64 members of the struct variable SB as an array. SBA[ 0 ] is
// SB.a1, SBA[ 63 ] is SB.h8.
const Bitboard SBA[] = {
    ALL_SQ_BIT_VALUES
};

// SNA, square names array. SNA[ 0 ] is "a1", SNA[ 1 ] is "b1", ...,
// SNA[ 63 ] is "h8".
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
// of a FEN record. The first element of the array is '-' and as such
// isn't one of the 12 letters. The order of the letters correspond
// to the Chessman enum type.
const char FEN_PIECE_LETTERS[] = "-KQRBNPkqrbnp";

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

const size_t
    // The minimum and maximum lengths for the PPF of a FEN string
    PPF_MIN_LENGTH = 17, PPF_MAX_LENGTH = 71,
    // The shortest possible FEN is something like "k7/8/8/8/8/8/8/K7 w - - 0 1"
    // which is 27 characters long. The following FEN is 90 chars long:
    // "rrrrkrrr/pppppppp/pppppppp/pppppppp/PPPPPPPP/PPPPPPPP/PPPPPPPP/RRRRKRRR w KQkq - 65535 65535"
    // Allowing FENs longer than this wouldn't seem to make sense.
    FEN_MIN_LENGTH = 27, FEN_MAX_LENGTH = 92;

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

// The maximum value for the numeric FEN fields (HMCF and FMNF)
const uint64_t FEN_NUMERIC_FIELD_MAX = 0xffffU; // 65535

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

/************************************
 **** Static function prototypes ****
 ************************************/

static Bitboard x_sq_set_of_diag( const int index );
static Bitboard x_sq_set_of_antidiag( const int index );
static bool x_chess960_start_pos_whites_first_rank( const Pos *p );
static bool x_chess960_start_pos_blacks_first_rank( const Pos *p );
static void x_fen_numeric_fields(
    const char *fen, uint16_t *i_hmc, uint16_t *i_fmn );
static void x_fen_to_pos_init_ppa( Pos *p, const char *ppf );
static void x_fen_to_pos_init_turn_and_ca_flags(
    Pos *p, const char *acf, const char *caf, const char *fen );
static void x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen );
static void x_fen_to_pos_init_epts_file( Pos *p, const char *eptsf );

/****************************
 **** External functions ****
 ****************************/

// Converts the FEN string argument 'fen' to a Pos variable
Pos *
fen_to_pos( const char *fen )
{
    Pos *p = (Pos *) malloc( sizeof( Pos ) );
    assert( p );
    char **ff = fen_fields( fen );
    assert( ff );

    x_fen_to_pos_init_ppa( p, ff[0] );
    x_fen_to_pos_init_turn_and_ca_flags( p, ff[1], ff[2], fen );
    x_fen_to_pos_init_irp( p, ff[2], fen );
    x_fen_to_pos_init_epts_file( p, ff[3] );
    p->hmc = (uint64_t) atoi( ff[4] ), p->fmn = (uint64_t) atoi( ff[5] );

    free_fen_fields( ff );
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

bool
white_has_a_side_castling_right( const Pos *p )
{
    return p->turn_and_ca_flags & 8;
}

bool
white_has_h_side_castling_right( const Pos *p )
{
    return p->turn_and_ca_flags & 4;
}

bool
black_has_a_side_castling_right( const Pos *p )
{
    return p->turn_and_ca_flags & 2;
}

bool
black_has_h_side_castling_right( const Pos *p )
{
    return p->turn_and_ca_flags & 1;
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
        !strcmp( side, "a_side" ) || !strcmp( side, "h_side" ) );

    bool white = !strcmp( color, "white" ), kingside =
        ( !strcmp( side, "kingside" ) || !strcmp( side, "h_side" ) );

    uint8_t bit = ( white ? 8 : 2 );
    if( !kingside ) bit >>= 1;
    assert( bb_is_sq_bit( (Bitboard) bit ) );

    return bit & p->turn_and_ca_flags;
}

// Returns true if 'p' is found to represent one of the 960 Chess960
// starting positions.
bool
chess960_start_pos( const Pos *p )
{
    return
        // p->info == 0x200001fU &&
        p->ppa[ EMPTY_SQUARE ] ==
            ( SS_RANK_3 | SS_RANK_4 | SS_RANK_5 | SS_RANK_6 ) &&
        p->ppa[ WHITE_PAWN ] == SS_RANK_2 &&
        p->ppa[ BLACK_PAWN ] == SS_RANK_7 &&
        x_chess960_start_pos_whites_first_rank( p ) &&
        x_chess960_start_pos_blacks_first_rank( p );
}

// Convert piece placement data from 'ppa' array to expanded PPF format. 'ppa' is
// the namesake member of the Pos struct. The following is an example of a PPF
// in both standard ("compressed") and expanded form.
//
// rn2kb1r/ppp1qppp/5n2/4p3/2B1P3/1Q6/PPP2PPP/RNB1K2R
// rn--kb-r/ppp-qppp/-----n--/----p---/--B-P---/-Q------/PPP--PPP/RNB-K--R
//
// It is assumed that parameter 'eppf' is a writable array with a size of at least
// PPF_MAX_LENGTH + 1 bytes.
void
ppa_to_eppf( const Bitboard *pp, char *eppf )
{
    int eppf_index = -1;

    for( int i = 56; i >= 0; i -= 8 )
        for( int j = 0; j <= 7; j++ ) {
            int bi = i + j;
            Bitboard sq_bit = SBA[ bi ];
            eppf_index += ( !( bi % 8 ) && bi != 56 ) ? 2 : 1;

            if(      sq_bit & pp[ EMPTY_SQUARE ] ) eppf[ eppf_index ] = '-';
            else if( sq_bit & pp[ WHITE_KING   ] ) eppf[ eppf_index ] = 'K';
            else if( sq_bit & pp[ WHITE_QUEEN  ] ) eppf[ eppf_index ] = 'Q';
            else if( sq_bit & pp[ WHITE_ROOK   ] ) eppf[ eppf_index ] = 'R';
            else if( sq_bit & pp[ WHITE_BISHOP ] ) eppf[ eppf_index ] = 'B';
            else if( sq_bit & pp[ WHITE_KNIGHT ] ) eppf[ eppf_index ] = 'N';
            else if( sq_bit & pp[ WHITE_PAWN   ] ) eppf[ eppf_index ] = 'P';
            else if( sq_bit & pp[ BLACK_KING   ] ) eppf[ eppf_index ] = 'k';
            else if( sq_bit & pp[ BLACK_QUEEN  ] ) eppf[ eppf_index ] = 'q';
            else if( sq_bit & pp[ BLACK_ROOK   ] ) eppf[ eppf_index ] = 'r';
            else if( sq_bit & pp[ BLACK_BISHOP ] ) eppf[ eppf_index ] = 'b';
            else if( sq_bit & pp[ BLACK_KNIGHT ] ) eppf[ eppf_index ] = 'n';
            else if( sq_bit & pp[ BLACK_PAWN   ] ) eppf[ eppf_index ] = 'p';
            else assert( false );
        }

    for( int i = 8; i <= 62; i += 9 ) eppf[ i ] = '/';
    eppf[ PPF_MAX_LENGTH ] = '\0';
}

// The inverse of ppa_to_eppf(). Keep in mind that 'eppf' is assumed to be at least
// PPF_MAX_LENGTH + 1 bytes and that the writable Bitboard array 'ppa' should have
// space for at least 13 elements.
void
eppf_to_ppa( const char *eppf, Bitboard *pp )
{
    for( int i = 0; i < 13; i++ ) pp[ i ] = 0;

    for( int i = 63, bi = 0; i >= 0; i -= 9 )
        for( int j = 0; j <= 7; j++, bi++ ) {
            int eppf_index = i + j;
            Bitboard sq_bit = SBA[ bi ];

            if(      eppf[ eppf_index ] == '-' ) pp[ EMPTY_SQUARE ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'K' ) pp[ WHITE_KING   ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'Q' ) pp[ WHITE_QUEEN  ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'R' ) pp[ WHITE_ROOK   ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'B' ) pp[ WHITE_BISHOP ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'N' ) pp[ WHITE_KNIGHT ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'P' ) pp[ WHITE_PAWN   ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'k' ) pp[ BLACK_KING   ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'q' ) pp[ BLACK_QUEEN  ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'r' ) pp[ BLACK_ROOK   ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'b' ) pp[ BLACK_BISHOP ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'n' ) pp[ BLACK_KNIGHT ] |= sq_bit;
            else if( eppf[ eppf_index ] == 'p' ) pp[ BLACK_PAWN   ] |= sq_bit;
            else assert( false );
        }
}

// Returns the integer equivalent of the FEN parameter's HMCF.
uint16_t
fen_hmcf( const char *fen )
{
    uint16_t i_hmc, unused;
    x_fen_numeric_fields( fen, &i_hmc, &unused );

    return i_hmc;
}

// Returns the integer equivalent of the FEN parameter's FMNF.
uint16_t
fen_fmnf( const char *fen )
{
    uint16_t i_fmn, unused;
    x_fen_numeric_fields( fen, &unused, &i_fmn );

    return i_fmn;
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

static bool
x_chess960_start_pos_whites_first_rank( const Pos *p )
{
    const Bitboard not_rank_1 = ~SS_RANK_1;

    if(
        p->ppa[ WHITE_KING ] & not_rank_1 ||
        p->ppa[ WHITE_QUEEN ] & not_rank_1 ||
        p->ppa[ WHITE_ROOK ] & not_rank_1 ||
        p->ppa[ WHITE_BISHOP ] & not_rank_1 ||
        p->ppa[ WHITE_KNIGHT ] & not_rank_1 ||
        num_of_sqs_in_sq_set( p->ppa[ WHITE_KING ] ) != 1 ||
        num_of_sqs_in_sq_set( p->ppa[ WHITE_QUEEN ] ) != 1 ||
        num_of_sqs_in_sq_set( p->ppa[ WHITE_ROOK ] ) != 2 ||
        num_of_sqs_in_sq_set( p->ppa[ WHITE_BISHOP ] ) != 2 ||
        num_of_sqs_in_sq_set( p->ppa[ WHITE_KNIGHT ] ) != 2
    )
        return false;

    Bitboard western_rook = SB.a1, king = p->ppa[ WHITE_KING ];
    while( !( western_rook & p->ppa[ WHITE_ROOK ] ) )
        western_rook <<= 1;
    Bitboard eastern_rook = ( western_rook ^ p->ppa[ WHITE_ROOK ] );

    if( !( western_rook < king ) || !( king < eastern_rook ) )
        return false;

    Bitboard black_sqs_of_rank_1 = SB.a1 | SB.c1 | SB.e1 | SB.g1,
        white_sqs_of_rank_1 = SB.b1 | SB.d1 | SB.f1 | SB.h1,
        bishops = p->ppa[ WHITE_BISHOP ];

    return bishops & black_sqs_of_rank_1 &&
        bishops & white_sqs_of_rank_1;
}

static bool
x_chess960_start_pos_blacks_first_rank( const Pos *p )
{
    return
        ( p->ppa[ WHITE_KING ] << 56 ) == p->ppa[ BLACK_KING ] &&
        ( p->ppa[ WHITE_QUEEN ] << 56 ) == p->ppa[ BLACK_QUEEN ] &&
        ( p->ppa[ WHITE_ROOK ] << 56 ) == p->ppa[ BLACK_ROOK ] &&
        ( p->ppa[ WHITE_BISHOP ] << 56 ) == p->ppa[ BLACK_BISHOP ] &&
        ( p->ppa[ WHITE_KNIGHT ] << 56 ) == p->ppa[ BLACK_KNIGHT ];
}

static void
x_fen_numeric_fields( const char *fen, uint16_t *i_hmc, uint16_t *i_fmn )
{
    int si = 0, space_count = 0, i = 0; // si, starting index
    while( space_count < 4 ) if( fen[ si++ ] == ' ' ) ++space_count;

    char s_hmc[ 5 + 1 ] = { '\0' }, s_fmn[ 5 + 1 ] = { '\0' };
    bool first = true;
    do {
        if( fen[ si ] == ' ' ) {
            first = false;
            i = 0;
            continue; }

        if( first ) s_hmc[ i++ ] = fen[ si ];
        else s_fmn[ i++ ] = fen[ si ];
    } while( fen[ ++si ] );

    size_t l1 = strlen( s_hmc ), l2 = strlen( s_fmn );
    assert( l1 >= 1 && l1 <= 5 && l2 >= 1 && l2 <= 5 );

    *i_hmc = atoi( s_hmc ), *i_fmn = atoi( s_fmn );
}

static void
x_fen_to_pos_init_ppa( Pos *p, const char *ppf )
{
    for( int i = 0; i < 13; i++ ) p->ppa[i] = 0;

    char eppf[PPF_MAX_LENGTH + 1], ppf_0_to_63[64 + 1];
    ppf_0_to_63[64] = '\0';

    expand_ppf( ppf, eppf );

    int k = -1;
    for( int i = 63; i >= 0; i -= 9 )
        for( int j = i; j <= i + 7; j++ )
            ppf_0_to_63[++k] = eppf[j];

    assert( k == 63 && strlen( ppf_0_to_63 ) == 64 );

    for( Chessman cm = EMPTY_SQUARE; cm <= BLACK_PAWN; cm++ )
        for( int index = 0; index < 64; index++ )
            if( FEN_PIECE_LETTERS[cm] == ppf_0_to_63[index] )
                p->ppa[cm] |= SBA[index];
}

static void
x_fen_to_pos_init_turn_and_ca_flags( Pos *p, const char *acf,
    const char *caf, const char *fen )
{
    char ecaf[10];
    EXPAND_CAF( caf, ecaf, fen )
    assert( strlen( ecaf ) == 4 );

    p->turn_and_ca_flags = 0;
    if( !strcmp( acf, "w" ) ) p->turn_and_ca_flags |= ( 1 << 7 );

    for( int i = 0; i < 4; i++ )
        if( ecaf[i] != '-' )
            p->turn_and_ca_flags |= ( 8 >> i );

    /*
    for( int i = 0; i < 4; i++ )
        if( ecaf[3 - i] != '-' )
            p->turn_and_ca_flags |= ( 1 << i );
    */
}

static void
x_fen_to_pos_init_irp( Pos *p, const char *caf, const char *fen )
{
    char ecaf[10];
    EXPAND_CAF( caf, ecaf, fen )
    assert( strlen( ecaf ) == 4 );

    p->irp[0] = 0, p->irp[1] = 0;

    if( ecaf[0] != '-' ) p->irp[0] |= ( 1 << ( ecaf[0] - 'A' ) );
    else if( ecaf[2] != '-' ) p->irp[0] |= ( 1 << ( ecaf[2] - 'a' ) );

    if( ecaf[1] != '-' ) p->irp[1] |= ( 1 << ( ecaf[1] - 'A' ) );
    else if( ecaf[3] != '-' ) p->irp[1] |= ( 1 << ( ecaf[3] - 'a' ) );

    assert( !p->irp[0] || !p->irp[1] || p->irp[0] < p->irp[1] );
}

static void
x_fen_to_pos_init_epts_file( Pos *p, const char *eptsf )
{
    p->epts_file = 0;
    if( !strcmp( eptsf, "-" ) ) return;

    char file = eptsf[0];
    assert( file >= 'a' && file <= 'h' );
    p->epts_file |= ( 1 << ( file - 'a' ) );
}

## The Chester FEN string validation scheme

*(TODO: There should be a document that describes what a FEN string is in Chester.)*

FEN string validation in Chester involves a number of tests which are numbered starting from one. The tests should be performed in order and one after the other, and if a particular test fails, no further tests should be done on the prospective FEN string. For example, if test 3 is done on a string, it implies that the string has already passed tests 1 and 2.

The prototype of the function for FEN string validation is defined in `chester.h` as follows:  
`enum che_fen_error che_fen_validator( const char *fen );`

Apart from `FEN_NO_ERRORS`, each of the `enum che_fen_error` constants represents the failure of a particular test. The test failure associated with each of the constants is described in the following. This also serves as a description of the tests themselves (a test is a success if and only if its associated error conditions are not fulfilled).

1.  `FEN_LENGTH_ERROR`  
    The string is either less than `FEN_MIN_LENGTH` chars or more than
    `FEN_MAX_LENGTH` chars long. The error is generated also when
    `che_fen_validator()` receives a null pointer.
2.  `FEN_CHARS_ERROR`  
    The string contains characters that cannot occur in any (Shredder-)FEN string.
    More specifically the string doesn't match the following regex:  
    `^[KkQqRrBbNnPp/AaCcDdEeFfGgHhw 0123456789-]*$`
3.  `FEN_FIELD_STRUCTURE_ERROR`  
    The string doesn't consist of six fields separated by single space
    characters. That is, the string doesn't match the following regex:  
    `^[^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+$`
4.  `FEN_PPF_STRUCTURE_ERROR`  
    The PPF (piece placement field) doesn't match the regex `^r/r/r/r/r/r/r/r$`
    where `r` stands for `[KkQqRrBbNnPp12345678]{1,8}`.
5.  `FEN_PPF_CONSECUTIVE_DIGITS_ERROR`  
    The PPF contains consecutive digits such as "12". As digits in a PPF
    represent consecutive empty squares and the maximum number of empty
    squares in a rank is eight, only one digit is needed to express a
    particular number of consecutive empty squares.
6.  `FEN_PPF_RANK_SUM_ERROR`  
    One (or more) of the ranks of the PPF has a rank sum not equal to eight.
    The rank sum means the square count of the rank. The PPF ranks "8", "RNBQKBNR"
    and "r1q1b2n" all have a rank sum of eight whereas the rank sum of "pp2pp" is six.
7.  `FEN_ACF_ERROR`  
    The ACF (active color field) is something else than "w" or "b".
8.  `FEN_CAF_ERROR`  
    The CAF (castling availability field) is not valid. Examples of valid CAF
    values include "-", "KQkq", "Kq", "k", "AHah", "HAha", "CEce", "CEc" and "Ce".  
    *(TODO: Provide a link to the comprehensive description of the CAF in Chester.)*
9.  `FEN_EPTSF_ERROR`  
    The EPTSF (en passant target square field) is a string that does not match
    the regex `^[a-h][36]$` and is not the string "-".
10. `FEN_HMCF_ERROR`  
    The HMCF (halfmove clock field) is not a decimal integer string with a value
    between 0 and `FEN_NUMERIC_FIELD_MAX`. Note that the only acceptable characters
    in the string are decimal digits.
11. `FEN_FMNF_ERROR`  
    The FMNF (fullmove number field) is not a decimal integer string with a value
    between 1 and `FEN_NUMERIC_FIELD_MAX`. Note that the only acceptable characters
    in the string are decimal digits.
12. `FEN_PPF_CONTRADICTS_CAF_ERROR`  
    The piece placement and castling availability fields don't make sense when
    considered together. For each of the four castling availabilities to make
    sense, there has to be a king and a rook on the appropriate squares. For
    example, if the CAF is "K" (which in Chester is a synonym for "H"), there
    should be a white rook on h1 and a white king somewhere else on the first
    rank excluding square a1. Another example: if the CAF is "BD", there should
    be white rooks on b1 and d1 and a white king on c1.
13. `FEN_EPTSF_CONTRADICTS_HMCF_ERROR`  
    The EPTSF is something else than "-" while the HMCF is something else than "0".
    For example, if the EPTSF is "e3", it follows that the HMCF is "0"; any other
    value is a contradiction.
14. `FEN_EPTSF_CONTRADICTS_ACF_ERROR`  
    This error occurs if (1) EPTSF matches the regex `^[a-h]3$` and
    AC is "w" or (2) EPTSF matches the regex `^[a-h]6$` and AC is "b".
15. `FEN_EPTSF_CONTRADICTS_PPF_ERROR`  
    The EPTSF is not "-" and yet there is no pawn of the appropriate color on the
    square after the EPTS. For example, if the EPTS is "e3", then the pawn on e2
    has just made a double advance to e4. So it follows from EPTS being "e3" that
    there is a white pawn on e4.
16. `FEN_WHITE_PAWN_ON_FIRST_RANK`  
    There are one or more white pawns on rank 1.
17. `FEN_BLACK_PAWN_ON_FIRST_RANK`  
    There are one or more black pawns on rank 8 (which is the first rank
    from Black's perspective).
18. `FEN_WHITE_PAWN_ON_LAST_RANK`  
    There are one or more white pawns on rank 8.
19. `FEN_BLACK_PAWN_ON_LAST_RANK`  
    There are one or more black pawns on rank 1.
20. `FEN_INVALID_NUMBER_OF_WHITE_KINGS`  
    There is either no white king on the chessboard or multiple white kings.
21. `FEN_INVALID_NUMBER_OF_BLACK_KINGS`  
    There is either no black king on the chessboard or multiple black kings.
22. `FEN_WHITE_KING_CAN_BE_CAPTURED`  
    It is Black's turn to move and the white king is on an attacked square.
23. `FEN_BLACK_KING_CAN_BE_CAPTURED`  
    It is White's turn to move and the black king is on an attacked square.

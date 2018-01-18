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
12. `FEN_KING_PLACEMENT_CONTRADICTS_CAF_ERROR`  
    One or both of the kings are placed in a manner that contradicts the fact that
    certain types of castling availabilities are present. The error cannot occur if
    the CAF is "-". The error occurs if either (1) the white king is on a square other
    than one in the set { b1, c1, d1, e1, f1, g1 } and White has an a-side or h-side
    castling availability or (2) the black king is on a square other than one in the
    set { b8, c8, d8, e8, f8, g8 } and Black has an a-side or h-side castling
    availability. The error also occurs if (3) both sides have at least one castling
    availability (either a-side or h-side) and the kings are not on the same file
    (if both sides have castling availability, it follows that neither of the kings
    has been moved during the game).
13. `FEN_ROOK_PLACEMENT_CONTRADICTS_CAF_ERROR`  
    The placement of one or more of the rooks contradicts the fact that certain
    types of castling availabilities are present. For example, if the CAF is "CF",
    then there should be white rooks on the squares c1 and f1 and the white king
    should be on either of the squares d1 or e1. Another example: if the CAF is "g",
    then there should be a black rook on g8 and the black king should be located on
    one of the squares in the set { b8, c8, d8, e8, f8 }.
14. `FEN_EPTSF_CONTRADICTS_HMCF_ERROR`  
    The EPTSF is something else than "-" while the HMCF is something else than "0".
    For example, if the EPTSF is "e3", it follows that the HMCF is "0"; any other
    value is a contradiction.
15. `FEN_EPTSF_CONTRADICTS_ACF_ERROR`  
    This error occurs if (1) EPTSF matches the regex `^[a-h]3$` and
    AC is "w" or (2) EPTSF matches the regex `^[a-h]6$` and AC is "b".
16. `FEN_EPTSF_CONTRADICTS_PPF_ERROR`  
    The EPTSF is not "-" and yet there is no pawn of the appropriate color on the
    square after the EPTS. For example, if the EPTS is "e3", then the pawn on e2
    has just made a double advance to e4. So it follows from EPTS being "e3" that
    there is a white pawn on e4.
17. `FEN_PAWN_ON_INVALID_RANK`  
    There are one or more pawns of either color on rank 1 or rank 8.
18. `FEN_INVALID_NUMBER_OF_KINGS`  
    There is either no king of a particular color on the chessboard or there are
    multiple kings of that particular color on the chessboard.
19. `FEN_KING_CAN_BE_CAPTURED`  
    It is either (1) Black's turn to move while the white king is on an attacked
    square or (2) White's turn to move while the black king is on an attacked square.

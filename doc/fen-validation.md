## The Chester FEN string validation scheme

FEN string validation in Chester involves a number of tests which are numbered starting from one. The tests should be performed in order and one after the other, and if a particular test fails, no further tests should be done of the prospective FEN string. For example, if test 3 is done on a string, it implies that the string has already passed tests 1 and 2.

The prototype of the function for FEN string validation is defined in `chester.h` as follows:  
`enum che_fen_error che_fen_validator( const char *fen );`

Apart from `FEN_NO_ERRORS`, each of the `enum che_fen_error` constants represents the failure of a particular test. The test failure associated with each of the constants is described in the following. This also serves as a description of the tests themselves (a test is a success if and only if its associated error conditions are not fulfilled).

1.  `FEN_LENGTH_ERROR`  
    The string is either less than `FEN_MIN_LENGTH` chars or more than
    `FEN_MAX_LENGTH` chars long. The error is generated also when
    `che_fen_validator()` receives a null pointer.
2.	FEN_STR_CHARS_ERROR
	The string contains characters that cannot occur in any FEN string.
	In more exact terms the string doesn't match the following regex:
	"^[KkQqRrBbNnPp/acdefghw 0123456789-]*$"
3.	FEN_STR_FIELD_STRUCTURE_ERROR
	The string doesn't consist of six fields separated by single space
	characters. That is, the string doesn't match the regex
	"^[^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+$"
4.	FEN_STR_PPF_STRUCTURE_ERROR
	The piece placement field (the first of the six fields) doesn't match
	the regex "^r/r/r/r/r/r/r/r$" where r = "[KkQqRrBbNnPp12345678]{1,8}".
5.	FEN_STR_PPF_CONSECUTIVE_DIGITS_ERROR
	The PPF contains consecutive digits such as "12". As digits in a PPF
	represent consecutive empty squares and the maximum number of empty
	squares in a rank is eight, only one digit is needed to express a
	particular number of consecutive empty squares.
6.	FEN_STR_PPF_RANK_SUM_ERROR
	One (or more) of the ranks of the PPF has a rank sum not equal to eight.
7.	FEN_STR_ACF_ERROR
	The active color field (the second of the six fields) is something
	else than "w" or "b".
8.	FEN_STR_CAF_ERROR
	The castling availability field (the third of the six fields) is
	a string not included in the following 16-element set:
	{ "-", "K", "Q", "k", "q", "KQ", "Kk", "Kq", "Qk", "Qq", "kq",
	"Qkq", "Kkq", "KQq", "KQk", "KQkq" }
9.	FEN_STR_EPTSF_ERROR
	The en passant target square field (the fourth of the six fields) is
	a string that does not match the regex "^[a-h][36]$" and is not the
	string "-".
10.	FEN_STR_HMCF_ERROR
	The halfmove clock field (the fifth of the six fields) is not a
	decimal integer string with a value between 0 and 2^12 - 1 = 4095.
	Note that the only acceptable characters in the string are
	decimal digits.
11.	FEN_STR_FMNF_ERROR
	The fullmove number field (the last of the six fields) is not a
	decimal integer string with a value between 1 and 2^12 - 1 = 4095.
	Note that the only acceptable characters in the string are
	decimal digits.
12.	FEN_STR_PPF_CONTRADICTS_CAF_ERROR
	The piece placement and castling availability fields don't make
	sense when considered together. For each of the four castling
	availabilities to make sense, there has to be a king and a rook
	in the appropriate places. For example, if the CAF is "K", there
	has to be a white king on e1 and a white rook on h1. NOTE: If
	CFSV_BF_CHESS960 is set, then the test is skipped and test 24 is
	done instead (after completing tests 13 to 23).
13.	FEN_STR_EPTSF_CONTRADICTS_HMCF_ERROR
	The EPTSF is something else than "-" while the HMCF is something
	else than "0". For example, if the EPTSF is "e3", it follows that
	the HMCF is "0"; any other value is a contradiction.
14.	FEN_STR_EPTSF_CONTRADICTS_ACF_ERROR
	This error occurs if (1) EPTSF matches the regex "^[a-h]3$" and
	AC is "w" or (2) EPTSF matches the regex "^[a-h]6$" and AC is "b".
15.	FEN_STR_EPTSF_CONTRADICTS_PPF_ERROR
	The EPTSF is not "-" and yet there is no pawn of the appropriate
	color on the square after the EPTS. For example, if the EPTS is "e3",
	then the pawn on e2 has just made a double advance to e4. So it
	follows from EPTS being "e3" that there is a white pawn on e4.
16.	FEN_STR_WHITE_PAWN_ON_FIRST_RANK
	There are one or more white pawns on rank 1.
17.	FEN_STR_BLACK_PAWN_ON_FIRST_RANK
	There are one or more black pawns on rank 8 (which is the first rank
	from Black's perspective).
18.	FEN_STR_WHITE_PAWN_ON_LAST_RANK
	There are one or more white pawns on rank 8.
19.	FEN_STR_BLACK_PAWN_ON_LAST_RANK
	There are one or more black pawns on rank 1.
20.	FEN_STR_INVALID_NUMBER_OF_WHITE_KINGS
	There is either no white king on the chessboard or multiple white kings.
21.	FEN_STR_INVALID_NUMBER_OF_BLACK_KINGS
	There is either no black king on the chessboard or multiple black kings.
22.	FEN_STR_WHITE_KING_CAN_BE_CAPTURED
	It is Black's turn to move and the white king is on an attacked square.
23.	FEN_STR_BLACK_KING_CAN_BE_CAPTURED
	It is White's turn to move and the black king is on an attacked square.
24.	FEN_STR_CHESS960_PPF_CONTRADICTS_CAF_ERROR
    The Chess960 version of FEN_STR_PPF_CONTRADICTS_CAF_ERROR. The test is done
	only if CFSV_BF_CHESS960 is set. If the a-side castling right "Q" or "q"
	is available, it means that the king is on one of the squares b1 to g1
	(or b8 to g8) inclusive and that there is a rook of the same color one or
	more squares to the left of the king. The same goes for the h-side castling
	right "K" or "k", except that the rook of the same color is to the right of
	the king.

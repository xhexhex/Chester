## Forsyth–Edwards Notation (FEN) in Chester

The [FEN string](https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)
(AKA the FEN record) is the means of representing chess
positions in Chester on the external level (internally chess positions
are represented using [Pos variables](pos-spec.md)). Just like the standard
FEN string, FEN strings in Chester consist of the following six fields:

1.  Piece placement field (PPF)
2.  Active color field (ACF)
3.  Castling availability field (CAF)
4.  En passant target square field (EPTSF)
5.  Halfmove clock field (HMCF)
6.  Fullmove number field (FMNF)

Fields 1, 2, 4, 5 and 6 are just like those used in the standard FEN string.
Field 3, the CAF, uses the so-called
[Shredder-FEN](https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation#FEN_adjustment_for_chess_variants_like_Chess960)
scheme. This means that the letters appearing in the CAF identify the files
on which the rooks began the game. For example, considering the standard
start position, the rooks are initially located on the a-file and h-file. This
translates to the equivalent CAFs "AHah" and "HAha". In a Shredder-FEN CAF
uppercase letters should appear before lowercase letters.

It's possible to use standard FEN CAFs such as "KQkq" in Chester. The
interpretation of standard CAFs are by the conversions `'K' → 'H'`,
`'Q' → 'A'`, `'k' → 'h'` and `'q' → 'a'`. For example, the standard CAF
"KQkq" would get converted to "HAha" or "AHah".

*(TODO: Add technical notes on the lengths and limits of the fields.)*

# TODO
> A list of things I haven't had the self-discipline to get done

* Try compiling and using Chester on Windows.
* Investigate how to use Valgrind to check Chester for memory leaks and the like.
* Rename `che_validate_fen_str()` to `che_fen_str_validator()`.
* Choose the bit index of a new bit flag in `Pos`. The flag indicates whether
  Chess960 castling rules are in effect.
* Rename module `types.*` to `base.*`.
* Add module `extra.*` for the things that shouldn't really be in Chester but
  are still somehow useful.
* Rename `enum sq_direction` to `enum sq_dir`.
* Rename `pieces` of the Pos struct to `cmen`.
* Document `che_fen_str_validator()`.
* If the game being played is Chess960, the original squares of the rooks should be
  saved to avoid ambiguity in castling. Let's say the rook that was originally on the
  right side of the white king has moved to the left side of the white king. White can
  still castle a-side, yet now there are two rooks to the left of the king. It's not
  clear from the position which is the rook involved in castling.

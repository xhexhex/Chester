# TODO
> A list of things I haven't had the self-discipline to get done

* Try compiling and using Chester on Windows.
* Investigate how to use Valgrind to check Chester for memory leaks and the like.
* Rename `che_validate_fen_str()` to `che_fen_str_validator()`.
* Add a bit flags parameter to the prototype of `che_fen_str_validator()`. The
  new parameter is for settings. The first settings flag indicates whether the
  castling rules of Chess960 apply to the FEN string being validated.
* Choose the bit index of a new bit flag in `Pos`. The flag indicates whether
  Chess960 castling rules are in effect.

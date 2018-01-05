# TODO
> A list of things I haven't had the self-discipline to get done yet

* Try compiling and using Chester on Windows.
* Investigate how to use Valgrind to check Chester for memory leaks and the like.
* Add module `extra.*` for the things that shouldn't really be in Chester but
  are still somehow useful.
* Document `che_fen_validator()`.
* `fen_str_to_pos_var()` involves a call to `malloc()`, and I've forgotten all about
  this. No doubt there's a memory leak problem in Chester.
* Rename function `num_of_sqs_in_sq_set()` to `bit_count()`.
* Rename `enum fen_str_error` to `enum che_fen_error`
* Let's assume Chess960 castling rules are in effect and consider the position
  `4k3/8/8/8/8/8/8/RR2K3 w Q - 0 50`. Does the list of available moves include O-O-O?
  https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation#FEN_adjustment_for_chess_variants_like_Chess960
* Work on the integrity test function(s) of `fen_to_pos()`.
* Rename `compress_expanded_ppf()` to `compress_eppf()`

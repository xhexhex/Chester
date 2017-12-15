# TODO
> A list of things I haven't had the self-discipline to get done yet

* Try compiling and using Chester on Windows.
* Investigate how to use Valgrind to check Chester for memory leaks and the like.
* Add module `extra.*` for the things that shouldn't really be in Chester but
  are still somehow useful.
* Rename `pieces` of the Pos struct to `cmen`.
* Document `che_fen_validator()`.
* fen_str_to_pos_var() involves a call to malloc(), and I've forgotten all about
  this. No doubt there's a memory leak problem in Chester.
* Rename function `num_of_sqs_in_sq_set()` to `bit_count()`.
* Rename `fen_str_to_pos_var()` to `fen_to_pos()`.
* Rename `enum fen_str_error` to `enum che_fen_error`

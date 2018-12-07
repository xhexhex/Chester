# TODO
> A list of things I haven't had the self-discipline to get done yet

* Document `che_fen_validator()`
* Rename function `num_of_sqs_in_sq_set()` to `bit_count()`
* Rename `bb_is_sq_bit()` to `is_sq_bit()` and review
* validation_ts: src/validation.c:368: x_validate_fen_test_20: Assertion `p->fmn >= 1 && p->fmn <= 30000` failed.
* `valid_sq_name()` => `is_sq_name()`
* `FEN_HMCF_CONTRADICTS_FMNF_ERROR` is undocumented
* If it's White's turn and EPTS is set, the full move number should be greater than one
* If EPTS is set, then the two squares behind should be empty along with the EPTS itself
* Refactor and document `rawmoves()`
* Have a look at x_validate_fen_test_11(). There seems to be a "Conditional jump or move depends on uninitialised value" problem (detected by Valgrind).

# TODO
> A list of things I haven't had the self-discipline to get done yet

* Optimize `attackers()`
* Review `che_fen_validator()` -- more tests?
* Optimize and document `che_fen_validator()`
* Investigate: validation_ts: src/validation.c:368: x_validate_fen_test_20:
  Assertion `p->fmn >= 1 && p->fmn <= 30000` failed.
* `FEN_HMCF_CONTRADICTS_FMNF_ERROR` is undocumented
* Consider: if it's White's turn and EPTS is set, the full move number should
  be greater than one
* Consider: if EPTS is set, then the two squares behind should be empty along
  with the EPTS itself
* Have a look at `x_validate_fen_test_11()`. There seems to be a "Conditional
   jump or move depends on uninitialised value" problem (detected by Valgrind).
* Remove `sq_nav()`
* Refactor and document `rawcodes()`
* Remove `SBA[]`
* Add hashing support to `che_perft()`
* Update `in_between()` so it makes use of square rays
* Refactor and document `che_{build,free}_fen_gt()`
* Function `is_castle()` has been optimized but functions `is_short_castle()`
  and `is_long_castle()` have not
* I should get started with Chester again!

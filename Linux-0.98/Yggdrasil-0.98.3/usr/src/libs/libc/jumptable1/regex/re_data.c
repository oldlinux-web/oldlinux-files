#ifndef NO_JUMP_TABLE
#include <sys/types.h>
#include <regex.h>

#if 0
/* Roughly the maximum number of failure points on the stack.  Would be
  exactly that if always used MAX_FAILURE_SPACE each time we failed.
   This is a variable only so users of regex can assign to it; we never
  change it ourselves.  */
int re_max_failures = 2000;
#endif

/* Set by re_set_syntax to the current regexp syntax to recognize.  Can
   also be assigned to more or less arbitrarily.  Since we use this as
   a collection of bits, declaring it unsigned maximizes portability. */

reg_syntax_t obscure_syntax = 0;
#endif

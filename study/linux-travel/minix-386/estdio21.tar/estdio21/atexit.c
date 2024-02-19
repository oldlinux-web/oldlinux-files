/*                               a t e x i t                               */

#include "stdiolib.h"

/*LINTLIBRARY*/

#define MAX_HANDLERS	32		/* maximum number of handlers */

/* Exit handler list */
static atexit_t __exithandlers[MAX_HANDLERS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* Exit handler list pointer */
static atexit_t *__exitpointer = &__exithandlers[0];

static void doexit F0()

{
  for ( ; __exitpointer > &__exithandlers[0]; )
    (*(*--__exitpointer))();
}

int atexit F1(atexit_t, fn)

{
  __Zatexit = doexit;

  return (__exitpointer < &__exithandlers[MAX_HANDLERS])
         ? (*__exitpointer++ = fn, 0)
	 : -1;
}

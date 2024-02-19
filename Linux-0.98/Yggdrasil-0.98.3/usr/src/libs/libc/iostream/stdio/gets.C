#include "stdioprivate.h"

#if defined(linux) || defined(HAVE_GNU_LD)
#include <gnu-stabs.h>
warn_references(gets, "the `gets' function is unreliable and should not be used.");
#else
#include <unistd.h>
static char w[] = "Warning: This program uses gets(), which is unsafe.\r\n";
static int warned = 0;
#endif

char * gets(char *s)
{
#if !defined(linux) && !defined(HAVE_GNU_LD)
  if (!warned) {
    (void) write (2, w, sizeof (w) - 1);
    warned++;
  }
#endif

  if (((streambuf*)stdin)->sgetline(s, 50000, '\n', 0) <= 0)
    return NULL;

  return s;
}

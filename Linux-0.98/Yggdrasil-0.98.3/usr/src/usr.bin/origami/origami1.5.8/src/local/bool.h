/* bool.h - defines a boolean type.  Maybe a future C standard causes compilers
   to produce optimized code for boolean evaluations. */

#ifndef BOOL_H

/* Use "if (b) ..." instead of "if (b==TRUE) ..." because TRUE is only one
   element from the set of non-false elements.
   "if (b==FALSE) ..." if ok, but "if (!b) ... is ok, too.
*/
   
typedef enum { FALSE, TRUE } bool;

#define BOOL_H

#endif

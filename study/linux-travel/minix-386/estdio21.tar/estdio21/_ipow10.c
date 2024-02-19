/*                              _ i p o w 1 0                              */

/* __ipow10 is a static table of integral powers of ten. The highest
 * power of ten stored in the table is obtained from __Mipow10.
 */

#include "stdiolib.h"

/*LINTLIBRARY*/

long __ipow10[] = {
    1L,      10L,      100L,      1000L,      10000L,
    100000L, 1000000L, 10000000L, 100000000L, 1000000000L,
  };

int __Mipow10 = sizeof(__ipow10) / sizeof(__ipow10[0]) - 1;

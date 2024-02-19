/*                               _ z _ c v t                               */

#include "stdiolib.h"

/*LINTLIBRARY*/
/*ARGSUSED*/

int __cvt F6(__stdiosize_t *, length, FV *, fv,
             char *, buf, VA_LIST *, argp, int, precision, int, fflag)

{
  static char nfpm[] = { '(','N','o',' ',
                         'F','l','o','a','t','i','n','g',' ',
                         'P','o','i','n','t',')' };
  fv[0].att = FV_F_VECTOR;
  fv[0].len = 0;
  fv[1].att = FV_F_VECTOR;
  fv[1].len = sizeof(nfpm);
  fv[1].arg = nfpm;
  *length   = sizeof(nfpm);

  return 2;
}

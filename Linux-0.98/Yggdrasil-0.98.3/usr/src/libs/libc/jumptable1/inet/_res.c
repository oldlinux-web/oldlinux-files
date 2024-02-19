#ifndef NO_JUMP_TABLE
#include "inetprivate.h"

struct state _res = {
  RES_TIMEOUT,		/* retransmition time interval */
  4,			/* number of times to retransmit */
  RES_DEFAULT,		/* options flags */
  1,			/* number of name servers */
};
#endif

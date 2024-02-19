#if 0
#include "../../linux/include/time.h"
#else
#include_next  <time.h>
#endif
#include "errno.h"

#define __CONSTVALUE    __const
#define __tzname	tzname
#define __tzset		tzset
#define __daylight	daylight
#define __timezone	timezone

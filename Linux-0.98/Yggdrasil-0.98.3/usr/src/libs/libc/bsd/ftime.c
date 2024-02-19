/*  Bezerkeley compatible ftime(3C).
    This call is deprecated, use gettimeofday(2) in new code.
    We use the Bezerkeley gettimeofday(2) call in the implementation, as
    there is no way to get millisecond resolution in POSIX (lose, lose). */

#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/time.h>

int
ftime(struct timeb *tb)
{
  struct timeval tv;
  struct timezone tz;

  if (gettimeofday(&tv, &tz) < 0 )
    return -1;
  tb->time = tv.tv_sec;
  tb->millitm = tv.tv_usec/1000; 
  tb->timezone = tz.tz_minuteswest;
  tb->dstflag = tz.tz_dsttime;
  return 0;
}

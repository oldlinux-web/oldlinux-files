#include <stdio.h>
#include <time.h>
int md[] =
{
   31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

long 
timelocal (t)
     struct tm *t;
{
   /* take t and convert it to seconds */
   long r;
   r = (long)t->tm_sec;
   r += (long)t->tm_min * 60L;
   r += (long)t->tm_hour * 3600L;
   r += (long)t->tm_mday * 24L * 3600L;
   r += (long)t->tm_mon * (long)md[t->tm_mon] * 24L * 3600L;
   r += (long)(t->tm_year - 70) * 365L * 24L * 3600L;
   /* how many leap years since 1970? */
   /* r += (long)((t->tm_year - 70) / 4) * 24L * 3600L; */
   return (r);
}
main ()
{
   long timelocal (), secs, secs2;
   struct tm *tm, *localtime ();
   char *ctime();

   time (&secs);
   tm = localtime (&secs);
   secs2 = timelocal (tm);
   printf ("off by %ld seconds\n", secs2-secs);
   printf ("secs=%ld - %s", secs, ctime(&secs));
   printf ("secs2=%ld - %s", secs2, ctime(&secs2));
}

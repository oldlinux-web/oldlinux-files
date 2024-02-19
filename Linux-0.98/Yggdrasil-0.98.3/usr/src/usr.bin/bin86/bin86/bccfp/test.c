/*
 *   bin86/bccfp/test.c
 *
 *   Copyright (C) 1992 Bruce Evans
 */

#include <sys/times.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

#define CONVTYPE	int
#define MAX		(MIN + NITER - 1)
#define MIN		INT_MIN

#define NITER	100000

double one = 1;
double two = 2;
double big = 1e99;

double d;
double d1;
float f;

int main()
{
    CONVTYPE cti;
    CONVTYPE cto;
    clock_t delta;
    struct tms finish;
    int i;
    struct tms start;

#if 0
    times(&start);
    for (cti = MIN; cti <= MAX; ++cti)
    {
	d = cti;
	cto = d;
	if (cti != cto)
	    printf("%08x %08x\n", cti, cto);
	if (cti % 10000000 == 0)
	{
	    printf("%8x ok ", cti);
	    fflush(stdout);
	}
    }
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for %d i -> d and d -> i conversions was %g s (%d t)\n",
	   MAX - MIN + 1, delta / (double) CLOCKS_PER_SEC, delta);
#endif

    times(&start);
    for (cti = MIN; cti <= MAX; ++cti)
	d = cti;
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for %d i -> d conversions was %g s (%d t)\n",
	   MAX - MIN + 1, delta / (double) CLOCKS_PER_SEC, delta);

    times(&start);
    for (cti = MIN; cti <= MAX; ++cti)
    {
	d = cti;
	cto = d;
    }
    times(&finish);
    delta = finish.tms_utime - start.tms_utime - delta;
    printf("Time for %d d -> i conversions was %g s (%d t)\n",
	   MAX - MIN + 1, delta / (double) CLOCKS_PER_SEC, delta);

    d = 0;
    times(&start);
    for (i = 0; i < NITER; ++i)
	d = d + 1;
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for adding %d 1.0's to 0.0 was %g s (%d t), result = %g\n",
	   NITER, delta / (double) CLOCKS_PER_SEC, delta, d);

    d = 0;
    times(&start);
    for (; d < NITER;)
	d = d + 1;
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for adding %d 1.0's to 0.0 (d index) was %g s (%d t), result = %g\n",
	   NITER, delta / (double) CLOCKS_PER_SEC, delta, d);

    times(&start);
    for (i = 1; i <= NITER; ++i)
    {
	d1 = i;
	d = d1 * d1;
    }
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for %d mults was %g s (%d t), result = %g\n",
	   NITER, delta / (double) CLOCKS_PER_SEC, delta, d);

    times(&start);
    for (i = 1; i <= NITER; ++i)
    {
	d1 = i;
	d = 1 / d1;
    }
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for %d divs was %g s (%d t), result = %g\n",
	   NITER, delta / (double) CLOCKS_PER_SEC, delta, d);

    f = 0;
    times(&start);
    for (i = 0; i < NITER; ++i)
	f = f + 1;
    times(&finish);
    delta = finish.tms_utime - start.tms_utime;
    printf("Time for adding %d 1.0f's to 0.0f was %g s (%d t), result = %g\n",
	   NITER, delta / (double) CLOCKS_PER_SEC, delta, f);

    return 0;
}

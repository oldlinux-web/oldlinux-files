#include <stdio.h>
#include <sys/types.h>
#include <sys/times.h>

/*
 * Use this program to determine the proper value of MSPTICK for
 * your system.
 */
main() {
	long old, curr, times();
	struct tms foo;
	float f;

	old = times(&foo);
	for (;;) {
		sleep(1);
		curr = times(&foo);
		f = 1000.0/(float)(curr - old);
		printf("%d actual value = %f, MSPTICK = %d\n", curr-old,
 f, (int)(f + 0.5));
		old = curr;
	}
}

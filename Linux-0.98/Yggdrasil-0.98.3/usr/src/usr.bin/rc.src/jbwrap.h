/* certain braindamaged compilers don't define jmp_buf as an array, so... */

#include <setjmp.h>

typedef struct jbwrap {
	jmp_buf j;
} jbwrap;

extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

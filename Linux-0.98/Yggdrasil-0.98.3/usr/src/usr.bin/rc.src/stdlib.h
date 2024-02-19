/*
   stdlib.h function prototypes as taken from Appendix B of K&R 2.
   Unused functions are:
	atof(), atoi(), atol(), strtod(), strtol(), strtoul(), rand(),
	srand(), calloc(), abort(), atexit(), system(), getenv(), bsearch(),
	abs(), labs(), div() and ldiv()
*/

extern void *malloc(SIZE_T);
extern void *realloc(void *, SIZE_T);
extern void free(void *);
extern void exit(int);
extern void qsort(void *, SIZE_T, SIZE_T, int (*)(const void *, const void *));

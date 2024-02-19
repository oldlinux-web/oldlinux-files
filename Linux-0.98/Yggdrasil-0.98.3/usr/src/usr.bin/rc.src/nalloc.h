extern Block *newblock(void);
extern void *nalloc(SIZE_T);
extern void nfree(void);
extern void restoreblock(Block *);

#undef offsetof
#define offsetof(t, m) ((SIZE_T) &((t *)0)->m)

/* memory allocation abbreviation */
#define nnew(x) ((x *) nalloc(sizeof(x)))
#define ncpy(x) (strcpy((char *) nalloc(strlen(x) + 1), x))

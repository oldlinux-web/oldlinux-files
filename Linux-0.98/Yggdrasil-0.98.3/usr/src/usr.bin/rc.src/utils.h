extern char *strprint(char *, int, int);
extern char *sprint(char *, char *,...);
extern int isabsolute(char *); /* not boolean because y.tab.c includes utils.h */
extern int a2u(char *);
extern int o2u(char *);
extern int starstrcmp(const void *, const void *);
extern void *ealloc(SIZE_T);
extern void *erealloc(void *, SIZE_T);
extern void efree(void *);
extern void fprint(int, char *,...);
extern void empty_fifoq(void);
extern void pr_error(char *);
extern void rc_error(char *);
extern void rc_exit(int); /* prototyped here, but defined in fn.c */
extern void sig(int);
extern void clear(char *, SIZE_T);
extern void uerror(char *);
extern void writeall(int, char *, SIZE_T);
extern SIZE_T strarraylen(char **);

/* memory allocation abbreviations */
#define enew(x) ((x *) ealloc(sizeof(x)))
#define ecpy(x) strcpy((char *) ealloc(strlen(x) + 1),x)

#define FPRINT_SIZE 16384

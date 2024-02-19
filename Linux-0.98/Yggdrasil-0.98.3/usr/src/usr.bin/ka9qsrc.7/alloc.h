#ifndef	NULL
#define	NULL	(void *)0
#endif

#define	NULLCHAR	(char *)NULL
union header {
	struct {
		union header *ptr;
		unsigned size;
	} s;
	long l;
};

typedef union header HEADER;
#define	NULLHDR	(HEADER *)NULL


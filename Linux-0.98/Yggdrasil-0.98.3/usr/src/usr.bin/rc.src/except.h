enum except { ERROR, BREAK, RETURN, VARSTACK, ARENA };
typedef struct Estack Estack;

struct Estack {
	enum except e;
	boolean interactive;
	jbwrap *jb;
	Block *b;
	char *name;
	Estack *prev;
};

extern void rc_raise(enum except);
extern void except(enum except, void *, Estack *);
extern void unexcept(void);

#include "config.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"


/* braindamaged IBM header files #define true and false */
#undef FALSE
#undef TRUE

enum bool { FALSE, TRUE };

typedef enum bool boolean;
typedef struct Rq Rq;
typedef struct Block Block;
typedef struct List List;
typedef struct Fq Fq;

struct List {
	char *w;
	char *m;
	List *n;
};

#include "node.h"

extern char *prompt, *prompt2;
extern Rq *redirq;
extern boolean dashdee, dashee, dashvee, dashex, dashell, dasheye, dashen, interactive;
extern int rc_pid;
extern int lineno;
extern Fq *fifoq;

#define arraysize(a) ((int)(sizeof(a)/sizeof(*a)))

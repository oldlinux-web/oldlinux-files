/* The <regexp.h> header is used by the (V8-compatible) regexp(3) routines. */

#ifndef _REGEXP_H
#define _REGEXP_H

#define CHARBITS 0377
#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

/* Function Prototypes. */
#ifndef _ANSI_H
#include <ansi.h>
#endif

_PROTOTYPE( regexp *regcomp, (char *_exp)				);
_PROTOTYPE( int regexec, (regexp *_prog, char *_string, int _bolflag)	);
_PROTOTYPE( void regsub, (regexp *_prog, char *_source, char *_dest)	);
_PROTOTYPE( void regerror, (char *_message) 				);

#endif /* _REGEXP_H */

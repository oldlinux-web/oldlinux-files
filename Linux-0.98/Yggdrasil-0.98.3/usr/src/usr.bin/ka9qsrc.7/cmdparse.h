#define NARG		40	/* Max number of args to commands */
#define POOLSIZE	400	/* Max number of chars in expanded args */

struct cmds {
	char *name;		/* Name of command */
	int (*func)();		/* Function to execute command */
	int  argcmin;		/* Minimum number of args */
	char *argc_errmsg;	/* Message to print if insufficient args */
	char *exec_errmsg;	/* Message to print if function fails */
};
#ifndef NULLCHAR
#define NULLCHAR	(char *)0
#endif

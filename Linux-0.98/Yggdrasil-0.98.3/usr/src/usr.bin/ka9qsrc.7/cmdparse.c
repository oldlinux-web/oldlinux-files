/* Parse command line, set up command arguments Unix-style, and call function.
 * Note: argument is modified (delimiters are overwritten with nulls)
 * Improved error handling by Brian Boesch of Stanford University
 */
#include <stdio.h>
#include <ctype.h>
#ifdef	UNIX
#undef	toupper
#undef	tolower
#endif
#include "global.h"
#include "cmdparse.h"

int
cmdparse(cmds,line)
struct cmds cmds[];
register char *line;
{
	register struct cmds *cmdp;
	char *argv[NARG],*cp,*pp,*vp,*ep,*rp,*getenv();
	static char pool[POOLSIZE];
	int argc,quote;
	int rslt;

	/* Remove cr/lf */
	rip(line);

	for(argc = 0;argc < NARG;argc++)
		argv[argc] = NULLCHAR;

	pp = pool;

	for(argc = 0;argc < NARG && *line != '\0';){
	    quote = 0;
	    /* Skip leading white space */
	    while(*line == ' ' || *line == '\t')
		line++;
	    if(*line == '\0')
		break;
	    /* Check for quoted token */
	    if(*line == '"' || *line == '\'')
		quote = *line++;	/* Suppress quote, remember it */
	    argv[argc++] = line;	/* Beginning of token */
	    /* Find terminating delimiter */
	    if(quote){
		/* Find quote, it must be present */
		if((line = index(line,quote)) == NULLCHAR)
		    return -1;

		*line++ = '\0';
	    } else {
		/* Find space or tab. If not present,
		 * then we've already found the last
		 * token.
		 */
		for (cp = line; *cp; cp++)
		    if (*cp == ' ' || *cp == '\t')
		    break;
		if (*cp != '\0')
		    *cp++ = '\0';
		line = cp;
	    }
	    /* Check for variablename */
	    if(quote != '\'' && index(cp = argv[argc - 1],'$') != NULLCHAR){
		argv[argc - 1] = pp;
		while (*cp != '\0') {
		    if (*cp != '$')
			*pp++ = *cp++;
		    else {
			if (*++cp == '{') {
			    if ((ep = index(++cp,'}')) == NULLCHAR)
			    return -1;
			    *ep++ = '\0';
			} else {
			    ep = NULLCHAR;
			}
			/* Scan to end of varname and mark it */
			for (vp = cp; isalnum(*vp) || *vp == '_'; vp++)
			    *vp = toupper(*vp);
			/* If a name found, get value */
			if (vp != cp){
			    rslt = *vp;			/* save next and make \0 */
			    *vp = '\0';
			    rp = getenv(cp);
			    *vp = rslt;			/* restore next character */
			    if (ep == NULLCHAR){	/* no braces */
				if (rp == NULLCHAR)	/* undef'd var */
				    rp = "";		/* then just empty */
			    } else {
				if (rp == NULLCHAR)	/* undef'd var */
				    switch (rslt) {	/* else look at next */
/*				    case '=': */
/*					dosetenv(2,(&cp)-1);/* assign and subst */
				    case '-':
					rp = vp + 1;	/* subst next word */
					break;
				    case '?':
					if (*++vp == '\0')
					vp = "undefined variable";
					printf("%s\n",vp);
					return -1;
				    default:
					rp = "";	/* default is empty */
				    }
				else
				    if (rslt == '+')	/* defined and subst */
					rp = vp + 1;
			    }
			    /* Copy the result to the pool */
			    while (*pp++ = *rp++)
				;
			    pp--;
			    cp = vp;
			}
			if (ep != NULLCHAR)
			    cp = ep;
		    }
		}
		*pp++ = '\0';
		if (pp > pool + POOLSIZE){	/* check pool overflow */
		    printf("panic: pool overflow\n");
		    fflush(stdout);
		    for (;;)			/* dangerous to continue */
			;
		}
	    }
	}
	if (argc < 1) {		/* empty command line */
		argc = 1;
		argv[0] = "";
	}
	/* Lines beginning with "#" are comments */
	if(argv[0][0] == '#')
		return 0;

	/* Look up command in table; prefix matches are OK */
	for(cmdp = cmds;cmdp->name != NULLCHAR;cmdp++){
		if(strncmp(argv[0],cmdp->name,strlen(argv[0])) == 0)
			break;
	}
	if(cmdp->name == NULLCHAR) {
		if(cmdp->argc_errmsg != NULLCHAR)
			printf("%s\n",cmdp->argc_errmsg);
		return -1;
	} else {
		if(argc < cmdp->argcmin) {
			/* Insufficient arguments */
		        printf("Usage: %s\n",cmdp->argc_errmsg); 
			return -1;
		} else {
			argv[0] = cmdp->name;
			rslt = (*cmdp->func)(argc,argv);
			if ((rslt < 0) && (cmdp->exec_errmsg != NULLCHAR))
				printf("%s\n",cmdp->exec_errmsg);
			return(rslt);
		}
	}
}

/* Call a subcommand based on the first token in an already-parsed line */
int
subcmd(tab,argc,argv)
struct cmds tab[];
int argc;
char *argv[];
{
	int rslt,len;
	register struct cmds *cmdp;

	/* Strip off first token and pass rest of line to subcommand */
	if (argc < 2) {
		if (argc < 1)
			printf("SUBCMD - Don't know what to do?\n");
		else
			printf("\"%s\" - takes at least one argument\n",argv[0]);
		return -1;
	}
	for(cmdp = tab;cmdp->name != NULLCHAR;cmdp++){
		if(strncmp(argv[1],cmdp->name,strlen(argv[1])) == 0){
			if(--argc < cmdp->argcmin) {
				if (cmdp->argc_errmsg != NULLCHAR)
					printf("Usage: %s\n",cmdp->argc_errmsg); 
				return -1;
			} else {
				(++argv)[0] = cmdp->name;
				rslt = (*cmdp->func)(argc,argv);
				if ((rslt < 0) && (cmdp->exec_errmsg != NULLCHAR))
					printf("%s\n",cmdp->exec_errmsg);
				return(rslt);
			}
		}
	}
	if (cmdp->argc_errmsg != NULLCHAR){
		if (cmdp->argc_errmsg[0] != '?'){
			printf("%s\n",cmdp->argc_errmsg);
		} else {
			printf("%s %s:",argv[0],cmdp->argc_errmsg + 1);
			len = strlen(argv[0]) + strlen(cmdp->argc_errmsg) + 1;
			for(cmdp = tab;cmdp->name != NULLCHAR;cmdp++){
				if ((len += strlen(cmdp->name) + 1) > 79){
					len = strlen(cmdp->name) + 9;
					printf("\n        ");
				}
				printf(" %s",cmdp->name);
			}
			printf("\n");
		}
	}
	return -1;
}

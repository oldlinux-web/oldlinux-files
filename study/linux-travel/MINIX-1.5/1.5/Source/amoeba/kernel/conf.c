#include "kernel.h"
#include "amoeba.h"
#include "global.h"
#include "conf.h"

/*
**	the following hack is imported from task.c under Amoeba to declare
**	a few pointers to the task table
*/ 
#define extern

#include "task.h"

#undef extern

/************************************************************************/
/*	TRANS CONFIGURATION						*/
/************************************************************************/

/*
**	various variables used for transactions
*/

port	NULLPORT;	/* used in trans.c, declared here for compatability */
long	ticker;		/* bogus global used by trans.c for statistics */

#ifndef NONET

unshort minloccnt	= MINLOCCNT;
unshort maxloccnt	= MAXLOCCNT;

unshort retranstime	= RETRANSTIME;
unshort crashtime	= CRASHTIME;
unshort clientcrash	= CLIENTCRASH;

unshort maxretrans	= MAXRETRANS;
unshort mincrash	= MINCRASH;
unshort maxcrash	= MAXCRASH;

#endif NONET

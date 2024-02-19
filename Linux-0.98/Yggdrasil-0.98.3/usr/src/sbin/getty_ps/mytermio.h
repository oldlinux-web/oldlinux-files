/*
 *  Found this in termio.h on some unix machines.
 *  Changed to fit Linux's defaults.
 */

#define CNUL    0
#define CERASE  127	/* ^? */
#define CKILL   025	/* ^U */
#define CINTR   03	/* ^C */
#define CQUIT   034     /* ^\ */
#define CSTART  021     /* ^Q */
#define CSTOP   023     /* ^S */
#define CEOF    04      /* ^D */
#define CMIN    06      /* satisfy read at 6 chars */
#define CTIME   01      /* .1 sec inter-character timer */

#define SSPEED	B9600	/* default speed */

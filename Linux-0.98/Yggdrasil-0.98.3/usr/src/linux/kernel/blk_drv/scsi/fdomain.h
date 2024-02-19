/* fdomain.h -- Header for Future Domain TMC-1660/TMC-1680 driver
 * Created: Sun May  3 18:47:33 1992
 * Revised: Sat May 23 22:42:55 1992 by root
 * Author: Rickard E. Faith, faith@cs.unc.edu
 * Copyright 1992 Rickard E. Faith
 * This program comes with ABSOLUTELY NO WARRANTY.
 *
 * $Log$
 */

#ifndef _FDOMAIN_H
#define _FDOMAIN_H

#define QUEUE         1		/* Enable command queueing */

int  fdomain_16x0_detect( int );
int  fdomain_16x0_command( unsigned char target, const void *cmnd,
	 		   void *buff, int bufflen);
int  fdomain_16x0_abort( int );
char *fdomain_16x0_info( void );
int  fdomain_16x0_reset( void ); 

#if QUEUE
int  fdomain_16x0_queue( unsigned char target, const void *cmnd,
			 void *buff, int bufflen, void (*done)(int,int) );

#define FDOMAIN_16X0  { "Future Domain TMC-1660/TMC-1680", \
			 fdomain_16x0_detect,              \
			 fdomain_16x0_info,                \
			 fdomain_16x0_command,             \
			 fdomain_16x0_queue,               \
		         fdomain_16x0_abort,               \
			 fdomain_16x0_reset,               \
			 1, 6, 0 }
#else
#define FDOMAIN_16X0  { "Future Domain TMC-1660/TMC-1680", \
			 fdomain_16x0_detect,              \
			 fdomain_16x0_info,                \
			 fdomain_16x0_command,             \
			 NULL,                             \
		         fdomain_16x0_abort,               \
			 fdomain_16x0_reset,               \
			 0, 6, 0 ,0}
#endif
#endif

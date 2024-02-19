/*  @(#) md-sun.h 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: Machine description for SUNS, all versions of SunOs
*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Note: when using gcc, make sure you have patched the SUN include files
using the shell script fixincludes  provided with gcc.   If you don't,
the terminal interface does not operate properly.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef SUNOS_3		/* how to find out? */
#define SUNOS_4 1
#else
#define SUNOS_4 0
#endif

#define SUNOS_4_1		0	/* SunOs 4.1 */

			/* compiler */
#define ANSI			0
#define PROTO			0
#define O_NO_LEFT_CAST		0
#define O_NO_VOID_POINTER	0
#define O_SHORT_SYMBOLS		0
#define O_ASM_SWITCH		0
			/* Operating system */
#define O_PROFILE		1
#define O_SIG_AUTO_RESET	0
#define O_SHARED_MEMORY		0
#define O_CAN_MAP		SUNOS_4
#define O_NO_SEGV_ADDRESS	0
#if sparc
#define MAX_VIRTUAL_ADDRESS	(512 * 1024 *1024)
#else
#define MAX_VIRTUAL_ADDRESS	(220 * 1024 *1024)
#endif
#define O_FOREIGN		1
#define O_STORE_PROGRAM		1
#define DEFAULT_PATH		":.:/usr/ucb:/bin:/usr/bin:";
#ifdef SUNOS_3
#define SIGNAL_HANDLER_TYPE	int
#define DESCRIPTOR_TABLE_SIZE	32
#define O_STRUCT_DIRECT		1
#define DIR_INCLUDE		<sys/dir.h>
#endif
			/* terminal driver */
#define O_TERMIOS 		SUNOS_4
#define O_EXTEND_ATOMS 		1
#define O_LINE_EDIT 		1
#define O_FOLD 			0
			/* Interfaces */
#define O_PCE 			1

#if sparc
#define MACHINE		  "sun4"
#else
#define MACHINE		  "sun3"
#endif

#ifdef SUNOS_3
#define OPERATING_SYSTEM  "3.x"
#else
#define OPERATING_SYSTEM  "4.0"
#endif

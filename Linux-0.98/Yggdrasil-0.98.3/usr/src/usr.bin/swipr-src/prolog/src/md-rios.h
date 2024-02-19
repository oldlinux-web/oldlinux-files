/*  @(#) md-rios.h 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: Machine description for IBM RISC/6000, AIX 3.1 (rios)
*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This is a partial port for  the  IBM-RISC/6000 (RS6000, rios) machine.
Basic functionality seems to be ok.  Dynamic stacks,  foreign language
interface, saved states and profiling are  prabably portable features,
but they are not yet operational.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

			/* compiler */
#define unix			1
#define _BSD			1
#define ANSI			1
#define PROTO			1
#define O_NO_LEFT_CAST		1
#define O_NO_VOID_POINTER	0
#define O_SHORT_SYMBOLS		0
#define O_ASM_SWITCH		0
#define O_ULONG_PREDEFINED	1

			/* Operating system */
#define O_DATA_AT_0X2		1 /* data space at 0x20000000 - 0x2fffffff */
#define O_PROFILE		0
#define O_SIG_AUTO_RESET	0
#define O_SHARED_MEMORY		0
#define O_CAN_MAP		0
#define O_NO_SEGV_ADDRESS	1
#define MAX_VIRTUAL_ADDRESS	(256 * 1024 *1024)
#define O_FOREIGN		0
#define O_RIOS_FOREIGN		1
#define O_STORE_PROGRAM		0
#define O_XWINDOWS		1
#define DEFAULT_PATH		":/usr/ucb:/bin:/usr/bin:/usr/local/bin:.:";

			/* terminal driver */
#define O_TERMIOS 		1
#define O_EXTEND_ATOMS 		1
#define O_LINE_EDIT 		1
#define O_FOLD 			0
			/* Interfaces */
#define O_PCE 			0

#define MACHINE			"rios"
#define OPERATING_SYSTEM	"AIX 3.1"

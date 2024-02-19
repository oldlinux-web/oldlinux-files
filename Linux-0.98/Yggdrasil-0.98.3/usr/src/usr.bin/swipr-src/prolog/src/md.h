/*  @(#) md-linux.h 1.5.0 (UvA SWI,<P>) 23-1-92 

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    Adapted for linux by Pieter Olivier.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: Machine description for linux (version 0.12) 
*/

			/* compiler */
#define ANSI			0
#define PROTO			0
#define O_NO_LEFT_CAST		0
#define O_NO_VOID_POINTER	0
#define O_SHORT_SYMBOLS		0
#define O_ASM_SWITCH		0
			/* Operating system */
#define O_PROFILE		0
#define O_SIG_AUTO_RESET	0
#define O_SHARED_MEMORY		0
#define O_CAN_MAP		0
#define O_NO_SEGV_ADDRESS	0
#define MAX_VIRTUAL_ADDRESS     (220*1024*1024) /* I don't know how bit it is */
#define O_FOREIGN		1
#define O_STORE_PROGRAM		0
#define DEFAULT_PATH		":.:/bin:/usr/bin:/usr/local/bin:";
#define SIGNAL_HANDLER_TYPE	int
#define DESCRIPTOR_TABLE_SIZE   32
#define O_STRUCT_DIRECT		0
#define DIR_INCLUDE		<sys/dir.h>
			/* terminal driver */
#define O_TERMIOS 		1
#define O_EXTEND_ATOMS 		1
#define O_LINE_EDIT 		1
#define O_FOLD 		 	79	
			/* Interfaces */
#define O_PCE 			0

#define MACHINE		  "Intel-386"

#define OPERATING_SYSTEM  "Linux"

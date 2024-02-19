/***********************************************************************
		Copyright 1994 Comtrol Corporation.
			All Rights Reserved.

The following source code is subject to Comtrol Corporation's
Developer's License Agreement.

This source code is protected by United States copyright law and 
international copyright treaties.

This source code may only be used to develop software products that
will operate with Comtrol brand hardware.

You may not reproduce nor distribute this source code in its original
form but must produce a derivative work which includes portions of
this source code only.

The portions of this source code which you use in your derivative
work must bear Comtrol's copyright notice:

		Copyright 1994 Comtrol Corporation.

***********************************************************************/

/*
	user definitions for Rocket Toolkit

	The following typedefs and defines must be established
	depending on the platform the toolkit is being used
	with.

*/

/************************************************************
The following sets up the world for use with Linux
************************************************************/

#include <asm/io.h>

typedef unsigned char Byte_t;
typedef unsigned int ByteIO_t;

typedef unsigned int Word_t;
typedef unsigned int WordIO_t;

typedef unsigned long DWord_t;
typedef unsigned int DWordIO_t;

#define sOutB(a, b) outb_p(b, a)
#define sOutW(a, b) outw_p(b, a)
#define sOutDW(a, b) outl_p(b, a)
#define sInB(a) (inb_p(a))
#define sInW(a) (inw_p(a))

#define sOutStrW(port, addr, count) outsw(port, addr, count)

#define sInStrW(port, addr, count) insw(port, addr, count)
	

/*---------------------------------------------------------------------------+
 |  control_w.h                                                              |
 |                                                                           |
 | Copyright (C) 1992    W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail apm233m@vaxc.cc.monash.edu.au    |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#ifndef _CONTROLW_H_
#define _CONTROLW_H_

#ifdef __ASSEMBLER__
#define	_Const_(x)	$##x
#else
#define	_Const_(x)	x
#endif

#define CW_RC		_Const_(0x0C00)	/* rounding control */
#define CW_PC		_Const_(0x0300)	/* precision control */
#define CW_PM		_Const_(0x0020)	/* precision mask */
#define CW_UM		_Const_(0x0010)	/* underflow mask */
#define CW_OM		_Const_(0x0008)	/* overflow mask */
#define CW_ZM		_Const_(0x0004)	/* divide by zero mask */
#define CW_DM		_Const_(0x0002)	/* denormalized operand mask */
#define CW_IM		_Const_(0x0001)	/* invalid operation mask */
#define CW_EXM		_Const_(0x007f)	/* all masks */

#define RC_RND		_Const_(0x0000)
#define RC_DOWN		_Const_(0x0400)
#define RC_UP		_Const_(0x0800)
#define RC_CHOP		_Const_(0x0C00)

#endif _CONTROLW_H_

//
// This file is part of the math library for Linux and is
// covered by the GNU General Public License version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"atan.S"
	.text

#ifndef NO_SHARED_LIBRARY
	.org	0xc,0x90
#endif
	.align	2,0x90
	.globl	_atan
_atan:
	fldl	4(%esp)
	fld1
	fpatan
	ret

#ifndef NO_SHARED_LIBRARY
	.org	0x3c,0x90
#endif
	.align	2,0x90
	.globl	_atan2
_atan2:
	fldl	4(%esp)
	fldl	12(%esp)
	fpatan
	ret

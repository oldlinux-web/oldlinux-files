//
// This file is part of the math library for Linux and is
// covered by the GNU General Public license version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"fabs.S"

	.text
#ifndef NO_SHARED_LIBRARY
	.org	0x44,0x90
#endif
	.align	2,0x90
	.globl	_fabs
_fabs:
	fldl	4(%esp)
	fabs
	ret

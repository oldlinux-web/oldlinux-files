//
// This file is part of the math library for Linux and is
// covered by the GNU General Public License version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"fmod.S"
	.text
	.globl	_fmod
#ifndef NO_SHARED_LIBRARY
//	.org	0x8,0x90
#endif
	.align	2,0x90
_fmod:
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jz	zero1
	fldl	12(%esp)
	ftst
	fnstsw	%ax
	sahf
	jz	zero2
	fxch	%st(1)
loop:
	fprem
	fnstsw	%ax
	sahf
	jpe	loop
	fstp	%st(1)
	ret
	
zero2:
	fstp	%st(0)
zero1:
	ret	

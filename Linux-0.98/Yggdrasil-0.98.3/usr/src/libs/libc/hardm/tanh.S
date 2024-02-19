//
// This file is part of the math library for Linux and is
// covered by the GNU General Public license version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"tanh.S"
        .data
	.align	2
LCW1:
	.word	0
LCW2:
	.word	0

	.text
#ifndef NO_SHARED_LIBRARY
	.org	0x12c,0x90
#endif
	.align	2,0x90
	.globl	_tanh
_tanh:
	fldl	4(%esp)
	ftst
	fstsw	%ax
	sahf
	jna	Lneg
	fchs
	xorl	%eax,%eax
	incl	%eax
	jmp	L1
Lneg:
	xorl	%eax,%eax
L1:
	fld	%st(0)
	faddp	%st,%st(1)
	fldl2e
	fmulp	%st,%st(1)
	fst	%st(1)
	fstcw	LCW1
	fstcw	LCW2
	fwait
	andw	$0xf3ff,LCW1
	fldcw	LCW1
	frndint
	fldcw	LCW2
	fst	%st(2)
	fsubrp	%st,%st(1)
	f2xm1
	fld1
	faddp	%st,%st(1)
	fscale
	fst	%st(1)

	fld1
	faddp	%st,%st(1)
	fxch	%st(1)
	fld1
	fsubrp	%st,%st(1)
	fdivp	%st,%st(1)

	testl	%eax,%eax
	jna	L2
	fchs
L2:
	ret
	.align	2,0x90

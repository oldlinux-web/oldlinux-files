//
// This file is part of the math library for Linux and is
// covered by the GNU General Public license version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"sinh.S"
        .data
	.align	2
LCW1:
	.word	0
LCW2:
	.word	0

	.text
	.align	2
LC2:
	.double 0d5.0000000000000000000000e-01

#ifndef NO_SHARED_LIBRARY
	.org	0x20,0x90
#endif
	.align	2,0x90
	.globl	_sinh
_sinh:
	fldl	4(%esp)
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
	fdivp	%st,%st(1)
	fsubrp	%st,%st(1)

	fldl	LC2
	fmulp	%st,%st(1)

	ret

#ifndef NO_SHARED_LIBRARY
	.org	0x120,0x90
#endif
	.align	2,0x90
	.globl	_cosh
_cosh:
	fldl	4(%esp)
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
	fdivp	%st,%st(1)
	faddp	%st,%st(1)

	fldl	LC2
	fmulp	%st,%st(1)

	ret
	.align	2,0x90

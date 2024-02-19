//
// This file is part of the math library for Linux and is
// covered by the GNU General Public License version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"sin.S"
	.text

	.align	2,0x90
LL1:
	fldpi
	fld	%st(0)			// 2 pi
	faddp	%st,%st(1)
	fxch	%st(1)
LL2:
	fprem1
	fstsw	%ax
	sahf
	jp	LL2
	fxch	%st(1)
	fstp	%st(0)
LL3:
	fcos
	fstsw	%ax
	sahf
	jp	LL1
	ret

	.align	2,0x90
L1:
	fldpi
	fld	%st(0)		// 2 pi
	faddp	%st,%st(1)
	fxch	%st(1)
L2:
	fprem1
	fstsw	%ax
	sahf
	jp	L2
	fxch	%st(1)
	fstp	%st(0)
L3:
	fsin
	fstsw	%ax
	sahf
	jp	L1
	ret

#ifndef NO_SHARED_LIBRARY
	.org	0x164,0x90
#endif
	.align	2,0x90
	.globl	_cos
_cos:
	fldl	4(%esp)
	jmp	LL3

#ifndef NO_SHARED_LIBRARY
	.org	0x184,0x90
#endif
	.align	2,0x90
	.globl	_sin
_sin:
	fldl	4(%esp)
	jmp	L3
	.align	2
	.align	2,0x90

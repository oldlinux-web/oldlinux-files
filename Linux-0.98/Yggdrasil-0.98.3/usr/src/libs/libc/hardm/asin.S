//
// This file is part of the math library for Linux and is
// covered by the GNU General Public License version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"asin.S"
	.text

#ifndef	NO_SHARED_LIBRARY
	.org	0x10,0x90
#endif
	.align	2,0x90
	.globl	_asin
_asin:
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jb	negative1		// less than 0
	fcoml	plus1
	fnstsw	%ax
	sahf
	ja	domainS			// x > 1
	jb	L1			// x > 0 && x < 1
	fstp	%st(0)			// x = 1
	fldl	plushalf
	fldpi
	fmulp	%st,%st(1)
	ret

negative1:
	fcoml	minus1
	fnstsw	%ax
	sahf
	jb	domainS			// x < -1
	ja	L1			// 0 > x && x > -1
	fstp	%st(0)			// x = -1
	fldl	minushalf
	fldpi
	fmulp	%st,%st(1)
	ret

L1:
	fld	%st(0)
	fmul    %st(1),%st
	fld1
	fsubp	%st,%st(1)
	fsqrt
	fpatan
	ret

	.align	2,0x90
minushalf:
	.double 0d-0.500000000000000000000e+00
plushalf:
	.double 0d0.500000000000000000000e+00
minus1:
	.double 0d-1.000000000000000000000e+00
plus1:
	.double 0d1.000000000000000000000e+00

	.align	2,0x90
LS:
	.asciz	"asin"
	.align	2,0x90
LC:
	.asciz	"ac.S"

	.align	2,0x90
domainS:
	pushl	$LS
	jmp	domain
domainC:
	pushl	$LC
domain:
	movl	$33,_errno
	call	_perror
	addl	$4,%esp
	fstp	%st(0)
	fldz
	ret

	.globl	_acos
#ifndef	NO_SHARED_LIBRARY
	.org	0xfc,0x90
#endif
	.align	2,0x90
_acos:
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jb	negative2		// x < 0
	fcoml	plus1
	fnstsw	%ax
	sahf
	ja	domainC			// x > 1
	jb	L2			// x < 1
	fstp	%st(0)			// x = 1
	fldz
	ret

negative2:
	fcoml	minus1
	fnstsw	%ax
	sahf
	jb	domainC			// x < -1
	ja	L2			// x > -1
	fstp	%st(0)			// x = -1
	fldpi
	ret

L2:
	fld1
	fsubp	%st,%st(1)
	fsqrt

	fldl	4(%esp)
	fld1
	faddp	%st,%st(1)
	fsqrt

	fpatan

	fld	%st(0)
	faddp	%st,%st(1)
	ret

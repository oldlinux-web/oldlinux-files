//
// This file is part of the alternative 80386 math library and is
// covered by the GNU General Public license with my modification
// as noted in the README file that accompanied this file.
//
// Copyright 1990 G. Geers
//
// Modified by H.J. Lu for Linux 1992
//

	.file	"atanh.S"
	.text
	.align	2,0x90
minus1:
	.double 0d-1.000000000000000000000e+00
plus1:
	.double 0d1.000000000000000000000e+00
Lhalf:
	.double 0d5.0000000000000000000000e-01
MINUS:
	.double 0d-1.7976931348623157e+308
PLUS:
	.double 0d1.7976931348623157e+308
	.align	2,0x90
ATANH:
	.asciz	"atanh"
	.align	2,0x90
ACOSH:
	.asciz	"acosh"

domainC:
	pushl   $ACOSH
	jmp	domain

domainT:
	pushl   $ATANH
	
domain:
	movl    $33,_errno
	call    _perror
	addl    $4,%esp
	fstp	%st(0)
	fldz
	ret

#ifndef NO_SHARED_LIBRARY
	.org	0x0188,0x90
#endif
	.align	2,0x90
	.globl	_asinh
_asinh:
	fldl	4(%esp)

	fmull	4(%esp)
	fld1
	faddp	%st,%st(1)
	fsqrt
	faddl	4(%esp)
	fldln2
	fxch	%st(1)
	fyl2x

	ret

#ifndef NO_SHARED_LIBRARY
	.org	0x01dc,0x90
#endif
	.align	2,0x90
	.globl	_acosh
_acosh:
	fldl	4(%esp)
	fcoml	plus1
	fnstsw	%ax
	sahf
	jb	domainC			// x < 1
	ja	LL1			// x > 1
	fstp	%st(0)
	fldz
	ret

LL1:
	fmull	4(%esp)

	fld1
	fsubrp	%st,%st(1)
	fsqrt

	faddl	4(%esp)

	fldln2

	fxch	%st(1)
	fyl2x

	ret

#ifndef NO_SHARED_LIBRARY
	.org	0x0288,0x90
#endif
	.align	2,0x90
	.globl	_atanh
_atanh:
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jb	negative		// less than 0
	fcoml	plus1
	fnstsw	%ax
	sahf
	ja	domainT			// x > 1
	jb	L1			// x > 0 && x < 1
	pushl   $ATANH			// x = 1, domain error
	movl    $33,_errno
	call    _perror
	addl    $4,%esp
	fstp	%st(0)
	fldl    PLUS
	ret

negative:
	fcoml	minus1
	fnstsw	%ax
	sahf
	jb	domainT			// x < -1
	ja	L1			// 0 > x && x > -1
	pushl   $ATANH			// x = -1, domain error
	movl    $33,_errno
	call    _perror
	addl    $4,%esp
	fstp	%st(0)
	fldl    MINUS
	ret

L1:
	fld1
	faddp	%st,%st(1)
	fld1
	fsubl	4(%esp)
	fdivrp	%st,%st(1)

	fldln2
	fxch	%st(1)
	fyl2x

	fldl	Lhalf
	fmulp	%st,%st(1)

	ret
	.align	2,0x90

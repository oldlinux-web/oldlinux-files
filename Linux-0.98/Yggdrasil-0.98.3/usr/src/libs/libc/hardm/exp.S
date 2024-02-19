/*
** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** Modified O.ROBERT 24, Avenue de Verdun 92170 VANVES, FRANCE
**
** E-mail: roberto@germinal.ibp.fr
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** or	03-Apr-1991	corrected bug about argument zero to pow
**			fyl2x didn't like it
*/

/* Modified by H.J. Lu for Linux, 1992 */

/* History:15,24 */
	.file	"exp.S"
	.data
	.align	2
LTMP:
	.long	0
LCW1:
	.word	0
LCW2:
	.word	0

	.text
	.align	2,0x90
LC0:
	.double	0d1.0000000000000000000e+00
LP:
	.asciz	"pow"

	.globl	_pow2
#ifndef NO_SHARED_LIBRARY
	.org	0x10,0x90
#endif
	.align	2,0x90
_pow2:
	fldl	4(%esp)
Lpow2:
	fstcw	LCW1
	fstcw	LCW2
	andw	$0xf3ff,LCW1
	fldcw	LCW1
	fldl	%st(0)
	frndint
	fldcw	LCW2
	fxch	%st(1)
	fsub	%st(1),%st
	f2xm1
	faddl	LC0
	fscale
	fstp	%st(1)
	ret

	.align	2,0x90
noninteger:
	pushl	$LP
	movl	$33,_errno
	call	_perror
	addl	$4,%esp
	fstp	%st(0)
	fstp	%st(0)
	fldz
	ret

	.align	2,0x90
zero1:
	fldl	12(%esp)
	ftst
	fnstsw %ax
	sahf
	jz	zero2
	fstp	%st(0)
	fstp	%st(0)
	fldz
	ret

	.align	2,0x90
zero2:
	fstp	%st(0)
	fstp	%st(0)
	fld1
	ret

#ifndef NO_SHARED_LIBRARY
	.org	0xf8,0x90
#endif
	.align	2,0x90
	.globl	_pow10
_pow10:
	fldl	4(%esp)
	fldl2t
	fmulp	%st,%st(1)
	jmp	Lpow2

#ifndef NO_SHARED_LIBRARY
	.org	0x114,0x90
#endif
	.align	2,0x90
	.globl	_exp
_exp:
	fldl	4(%esp)
	fldl2e
	fmulp	%st,%st(1)
	jmp	Lpow2

#ifndef NO_SHARED_LIBRARY
	.org	0x134,0x90
#endif
	.align	2,0x90
	.globl	_pow
_pow:
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jz	zero1
	jb	negative
	fldl	12(%esp)
	ftst
	fnstsw %ax
	sahf
	jz	zero2
	fxch	%st(1)
	fyl2x
	jmp	Lpow2
negative:
	fchs
	fldl	12(%esp)
	ftst
	fnstsw	%ax
	sahf
	je	zero2
	fnstcw	LCW1
	fnstcw	LCW2
	orw	$3072,LCW1
	fldcw	LCW1
	fistl	LTMP
	fldcw	LCW2
	ficoml	LTMP
	fnstsw	%ax
	sahf
	jne	noninteger
	xorl	%ecx,%ecx
	movl	LTMP,%edx
	movl	%edx,%eax
	testl	%eax,%eax
	jge	L3
	incl	%eax
L3:
	andb	$254,%al
	subl	%eax,%edx
	movl	%edx,%eax
	testl	%eax,%eax
	jne	L2
	incl	%ecx
L2:
	fxch	%st(1)
	fyl2x
	call	Lpow2
	testl	%ecx,%ecx
	jne	L1
	fchs
L1:
	ret

	.align	2,0x90

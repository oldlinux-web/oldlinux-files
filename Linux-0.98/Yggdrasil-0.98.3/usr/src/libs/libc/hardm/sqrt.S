//
// This file is part of the math library for Linux and is
// covered by the GNU General Public License version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"sqrt.S"
	.text
	.align	2
LS:
	.asciz	"sqrt"

	.align	2,0x90
domain:
	pushl	$LS
        movl	$33,_errno
	call	_perror
        addl	$4,%esp
        fstp	%st(0)
        fldz
        ret

	.globl	_sqrt
	.org	0x48,0x90
	.align	2,0x90
_sqrt:
	fldl	4(%esp)
	ftst
        fnstsw	%ax
        sahf
        jb	domain
	jz	zero
	fsqrt
zero:
	ret

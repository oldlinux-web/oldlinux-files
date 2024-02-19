//
// This file is part of the math library for Linux and is
// covered by the GNU General Public License version 2, or
// any later version.
//
// Copyright 1992 by H.J. Lu
//
	.file	"log.S"
	.text
	.align	2
MINUS:
	.double	0d-1.7976931348623157e+308
LN:
	.asciz	"log"
	.align	2,0x90
LG:
	.asciz	"log10"

	.globl	_log
#ifndef NO_SHARED_LIBRARY
	.org	0x14,0x90
#endif
	.align	2,0x90
_log:
	fldln2
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jbe	domainN
	fyl2x
	ret

	.align	2
domainN:
	pushl	$LN
	jmp	domain
domainG:
	pushl	$LG
domain:
	movl	$33,_errno
	call	_perror
	addl	$4,%esp
	fstp	%st(0)
	fstp	%st(0)
	fldl	MINUS
	ret

#ifndef NO_SHARED_LIBRARY
	.org	0xe8,0x90
#endif
	.align	2,0x90
	.globl	_log10
_log10:
	fldlg2
	fldl	4(%esp)
	ftst
	fnstsw	%ax
	sahf
	jbe	domainG
	fyl2x
	ret
	.align	2,0x90

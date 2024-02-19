//
// This file is part of the alternative 80386 math library and is
// covered by the GNU General Public license with my modification
// as noted in the README file that accompanied this file.
//
// Copyright 1990 G. Geers
//
// Modified by H.J. Lu for Linux 1992
//

	.file "hypot.S"

	.text
#ifndef NO_SHARED_LIBRARY
	.org	0x6,0x90
#endif
	.align	2,0x90
	.globl	_hypot
_hypot:
	fldl	4(%esp)
	fmull	4(%esp)
	fldl	12(%esp)
	fmull	12(%esp)
	faddp	%st,%st(1)
	fsqrt
	ret

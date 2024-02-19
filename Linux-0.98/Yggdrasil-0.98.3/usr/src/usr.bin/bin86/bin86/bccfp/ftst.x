! bcc 386 floating point routines (version 2) -- Ftst, Ftstd, Ftstf
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

#if 0 /* bcc doesn't generate Ftst (but it might in future) */
	.globl	Ftst
#endif
	.align	ALIGNMENT
Ftst:
	cmp	dword PC_SIZE+D_HIGH[esp],#0	! need only test upper dword of x
	ret	#D_SIZE

! Compare double at address [ebx] with 0

	.globl	Ftstd
	.align	ALIGNMENT
Ftstd:
	cmp	dword D_HIGH[ebx],#0	! need only test upper dword of x
	ret

! Compare float at address [ebx] with 0

	.globl	Ftstf
	.align	ALIGNMENT
Ftstf:
	cmp	dword F_HIGH[ebx],#0
	ret

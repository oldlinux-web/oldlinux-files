! bcc 386 floating point routines (version 2) -- _fabs
! author: Bruce Evans

#include "fplib.h"

! double fabs(double value);
! returns the absolute value of a number
! this works for all NaNs, like the 80*87 fabs, but perhaps we should check
! for exceptions that can happen when an 80*87 register is loaded

	.globl	_fabs
	.align	ALIGNMENT
_fabs:
	mov	eax,PC_SIZE+D_LOW[esp]
	mov	edx,PC_SIZE+D_HIGH[esp]
	and	edx,~D_SIGN_MASK
	ret

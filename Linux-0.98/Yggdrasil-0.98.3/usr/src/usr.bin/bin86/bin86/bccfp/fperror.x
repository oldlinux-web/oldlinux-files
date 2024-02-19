! bcc 386 floating point routines (version 2)
! --- fpdenormal, fperror, fpinfinity, fpNaN, fpoverflow, fpunderflow,fpdivzero
! author: Bruce Evans

#include "fperr.h"
#include "fplib.h"

	.extern	_fperr

! Cause a denormal-operand exception
! Preserves all general registers if signal handler returns

	.globl	fpdenormal
	.align	ALIGNMENT
fpdenormal:
#if 0
	push	eax
	mov	eax,#EFDENORMAL
	call	fperror
	pop	eax
#endif
	ret

! Cause an exception with error code eax, preserving all genregs except eax

	.globl	fperror
	.align	ALIGNMENT
fperror:
	push	ebp		! set up usual frame ...
	mov	ebp,esp		! ... for debugging
	push	edx		! save default
	push	ecx
	push	eax		! error code is arg to C routine
	call	_fperr
	add	esp,#GENREG_SIZE
	pop	ecx		! restore default
	pop	edx
	pop	ebp
	ret

	.align	ALIGNMENT
fphuge:
	mov	ecx,#D_HUGE_LOW	! prepare number +-HUGEVAL
	or	edx,#D_HUGE_HIGH	! ... in case signal handler returns
	jmp	fperror

! Cause an infinite-operand exception
! Return +-HUGEVAL in edx:ecx with sign from edx

	.globl	fpinfinity
	.align	ALIGNMENT
fpinfinity:
	mov	eax,#EFINFINITY
	jmp	fphuge		! almost right

! Cause an NaN-operand exception
! Return +-HUGEVAL in edx:ecx with sign from edx

	.globl	fpNaN
	.align	ALIGNMENT
fpNaN:
	mov	eax,#EFNAN	! there are different types of NaNs but...
	jmp	fphuge		! WRONG

! Cause an overflow exception
! Return +-HUGEVAL in edx:ecx with sign from edx

	.globl	fpoverflow
	.align	ALIGNMENT
fpoverflow:
	mov	eax,#EFOVERFLOW
	jmp	fphuge		! almost right

! Cause an underflow exception (actually assume it is masked for now)
! Return denormal or 0.0 in edx:ecx
! XXX - this should cause a denormal exception or none for the denormal case
! Args: sign in edx, fraction in esi:eax, right shift in edi
! Returns: denormalized number in edx:eax

	.globl	fpunderflow
	.align	ALIGNMENT
fpunderflow:
#if 0
	mov	eax,#EFUNDERFLOW
	jmp	fperror
#endif
	cmp	edi,#REG_BIT
	jb	denormalize1
	mov	eax,esi
	sub	esi,esi
	sub	edi,#REG_BIT
	cmp	edi,#REG_BIT
	jb	denormalize1
denormalize_underflow:
#if 0
	mov	eax,#EFUNDERFLOW
	jmp	fperror
#endif
	sub	eax,eax
	mov	edx,eax
	ret

	.align	ALIGNMENT
denormalize1:
	mov	ecx,edi
	shrd	eax,esi,cl
	shr	esi,cl
	mov	ecx,esi
	or	ecx,eax
	jz	denormalize_underflow
	and	edx,#D_SIGN_MASK
	or	edx,esi
	ret

! Cause an fp division by zero exception
! Return +-HUGEVAL in edx:ecx with sign from edx

	.globl	fpdivzero
	.align	ALIGNMENT
fpdivzero:
	mov	eax,#EFDIVZERO
	test	edx,#D_EXP_MASK
	jnz	fphuge		! almost right
	sub	ecx,ecx
	mov	edx,ecx
	jmp	fperror

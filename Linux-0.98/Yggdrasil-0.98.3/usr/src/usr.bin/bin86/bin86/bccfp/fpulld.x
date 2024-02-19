! bcc 386 floating point routines (version 2) -- Fpulld
! authors: Timothy Murphy (tim@maths.tcd.ie), Bruce Evans

#include "fplib.h"

! Pop double from stack and store at address [ebx]

	.globl	Fpulld
	.align	ALIGNMENT
Fpulld:
	pop	ecx
	pop	dword D_LOW[ebx]
	pop	dword D_HIGH[ebx]
	jmp	ecx		! return

! This popping method is much slower on 486's because popping to memory
! takes 5+ while moving twice takes 2 and the return address doesn't
! have to be moved. However, popping is a little faster on a non-cached
! 386/20 with static column RAM although the memory access pattern is
! better for a double-width move than for popping. What about a cached 386?

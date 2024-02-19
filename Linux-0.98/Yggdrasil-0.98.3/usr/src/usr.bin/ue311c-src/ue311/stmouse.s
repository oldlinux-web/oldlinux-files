/	stmouse.s, a handler that preserves A0 across the call, and calls
/	two mouse routines.
/
/	31-Dec-1987	Jeff Lomicka
/
/	mymouse( hdr1, hdr2) calls both handlers with A0 and stack parameter
/
	.globl	mymouse_
	.globl	first_mhandler_
	.globl	second_mhandler_
	.shri
mymouse_:			/ Replacement mouse handler
	move.l	a0, -(a7)	/ Save value for second handler
	move.l	a0, -(a7)	/ Pass to first handler
	movea.l	first_mhandler_, a1	/ Pick up second handler
	jsr	(a1)		/ Call first handler
	addq	$4,a7		/ Recover stack
	movea.l	(a7), a0	/ Recover A0
	movea.l	second_mhandler_, a1	/ Pick up second handler
	jsr	(a1)
	addq	$4,a7		/ Recover stack
	rts

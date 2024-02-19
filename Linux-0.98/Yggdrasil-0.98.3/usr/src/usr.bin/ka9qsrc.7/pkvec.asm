	include pmacros.h

	extproc	pkint

; pkvec - Packet driver receive call handler

	pubproc	pkvec

	; save regs, making them available to pkint()
	push	es
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di

	getds
	mov	ds,ax

	call	pkint@

	pop	di
	pop	si
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	es

	ret

	pend	pkvec

	end


	include	model.h

;
;	VGAKIT Version 4.1
;
;	Copyright 1988,89,90,91 John Bridges
;	Free for use in commercial, shareware or freeware applications
;
;	RDPOINT.ASM
;
;
.data

	extrn	curbk:word
	extrn	maxx:word,maxy:word,xwidth:word

.code

	extrn	newbank:proc

	public	rdpoint
	public	rdpoint13x

rdpoint	proc	xpos:word,ypos:word
	mov	ax,[xwidth]		;640 bytes wide in most cases
	mul	[ypos]
	add	ax,[xpos]
	adc	dx,0
	mov	bx,ax
	mov	ax,dx
	cmp	ax,[curbk]
	jz	nonew
	call	newbank			;switch banks if a new bank entered
nonew:	mov	ax,0a000h		;setup screen segment A000
	mov	es,ax
	mov	al,es:[bx]
	mov	ah,0
	ret
rdpoint	endp

rdpoint13x proc	xpos:word,ypos:word
	mov	ax,[xwidth]		;80 bytes wide (for 360x480 mode)
	mul	[ypos]
	mov	bx,[xpos]
	mov	cl,bl
	shr	bx,1
	shr	bx,1
	add	bx,ax
	mov	ah,cl
	and	ah,3
	mov	al,4
	mov	dx,3ceh
	out	dx,ax			;set EGA bit plane read register
	mov	ax,0a000h		;setup screen segment A000
	mov	es,ax
	mov	al,es:[bx]
	mov	ah,0
	ret
rdpoint13x endp

	end


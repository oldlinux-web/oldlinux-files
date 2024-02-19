
	include	model.h

;
;	VGAKIT Version 4.1
;
;	Copyright 1988,89,90,91 John Bridges
;	Free for use in commercial, shareware or freeware applications
;
;	POINT.ASM
;
;
.data

	extrn	curbk:word
	extrn	maxx:word,maxy:word,xwidth:word

.code

	extrn	newbank:proc

	public	point
	public	point13x

point	proc	xpos:word,ypos:word,color:word
	mov	bx,[xpos]
	mov	ax,[ypos]
	mov	dx,[maxx]
	cmp	bx,0
	jl	nope1
	cmp	bx,dx
	jge	nope1
	cmp	ax,0
	jl	nope1
	cmp	ax,[maxy]
	jge	nope1
	mul	[xwidth]			;640 bytes wide in most cases
	add	bx,ax
	adc	dx,0
	mov	ax,dx
	cmp	ax,[curbk]
	jz	nonew
	call	newbank			;switch banks if a new bank entered
nonew:	mov	ax,0a000h		;setup screen segment A000
	mov	es,ax
	mov	al,byte ptr [color]	;get color of pixel to plot
	mov	es:[bx],al
nope1:	ret
point	endp

point13x proc	xpos:word,ypos:word,color:word
	mov	bx,[xpos]
	mov	ax,[ypos]
	mov	dx,[maxx]
	cmp	bx,0
	jl	nope2
	cmp	bx,dx
	jge	nope2
	cmp	ax,0
	jl	nope2
	cmp	ax,[maxy]
	jge	nope2
	mul	[xwidth]			;360 dots wide (for 360x480 mode)
	mov	cx,bx
	shr	bx,1
	shr	bx,1
	add	bx,ax
	mov	ax,102h	
	and	cl,3
	shl	ah,cl			;create bit plane mask
	mov	dx,3c4h
	out	dx,ax			;set EGA bit plane mask register
	mov	ax,0a000h		;setup screen segment A000
	mov	es,ax
	mov	al,byte ptr [color]	;get color of pixel to plot
	mov	es:[bx],al
nope2:	ret
point13x endp

	end


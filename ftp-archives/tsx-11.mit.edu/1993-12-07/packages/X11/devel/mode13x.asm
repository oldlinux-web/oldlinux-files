	include	model.h

;
;	VGAKIT Version 4.1
;
;	Copyright 1988,89,90,91 John Bridges
;	Free for use in commercial, shareware or freeware applications
;
;	MODE13X.ASM
;
;	Set the VGA 360 by 480 256 color mode.
;
;
.data
	extrn	maxx:word,maxy:word,xwidth:word
.code

vptbl	dw	06b00h	; horz total
	dw	05901h	; horz displayed
	dw	05a02h	; start horz blanking
	dw	08e03h	; end horz blanking
	dw	05e04h	; start h sync
	dw	08a05h	; end h sync
	dw	00d06h	; vertical total
	dw	03e07h	; overflow
	dw	04009h	; cell height
	dw	0ea10h	; v sync start
	dw	0ac11h	; v sync end and protect cr0-cr7
	dw	0df12h	; vertical displayed
	dw	02d13h	; offset
	dw	00014h	; turn off dword mode
	dw	0e715h	; v blank start
	dw	00616h	; v blank end
	dw	0e317h	; turn on byte mode
vpend	label	word


mode13x	proc
	mov	[maxx],360
	mov	[xwidth],90
	mov	[maxy],480
	push	ds
	mov	ax,cs
	mov	ds,ax

	mov	ax,13h		; start with standard mode 13h
	int	10h		; let the bios set the mode

	mov	dx,3c4h		; alter sequencer registers
	mov	ax,0604h	; disable chain 4
	out	dx,ax

	mov	ax,0f02h	; set write plane mask to all bit planes
	out	dx,ax
	push	di
	xor	di,di
	mov	ax,0a000h	; screen starts at segment A000
	mov	es,ax
	mov	cx,21600	; ((XSIZE*YSIZE)/(4 planes))/(2 bytes per word)
	xor	ax,ax
	cld
	rep	stosw		; clear the whole of the screen
	pop	di

	mov	ax,0100h	; synchronous reset
	out	dx,ax		; asserted
	mov	dx,3c2h		; misc output
	mov	al,0e7h		; use 28 mHz dot clock
	out	dx,al		; select it
	mov	dx,3c4h		; sequencer again
	mov	ax,0300h	; restart sequencer
	out	dx,ax		; running again

	mov	dx,3d4h		; alter crtc registers

	mov	al,11h		; cr11
	out	dx,al		; current value
	inc	dx		; point to data
	in	al,dx		; get cr11 value
	and	al,7fh		; remove cr0 -> cr7
	out	dx,al		;    write protect
	dec	dx		; point to index
	cld
	mov	si,offset vptbl
	mov	cx,((offset vpend)-(offset vptbl)) shr 1
outlp:	lodsw
	out	dx,ax
	loop	outlp
	pop	ds
	ret
mode13x	endp

	end


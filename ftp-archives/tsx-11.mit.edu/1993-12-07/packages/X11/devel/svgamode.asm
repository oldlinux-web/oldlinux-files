
	include model.h

;
;	VGAKIT Version 4.1
;
;	Copyright 1988,89,90,91 John Bridges
;	Free for use in commercial, shareware or freeware applications
;
;	SVGAMODE.ASM
;
;
.data
	extrn	cirrus:byte,everex:byte,paradise:byte,tseng:byte,trident:byte
	extrn	t8900:byte,ativga:byte,aheada:byte,aheadb:byte
	extrn	oaktech:byte,video7:byte,chipstech:byte,tseng4:byte,genoa:byte
	extrn	ncr:byte,compaq:byte,vesa:byte
	extrn	vga512:byte,vga1024:byte

	extrn	curbk:word

	public	maxx,maxy,xwidth

maxx	dw	?		;xwidth of screen in pixels
maxy	dw	?		;height of screen in pixels
xwidth	dw	?		;actual xwidth of screen in bytes

.code

	public	svgamode
	public	txtmode
	public	setmany

svgamode proc			;Set 640x480x256 on most SVGA cards
	cmp	[vesa],0
	jz	novs
	mov	ax,4f02h
	mov	bx,101h
	jmp	godo
novs:	cmp	[compaq],0
	jz	nocp
	mov	ax,2eh
	jmp	godo
nocp:	cmp	[genoa],0
	jz	nogn
	mov	ax,5ch
	jmp	godo
nogn:	cmp	[ncr],0
	jz	noncr
	mov	ax,5fh
	jmp	godo
noncr:	cmp	[oaktech],0
	jz	noak
	mov	ax,53h
	jmp	short godo
noak:	cmp	[aheada],0
	jnz	@f
	cmp	[aheadb],0
	jz	noab
@@:	mov	ax,61h
	jmp	short godo
noab:	cmp	[everex],0
	jz	noev
	mov	ax,70h
	mov	bl,30h
	jmp	short godo
noev:	cmp	[ativga],0
	jz	noati
	mov	ax,62h
	jmp	short godo
noati:	cmp	[trident],0
	jz	notr
	mov	ax,5dh
	jmp	short godo
notr:	cmp	[video7],0
	jz	nov7
	mov	ax,6f05h
	mov	bl,67h
	jmp	short godo
nov7:	cmp	[chipstech],0
	jz	noct
	mov	ax,79h
	jmp	short godo
noct:	cmp	[paradise],0
	jz	nopd
	mov	ax,5fh
	jmp	short godo
nopd:	cmp	[tseng],0
	jz	nots
	mov	ax,2eh
godo:	int	10h

	mov	[curbk],-1
	mov	ax,640
	mov	[maxx],ax		;default xwidth to 640 for now
	cmp	[compaq],0
	jz	nokludge
	mov	ax,1024
nokludge:
	mov	[xwidth],ax
	mov	ax,40h
	mov	es,ax
	mov	al,es:[84h]		;get height of screen
	inc	al
	mul	byte ptr es:[85h]
	mov	[maxy],ax

nots:	ret
svgamode endp

txtmode	proc
	mov	ax,3
	int	10h
	ret
txtmode	endp

setmany proc	palbuf:ptr byte,begcol:word,numcol:word
if @Datasize
	les	dx,[palbuf]
else
	mov	ax,ds
	mov	es,ax
	mov	dx,[palbuf]
endif
	mov	bx,[begcol]
	mov	cx,[numcol]
	mov	ax,1012h
	int	10h
	ret
setmany endp

	end


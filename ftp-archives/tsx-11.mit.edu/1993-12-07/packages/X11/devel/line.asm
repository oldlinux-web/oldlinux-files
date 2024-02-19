
	include	model.h

;
;	VGAKIT Version 4.1
;
;	Copyright 1988,89,90,91 John Bridges
;	Free for use in commercial, shareware or freeware applications
;
;	LINE.ASM
;
;	contains:
;
;	bline
;
;
.code
	public	bline

callpt	macro			;C order of parameter passing
	push	[color]
	push	di
	push	si
	call	[func]
	add	sp,6
	endm

;bline(x,y,x2,y2,func())
;
;input   : endpoints of line and function to use as point
;output  : nothing
;utility : draw line between endpoints using function
;          converted c routine
;
bline	proc	uses si di, x1:word,y1:word,x2:word,y2:word,color:word,func:ptr proc
	local	cnt:word,acc:word
	local	deltax:word,deltay:word,dirx:word,diry:word

	mov	si,[x1]
	mov	di,[y1]
	xor	ax,ax
	mov	[acc],ax
	mov	ax,[x2]
	sub	ax,si
	mov	dx,1
	jns	nosx
	neg	ax
	mov	dx,-1
nosx:	mov	[dirx],dx
	mov	[deltax],ax
	mov	ax,[y2]
	sub	ax,di
	mov	dx,1
	jns	nosy
	neg	ax
	mov	dx,-1
nosy:	mov	[diry],dx
	mov	[deltay],ax
	cmp	ax,[deltax]
	jge	ylp
	callpt
	mov	ax,[deltax]
	mov	[cnt],ax
lp1:
	dec	[cnt]
	js	done
	add	si,[dirx]
	mov	ax,[deltay]
	add	[acc],ax
	mov	ax,[acc]
	cmp	ax,[deltax]
	jb	noincy
	mov	ax,[deltax]
	sub	[acc],ax
	add	di,[diry]
noincy:
	callpt
	jmp	short lp1
ylp:
	callpt
	mov	ax,[deltay]
	mov	[cnt],ax
lp2:
	dec	[cnt]
	js	done
	add	di,[diry]
	mov	ax,[deltax]
	add	[acc],ax
	mov	ax,[acc]
	cmp	ax,[deltay]
	jb	noincx
	mov	ax,[deltay]
	sub	[acc],ax
	add	si,[dirx]
noincx:
	callpt
	jmp	short lp2
done:	ret
bline	endp

	end


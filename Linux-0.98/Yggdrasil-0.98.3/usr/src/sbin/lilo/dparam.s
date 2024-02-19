/* dparam.s */

/* Written 1992 by Werner Almesberger */

	.text

	.globl	_main
	.org	0x100

_main:	mov	bl,0x80			! NUL-terminate the command line
	xor	bh,bh
	mov	0x81(bx),bh
	mov	bx,#0x81
aloop:	mov	al,(bx)			! get the next character
	inc	bx
	cmp	al,#32			! space ?
	je	aloop			! yes -> skip it
	cmp	al,#48			! '0' ?
	jne	error			! no -> invalid
	cmpb	(bx),#120		! 'x' ?
	je	okay			! yes -> okay
	cmpb	(bx),#88		! 'X' ?
	jne	error			! no -> invalid
okay:	cmpb	1(bx),#56		! '8' ?
	jne	error			! no -> invalid
	cmpb	3(bx),#0		! one more digit ?
	jne	error			! no -> invalid
	mov	dl,2(bx)		! get the digit
	sub	dl,#48			! convert to a number
	cmp	dl,#1			! valid ?
	ja	error			! no -> error
	add	dl,#0x80		! adjust the number
	mov	ah,#8			! get the drive parameters
	int	0x13
	or	ah,ah			! error ?
	jnz	derror			! yes -> display a message
	push	cx			! save the parameters
	push	dx
	mov	al,cl			! number of sectors
	and	al,#0x3f
	xor	ah,ah
	call	decout
	call	space
	pop	dx			! number of heads
	mov	al,dh
	inc	al
	xor	ah,ah
	call	decout
	call	space
	pop	ax			! number of cylinders
	xchg	ah,al
	mov	cl,#6
	shr	ah,cl
	inc	ax
	call	decout
	mov	dx,#crlf		! crlf
	mov	ah,#9
	int	0x21
	ret				! done

error:	mov	dx,#errmsg		! display the error message
	jmp	dispit

derror:	mov	dx,#invdrv		! display the error message
	jmp	dispit

space:	mov	dx,#spc			! display two blanks
	jmp	dispit

decout:	mov	bx,#decend		! set the pointer to the end
declp:	xor	dx,dx			! divide by 10
	mov	cx,#10
	div	cx
	add	dl,#48			! make a digit
	dec	bx			! store digit
	mov	(bx),dl
	or	ax,ax			! zero ?
	jnz	declp			! no -> go on
	mov	dx,bx			! display the string
dispit:	mov	ah,#9
	int	0x21
	ret				! done

errmsg:	.ascii	"usage: dparam 0x80"
	.byte	13,10
	.ascii	"   or  dparam 0x81"
crlf:	.byte	13,10
	.ascii	"$"

invdrv:	.ascii	"Invalid drive"
	.byte	13,10
	.ascii	"$"

spc:	.ascii	"  $"

decbuf:	.byte	0,0,0,0,0
decend:	.ascii	"$"

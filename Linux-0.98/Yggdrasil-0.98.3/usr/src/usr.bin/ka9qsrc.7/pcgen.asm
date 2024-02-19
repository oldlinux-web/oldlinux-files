	include	pmacros.h

dseg
jtable	dw	l0,l1,l2,l3,l4,l5,l6,l7,l8,l9,l10,l11,l12,l13,l14,l15	
pubvar	sssave,<dw ?>
pubvar	spsave,<dw ?>
	db	512 dup(?)
pubvar	intstk,<label byte>
pubvar	mtasker,<db ?>
extvar	isat,<byte>
dsegend

; common routine for interrupt return
	pubproc	doret
	pop	es
	pop	di
	pop	si
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	cmp	isat@,1
	jnz	notat		; Only one 8259, so skip this stuff
	mov	al,0bh		; read in-service register from
	out	0a0h,al		; secondary 8259
	nop			; settling delay
	nop
	nop
	in	al,0a0h		; get it
	or	al,al		; Any bits set?
	jz	notat		; nope, not a secondary interrupt
	mov	al,20h		; Get EOI instruction
	out	0a0h,al		; Secondary 8259 (PC/AT only)
notat:	mov	al,20h		; 8259 end-of-interrupt command
	out	20h,al		; Primary 8259
	mov	ss,sssave@
	mov	sp,spsave@	; restore original stack context
	pop	ds
	pop	ax
	iret
	pend	doret

if TURBO
else
; setvect - set interrupt vector
; called from C as follows
; setvect(vec,vecval)
; char vec;		/* Interrupt number */
; void (*vecval)();	/* offset (and segment in large code model) */

	procdef	setvect,<<vec,byte>,<ipval,fptr>>
	mov	ah,25h
	mov	al,vec
	push	ds		; save
ifdef	LARGECODE
	lds	dx,ipval
else
	mov	dx,ipval
	push	cs
	pop	ds
endif
	int	21h
	pop	ds		; restore
	pret
	pend	setvect

; getvect - return current interrupt vector
; called from C as
; long		/* Returns CS in high word, IP in low word */
; getvect(vecnum)
; char vecnum;	/* Interrupt number */

	procdef	getvect,<<vecnum,byte>>
	mov	ah,35h
	mov	al,vecnum
	push	es	; save, since DOS uses it for a return value
	int	21h
	mov	dx,es	; CS value into DX (C high word)
	mov	ax,bx	; IP value into AX (C low word)
	pop	es	; restore es
	pret
	pend	getvect
endif

; kbraw - raw, nonblocking read from console
; If character is ready, return it; if not, return -1

	procdef	kbraw
	mov	ah,06h	; Direct Console I/O
	mov	dl,0ffh	; Read from keyboard
	int	21h	; Call DOS
	jz	nochar	; zero flag set -> no character ready
	mov	ah,0	; valid char is 0-255
	pret
nochar:
	mov	ax,-1	; no char, return -1
	pret
	pend	kbraw

; disable - disable interrupts and return previous state: 0 = disabled,
;           1 = enabled

	procdef	disable
	pushf			; save state on stack
	cli			; interrupts off
	pop	ax		; original flags -> ax
	and	ax,200h		; 1<<9 is IF bit
	jnz	ion		; nonzero -> interrupts were on
	pret
ion:	mov	ax,1
	pret
	pend	disable

; restore - restore interrupt state: 0 = off, nonzero = on

	procdef	restore,<<istate,byte>>
	test	istate,0ffh
	jz	ioff
	sti
ioff:	pret
	pend	restore

; Halt until an interrupt occurs, then return
	procdef eihalt
	sti	; make sure interrupts are enabled
	hlt
	pret
	pend	eihalt

; multitasker types
NONE		equ	0
DOUBLEDOS	equ	1
DESQVIEW	equ	2

; Relinquish processor so other task can run
	procdef	giveup
	cmp	mtasker@, DOUBLEDOS
	jnz	givedesqview
	mov	al,2		; 110 ms
	mov	ah,0eeh
	int	21h

givedesqview:
	cmp	mtasker@, DESQVIEW
	jnz	notask
	mov	ax, 1000h
	int	15h
notask:
	pret
	pend	giveup

; check for a multitasker running
	procdef chktasker
	mov	mtasker@, NONE
	; do the doubledos test
	mov	ah, 0e4h
	int	21h
	cmp	al, 1
	jz	isdos
	cmp	al, 2
	jnz	test_desq
isdos:	mov	mtasker@, DOUBLEDOS
	pret

	; test for desqview
test_desq:
	mov	ax, 2b01h
	mov	cx, 4445h
	mov	dx, 5351h
	int	21h
	cmp	al, 0ffh
	jnz	isdesq
	pret
isdesq:	mov	mtasker@, DESQVIEW
	pret
	pend	chktasker

; getds - Read DS for debugging purposes
	procdef	getds
	getds
	pret
	pend	getds

; Internet checksum subroutine
; Compute 1's-complement sum of data buffer
; Uses an unwound loop inspired by "Duff's Device" for performance
;
; Called from C as
; unsigned short
; lcsum(buf,cnt)
; unsigned short *buf;
; unsigned short cnt;
	procdef	lcsum,<<buf,ptr>,<cnt,word>>
	pushds			; save if using large model
	push	si
	ldptr	si,buf,ds	; ds:si = buf
	mov	cx,cnt		; cx = cnt
	cld			; autoincrement si

	mov	ax,cx
	shr	cx,1		; cx /= 16, number of loop iterations
	shr	cx,1
	shr	cx,1
	shr	cx,1

	inc	cx		; make fencepost adjustment for 1st pass
	and	ax,15		; ax = number of words modulo 16
	shl	ax,1		; *=2 for word table index
	lea	bx,jtable	; bx -> branch table
	add	bx,ax		; index into jump table
	clc			; initialize carry = 0
	mov	dx,0		; clear accumulated sum
	jmp	word ptr[bx]	; jump into loop

; Here the real work gets done. The numeric labels on the lodsw instructions
; are the targets for the indirect jump we just made.
;
; Each label corresponds to a possible remainder of (count / 16), while
; the number of times around the loop is determined by the quotient.
;
; The loop iteration count in cx has been incremented by one to adjust for
; the first pass.
; 
deloop:	lodsw
	adc	dx,ax
l15:	lodsw
	adc	dx,ax
l14:	lodsw
	adc	dx,ax
l13:	lodsw
	adc	dx,ax
l12:	lodsw
	adc	dx,ax
l11:	lodsw
	adc	dx,ax
l10:	lodsw
	adc	dx,ax
l9:	lodsw
	adc	dx,ax
l8:	lodsw
	adc	dx,ax
l7:	lodsw
	adc	dx,ax
l6:	lodsw
	adc	dx,ax
l5:	lodsw
	adc	dx,ax
l4:	lodsw
	adc	dx,ax
l3:	lodsw
	adc	dx,ax
l2:	lodsw
	adc	dx,ax
l1:	lodsw
	adc	dx,ax
l0:	loop	deloop		; :-)

	adc	dx,0		; get last carries
	adc	dx,0
	mov	ax,dx		; result into ax
	xchg	al,ah		; byte swap result (8088 is little-endian)
	pop	si
	popds			; all done
	pret
	pend	lcsum
;
; dos service routine used by JK1NNT installed by N3EUA
;
	procdef	jkintdos,<<reg_a,word>,<reg_b,word>,<reg_d,word>>
	mov	ax,reg_a
	mov	bx,reg_b
	mov	dx,reg_d
	int	21h
	pret
	pend	jkintdos
;
; NEC PC-9801 speed setting routine by JK1NNT installed by N3EUA
;
	procdef	set_speed,<<s_code,word>>
	push	ds
	mov	ax,40h
	mov	ds,ax
	mov	dx,s_code
	mov	ds:[269h],dl
	mov	dh,4ch
	mov	ds:[268h],dh
	pop	ds
	mov	cl,0ah
	int	0dch
	pret
	pend	set_speed

;
; This routine is a generic int 21h handler, is used only by disk free
; routine.  Added by K3MC  3 Dec 87
;
;
; General-purpose int 21h caller, currently used only to return AX, BX, CX, DX
; for computing amount of free disk space, and amount of total disk space.
;
; declared as: void isfree();
;
; Called from C as:
;
; isfree(&ax,&bx,&cx,&dx)
; unsigned short ax,bx,cx,dx;
;
	procdef	isfree,<<c_ax,ptr>,<c_bx,ptr>,<c_cx,ptr>,<c_dx,ptr>>
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di
	ldptr	di,c_ax,ds
	mov	ax,[di]
	ldptr	di,c_bx,ds
	mov	bx,[di]
	ldptr	di,c_cx,ds
	mov	cx,[di]
	ldptr	di,c_dx,ds
	mov	dx,[di]
	int 	21h
	ldptr	di,c_ax,ds
	mov	[di],ax
	ldptr	di,c_bx,ds
	mov	[di],bx
	ldptr	di,c_cx,ds
	mov	[di],cx
	ldptr	di,c_dx,ds
	mov	[di],dx
	pop	di
	pop	si
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pret
	pend	isfree

	end

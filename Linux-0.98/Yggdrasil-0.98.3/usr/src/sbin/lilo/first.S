/* first.S  -  LILO first stage boot loader */

/* Written 1992 by Werner Almesberger */


#include <linux/config.h>

#define LILO_ASM
#include "lilo.h"


	.text

	.globl	_main
	.org	0

_main:	jmp	start

	.org	2

! Boot device parameters. They are set by the installer.

	.ascii	"LILO"
	.word	STAGE_FIRST
	.word	VERSION

delay:	.word	0		! boot delay
filler:	.word	0		! (unused)

d_cx:	.word	0		! descriptor sector address
d_dx:	.word	0
d_al:	.byte	0		! (unused)

d_addr:				! second stage sector addresses

	.org	CODE_START_1

start:	mov	ax,#BOOTSEG	! beam us up ...
	mov	ds,ax
	mov	ax,#INITSEG
	mov	es,ax
	mov	cx,#256
	sub	si,si
	sub	di,di
	cld
	rep
	movsw
	jmpi	go,INITSEG

go:	cli			! no interrupts
	mov	sp,#STACK	! set the stack

	mov	ds,ax		! AX is already set
	mov	es,ax		! (ES may be wrong when restarting)
	mov	ss,ax
	sti			! now it is safe

	mov	ax,#0xe0d	! gimme a CR ...
	xor	bh,bh
	int	0x10
	mov	ax,#0xe0a	! ... an LF ...
	xor	bh,bh
	int	0x10
	mov	ax,#0xe4c	! ... an 'L' ...
	xor	bh,bh
	int	0x10

lagain:	mov	si,#d_addr	! ready to load the second stage loader
	mov	bx,#SECOND
	cld
sload:	lodsw			! get CX
	mov	cx,ax
	lodsw			! get DX
	mov	dx,ax
	or	ax,cx		! CX:DX == 0 ?
	jz	done		! yes -> start it
	inc	si		! skip the length byte
	mov	ax,#0x201	! load the sector
	int	0x13
	or	ah,ah		! okay ?
	jnz	error		! no -> start over again
	add	bx,#512		! next sector
	jmp	sload
error:	xor	ax,ax		! reset the FDC
	int	0x13
	jmp	lagain		! redo from start
done:	mov	ax,#0xe49	! display an 'I'
	xor	bh,bh
	int	0x10
	jmpi	0,SECONDSEG	! start the second stage loader

/* Here are at least 64 bytes of free space. This is reserved for the
   partition table. */

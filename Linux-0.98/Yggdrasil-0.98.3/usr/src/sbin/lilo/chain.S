/* chain.S  -  LILO boot chainer */

/* Written 1992 by Werner Almesberger */


#include <linux/config.h>

#define LILO_ASM
#include "lilo.h"


	.text

	.globl	_main
	.org	0

_main:	jmp	start

	.org	2

	.ascii	"LILO"
	.word	STAGE_CHAIN
	.word	VERSION

offset:	.word	0

start:	mov	ax,#SETUPSEG		! move boot sector to default location
	mov	ds,ax
	xor	ax,ax
	mov	es,ax
	mov	cx,#256
	mov	si,#512
	mov	di,#BOOTSEG*16
	rep
	movsw
	mov	cx,#0x20		! move partition table
	mov	si,#PART_TABLE_OFFSET
	mov	di,#PART_TABLE
	rep
	movsw
	mov	si,offset		! DS:SI points to partition table entry
	add	si,#PART_TABLE
	mov	ds,ax			! set DS and ES to zero
	jmpi	#BOOTSEG*16,0		! start boot sector

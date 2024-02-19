/* second.S  -  LILO second stage boot loader */

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
	.word	STAGE_SECOND
	.word	VERSION

	.org	CODE_START_2

start:	mov	ax,#0xe4c	! display an 'L'
	xor	bh,bh
	int	0x10
restrt: mov	ax,cs		! adjust segment registers
	mov	ds,ax
	mov	es,ax
ldsc:	mov	ax,#0x201	! load the descriptor table
	mov	bx,#DESCR
	seg	ss
	mov	cx,DSC_OFF
	seg	ss
	mov	dx,DSC_OFF+2
	int	0x13
	or	ah,ah		! okay ?
	jz	dokay		! yes -> go on
	xor	ax,ax		! reset FDC
	int	0x13
	jmp	ldsc		! retry

list:	mov	bx,#crlf	! display a CRLF
	call	say
	mov	si,#DESCR	! list all images
	mov	cx,#IMAGES
lloop:	testb	(si),#0xff	! done ?
	jz	ldone		! yes
	mov	bx,si		! display the name
	call	say
	add	si,#MAX_IMAGE_NAME
fill:	push	bx		! fill with spaces
	mov	ax,#0xe20
	xor	bh,bh
	int	0x10
	pop	bx
	inc	bx
	cmp	bx,si
	jbe	fill
	add	si,#DESCR_SIZE-MAX_IMAGE_NAME
	loop	lloop		! next image
ldone:	mov	bx,#crlf	! display a CRLF
	call	say
	jmp	iloop		! done

dokay:	mov	bx,#ospc	! display 'O '
	call	say
	mov	ospc,#0		! disable the message
	xor	ah,ah		! get the current timer
	int	0x1a
	seg	ss		! add the delay
	add	dx,DSC_OFF-4
	xor	ax,ax
	adc	cx,ax
	seg	ss		! no delays when retrying to boot
	mov	DSC_OFF-4,ax
	mov	to_low,dx	! save the timeout
	mov	to_high,cx

actlp:	mov	ah,#2		! get shift keys
	int	0x16
	and	al,#0x5f	! anything set ? (except NumLock)
	jnz	iloop		! yes -> enter interactive mode
	xor	ah,ah		! get timer
	int	0x1a
	cmp	cx,to_high	! compare MSW
	ja	tob1st		! -> boot first image
	jb	actlp		! -> wait
	cmp	dx,to_low	! compare LSW
	jae	tob1st		! -> boot first image
	jmp	actlp		! try again

tolist:	jmp	list		! ...

iloop:	mov	bx,#msg_p	! display boot prompt
	call	say
	mov	bx,#name
input:	push	bx
	xor	ah,ah		! read a key
	int	0x16
	pop	bx
	cmp	al,#9		! TAB ?
	je	tolist		! yes -> list images
	cmp	al,#63		! "?" ?
	je	tolist		! yes -> list images
	cmp	al,#13		! CR ?
	je	cr		! yes -> go on
	cmp	al,#8		! BS ?
	je	delch		! yes -> erase one character
	cmp	al,#127		! DEL ?
	je	delch		! yes -> erase one character
	cmp	al,#21		! ^U ?
	je	delline		! yes -> erase the line
	cmp	al,#24		! ^X ?
	je	delline		! yes -> erase the line
	cmp	al,#32		! ignore non-printable characters
	jbe	input
	cmp	al,#127
	jae	input
	mov	(bx),al		! store in the buffer
	inc	bx		! increment pointer
	push	bx
	mov	ah,#14		! echo
	xor	bh,bh
	int	0x10
	pop	bx
	cmp	bx,#name+MAX_IMAGE_NAME ! at end of buffer ?
	jb	input		! no -> next input
cr:	xor	al,al		! mark end
	mov	(bx),al
	push	bx		! display a CRLF
	mov	bx,#crlf
	call	say
	pop	bx
	cmp	bx,#name	! empty line ?
	je	bfirst		! yes -> boot first image
	mov	cx,#IMAGES	! test all names
	mov	bx,#DESCR
nextn:	mov	si,bx		! compare the names
	mov	di,#name
nextc:	mov	al,(si)		! character equal ?
	cmp	al,(di)
	jne	skipn		! no -> try next one
	or	al,al		! NUL ?
	jz	boot		! yes -> names match
	inc	si		! test next character
	inc	di
	jmp	nextc
toinput:jmp	input		! ...
tob1st:	jmp	bfirst		! ...
skipn:	add	bx,#DESCR_SIZE	! test next name
	loop	nextn
	mov	bx,#msg_nf	! not found -> display a message
	call	say
	jmp	iloop		! get more input
delch:	cmp	bx,#name	! at the beginning ?
	je	input		! yes -> do nothing
	push	bx		! display BS,SPC,BS
	mov	bx,#bs
	call	say
	pop	bx
	dec	bx		! move the pointer
	jmp	toinput		! go on
delline:cmp	bx,#name	! done ?
	je	toinput		! yes -> go on
	push	bx		! display BS,SPC,BS
	mov	bx,#bs
	call	say
	pop	bx
	dec	bx		! move the pointer
	jmp	delline		! next one

bfirst:	mov	bx,default	! boot the first image

boot:	mov	default,bx	! make this the new default
	push	bx		! save image descr
	mov	bx,#msg_l	! say hi
	call	say
	pop	bx		! display the image name
	push	bx
	call	say
	pop	si
	add	si,#ADDR_OFFS
	cld
	lodsw			! address of the first map sector
	mov	cx,ax
	lodsw
	mov	dx,ax
	inc	si		! skip number of sectors
	inc	si
	lodsw			! copy swap & root parameters
	seg	ss
	mov	506,ax
	lodsw
	seg	ss
	mov	508,ax
	lodsw			! copy BSS parameters
	mov	bssseg,ax
	lodsw
	mov	bsssgs,ax
	lodsw
	mov	bsswrd,ax
#ifdef DEBUG
	mov bx,#step1
	call say
#endif
	mov	bx,#MAP		! load the first map sector
	mov	al,#1
	call	sread
#ifdef DEBUG
	mov bx,#step2
	call say
#endif
	mov	ax,#SETUPSEG	! load the setup
	mov	es,ax
	xor	bx,bx
	mov	cx,#SETUPSECS
lsetup:	push	cx
	call	load
	pop	cx
	loop	lsetup
#ifdef DEBUG
	mov bx,#step3
	call say
#endif
	mov	ax,#SYSSEG	! load the system
	mov	es,ax
	xor	bx,bx
lsys:	call	load
	jmp	lsys

load:	push	es		! save ES:BX
	push	bx
lfetch:	mov	si,moff		! get map offset
	mov	cx,MAP(si)	! get address
	mov	dx,MAP+2(si)
	mov	al,MAP+4(si)
	or	cx,cx		! at EOF ?
	jz	launch		! yes -> start system
	add	si,#5		! increment pointer
	mov	moff,si
	cmp	si,#508		! page end ?
	jb	doload		! no -> load it
	mov	moff,#0		! reset pointer
	push	cs		! adjust ES
	pop	es
	mov	bx,#MAP		! load map page
	call	sread
	mov	ax,#0xe2e	! print a dot
	xor	bh,bh
	int	0x10
	jmp	lfetch		! try again
launch: mov	bx,#crlf	! display a CRLF
	call	say
	mov	dx,#0x3f2	! stop the floppy motor
	xor	al,al
	outb
	xor	ax,ax		! reset the FDC
	int	0x13
	mov	ax,bssseg	! clear BSS ?
	or	ax,ax
	jz	nobss		! no -> go on
	mov	es,ax
	mov	bx,bsssgs	! zero whole segments
zsegl:	or	bx,bx		! done with segments ?
	jz	segdn		! yes -> go on
	xor	ax,ax		! zero 32768 words
	xor	di,di
	mov	cx,#0x8000
	rep
	stosw
	dec	bx		! next segment
	mov	ax,es
	add	ax,#0x1000
	mov	es,ax
	jmp	zsegl
segdn:	mov	ax,bsswrd	! remaining words
	or	ax,ax		! any left ?
	jz	nobss		! no -> go on
	mov	cx,ax		! zero them
	xor	di,di
	xor	ax,ax
	rep
	stosw
nobss:	mov	ax,#INITSEG	! adjust segment registers
	mov	ds,ax
	mov	es,ax
	jmpi	0,SETUPSEG	! start the setup

doload:	pop	bx		! restore ES:BX
	pop	es

sread:	
#ifdef DEBUG
	push ax
	push bx
	push cx
	push dx
	push dx
	push cx
	push bx
	push ax
	mov bx,#sax
	call say
	pop ax
	call wout
	mov bx,#sbx
	call say
	pop ax
	call wout
	mov bx,#scx
	call say
	pop ax
	call wout
	mov bx,#sdx
	call say
	pop ax
	call wout
	mov bx,#sdone
	call say
	pop dx
	pop cx
	pop bx
	pop ax
#endif
	push	bx		! save registers
	push	cx
	push	dx
	mov	ah,#0x2		! read
	mov	tempal,al	! save AL
	int	0x13
	or	ah,ah		! okay ?
	jnz	rerror		! no -> complain
	pop	dx		! restore registers
	pop	cx
rokay:	pop	bx
	xor	ax,ax		! compute offset
	mov	ah,tempal
	add	ah,ah
	add	bx,ax		! move BX
	jnc	nowrap		! same segment -> go on
	mov	ax,es		! move ES
	add	ax,#0x1000
	mov	es,ax
nowrap:
aret:	ret			! done

rerror:	xor	ax,ax		! reset the disk
	int	0x13
	pop	dx		! try again
	pop	cx
	pop	bx
	push	bx
	mov	al,tempal
	mov	ah,#0x2
	int	0x13
	or	ah,ah		! okay ?
	jz	rokay		! yes -> go on
	push	ax
	mov	bx,#msg_re	! say something
	call	say
	pop	ax		! display the error code
	mov	al,ah
	call	bout
	mov	bx,#crlf	! a CRLF
	call	say
	mov	moff,#0		! restore initial state
	jmpi	restrt,SECONDSEG

! Display a hexadecimal byte

#ifdef DEBUG
wout:	push	ax
	mov	al,ah
	call	bout
	pop	ax
#endif
bout:	push	ax		! save byte
	shr	al,#4		! display upper nibble
	call	nout
	pop	ax
nout:	and	al,#15		! lower nible only
	add	al,#48		! display lower nibble
	cmp	al,#58		! convert to ASCII
	jb	nokay
	add	al,#7
nokay:	xor	bh,bh		! display it
	mov	ah,#14
	int	0x10
	ret			! done

! Display a NUL-terminated string on the console

say:	mov	al,(bx)		! get byte
	or	al,al		! NUL ?
	jz	aret		! yes -> done
	push	bx		! save pointer
	mov	ah,#14		! display, tty-style
	xor	bh,bh
	int	0x10
	pop	bx
	inc	bx		! next one
	jmp	say

! Some messages

msg_p:	.ascii	"boot: "
	.byte	0

msg_l:	.ascii	"Loading "
	.byte	0

msg_re:	.byte	13,10
	.ascii	"Error 0x"
	.byte	0

msg_nf:	.ascii	"No such image. [Tab] shows a list."
	.byte	13,10,0

ospc:	.ascii	"O"
	.byte	32,0

crlf:	.byte	13,10,0

bs:	.byte	8,32,8,0

#ifdef DEBUG
step1:	.ascii " map page,"
	.byte 0
step2:	.ascii " setup,"
	.byte 0
step3:	.ascii " system,"
	.byte 0

sax:	.ascii "AX="
	.byte 0
sbx:	.ascii " BX="
	.byte 0
scx:	.ascii " CX="
	.byte 0
sdx:	.ascii " DX="
	.byte 0
sdone:	.byte 13,10
	.byte 0
#endif

default:.word	DESCR		! default descriptor
tempal:	.byte	0
moff:	.word	0		! map offset

to_low:	.word	0		! timeout
to_high:.word	0

bssseg:	.word	0		! BSS parameters
bsssgs:	.word	0
bsswrd:	.word	0

name:

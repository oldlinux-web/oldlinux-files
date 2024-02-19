
	include	model.h

;
;	VGAKIT Version 4.1
;
;	Copyright 1988,89,90,91 John Bridges
;	Free for use in commercial, shareware or freeware applications
;
;	BANKS.ASM
;
.data

OSEG	equ	DS:		;segment override for variable access

bankadr	dw	?
if @Codesize
bankseg	dw	?
endif

	public	curbk

curbk	dw	?

	public	vga512,vga1024

vga512	dw	?
vga1024	dw	?

	public	cirrus,everex,paradise,tseng,trident,t8900
	public	ativga,aheada,aheadb,oaktech,video7
	public	chipstech,tseng4,genoa,ncr,compaq,vesa

cirrus	dw	?
everex	dw	?
paradise dw	?
tseng	dw	?
trident	dw	?
t8900	dw	?
ativga	dw	?
aheada	dw	?
aheadb	dw	?
oaktech	dw	?
video7	dw	?
chipstech dw	?
tseng4	dw	?
genoa	dw	?
ncr	dw	?
compaq	dw	?
vesa	dw	?

first	dw	?		;flag so whichvga() is only called once
retval	dw	?		;first return value from whichvga()

vgainfo	label	word
vesaid	db	'VESA'		; 4 signature bytes
vesaver	dw	?		; VESA version number
oemstr	dd	?		; Pointer to OEM string
capabil	db	4 dup (?)	; Capabilities of the video environment
modelst	dd	?		; Pointer to supported Super VGA modes

vesashift db	0		; number of bits to shift bank number left

.code

	public	newbank
	public	whichvga

newbank	proc			;bank number is in AX
	cli
	mov	OSEG[curbk],ax
if @Codesize
	jmp	dword ptr OSEG[bankadr]
else
	jmp	word ptr OSEG[bankadr]
endif


_tseng::				;Tseng
	push	ax
	push	dx
	and	al,7
	mov	ah,al
	shl	al,1
	shl	al,1
	shl	al,1
	or	al,ah
	or	al,01000000b
	mov	dx,3cdh
	out	dx,al
	sti
	pop	dx
	pop	ax
	ret


_tseng4::				;Tseng 4000 series
	push	ax
	push	dx
	mov	ah,al
	mov	dx,3bfh			;Enable access to extended registers
	mov	al,3
	out	dx,al
	mov	dl,0d8h
	mov	al,0a0h
	out	dx,al
	and	ah,15
	mov	al,ah
	shl	al,1
	shl	al,1
	shl	al,1
	shl	al,1
	or	al,ah
	mov	dl,0cdh
	out	dx,al
	sti
	pop	dx
	pop	ax
	ret


_trident::			;Trident
	push	ax
	push	dx
	mov	dx,3ceh		;set pagesize to 64k
	mov	al,6
	out	dx,al
	inc	dl
	in	al,dx
	dec	dl
	or	al,4
	mov	ah,al
	mov	al,6
	out	dx,ax

	mov	dl,0c4h		;switch to BPS mode
	mov	al,0bh
	out	dx,al
	inc	dl
	in	al,dx
	dec	dl

	mov	ah,byte ptr OSEG[curbk]
	xor	ah,2
	mov	dx,3c4h
	mov	al,0eh
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret


_video7::			;Video 7
	push	ax
	push	dx
	push	cx
	and	ax,15
	mov	ch,al
	mov	dx,3c4h
	mov	ax,0ea06h
	out	dx,ax
	mov	ah,ch
	and	ah,1
	mov	al,0f9h
	out	dx,ax
	mov	al,ch
	and	al,1100b
	mov	ah,al
	shr	ah,1
	shr	ah,1
	or	ah,al
	mov	al,0f6h
	out	dx,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,not 1111b
	or	ah,al
	mov	al,0f6h
	out	dx,ax
	mov	ah,ch
	mov	cl,4
	shl	ah,cl
	and	ah,100000b
	mov	dl,0cch
	in	al,dx
	mov	dl,0c2h
	and	al,not 100000b
	or	al,ah
	out	dx,al
	sti
	pop	cx
	pop	dx
	pop	ax
	ret


_paradise::			;Paradise
	push	ax
	push	dx
	mov	dx,3ceh
	mov	ax,50fh		;turn off write protect on VGA registers
	out	dx,ax
	mov	ah,byte ptr OSEG[curbk]
	shl	ah,1		;change 64k bank number into 4k bank number
	shl	ah,1
	shl	ah,1
	shl	ah,1
	mov	al,9
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret


_chipstech::			;Chips & Tech
	push	ax
	push	dx
	mov	dx,46e8h	;place chip in setup mode
	mov	ax,1eh
	out	dx,ax
	mov	dx,103h		;enable extended registers
	mov	ax,0080h
	out	dx,ax
	mov	dx,46e8h	;bring chip out of setup mode
	mov	ax,0eh
	out	dx,ax
	mov	ah,byte ptr OSEG[curbk]
	shl	ah,1		;change 64k bank number into 16k bank number
	shl	ah,1
	mov	al,10h
	mov	dx,3d6h
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret


_ativga::			;ATI VGA Wonder
	push	ax
	push	dx
	mov	ah,al
	mov	dx,1ceh
	mov	al,0b2h
	out	dx,al
	inc	dl
	in	al,dx
	shl	ah,1
	and	al,0e1h
	or	ah,al
	mov	al,0b2h
	dec	dl
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret


_everex::			;Everex
	push	ax
	push	dx
	push	cx
	mov	cl,al
	mov	dx,3c4h
	mov	al,8
	out	dx,al
	inc	dl
	in	al,dx
	dec	dl
	shl	al,1
	shr	cl,1
	rcr	al,1
	mov	ah,al
	mov	al,8
	out	dx,ax
	mov	dl,0cch
	in	al,dx
	mov	dl,0c2h
	and	al,0dfh
	shr	cl,1
	jc	nob2
	or	al,20h
nob2:	out	dx,al
	sti
	pop	cx
	pop	dx
	pop	ax
	ret


_aheada::			;Ahead Systems Ver A
	push	ax
	push	dx
	push	cx
	mov	ch,al
	mov	dx,3ceh		;Enable extended registers
	mov	ax,200fh
	out	dx,ax
	mov	dl,0cch		;bit 0
	in	al,dx
	mov	dl,0c2h
	and	al,11011111b
	shr	ch,1
	jnc	skpa
	or	al,00100000b
skpa:	out	dx,al
	mov	dl,0cfh		;bits 1,2,3
	mov	al,0
	out	dx,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,11111000b
	or	al,ch
	mov	ah,al
	mov	al,0
	out	dx,ax
	sti
	pop	cx
	pop	dx
	pop	ax
	ret


_aheadb::			;Ahead Systems Ver A
	push	ax
	push	dx
	push	cx
	mov	ch,al
	mov	dx,3ceh		;Enable extended registers
	mov	ax,200fh
	out	dx,ax
	mov	ah,ch
	mov	cl,4
	shl	ah,cl
	or	ah,ch
	mov	al,0dh
	out	dx,ax
	sti
	pop	cx
	pop	dx
	pop	ax
	ret


_oaktech::			;Oak Technology Inc OTI-067
	push	ax
	push	dx
	and	al,15
	mov	ah,al
	shl	al,1
	shl	al,1
	shl	al,1
	shl	al,1
	or	ah,al
	mov	al,11h
	mov	dx,3deh
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret

_genoa::			;GENOA GVGA
	push	ax
	push	dx
	mov	ah,al
	shl	al,1
	shl	al,1
	shl	al,1
	or	ah,al
	mov	al,6
	or	ah,40h
	mov	dx,3c4h
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret

_ncr::				;NCR 77C22E
	push	ax
	push	dx
	shl	al,1		;change 64k bank number into 16k bank number
	shl	al,1
	mov	ah,al
	mov	al,18h
	mov	dx,3c4h
	out	dx,ax
	mov	ax,19h
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret

_compaq::			;Compaq
	push	ax
	push	dx
	mov	dx,3ceh
	mov	ax,50fh		;unlock extended registers
	out	dx,ax
	mov	ah,byte ptr OSEG[curbk]
	shl	ah,1		;change 64k bank number into 4k bank number
	shl	ah,1
	shl	ah,1
	shl	ah,1
	mov	al,45h
	out	dx,ax
	sti
	pop	dx
	pop	ax
	ret

_vesa::				;Vesa SVGA interface
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cl,[vesashift]
	shl	ax,cl
	mov	dx,ax
	xor	bx,bx
	mov	ax,4f05h
	int	10h
	sti
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

_nobank::
	sti
	ret

newbank	endp

bkadr	macro	func
	mov	[func],1
	mov	[bankadr],offset _&func
if @Codesize
	mov	[bankseg],seg _&func
endif
	endm

nojmp	macro
	local	lbl
	jmp	lbl
lbl:
	endm


whichvga proc	uses si di
	local	vesabuf[256]:byte

	cmp	[first],'FI'
	jnz	gotest
	mov	ax,[retval]
	ret
gotest:	mov	[bankadr],offset _nobank
if @Codesize
	mov	[bankseg],seg _nobank
endif
	xor	ax,ax
	mov	[curbk],ax
	mov	[vga512],ax
	mov	[vga1024],ax
	mov	[cirrus],ax
	mov	[everex],ax
	mov	[paradise],ax
	mov	[tseng],ax
	mov	[trident],ax
	mov	[t8900],ax
	mov	[ativga],ax
	mov	[aheada],ax
	mov	[aheadb],ax
	mov	[oaktech],ax
	mov	[video7],ax
	mov	[chipstech],ax
	mov	[tseng4],ax
	mov	[genoa],ax
	mov	[ncr],ax
	mov	[compaq],ax
	mov	[vesa],ax
	mov	[first],'FI'

	mov	ax,ss
	mov	es,ax
	lea	di,vesabuf[0]
	mov	ax,4f00h
	int	10h
	cmp	ax,4fh
	jnz	novesa
	lea	si,vesabuf[0]
	mov	di,offset vgainfo
	mov	ax,ds
	mov	es,ax
	push	ds
	mov	ax,ss
	mov	ds,ax
	mov	cx,7
	cld
	rep	movsw
	pop	ds
	bkadr	vesa
	mov	[vga512],1
	mov	[vga1024],1
;	jmp	fini

novesa:	mov	si,1
	mov	ax,0c000h
	mov	es,ax
	cmp	word ptr es:[40h],'13'	;ATI Signiture on the Video BIOS
	jnz	noati
	bkadr	ativga
	cli
	mov	dx,1ceh
	mov	al,0bbh
	out	dx,al
	inc	dl
	in	al,dx
	sti
	and	al,20h
	jz	no512
	mov	[vga512],1
no512:	jmp	fini

noati:	mov	ax,7000h		;Test for Everex
	xor	bx,bx
	cld
	int	10h
	cmp	al,70h
	jnz	noev
	bkadr	everex
	and	ch,11000000b		;how much memory on board
	jz	skp
	mov	[vga512],1
skp:					;fall through for Everex boards using Trident or Tseng4000

noev:	mov	ax,0bf03h		;Test for Compaq
	xor	bx,bx
	mov	cx,bx
	int	10h
	cmp	ax,0bf03h
	jnz	nocp
	test	cl,40h			;is 640x480x256 available?
	jz	nocp
	bkadr	compaq
	mov	[vga512],1
	jmp	fini

nocp:	mov	dx,3c4h			;Test for NCR 77C22E
	mov	ax,0ff05h
	call	_isport2
	jnz	noncr
	mov	ax,5			;Disable extended registers
	out	dx,ax
	mov	ax,0ff10h		;Try to write to extended register 10
	call	_isport2		;If it writes then not NCR
	jz	noncr
	mov	ax,105h			;Enable extended registers
	out	dx,ax
	mov	ax,0ff10h
	call	_isport2
	jnz	noncr			;If it does NOT write then not NCR
	bkadr	ncr
	mov	[vga512],1
	jmp	fini

noncr:	mov	dx,3c4h			;Test for Trident
	mov	al,0bh
	out	dx,al
	inc	dl
	in	al,dx
	cmp	al,06h
	ja	notri
	cmp	al,2
	jb	notri
	bkadr	trident
	cmp	al,3
	jb	no89
	mov	[t8900],1
	mov	dx,3d5h
	mov	al,1fh
	out	dx,al
	inc	dx
	in	al,dx
	and	al,3
	cmp	al,1
	jb	notmem
	mov	[vga512],1
	je	notmem
	mov	[vga1024],1
notmem:	jmp	fini

no89:	mov	[vga512],1
	jmp	fini

notri:	mov	ax,6f00h		;Test for Video 7
	xor	bx,bx
	cld
	int	10h
	cmp	bx,'V7'
	jnz	nov7
	bkadr	video7
	mov	ax,6f07h
	cld
	int	10h
	and	ah,7fh
	cmp	ah,1
	jbe	skp2
	mov	[vga512],1
skp2:	cmp	ah,3
	jbe	skp3
	mov	[vga1024],1
skp3:	jmp	fini

nov7:	mov	dx,3d4h			;Test for GENOA GVGA
	mov	ax,032eh		;check for Herchi Register
	call	_isport2
	jnz	nogn
	mov	dx,3c4h			;check for memory segment register
	mov	ax,3f06h
	call	_isport2
	jnz	nogn
	bkadr	genoa
	mov	[vga512],1
	jmp	fini

nogn:	call	_cirrus			;Test for Cirrus
	cmp	[cirrus],0
	je	noci
	jmp	fini

noci:	mov	dx,3ceh			;Test for Paradise
	mov	al,9			;check Bank switch register
	out	dx,al
	inc	dx
	in	al,dx
	dec	dx
	or	al,al
	jnz	nopd

	mov	ax,50fh			;turn off write protect on VGA registers
	out	dx,ax
	mov	dx,offset _pdrsub
	mov	cx,1
	call	_chkbk
	jc	nopd			;if bank 0 and 1 same not paradise
	bkadr	paradise
	mov	dx,3ceh
	mov	al,0bh			;512k detect from Bob Berry
	out	dx,al
	inc	dx
	in	al,dx
	test	al,80h			;if top bit set then 512k
	jz	nop512
	mov	[vga512],1
nop512:	jmp	fini

nopd:	mov	ax,5f00h		;Test for Chips & Tech
	xor	bx,bx
	cld
	int	10h
	cmp	al,5fh
	jnz	noct
	bkadr	chipstech
	cmp	bh,1
	jb	skp4
	mov	[vga512],1
skp4:	jmp	fini

noct:	mov	ch,0
	mov	dx,3d4h			;check for Tseng 4000 series
	mov	ax,0f33h
	call	_isport2
	jnz	not4
	mov	ch,1

	mov	dx,3bfh			;Enable access to extended registers
	mov	al,3
	out	dx,al
	mov	dx,3d8h
	mov	al,0a0h
	out	dx,al
	jmp	short yes4

not4:	mov	dx,3d4h			;Test for Tseng 3000 or 4000
	mov	ax,1f25h		;is the Overflow High register there?
	call	_isport2
	jnz	nots
	mov	al,03fh			;bottom six bits only
	jmp	short yes3
yes4:	mov	al,0ffh
yes3:	mov	dx,3cdh			;test bank switch register
	call	_isport1
	jnz	nots
	bkadr	tseng
	cmp	ch,0
	jnz	t4mem
	mov	[vga512],1
	jmp	fini

t4mem:	mov	dx,3d4h			;Tseng 4000 memory detect 1meg
	mov	al,37h
	out	dx,al
	inc	dx
	in	al,dx
	test	al,1000b		;if using 64kx4 RAMs then no more than 256k
	jz	nomem
	and	al,3
	cmp	al,1			;if 8 bit wide bus then only two 256kx4 RAMs
	jbe	nomem
	mov	[vga512],1
	cmp	al,2			;if 16 bit wide bus then four 256kx4 RAMs
	je	nomem
	mov	[vga1024],1		;full meg with eight 256kx4 RAMs
nomem:	bkadr	tseng4
	jmp	short fini

nots:
	mov	dx,3ceh			;Test for Above A or B chipsets
	mov	ax,200fh
	out	dx,ax
	inc	dx
	nojmp
	in	al,dx
	cmp	al,21h
	jz	verb
	cmp	al,20h
	jnz	noab
	bkadr	aheada
	mov	[vga512],1
	jmp	short fini

verb:	bkadr	aheadb
	mov	[vga512],1
	jmp	short fini

noab:	mov	dx,3deh			;Test for Oak Technology
	mov	ax,0ff11h		;look for bank switch register
	call	_isport2
	jnz	nooak
	bkadr	oaktech
	mov	al,0dh
	out	dx,al
	inc	dx
	nojmp
	in	al,dx
	test	al,80h
	jz	no4ram
	mov	[vga512],1
no4ram:	jmp	short fini

nooak:	mov	si,0

fini:	mov	ax,si
	mov	[retval],ax
	ret
whichvga endp


_cirrus	proc	near
	mov	dx,3d4h		; assume 3dx addressing
	mov	al,0ch		; screen a start address hi
	out	dx,al		; select index
	inc	dx		; point to data
	mov	ah,al		; save index in ah
	in	al,dx		; get screen a start address hi
	xchg	ah,al		; swap index and data
	push	ax		; save old value
	push	dx		; save crtc address
	xor	al,al		; clear crc
	out	dx,al		; and out to the crtc

	mov	al,1fh		; Eagle ID register
	dec	dx		; back to index
	out	dx,al		; select index
	inc	dx		; point to data
	in	al,dx		; read the id register
	mov	bh,al		; and save it in bh

	mov	cl,4		; nibble swap rotate count
	mov	dx,3c4h		; sequencer/extensions
	mov	bl,6		; extensions enable register

	ror	bh,cl		; compute extensions disable value
	mov	ax,bx		; extensions disable
	out	dx,ax		; disable extensions
	inc	dx		; point to data
	in	al,dx		; read enable flag
	or	al,al		; disabled ?
	jnz	exit		; nope, not an cirrus

	ror	bh,cl		; compute extensions enable value
	dec	dx		; point to index
	mov	ax,bx		; extensions enable
	out	dx,ax		; enable extensions
	inc	dx		; point to data
	in	al,dx		; read enable flag
	cmp	al,1		; enabled ?
	jne	exit		; nope, not an cirrus
	mov	[cirrus],1
	mov	[bankadr],offset _nobank
if @Codesize
	mov	[bankseg],seg _nobank
endif
exit:	pop	dx		; restore crtc address
	dec	dx		; point to index
	pop	ax		; recover crc index and data
	out	dx,ax		; restore crc value
	ret
_cirrus	endp

_chkbk	proc	near		;bank switch check routine
	mov	di,0b800h
	mov	es,di
	xor	di,di
	mov	bx,1234h
	call	_gochk
	jnz	badchk
	mov	bx,4321h
	call	_gochk
	jnz	badchk
	clc
	ret
badchk:	stc
	ret
_chkbk	endp

_gochk	proc	near
	push	si
	mov	si,bx

	mov	al,cl
	call	dx
	xchg	bl,es:[di]
	mov	al,ch
	call	dx
	xchg	bh,es:[di]

	xchg	si,bx

	mov	al,cl
	call	dx
	xor	bl,es:[di]
	mov	al,ch
	call	dx
	xor	bh,es:[di]

	xchg	si,bx

	mov	al,ch
	call	dx
	mov	es:[di],bh
	mov	al,cl
	call	dx
	mov	es:[di],bl

	mov	al,0
	call	dx
	or	si,si
	pop	si
	ret
_gochk	endp


_pdrsub	proc	near		;Paradise
	push	dx
	mov	ah,al
	mov	dx,3ceh
	mov	al,9
	out	dx,ax
	pop	dx
	ret
_pdrsub	endp


_isport2 proc	near
	push	bx
	mov	bx,ax
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	xchg	al,ah
	push	ax
	mov	ax,bx
	out	dx,ax
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,bh
	cmp	al,bh
	jnz	noport
	mov	al,ah
	mov	ah,0
	out	dx,ax
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,bh
	cmp	al,0
noport:	pop	ax
	out	dx,ax
	pop	bx
	ret
_isport2 endp

_isport1 proc	near
	mov	ah,al
	in	al,dx
	push	ax
	mov	al,ah
	out	dx,al
	in	al,dx
	and	al,ah
	cmp	al,ah
	jnz	noport
	mov	al,0
	out	dx,al
	in	al,dx
	and	al,ah
	cmp	al,0
noport:	pop	ax
	out	dx,al
	ret
_isport1 endp

	end


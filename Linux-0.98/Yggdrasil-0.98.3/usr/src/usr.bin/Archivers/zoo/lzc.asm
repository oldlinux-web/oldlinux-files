title	Lempel-Ziv Compressor
; $Source: /usr/home/dhesi/zoo/RCS/lzc.asm,v $
; $Id: lzc.asm,v 1.4 91/07/07 09:36:18 dhesi Exp $

;Derived from Tom Pfau's public domain assembly code.
;The contents of this file are hereby released to the public domain.
;                                   -- Rahul Dhesi 1988/08/24

UNBUF_IO	equ	1		;use unbuffered I/O

public	_lzc

	include asmconst.ai
	include macros.ai

check_gap	equ	4000		;Check ratio every so many bits
scale_limit	equ	32000		;scale down if bitsout > scale_limit
rat_thresh	equ	0ffffh		;don't reset if rat > rat_thresh

;Hash table entry
hash_rec	struc
first	dw	?			; First entry with this value
next	dw	?			; Next entry along chain
char	db	?			; Suffix char
hash_rec	ends

extrn	docrc:near			;Procedure for block CRC in lzd.asm

ifdef	UNBUF_IO
extrn	_read:near
extrn	_blockwrite:near
else
extrn	_zooread:near
extrn	_zoowrite:near
endif

;Declare Segments
_text	segment byte public 'code'
_text	ends

dgroup	group	_data
	assume ds:dgroup,es:dgroup
_data	segment word public 'data'
extrn	_out_buf_adr:word		;address of C output buffer
extrn	_in_buf_adr:word		;address of C input buffer

extrn	memflag:byte			;got memory? flag

save_sp		dw	?

bytesin		dw	?		;count of bytes read
bitsout		dw	?		;count of bits written
ratio		dw	?		;recent ratio
ratflag		db	?		;flag to remind us to check ratio
bit_interval	dw	?		;interval at which to test ratio

input_handle	dw	?
output_handle	dw	?
hash_seg	dw	?
prefix_code	dw	?
free_code	dw	?
max_code	dw	?
nbits		dw	?
k		db	?
bit_offset	dw	?
input_offset	dw	0
input_size	dw	0
_data	ends

memory	segment para public 'memory'
hash	label	hash_rec
memory	ends

add_code macro
	local	ac1,ac2,ac3
	mov	bx,free_code		;Get code to use
	push	ds			;point to hash table
	mov	ds,hash_seg
	or	di,di			;First use of this prefix?
	jz	ac1			;zero means yes
	mov	[si].next,bx		;point last use to new entry
	jmp	short ac2
ac1:	mov	[si].first,bx		;Point first use to new entry
ac2:	cmp	bx,maxmax		;Have we reached code limit?
	je	ac3			;equal means yes, just return

	;call	index			;get address of new entry
	call_index			;macro for speed

	mov	[si].first,-1		;initialize pointers
	mov	[si].next,-1
	mov	[si].char,al		;save suffix char
	inc	es:free_code		;adjust next code
ac3:	pop	ds			;restore seg reg
	endm

read_char macro				;Macro for speed
	local m$1,m$2,m$3,m$4
	inc	[bytesin]		;Maintain input byte count for ratio 
	mov	di,input_offset		;Anything left in buffer?
	cmp	di,input_size
	jb	m$1			;less means yes

	mov	cx,inbufsiz
	call	doread			;read block

	cmp	ax,-1
	jnz	m$3
	jmp	IO_err			;input error
m$3:
	mov	dx,_in_buf_adr		;for docrc
	call	docrc
	or	ax,ax			;Anything left?
	jz	m$2			;zero means no, finished
	mov	input_size,ax		;Save bytes read
	mov	input_offset,0		;Point to beginning of buffer
	mov	di,0
m$1:	
	mov	si,_in_buf_adr
	add	si,di
	lodsb				;Read it in
	inc	input_offset		;Adjust pointer
	clc				;Success
	jmp	short m$4
m$2:	stc				;Nothing left
m$4:
	endm

;Start writing code
_text	segment
	assume	cs:_text,ds:dgroup,es:dgroup,ss:nothing

_lzc	proc	near
	push	bp			;Standard C entry code
	mov	bp,sp
	push	di
	push	si
	
	push	ds			;Save ds to be sure
	mov	bx,ds
	mov	es,bx

;Get two parameters, both integers, that are input file handle and
;output file handle
	mov	ax,[bp+4]
	mov	[input_handle],ax
	mov	ax,[bp+6]
	mov	[output_handle],ax

	call	compress		;Compress file
					;Status received back in AX
	pop	ds
	pop	si			;Standard C return code
	pop	di
	mov	sp,bp
	pop	bp
	ret

_lzc	endp

compress	proc	near
	mov	[save_sp],sp		;Save SP in case of error return

;Initialize variables for serial re-entrancy
	mov	[bit_offset],0
	mov	[input_offset],0
	mov	[input_size],0

	test	memflag,0ffH		;Memory allocated?
	jnz	gotmem			;If allocated, continue
	malloc	<((maxmax * 5) / 16 + 20)>	;allocate it
	jnc	here1
	jmp	MALLOC_err1
here1:
	mov	hash_seg,ax		;Save segment address of mem block
	mov	memflag,0ffh		;Set flag to remind us later
gotmem:

l1:	call	init_table		;Initialize the table and some vars
	mov	ax,clear		;Write a clear code
	call	write_code
	;call	read_char		;Read first char
	read_char			;macro for speed
l4:	

;new code to check compression ratio
	test	[ratflag],0FFH		;Time to check ratio?
	jz	rd$1			;Skip checking if zero
	call	check_ratio
rd$1:
	xor	ah,ah			;Turn char into code
l4a:	mov	prefix_code,ax		;Set prefix code
	;call	read_char		;Read next char
	read_char			;macro for speed
	jc	l17			;Carry means eof
	mov	k,al			;Save char in k
	mov	bx,prefix_code		;Get prefix code

	call	lookup_code		;See if this pair in table

	jnc	l4a			;nc means yes, new code in ax
	;call	add_code		;Add pair to table
	add_code			;Macro for speed
	push	bx			;Save new code
	mov	ax,prefix_code		;Write old prefix code
	call	write_code
	pop	bx
	mov	al,k			;Get last char
	cmp	bx,max_code		;Exceed code size?

	jnb	l4$
	jmp	l4
l4$:
	cmp	nbits,maxbits		;Currently less than maxbits?
	jb	l14			;yes
	mov	ax,clear		;Write a clear code
	call	write_code
	call	init_table		;Reinit table
	mov	al,k			;get last char
	jmp	l4			;Start over
l14:	inc	nbits			;Increase number of bits
	shl	max_code,1		;Double max code size
	jmp	l4			;Get next char
l17:	mov	ax,prefix_code		;Write last code
	call	write_code
	mov	ax,eof			;Write eof code
	call	write_code
	mov	ax,bit_offset		;Make sure buffer is flushed to file
	cmp	ax,0
	je	OK_ret
	mov	dx,ax			;dx <- ax
	shr	ax,1
	shr	ax,1
	shr	ax,1			;ax <- ax div 8
	and	dx,07			;dx <- ax mod 8
					;If extra bits, make sure they get
	je	l17a			;written
	inc	ax
l17a:	call	flush
OK_ret:	
	xor	ax,ax			;Normal return -- compressed
	ret
IO_err:
	mov	ax,2			;I/O error return 
	mov	sp,[save_sp]		;Restore stack pointer
	ret	

MALLOC_err1:				;hash table alloc error
	mov	ax,1			;Malloc error return
	mov	sp,[save_sp]		;Restore stack pointer
	ret
compress	endp

init_table	proc	near
	mov	[bytesin],0		;Input byte count
	mov	[bitsout],0		;Output bit count
	mov	[ratio],0
	mov	[ratflag],0
	mov	[bit_interval],check_gap

	mov	nbits,9			;Set code size to 9
	mov	max_code,512		;Set max code to 512
	push	es			;Save seg reg
	mov	es,hash_seg		;Address hash table
	mov	ax,-1			;Unused flag
	mov	cx,640			;Clear first 256 entries
	mov	di,offset hash		;Point to first entry
rep	stosw				;Clear it out
	pop	es			;Restore seg reg
	mov	free_code,first_free	;Set next code to use
	ret				;done
init_table	endp

write_code	proc	near
	push	ax			;Save code
	mov	cx,nbits
	add	[bitsout],cx		;Maintain output bit count for ratio
	sub	[bit_interval],cx
	jg	rd$2			;OK if not reached interval
	mov	[ratflag],1		;else set flag -- check ratio soon
rd$2:
	mov	ax,bit_offset		;Get bit offset
	mov	cx,nbits		;Adjust bit offset by code size
	add	bit_offset,cx

	mov	dx,ax			;dx <- ax
	shr	ax,1
	shr	ax,1
	shr	ax,1			;ax <- ax div 8
	and	dx,07			;dx <- ax mod 8

	;Now ax contains byte offset and
	;dx contains bit offset within that byte (I think)

	cmp	ax,outbufsiz-4		;Approaching end of buffer?
	jb	wc1			;less means no
	call	flush			;Write the buffer

	push	dx			;dx contains offset within byte
	add	dx,nbits		;adjust by code size
	mov	bit_offset,dx		;new bit offset
	pop	dx			;restore dx

;ax is an index into output buffer.  Next, ax <- address of buffer[ax]
	add	ax,[_out_buf_adr]
	mov	si,ax			;put in si
	mov	al,byte ptr [si]	;move byte to first position

;put value of al into first byte of output buffer
	push	bx
	mov	bx,[_out_buf_adr]
	mov	[bx],al
	pop	bx
	xor	ax,ax			;Byte offset of zero
wc1:	add	ax,[_out_buf_adr]	;Point into buffer
	mov	di,ax			;Destination
	pop	ax			;Restore code
	mov	cx,dx			;offset within byte
	xor	dx,dx			;dx will catch bits rotated out
	jcxz	wc3			;If offset in byte is zero, skip shift
wc2:	shl	ax,1			;Rotate code
	rcl	dx,1
	loop	wc2
	or	al,byte ptr [di]	;Grab bits currently in buffer
wc3:	stosw				;Save data
	mov	al,dl			;Grab extra bits
	stosb				;and save
	ret
write_code	endp

flush	proc	near
	push	ax			;Save all registers
	push	bx			;AX contains number of bytes to write
	push	cx
	push	dx

	push	si			;may not be necessary to save si & di
	push	di

	push	ax			;save byte count

	push	ax			;byte count
	push	_out_buf_adr		;buffer address
	push	output_handle		;zoofile
ifdef	UNBUF_IO
	call	_blockwrite
else
	call	_zoowrite
endif
	add	sp,6

	pop	cx			;recover byte count

	cmp	ax,cx

	jz	here2
	jmp	IO_err		;I/O error

here2:
	pop	di
	pop	si

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
flush		endp

lookup_code	proc	near
	push	ds			;Save seg reg
	mov	ds,hash_seg		;point to hash table

	;call	index			;convert code to address
	call_index			;macro for speed

	mov	di,0			;flag
	mov	bx,[si].first
	cmp	bx,-1			;Has this code been used?
	je	gc4			;equal means no
	inc	di			;set flag
gc2:	
	;call	index			;convert code to address
	call_index			;macro for speed

	cmp	[si].char,al		;is char the same?
	jne	gc3			;ne means no
	clc				;success
	mov	ax,bx			;put found code in ax
	pop	ds			;restore seg reg
	ret				;done
gc3:	
	mov	bx,[si].next
	cmp	bx,-1			;More left with this prefix?
	je	gc4			;equal means no
	jmp	gc2			;try again
gc4:	stc				;not found
	pop	ds			;restore seg reg
	ret				;done
lookup_code	endp

comment #
index	proc	near
	mov	si,bx			;si = bx * 5 (5 byte hash entries)
	shl	si,1			;si = bx * 2 * 2 + bx
	shl	si,1
	add	si,bx
	ret
index		endp
# end comment

check_ratio	proc	near
	push	ax

;	mov	dl,'*'			;'*' printed means checking ratio
;	call	sendout

	;Getting ready to check ratios.  If bitsout is over scale_limit,
	;then we scale down both bitsout and bytesin by a factor 
	;of 4.  This will avoid overflow.
	mov	cx,[bitsout]
	cmp	cx,scale_limit
	jb	scale_ok

	mov	cl,2
	shr	[bytesin],cl
	shr	[bitsout],cl

scale_ok:
	;Note MASM bug:  "mov ah,high [bytesin]" and
	;"mov al,low [bytesin]" don't work.
	mov	ah,byte ptr [bytesin]
	mov	dl,byte ptr [bytesin+1]

	sub	al,al
	sub	dh,dh			;dx:ax = 8 * bitsin = 256 * bytesin
	mov	cx,[bitsout]		;cx <- bitsout
	or	cx,cx			;Division by zero?
	jnz	candivide		;No -- go ahead divide
	mov	ax,0FFFFH		;yes -- assume max poss value	
	jmp	short divided
candivide:
	;Calculate cx as (bytesin * 256) / bitsout  = bitsin * 8 / bitsout
	div	cx			;ax <- rat <- dx:ax / cx
	shl	ax,1
	shl	ax,1			;rat <- 4 * bytes_in / bytes_out
divided:
	;Enter here with ax = rat = bitsin / bitsout.

;	call print_data			;print info for debugging

	;If rat > rat_thresh then ratio is good;  do not reset table
;	cmp	ax,rat_thresh
;	ja	ratdone

	;Compare rat against ratio
	mov	bx,ax			;save rat in bx
	cmp	ax,[ratio]		;cmp rat,ratio
	jb	ratless			;trouble if ratio is now smaller
	mov	ax,[ratio]
	call	mul7			;ax <- 7 * ratio
	add	ax,bx			;ax = 7 * ratio + rat
	shr	ax,1
	shr	ax,1
	shr	ax,1			;ax = (7 * ratio + rat) / 8
	mov	[ratio],ax		;ratio = (7 * ratio + rat) / 8
	jmp	short ratdone
ratless:				;ratio is going down, so...
	mov	[bytesin],0
	mov	[bitsout],0

;	mov	dl,'#'			;'#' printed means table reset
;	call	sendout

	mov	ax,clear		;Write a clear code
	call	write_code
	call	init_table		;Reinit table
ratdone:
	mov	[ratflag],0
	mov	[bit_interval],check_gap
	pop	ax
	ret
check_ratio	endp

comment #
sendout	proc	near			;char in dl send to console
	push	ax
	mov	ah,02
	int	21H
	pop	ax
	ret
sendout	endp
# end comment

mul7	proc	near			;multiply ax by 7
	push	dx
	mov	dx,7
	mul	dx			;dx:ax <- 7 * ax
	pop	dx
	ret
mul7	endp

comment #
mul3	proc	near			;multiply ax by 3
	push	dx
	mov	dx,3
	mul	dx			;dx:ax <- 3 * ax
	pop	dx
	ret
mul3	endp
# end comment

comment #
mul1_125 proc	near			;multiply ax by 1.125
	push	bx
	mov	bx,ax
	shr	bx,1
	shr	bx,1
	shr	bx,1			;bx = n / 8
	add	ax,bx			;ax <- n + n / 8
	pop	bx
	ret
mul1_125 endp
# end comment

comment #
print_data proc near
	;Debugging -- print bytesin, bitsout, rat, and ratio
	push	ax
	push	bx
	push	cx
	push	dx

	push	ax		;print rat
	call	_prtint
	add	sp,2

	push	[ratio]		;print ratio
	call	_prtint
	add	sp,2

	push	[bytesin]
	call	_prtint
	add	sp,2

	push	[bitsout]
	call	_prtint
	add	sp,2

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
print_data endp
# end comment

;doread reads cx characters and stores them in input buffer
;return value from zooread is returned in ax
doread	proc	near		;reads block
	push	bx
	push	cx
	push	dx
	push	si
	push	di

	push	cx			;byte count
	push	_in_buf_adr		;buffer address
	push	input_handle		;zoofile
ifdef	UNBUF_IO
	call	_read
else
	call	_zooread
endif
	add	sp,6

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	ret
doread	endp

_text	ends

end


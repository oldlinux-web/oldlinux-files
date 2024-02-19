title	Lempel-Ziv Decompressor
; $Source: /usr/home/dhesi/zoo/RCS/lzd.asm,v $
; $Id: lzd.asm,v 1.3 91/07/07 09:36:23 dhesi Exp $

;Derived from Tom Pfau's public domain assembly code.
;The contents of this file are hereby released to the public domain.
;                                   -- Rahul Dhesi 1988/08/24

UNBUF_IO	equ	1		;use unbuffered I/O

public	_lzd,memflag,docrc

	include	asmconst.ai
	include macros.ai

;Hash table entry
hash_rec	struc
next	dw	?			; prefix code
char	db	?			; suffix char
hash_rec	ends

extrn	_addbfcrc:near			;External C function for CRC

ifdef	UNBUF_IO
extrn	_read:near
extrn	_blockwrite:near
else
extrn	_zooread:near
extrn	_zoowrite:near
endif

;Declare segments
_text	segment byte public 'code'
_text	ends

dgroup	group	_data
	assume ds:dgroup,es:dgroup
_data	segment word public 'data'
extrn	_out_buf_adr:word		;address of C output buffer
extrn	_in_buf_adr:word		;address of C input buffer

memflag		db	0		;Memory allocated?  flag
save_bp		dw	?
save_sp		dw	?

input_handle	dw	?
output_handle	dw	?
hash_seg	dw	?
cur_code	dw	?
old_code	dw	?
in_code		dw	?
free_code	dw	first_free

;Note:  for re-entrancy, following 3 must be re-initialized each time
stack_count	dw	0
nbits		dw	9
max_code	dw	512

fin_char	db	?
k		db	?
masks		dw	1ffh,3ffh,7ffh,0fffh,1fffh

;Note:  for re-entrancy, following 2 must be re-initialized each time
bit_offset	dw	0
output_offset	dw	0
_data	ends

memory	segment para public 'memory'
hash	label	hash_rec
memory	ends

call_index macro
	mov	bp,bx			;bx = bx * 3 (3 byte entries)
	shl	bx,1			;bp = bx
	add	bx,bp			;bx = bx * 2 + bp
	endm

call_write_char macro
	local	wc$1
	mov	di,output_offset	;Get offset in buffer
	cmp	di,outbufsiz		;Full?
	jb	wc$1			;no
	call	write_char_partial
	sub	di,di			;so we add zero in next statement
wc$1:	add	di,[_out_buf_adr]	;di <- buffer address + di
	stosb				;Store char
	inc	output_offset		;Increment number of chars in buffer
	endm

add_code macro
	mov	bx,free_code		;Get new code
	;call	index			;convert to address
	call_index
	push	es			;point to hash table
	mov	es,hash_seg
	mov	al,k			;get suffix char
	mov	es:[bx].char,al		;save it
	mov	ax,old_code		;get prefix code
	mov	es:[bx].next,ax		;save it
	pop	es
	inc	free_code		;set next code
	endm

;Start coding
_text	segment
	assume	cs:_text,ds:dgroup,es:dgroup,ss:nothing

write_char_partial proc	near
	push	cx
	mov	cx,di			;byte count
	call	write_block
	pop	cx
	mov	output_offset,0		;Restore buffer pointer
	ret
write_char_partial endp

_lzd	proc	near

	push	bp			;Standard C entry code
	mov	bp,sp
	push	di
	push	si
	
	push	ds			;Save ds to be sure
	mov	[save_bp],bp		;And bp too!
	mov	bx,ds
	mov	es,bx

;Get two parameters, both integers, that are input file handle and
;output file handle
	mov	ax,[bp+4]
	mov	[input_handle],ax
	mov	ax,[bp+6]
	mov	[output_handle],ax

	call	decompress		;Compress file & get status in AX

	mov	bp,[save_bp]		;Restore bp
	pop	ds
	pop	si			;Standard C return code
	pop	di
	mov	sp,bp
	pop	bp
	ret
_lzd	endp

;Note:  Procedure decompress returns AX=0 for successful decompression and
;	AX=1 for I/O error and AX=2 for malloc failure.  
decompress	proc	near
	mov	[save_sp],sp		;Save SP in case of error return

;Initialize variables -- required for serial re-entrancy
	mov	[nbits],9
	mov	[max_code],512
	mov	[free_code],first_free
	mov	[stack_count],0
	mov	[bit_offset],0
	mov	[output_offset],0

	test	memflag,0ffH		;Memory allocated?
	jnz	gotmem			;If allocated, continue
	malloc	<((maxmax * 3) / 16 + 20)>	;allocate it
	jnc	here1
	jmp	MALLOC_err
here1:
	mov	hash_seg,ax		;Save segment address of mem block
	mov	memflag,0ffh		;Set flag to remind us later
gotmem:

	mov	ax,inbufsiz
	push	ax			;byte count
	push	_in_buf_adr		;buffer address
	push	input_handle		;zoofile
ifdef	UNBUF_IO
	call	_read
else
	call	_zooread
endif
	add	sp,6

	cmp	ax,-1
	jz	IO_err			;I/O error
here2:

l1:	call	read_code		;Get a code
	cmp	ax,eof			;End of file?
	jne	l2			;no
	cmp	output_offset,0		;Data in output buffer?
	je	OK_ret			;no
	mov	cx,[output_offset]	;byte count
	call	write_block		;write block of cx bytes
OK_ret:	
	xor	ax,ax			;Normal return -- decompressed
	ret				;done
IO_err:
	mov	ax,2			;I/O error return 
	mov	sp,[save_sp]		;Restore stack pointer
	ret	

MALLOC_err:
	mov	ax,1			;Malloc error return
	mov	sp,[save_sp]		;Restore stack pointer
	ret

l2:	cmp	ax,clear		;Clear code?
	jne	l7			;no
	call	init_tab		;Initialize table
	call	read_code		;Read next code
	mov	cur_code,ax		;Initialize variables
	mov	old_code,ax
	mov	k,al
	mov	fin_char,al
	mov	al,k
	;call	write_char		;Write character
	call_write_char
	jmp	l1			;Get next code
l7:	mov	cur_code,ax		;Save new code
	mov	in_code,ax
	mov	es,hash_seg		;Point to hash table
	cmp	ax,free_code		;Code in table? (k<w>k<w>k)
	jb	l11			;yes
	mov	ax,old_code		;get previous code
	mov	cur_code,ax		;make current
	mov	al,fin_char		;get old last char
	push	ax			;push it
	inc	stack_count

;old code -- two memory references
;l11:	
;	cmp	cur_code,255		;Code or character?
;	jbe	l15			;Char
;	mov	bx,cur_code		;Convert code to address
;new code -- 0 or 1 memory references
	mov	ax,cur_code
l11:
	;All paths in must have ax containing cur_code
	cmp	ax,255
	jbe	l15
	mov	bx,ax
;end code
	;call	index
	call_index
	mov	al,es:2[bx]		;Get suffix char
	push	ax			;push it
	inc	stack_count
	mov	ax,es:[bx]		;Get prefix code
	mov	cur_code,ax		;Save it
	jmp	l11			;Translate again
l15:	
;old code
;	push	ds			;Restore seg reg
;	pop	es
;new code
	mov	ax,ds			;faster than push/pop
	mov	es,ax
;end code
	mov	ax,cur_code		;Get code
	mov	fin_char,al		;Save as final, k
	mov	k,al
	push	ax			;Push it

;old code
;	inc	stack_count
;	mov	cx,stack_count		;Pop stack
;new code -- slightly faster because INC of memory is slow
	mov	cx,stack_count
	inc	cx
	mov	stack_count,cx
;end code
	jcxz	l18			;If anything there
l17:	pop	ax
	;call	write_char
	call_write_char
	loop	l17

;old code
;l18:	
;	mov	stack_count,cx		;Clear count on stack
;new code -- because stack_count is already zero on earlier "jcxz l18"
	mov	stack_count,cx
l18:
;end code

	;call	add_code		;Add new code to table
	add_code
	mov	ax,in_code		;Save input code
	mov	old_code,ax
	mov	bx,free_code		;Hit table limit?
	cmp	bx,max_code
	jb	l23			;Less means no
	cmp	nbits,maxbits		;Still within maxbits?
	je	l23			;no (next code should be clear)
	inc	nbits			;Increase code size
	shl	max_code,1		;Double max code
l23:	jmp	l1			;Get next code
decompress	endp	

read_code	proc	near

;old code
;	mov	ax,bit_offset		;Get bit offset
;	add	ax,nbits		;Adjust by code size
;	xchg	bit_offset,ax		;Swap
;	mov	dx,ax			;dx <- ax
;new code
	mov	ax,bit_offset
	mov	dx,ax			;dx <- bit_offset
	add	ax,nbits
	mov	bit_offset,ax
	mov	ax,dx
;end code

	shr	ax,1
	shr	ax,1
	shr	ax,1			;ax <- ax div 8
	and	dx,07			;dx <- ax mod 8
	cmp	ax,inbufsiz-3		;Approaching end of buffer?
	jb	rd0			;no
	push	dx			;Save offset in byte
	add	dx,nbits		;Calculate new bit offset
	mov	bit_offset,dx
	mov	cx,inbufsiz
	mov	bp,ax			;save byte offset
	sub	cx,ax			;Calculate bytes left
	add	ax,_in_buf_adr
	mov	si,ax
	mov	di,_in_buf_adr
rep	movsb				;Move last chars down

	push	bp			;byte count
	push	di			;buffer address
	push	input_handle		;zoofile
ifdef	UNBUF_IO
	call _read
else
	call	_zooread
endif
	add	sp,6

	cmp	ax,-1
	jnz	here4
	jmp	IO_err			;I/O error

here4:
	xor	ax,ax			;Clear ax
	pop	dx			;Restore offset in byte
rd0:	
	add	ax,_in_buf_adr
	mov	si,ax
	lodsw				;Get word
	mov	bx,ax			;Save in AX
	lodsb				;Next byte
	mov	cx,dx			;Offset in byte
	jcxz	rd2			;If zero, skip shifts
rd1:	shr	al,1			;Put code in low (code size) bits of BX
	rcr	bx,1
	loop	rd1
rd2:	mov	ax,bx			;put code in ax
	mov	bx,nbits		;mask off unwanted bits
	sub	bx,9
	shl	bx,1
	and	ax,masks[bx]
	ret
read_code	endp

init_tab	proc	near
	mov	nbits,9			;Initialize variables
	mov	max_code,512
	mov	free_code,first_free
	ret
init_tab	endp

comment #
index		proc	near
	mov	bp,bx			;bx = bx * 3 (3 byte entries)
	shl	bx,1			;bp = bx
	add	bx,bp			;bx = bx * 2 + bp
	ret
index		endp
#end comment

docrc	proc	near
;On entry, ax=char count, dx=buffer address.
;Do crc on character count, in buffer.
;****** Update CRC value -- call external C program
	;External program is:	addbfcrc(buffer, count)
	;			char *buffer;
	;			int count;

	push	ax		;SAVE AX
	push	bx		;SAVE BX
	push	cx
	push	dx

	push	ax		;param 2: char count
	push	dx		;param 1: buffer address
	call	_addbfcrc
	add	sp,4		;Restore 2 params from stack

	pop	dx
	pop	cx
	pop	bx		;RESTORE BX
	pop	ax		;RESTORE AX
	ret
docrc	endp

write_block proc near
;Input:  CX=byte count to write
	push	ax
	push	bx
	push	cx
	push	dx
	push	si			;may not be necessary to save si & di
	push	di

	push	cx			;save count

	push	cx			;count
	push	_out_buf_adr		;buffer
	push	output_handle		;zoofile
ifdef	UNBUF_IO
	call	_blockwrite
else
	call	_zoowrite
endif
	add	sp,6

	pop	cx			;restore count

	;ax = actual number of bytes written
	cmp	ax,cx			;all bytes written?
	je	written			;if yes, OK
	jmp	IO_err
written:
	mov	dx,_out_buf_adr
	call	docrc			;do crc on ax bytes in buffer dx
	mov	output_offset,0		;restore buffer ptr to zero

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
write_block endp

_text	ends

	end

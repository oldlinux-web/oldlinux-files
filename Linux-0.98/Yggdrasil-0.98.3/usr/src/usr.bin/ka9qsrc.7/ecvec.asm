	include	pmacros.h

	inthandler	ec, 0
	inthandler	ec, 1
	inthandler	ec, 2

; fast buffer I/O routines -- used by 3-COM Ethernet controller

; outbuf - put a buffer to an output port
	procdef outbuf,<<oport,word>,<obuf,ptr>,<ocnt,word>>
	pushf
	push	si
	pushds
	mov	dx,oport
	mov	cx,ocnt
	ldptr	si,obuf,ds	; ds:si = obuf
	cld

; If buffer doesn't begin on a word boundary, send the first byte
	test	si,1	; (buf & 1) ?
	jz	obufeven ; no
	lodsb		; al = *si++;
	out	dx,al	; out(dx,al);
	dec	cx	; cx--;
	mov	ocnt,cx	; save for later test
obufeven:
	shr	cx,1	; cx = cnt >> 1; (convert to word count)
; Do the bulk of the buffer, a word at a time
	jcxz	onobuf	; if(cx != 0){
xb:	lodsw		; do { ax = *si++; (si is word pointer)
	out	dx,al	; out(dx,lowbyte(ax));
	mov	al,ah
	out	dx,al	; out(dx,hibyte(ax));
	loop	xb	; } while(--cx != 0); }
; now check for odd trailing byte
onobuf:	mov	cx,ocnt
	test	cx,1
	jz	ocnteven
	lodsb		; al = *si++;
	out	dx,al
ocnteven:
	popds
	pop	si
	popf
	pret
	pend	outbuf

; inbuf - get a buffer from an input port
	procdef inbuf,<<iport,word>,<ibuf,ptr>,<icnt,word>>
	pushf
	push	di
	pushes
	mov	dx,iport
	mov	cx,icnt
	ldptr	di,ibuf,es	; es:di = ibuf (es already set in small model)
	cld

; If buffer doesn't begin on a word boundary, get the first byte
	test	di,1	; if(buf & 1){
	jz	ibufeven ;
	in	al,dx	; al = in(dx);
	stosb		; *di++ = al
	dec	cx	; cx--;
	mov	icnt,cx	; icnt = cx; } save for later test
ibufeven:
	shr	cx,1	; cx = cnt >> 1; (convert to word count)
; Do the bulk of the buffer, a word at a time
	jcxz	inobuf	; if(cx != 0){
rb:	in	al,dx	; do { al = in(dx);
	mov	ah,al
	in	al,dx	; ah = in(dx);
	xchg	al,ah
	stosw		; *si++ = ax; (di is word pointer)
	loop	rb	; } while(--cx != 0);
; now check for odd trailing byte
inobuf:	mov	cx,icnt
	test	cx,1
	jz	icnteven
	in	al,dx
	stosb		; *di++ = al
icnteven:
	popes
	pop	di
	popf
	pret
	pend	inbuf

	end

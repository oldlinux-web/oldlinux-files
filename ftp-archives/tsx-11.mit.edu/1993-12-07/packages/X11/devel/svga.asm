;  svga.asm   small model C routines for VGA graphics 12/28/88 hk

_bss    segment word public 'bss'
_bss    ends
_data   segment word public 'data'
_data   ends
dgroup  group   _bss,_data

_text   segment byte public 'code'
        assume  cs: _text, ds: dgroup, ss:dgroup, es:dgroup
_text   ends

_data   segment word public 'data'
_hlist  db      320 dup(?)
_data   ends
_text   segment byte public 'code'
 
ARG0    equ     4               ; small model parameter stack offsets
ARG1    equ     6
ARG2    equ     8
ARG3    equ     10
ARG4    equ     12
ARG5    equ     14
ARG6    equ     16

        public  _paint          ; paint(x,y,color)
        public  _hl_init        ; prepare for hidden lines : line(,,,,0xffff)
        public  _getbit         ; getbit(x,y,xsize,ysize,*buffer)
        public  _putbit         ; putbit(x,y,*buffer)
        public  _putlev         ; putlev(x,y,level,*buffer)

        public  _mbutton        ; mbutton(&left,&right)   *int #presses
        public  _mmove          ; mmove(&dx,&dy)      *int relative moves
        public  _allkey         ; return ascii & scode, or 0
        public  _key            ; return ascii, or 0
        public  _video          ; video(n) 
        public  _ct             ; ct(n,b) clear to color n, edge color b
        public  _vdot           ; (x,y,color)  for 320x200 color mode
        public  _vrdot          ; returns color at (x,y)
        public  _line           ; line(x1,y1,x2,y2,color)
	public  _vcolor
	public  _vpalette
        public  _cursor         ; cursor(line,tab)
        public  _gpoke          ; gpoke(seg,off,val)
        public  _gpeek          ; gpeek(seg,off)
        public  _rnd
        public  _hlist

ARG0    equ     4               ; parameter stack offsets
ARG1    equ     6
ARG2    equ     8 
ARG3    equ     10
ARG4    equ     12
ARG5    equ     14
XMAX    equ     319
YMAX    equ     199

screen  dw      0a000h
delx    dw      ?               ; variables for line()
dely    dw      ?
px      dw      ?
py      dw      ?
tot     dw      ?
tadd    dw      ?
gcolor  dw      ?
color   db      ?
vmode   dw      ?               ; current video mode number

setstack macro
        push    bp
        mov     bp,sp
        push    si
        push    di
        endm
getout  macro
        pop     di
        pop     si
        pop     bp
        ret
        endm

coljump label   word            ; for 320x200 256 color mode
        x = 0
        rept    200
        dw      x
        x=x+320
        endm
stk     dw      1000 dup(?)

_hl_init proc   near
        mov     bx,0
        mov     cx,160
        mov     ax,0ffffh
hll:    mov     word ptr _hlist[bx],ax
        add     bx,2
        loop    hll
        ret
_hl_init endp

_vdot   proc    near
        setstack
        push    es
        mov     es,screen
        mov     bx,[bp]+ARG1    ; y line

        cmp     bx,0
        jb      vvv
        cmp     bx,YMAX
        ja      vvv

        shl     bx,1
        mov     bx,coljump[bx]  ; y offset

        mov     ax,[bp]+ARG0    ; x offset
        and     ah,127          ; strip 'negative' number
        cmp     ax,XMAX
        ja      vvv
        add     bx,ax

        mov     ax,[bp]+ARG2    ; al = color
        mov     es:[bx],al
vvv:    pop     es
        getout
_vdot   endp

_vrdot  proc    near
        setstack
        push    es
        mov     es,screen
        mov     bx,[bp]+ARG1    ; y line
        shl     bx,1
        mov     bx,coljump[bx]  ; y offset
        add     bx,[bp]+ARG0    ; x offset
        mov     ax,[bp]+ARG2    ; al = color
        mov     al,es:[bx]
        mov     ah,0
        pop     es
        getout
_vrdot  endp


_gpoke  proc    near
        setstack
        push    es
        mov     es,[bp]+ARG0
        mov     bx,[bp]+ARG1
        mov     ax,[bp]+ARG2
        mov     es:[bx],al
        pop     es
        getout
_gpoke  endp

_gpeek  proc    near
        setstack
        push    es
        mov     es,[bp]+ARG0
        mov     bx,[bp]+ARG1
        mov     al,es:[bx]
        mov     ah,0          
        pop     es
        getout
_gpeek  endp

_video  proc    near
        setstack
        mov     ax,[bp]+ARG0    ; al= video mode number
        mov     vmode,ax
        int     10h             ; call BIOS to change video mode
        getout
_video  endp

MARK    equ     0ffffh

SAVE    macro   value           ; save value to 'stack'
        mov     word ptr stk[si],value
        inc     si
        inc     si
        endm
LOAD    macro   value           ; set bx to 'stack value'
        dec     si
        dec     si
        mov     bx,stk[si]
        endm

_paint  proc    near
        setstack
        push    es
        mov     cx,[bp]+8       ; cl = color
        mov     es,screen
        xor     si,si           ; init stack index
        SAVE    MARK            ; save 'end of list' marker
        mov     bx,[bp+6]
        shl     bx,1
        mov     ax,coljump[bx]  ; calc initial offset of seed
        add     ax,[bp+4]       ;x
        SAVE    ax              ; & save on stack

ploop:  LOAD                    ; get another line seed address
        cmp     bx,MARK         ; did whole picture
        jne     pl2
        pop     es
        getout
pl2:    sub     dx,dx           ; up=dn=0    dh=up, dl=dn

pline:  mov     al,es:[bx]      ; tried to paint a filled area?
        or      al,al
        jne     ploop           ; yes. don't continue here
pdec:   dec     bx              ; back up to left edge
        mov     al,es:[bx]
        or      al,al
        je      pdec
        inc     bx              ; set BX to 1st open pixel on this line

chk_up: mov     di,bx           ; di = adr of line above
        sub     di,320
        or      dh,dh
        jne     up_on
up_off: cmp     byte ptr es:[di],0  ; if(!up && !pix) { save(pix) up=1 }
        jne     chk_dn
        SAVE    di
        inc     dh
        jmp     short chk_dn
up_on:  cmp     byte ptr es:[di],0       ; if(up && pix) up=0
        je      chk_dn
        xor     dh,dh
chk_dn: mov     di,bx           ; di = adr of line above
        add     di,320
        or      dl,dl
        jne     dn_on
dn_off: cmp     byte ptr es:[di],0       ; if(!dn && !pix) { save(pix) dn=1 }
        jne     next
        SAVE    di
        inc     dl
        jmp     short next
dn_on:  cmp     byte ptr es:[di],0       ; if(dn && pix) dn=0
        je      next
        xor     dl,dl
next:   mov     es:[bx],cl      ; set pixel
        inc     bx
        jmp     pline
_paint  endp


_ct proc    near
	push	es  
	push	di
        mov     es,screen
	mov	di,0
	mov	ax,0 
	cld
        mov cx,32000
        rep stosw
        pop di
	pop	es
	ret
_ct endp

; -------------------------------------------------------------------------
;  ddot : called by line(),  it places a dot at  x=di, y=dx, gcolor=mode
;         es = VGA segment
; save ax,cx,dx,di

_ddot   proc    near            ; dot(di,dx,mode) uses bx
        cmp     di,0            ; check out of limits
        jb      baddot          ; if dot is off screen, getout
        cmp     di,XMAX
        ja      baddot
        cmp     dx,0
        jb      baddot
        cmp     dx,YMAX
        jna     okdot
baddot: ret

okdot:  push	ax
        mov     bx,dx
        shl     bx,1
        mov     bx,coljump[bx]  ; y offset
        add     bx,di           ; x offset
        mov     ax,gcolor
        cmp     ah,255          ; hidden line?
        je      hidden
        cmp     ah,0            ; if(color>255) then use XOR
        je      okcol
        mov     al,es:[bx]      ; not     byte ptr es:[bx]
        not     al
okcol:  mov     es:[bx],al
okc2:   pop     ax
        ret
hidden: cmp     dl,_hlist[di]    ; Y must be lower to be visible
        jae     okc2
        mov     _hlist[di],dl    ; visible : update hidden list
        jmp     okcol
_ddot   endp

;-------------------------------------------------------------------
; line(x1,y1,x2,y2,color)
;-------------------------------------------------------------------

_line   proc    near           
        setstack              
        mov     ax,[bp]+ARG4    ; gcolor = line draw mode
        mov     gcolor,ax
        mov     di,[bp]+ARG0    ;  di=xcoord  x1
        mov     dx,[bp]+ARG1    ;  dl=ycoord  y1
        mov     ax,[bp]+ARG2    ;  ax = si = abs(dx)
        mov     bx,[bp]+ARG3
        mov     px,1            ; assume positive movement
        mov     py,1            ; px,py = amounts to move coords.
        sub     ax,di
        jge     d1pos
        neg     ax
        neg     px              ;  negative x movement
d1pos:  mov     si,ax         ; ax = si = abs(x2-x1), px = 1 or -1
        sub     bx,dx
        jge     d2pos
        neg     bx              ; bx = dely = abs(dy), py = 1 or -1
        neg     py
d2pos:  mov     dely,bx     
        push    es              ; get screen's segment
        mov     es,screen
        cmp     ax,bx           ; if deltay>deltax, then use yloop version
        jl      ybig
        cmp     ax,0            ; error trap x1==x2, y1==y2
        jne     xbig
        call    _ddot           ; if just one point, then draw it & exit
        jmp     ldone

xbig:   mov     tadd,bx         ; xline   tadd=dely
        mov     cx,ax           ;         count=si    ax=tot
xloop:  call    _ddot           ; draw current dot
        add     di,px           ; adjust xcoord
        add     ax,tadd         ; add deltay to overflow register
        cmp     ax,si         ; see of overflow > deltax
        jle     xgoon           ; no.  continue
        sub     ax,si         ; yes. reset overflow register
        add     dx,py           ;      & adjust ycoord
xgoon:  loop    xloop           ; loop for all xcoords
ldone:  pop     es
        getout       

ybig:   mov     tadd,ax         ;  yline   tadd=si
        mov     ax,bx           ;          tot=dely   (overflow register)
        mov     cx,bx           ;          count=dely     ax=tot
yloop:  call    _ddot           ; draw current dot
        add     dx,py           ; adjust ycoord
        add     ax,tadd         ; add deltax to overflow
        cmp     ax,dely         ; see if overflow>deltay
        jle     ygoon           ; no.  continue
        sub     ax,dely         ; yes. reset overflow counter
        add     di,px           ;      & adjust xcoord
ygoon:  loop    yloop
        jmp	ldone
_line   endp

_vcolor proc	near		; vcolor(#colors,*table (3bytes each) )
	push	bp
	mov	bp,sp
	push	es
	push	ds
	pop	es
	mov	cx,[bp]+4	; #regs
	mov	dx,[bp]+6	; table offset
	mov	bx,1		; starting reg#
        push    dx

        mov     dx,3dah
        mov     ah,10h

vest:   in      al,dx
        test    al,8            ; wait for NOT in Vsync
        jnz     vest
vest2:  in      al,dx
        test    al,8            ; wait for in Vsync
        jz      vest2

        pop     dx
        mov     al,12h          ; ah=10 al=12
	int	10h
	pop	es
	pop	bp
	ret				
_vcolor	endp    

_vpalette proc	near		; vpalette(*17 bytes of colors)
	push	bp
	mov	bp,sp
	push	es
	push	ds
	pop	es
	mov	dx,[bp]+4	; offset
	mov	ax,1002h 	; ah=10 al=2
	int	10h
	pop	es
	pop	bp
	ret				    
_vpalette endp

_allkey proc    near      
        mov     ah,1
        int     16h
        jz      ncode           ; is there a keypress ?
        mov     ah,0
        int     16h             ;   yes : get it
        ret
ncode:  mov     ax,0
        ret                     ; if =0 then empty
_allkey endp

_key    proc    near        
        call    _allkey
        mov     ah,0
        ret
_key    endp

_cursor proc    near            
        setstack
        mov     cx,[bp]+ARG0       ; cl=row
        mov     dx,[bp]+ARG1       ; dl=col
        mov     dh,cl
        mov     ah,2
        mov     bh,0            ; page number
        int     10h
        getout
_cursor endp	   

rr      dw      ?

_rnd    proc    near            ; rnd(max)
        setstack                ; rr+=inp(64)+(rr>>3);  rt=rr&255;
        in      ax,64
        add     rr,ax
        mov     ax,rr
        mov     cl,3
        shr     ax,cl
        add     ax,rr
        xor     ah,ah
        mov     bx,[bp]+ARG0    ; bl = max
        mov     dx,127
rrl:    cmp     ax,bx
        jb      rdone
        and     ax,dx
        shr     dx,1
        mov     cx,ax
        in      al,64
        cmp     al,127
        jle     rr2
        xor     cx,255
        mov     ch,0
rr2:    mov     ax,cx
        jmp     rrl
rdone:  getout
_rnd    endp

_getbit proc    near    ; x=ARG0, y=ARG1, xs=ARG2, ys=ARG3, buf=ARG4
        setstack
        push    es
        mov     ax,0a000h       ; VGA segment
        mov     es,ax
        mov     di,[bp]+ARG4    ; buffer pointer
        mov     ax,[bp]+ARG2
        mov     [di],al
        mov     ch,al
        inc     di              ; get sizes first
        mov     ax,[bp]+ARG3
        mov     [di],al
        inc     di
        mov     ah,ch           ; ah=xsize, al=ysize
        mov     ch,0
        mov     si,[bp]+ARG1    ; y offset
        shl     si,1
        mov     si,coljump[si]
        add     si,[bp]+ARG0    ; di = screen offset
        mov     bx,si           ; copy of screen offset of left edge
        push    ds
        push    ds
        push    es
        pop     ds
        pop     es
gb1:    mov     si,bx
        mov     cl,ah           ; cx = xsize
        rep     movsb           ; es:[di++] = ds:[si++]  (cx # times)
        add     bx,320          ; move down 1 line
        dec     al
        jne     gb1
        pop     ds
        pop     es
        getout
_getbit endp

_putbit proc    near    ; x=ARG0, y=ARG1, buf=ARG2
        setstack
        push    es
        mov     ax,0a000h       ; VGA segment
        mov     es,ax
        mov     si,[bp]+ARG2    ; buffer pointer
        mov     ah,[si]
        inc     si              ; get sizes first
        mov     al,[si]         ; ah=xsize  al=ysize
        inc     si
        mov     ch,0
        mov     di,[bp]+ARG1    ; y offset
        shl     di,1
        mov     di,coljump[di]
        add     di,[bp]+ARG0    ; di = screen offset
        mov     bx,di           ; copy of screen offset of left edge
 ;      call    vsync

gb3:    mov     di,bx
        mov     cl,ah           ; cx = xsize
        rep     movsb           ; es:[di++] = ds:[si++]  (cx # times)
        add     bx,320          ; move down 1 line
        dec     al
        jne     gb3
        pop     es
        getout
_putbit endp

_putlev proc    near    ; x=ARG0, y=ARG1, level# = ARG2, buf=ARG3
        setstack
        push    es
        mov     ax,0a000h       ; VGA segment
        mov     es,ax
        mov     bx,[bp]+ARG3    ; buffer pointer
        mov     dl,[bx]
        inc     bx              ; get sizes first
        mov     cl,[bx]
        inc     bx
        mov     ch,0
        mov     di,[bp]+ARG1    ; y offset
        shl     di,1
        mov     di,coljump[di]
        add     di,[bp]+ARG0    ; di = screen offset
        mov     dh,dl           ; dh = copy of xsize
        mov     si,di           ; copy of screen offset of left edge
        mov     ax,[bp]+ARG2    ; al = level#
;       call    vsync
gb6:    mov     di,si
        mov     dl,dh
gb7:    mov     ah,[bx]
        cmp     ah,0
        je      gb8             ; always ignore 0
        cmp     es:[di],al      ; if bkgnd>level, then go behind (don't draw)
        ja      gb8
        mov     es:[di],ah
gb8:    inc     bx
        inc     di
        dec     dl
        jne     gb7
        add     si,320           ; move down 1 line
        loop    gb6
        pop     es
        getout
_putlev endp

vsync   proc    near         
        push    dx
        push    ax
        mov     dx,3dah      
vtest:  in      al,dx
        test    al,8            ; if vertical sync, go right ahead
        jz      vtest
        pop     ax
        pop     dx
        ret
vsync   endp

_mbutton proc   near
	setstack
	mov	ax,5		; get button presses
	mov	bx,0		; check button 0
	int	33h
	mov	si,[bp]+ARG0
	mov	[si],bx     	; stash #releases
	mov	ax,5
	mov	bx,1		; check button 1
	int	33h	
	mov	si,[bp]+ARG1
	mov	[si],bx    	; stash #releases
	getout
_mbutton endp

_mmove  proc	near	      	; returns #mickeys of movement for x & y
	setstack
	mov	ax,11		; check motion counters
	int	33h
	mov	bx,[bp]+ARG0
	mov	[bx],cx    	; dx
	mov	bx,[bp]+ARG1
	mov	[bx],dx    	; dy
	getout
_mmove  endp


_text   ends
        end



;Assembly language macros for C compiler independence.  Assemble with /mx
;to ensure mixed case symbols.

  ifndef MSC
MSC		equ	0
  endif
  ifndef TURBO
TURBO		equ	0
  endif
  ifndef AZTEC
AZTEC		equ	0
  endif
  if MSC + TURBO + AZTEC NE 1
    .err One and only one of MSC, TURBO, and AZTEC should be defined.
  endif


;Copyright, 1987, 1988, Russell Nelson
;Permission to use, copy, modify and distribute this software for any purpose
;and without fee is hereby granted, provided that the above copyright notice
;appears in all copies and that both that copyright notice and this permission
;appear in supporting documentation.  Russell Nelson MAKES NO REPRESENTATIONS
;ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.  IT IS PROVIDED
;"AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.

	.xlist

;Note that symbols defined by these macros (pubproc, extproc, pubvar, extvar)
;should be referred by users of this package as 's@' where 's' is the defined
;symbol.  This hides the compiler's underscore convention.  For example,
;	include pmacros.h
;	extproc	foo
;	pubproc	bar
;	call	foo@
;	pend	bar

;begin a public procedure and label it 'a'.  If 'd' is absent, then default
;to what LARGECODE says, otherwise 'd' must be 'far' or 'near'.
;Terminated by pend.
pubproc	macro	a, d
  if AZTEC
	assume	cs:_text, ds:dgroup
  endif
  if TURBO
	assume	cs:_text, ds:dgroup
  endif
  if MSC
	assume	cs:_TEXT, ds:DGROUP
  endif
	cseg
  if AZTEC
a&@	equ	a&_
	public	a&_
  else
a&@	equ	_&a
	public	_&a
  endif
  ifb	<d>
  ifdef LARGECODE
  if AZTEC
a&_	proc	far
  else
_&a	proc	far
  endif
first_param	=	6
  else
    if AZTEC
a&_	proc	near
    else
_&a	proc	near
    endif
first_param	=	4
  endif
  else	;ifb
    if AZTEC
a&_	proc	d
    else
_&a	proc	d
    endif
  ifidn	<d>,<far>
first_param	=	6
  else
first_param	=	4
  endif
  endif
	endm

;used internally to expand first_param and increment it.
_param	macro	a, o, t, n
a	equ	t ptr o[bp]
first_param = first_param + n
	endm


;declare a parameter.  Use byte, word, ptr, or fptr (function pointer).
param	macro	a, t
  ifidn <t>,<byte>
	_param	a, %first_param, byte, 2
  endif
  ifidn <t>,<word>
	_param	a, %first_param, word, 2
  endif
  ifidn <t>,<ptr>
  ifdef LARGEDATA
	_param	a, %first_param, dword, 4
  else
	_param	a, %first_param, word, 2
  endif
  endif
  ifidn <t>,<fptr>
  ifdef LARGECODE
	_param	a, %first_param, dword, 4
  else
	_param	a, %first_param, word, 2
  endif
  endif
	endm

;load a data pointer.
ldptr	macro	off_reg,var,seg_reg
  ifdef LARGEDATA
	l&seg_reg	off_reg,var
  else
	mov	off_reg,var
  endif
	endm


;define a public procedure with parameters, like so:
;procdef	foo, <<bar, word>, <baz, ptr>>
;Terminated by pret, pend
procdef	macro	n, a
	pubproc	n
	irp	one_param, <a>
	param	one_param
	endm
	push	bp
	mov	bp,sp
	endm

;return from a procedure with parameters.  Pairs with procdef
pret	macro
	pop	bp
	ret
	endm

;end a public procedure.  Pairs with pubproc
pend	macro	a
  if AZTEC
a&_	endp
  else
_&a	endp
  endif
	csegend
	endm

;define an external procedure called 'a'.  If 'd' is absent, then default
;to what LARGECODE says, otherwise 'd' must be 'far' or 'near'.

extproc	macro	a, d
  ifb	<d>
  ifdef LARGECODE
    if AZTEC
	extrn	a&_: far
    else
	extrn	_&a: far
    endif
  else
    if AZTEC
	extrn	a&_: near
    else
	extrn	_&a: near
    endif
  endif
  else	;ifb
    if AZTEC
	extrn	a&_: d
    else
	extrn	_&a: d
    endif
  endif
  if AZTEC
a&@	equ	a&_
  else
a&@	equ	_&a
  endif
	endm

;open the code segment
cseg	macro
  if AZTEC
codeseg	segment byte public 'code'
	assume	cs:code, ds:dataseg
  endif
  if TURBO
_TEXT	segment byte public 'CODE'
dgroup	group	_DATA,_BSS
	assume	cs:_TEXT, ds:dgroup
  endif
  if MSC
_TEXT	SEGMENT BYTE PUBLIC 'CODE'
DGROUP	GROUP	CONST,	_BSS,	_DATA
	assume	cs:_TEXT, ds:DGROUP
  endif
	endm

;close the code segment
csegend	macro
  if AZTEC
codeseg	ends
  endif
  if TURBO or MSC
_TEXT	ends
  endif
	endm

;assume that the ds is the code segment
dscode	macro
  if AZTEC
	assume	ds:codeseg
  endif
  if TURBO or MSC
	assume	ds:_TEXT
  endif
	endm


;Return the data segment
getds	macro
  if AZTEC
	mov	ax,dataseg
  endif
  if TURBO
	mov	ax,dgroup
  endif
  if MSC
	mov	ax,DGROUP
  endif
	endm


;open the data segment
dseg	macro
  if AZTEC
dataseg	segment word public 'data'
  endif
  if TURBO or MSC
_DATA	segment word public 'DATA'
  endif
	endm

;close the data segment
dsegend	macro
  if AZTEC
dataseg	ends
  endif
  if TURBO or MSC
_DATA	ends
  endif
	endm

;open the uninitialized data segment
bseg	macro
  if AZTEC
bssseg	segment word public 'data'
  endif
  if TURBO
_BSS	segment word public 'DATA'
  endif
  if MSC
_BSS	segment word public 'BSS'
  endif
	endm

;close the uninitialized data segment
bsegend	macro
  if AZTEC
bssseg	ends
  endif
  if TURBO or MSC
_BSS	ends
  endif
	endm

;define a public variable
;use it like so: pubvar a,<dw 0>
pubvar	macro	n,d
  if AZTEC
	public	n
n&@	equ	n
n	d
  endif
  if TURBO or MSC
	public	_&n
n&@	equ	_&n
_&n	d
  endif
	endm

;define an external variable
;use it like so: extvar a,word
; or extvar a,byte
extvar	macro	n,d
  if AZTEC
n&@	equ	n&_
	extrn	n&_: d
  endif
  if TURBO or MSC
n&@	equ	_&n
	extrn	_&n: d
  endif
	endm

; Conditional DS save/restore macros
pushds	macro
  ifdef LARGEDATA
	push	ds
  endif
	endm

popds	macro
  ifdef LARGEDATA
	pop	ds
  endif
	endm

; Conditional ES save/restore macros
pushes	macro
  ifdef LARGEDATA
	push	es
  endif
	endm

popes	macro
  ifdef LARGEDATA
	pop	es
  endif
	endm

;define the segments so that they exist.
dseg
dsegend
bseg
bsegend
cseg
csegend

;define an interrupt handler.
inthandler	macro	xxxname, xxxnum

	dseg
	extvar	sssave,word
	extvar	spsave,word
	extvar	intstk,byte
	dsegend

	extproc	doret
	extproc	xxxname&int

	pubproc	xxxname&xxxnum&vec

	push	ax		; save ax first.
	push	ds		; save on user stack
	getds
	mov	ds,ax

	mov	sssave@,ss	; stash user stack context
	mov	spsave@,sp

	mov	ss,ax		; set up interrupt stack
	lea	sp,intstk@

	push	bx		; save user regs on interrupt stack
	push	cx
	push	dx
	push	bp
	push	si
	push	di
	push	es

	push	ds
	pop	es

	mov	ax,xxxnum	; arg for service routine
	push	ax
	call	xxxname&int@
	pop	ax
	jmp	doret@
	pend	xxxname&xxxnum&vec

	endm

	.list

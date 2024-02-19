;	EXECPR: execute a program directly using dos exec call.
;	
;	calling sequence = execpr(prog, pblock);
;		char *prog = 
;			where progname is the full file spec with extension
;		char *pblock =
;			ptr to the parameter block for the exec call
;	RETURNS:
;			-n	DOS error number
;			 0	successfull call
;			+n	child return code
;
;	THIS IS A FAR CALL/LARGE DATA VERSION

args	equ 6	; offset of arguments -- large model

_TEXT	segment	byte public 'CODE'
;DGROUP	group	_CODE
	assume	cs:_TEXT

;	declare procedure names

	public	_execpr

_execpr	proc	far

;  do C setup

	push	bp
	mov	bp,sp
	push	ds

;	set up the pointer to the asciiz string with program/path names

	mov ax, [bp+args+2]
	mov ds, ax
	mov dx, [bp+args+0]

;	set up the pointer to the parameter block

	mov ax, [bp+args+6]
	mov es, ax
	mov bx, [bp+args+4]

;	set the function number

	mov ah, 4bh		;execute a program [DOS 4Bh call]

;	set up the function value

	mov al, 0		;load and execute program

; save all registers in our stack area
	push bx
	push cx
	push dx
	push si
	push di
	push ds
	push es
	push bp

	mov cs:ss_save,ss		; save ss
	mov cs:sp_save,sp		; save sp
	int 21h				; go exec program
	mov ss, cs:ss_save		; restore the stack seg/pointer
	mov sp, cs:sp_save
	jc failed			; jump if dos error occurs

; exec successful - get child's return code

	mov ah, 4dh			; Get return code of a Subprocess
	int 21h
	jmp return			; return code is now in AX

;	exec failed. Negate and pass through the AX return code

failed:
	neg ax

; now reset all registers and return

return:

; restore all registers from our stack
	pop bp
	pop es
	pop ds
	pop di
	pop si
	pop dx
	pop cx
	pop bx

	pop ds
	pop bp				; restore for C
	ret

_execpr	endp

;
; data area for exec prog
ss_save		dw	?
sp_save		dw	?

_TEXT	ends
	end

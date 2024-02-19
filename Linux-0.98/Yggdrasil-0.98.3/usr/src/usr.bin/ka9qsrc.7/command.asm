	include	pmacros.h
verify	equ	0

dseg
  if verify
verify_copy	db	1,2,3,4,5,6,7,8,9
  endif
	db	4096 dup(?)
stack	label	byte
  if verify
verify_original	db	1,2,3,4,5,6,7,8,9
verify_len	equ	$-verify_original
verify_err	db	'Stack overflow','$'
  endif
dsegend

cseg

intds@	dw	dgroup

segoff	struc
offs	dw	?
segm	dw	?
segoff	ends

their_8h	dd ?
their_10h	dd ?
their_13h	dd ?
their_1Bh	dd ?
their_21h	dd ?
their_23h	dd ?
their_24h	dd ?
their_28h	dd ?

dos_segment	dw ?			;segment of internal DOS flags
indos_offset	dw ?			;offset of INDOS flag
errflag_offset	dw ?			;offset of critical error flag
program_status	db 0			;popup status
flag_10h	db 0			;status of interrupt 10h
flag_13h	db 0			;status of interrupt 13h
zflag		db ?			;save and restore critical error.
dos_version	db ?			;dos major version.
main_countdown	db 20
ss_register	dw ?			;SS register storage
sp_register	dw ?			;SP register storage
my_psp		dw ?			;our PSP.
their_psp		dw ?			;PSP segment storage

tick_counter	dw ?
indos_counter	dw ?
errflag_counter	dw ?
status_counter	dw ?
bp_counter	dw ?
main_counter	dw ?


their_dta	dd	?
;
;------------------------------------------------------------------------------
;Interrupt 8 handling routine.
;------------------------------------------------------------------------------
timer	proc near
	pushf				;call BIOS routine
	call their_8h
	inc tick_counter
	cmp program_status,0		;are we already running?
	jne timer_status		;yes, then suspend ticking.
	cmp flag_10h,0			;video flag set?
	jne timer_exit			;yes, then exit
	cmp flag_13h,0			;disk flag set?
	jne timer_exit			;yes, then exit
	push es				;save ES and DI
	push di
	mov es,dos_segment		;check INDOS flag
	mov di,indos_offset
	cmp byte ptr es:[di],0
	jne timer_indos			;exit if it's set
	mov di,errflag_offset		;check critical error flag
	cmp byte ptr es:[di],0
	jne timer_errflag		;exit if it's set
	call main			;call body of program
	pop di
	pop es
timer_exit:
	iret
timer_indos:
	inc indos_counter
	pop di				;restore registers
	pop es
	iret
timer_errflag:
	inc errflag_counter
	pop di				;restore registers
	pop es
	iret
timer_status:
	inc status_counter
	iret
timer	endp
;
;------------------------------------------------------------------------------
;Interrupt 10h handling routine.
;------------------------------------------------------------------------------
video	proc near
	pushf				;push flags onto stack
	inc flag_10h			;increment flag
	call their_10h			;call BIOS routine
	dec flag_10h			;decrement flag
	iret
video	endp
;
;------------------------------------------------------------------------------
;Interrupt 13h handling routine.
;------------------------------------------------------------------------------
my_13	proc far
	pushf				;push flags onto stack
	inc flag_13h			;set 'busy' flag
	call their_13h			;call BIOS routine
	pushf				;save output flags
	dec flag_13h			;clear flag
	popf				;restore output flags
	ret 2				;exit without destroying flags
my_13	endp
;
;------------------------------------------------------------------------------
;Interrupt 28h handling routine.
;------------------------------------------------------------------------------
my_28	proc near
	pushf				;call original routine
	call their_28h
	inc bp_counter
	cmp flag_10h,0			;video flag set?
	jne bp_exit			;yes, then exit
	cmp flag_13h,0			;disk flag set?
	jne bp_exit			;yes, then exit
	push es				;save ES and DI
	push di
	mov es,dos_segment		;check critical error flag
	mov di,errflag_offset
	cmp byte ptr es:[di],0
	pop di				;clean up the stack
	pop es
	jne bp_errflag
	call main			;call main routine
bp_exit:
	iret				;done - exit
bp_errflag:
	inc errflag_counter
	iret				;done - exit
my_28	endp

;
;------------------------------------------------------------------------------
;Interrupt 21h handling routine.
;------------------------------------------------------------------------------
my_21	proc far
	pushf				;save the flags
	or	ah,ah			;Doing function zero?
	je	jump_to_dos		;If yes, take the jump
	cmp	ah,4bh			;Doing EXEC function?
	je	jump_to_dos		;If yes, take the jump
	popf

	pushf
	call	cs:their_21h		;Do the DOS function

	pushf				;Save the result flags
	cmp	cs:program_status,0	;are we already running?
	jne	no_recursion		;yes, don't recurse.
	dec	cs:main_countdown
	jne	no_recursion
	mov	cs:main_countdown,20
	call	main			;Safe to access disk now
no_recursion:
	popf				;Recover DOS result flags

	sti				;Must return with interrupts on
	ret	2			;Return with DOS result flags
jump_to_dos:
	popf
	jmp     cs:their_21h

my_21	endp

;
;------------------------------------------------------------------------------
;Interrupt 24h handling routine (DOS 3.X only).
;------------------------------------------------------------------------------
my_24	proc near
	mov al,3			;fail the call in progress
ioexit:
	iret				;give control back to DOS
my_24	endp
;
;------------------------------------------------------------------------------
;MAIN is the routine called periodically.
;------------------------------------------------------------------------------
main	proc near
	inc main_counter
	push	ax
	mov program_status,1		;set program active flag
	cli				;make sure interrupts are off
	mov ss_register,ss		;save stack registers
	mov sp_register,sp
	mov ss,cs:intds@
	mov sp,offset stack
	sti				;enable interrupts
	push bx
	push cx
	push dx
	push si
	push di
	push ds
	push es
	push bp
;
;Set DS and ES segment registers.
;
	push cs				;set DS to code segment
	pop ds
	dscode
  if verify
	call	check_stack
  endif
;
;Save the current active PSP address and activate this PSP.
;
	mov zflag,0			;clear flag
	cmp dos_version,2		;DOS version 2.X?
	jne main5
	mov es,dos_segment		;point ES:DI to INDOS
	mov di,indos_offset
	cmp byte ptr es:[di],0		;INDOS clear?
	je main5			;yes, then branch
	mov di,errflag_offset		;point ES:DI to error flag
	cmp byte ptr es:[di],0		;critical error flag clear?
	jne main5			;no, then branch
	mov byte ptr es:[di],1		;set critical error flag manually
	mov zflag,1			;set change flag
main5:
	mov ah,51h			;get current PSP segment
	int 21h
	mov their_psp,bx			;save it

	mov ah,50h			;make this the active PSP
	mov bx,my_psp
	int 21h

	cmp zflag,0			;ZFLAG clear?
	je main6			;yes, then branch
	mov di,errflag_offset		;point ES:DI to error flag
	mov byte ptr es:[di],0		;restore error flag value
main6:
;
;Reset the interrupt 1Bh, 23h, and 24h vectors.
;
	call ioset			;reset interrupt vectors
;
;Save the current dta and subdirectory
;
	mov	ah,2fh			;get disk transfer address
	int	21h
	mov	their_dta.segm,es
	mov	their_dta.offs,bx

;
;Call the commutator loop of net until nothing gets queued up.
;
	mov	ds,intds@
	assume	ds:nothing
	extproc	cycle
	call	cycle@
;
;Restore the current dta and subdirectory
;
	lds	dx,their_dta
	mov	ah,1ah
	int	21h

;
;Restore interrupt vectors and former active PSP.
;
	mov ah,50h			;restore active PSP label
	mov bx,their_psp
	int 21h
	call ioreset			;restore interrupt vectors
;
;Restore registers and stack before exit.
;
	pop bp				;restore registers and exit
	pop es
	pop ds
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	cli				;interrupts off
	mov ss,ss_register		;switch to original stack
	mov sp,sp_register
	sti				;interrupts on
	pop ax
	mov program_status,0		;clear status flag
	ret
main	endp
;
;------------------------------------------------------------------------------
;IOSET vectors interrupts 1Bh, 23h and 24h to internal handlers.  IORESET
;restores the original vector values.
;------------------------------------------------------------------------------
ioset	proc near
	dscode
	push es				;save ES
	mov ax,351Bh			;get interrupt 1Bh vector
	int 21h
	mov their_1Bh.segm,es		;save it
	mov their_1Bh.offs,bx
	mov ah,25h			;point it to an IRET instruction
	mov dx,offset ioexit
	int 21h
	mov ax,3523h			;get interrupt 23h vector
	int 21h
	mov their_23h.segm,es		;save it
	mov their_23h.offs,bx
	mov ah,25h			;point it to an IRET instruction
	mov dx,offset ioexit
	int 21h
	mov ax,3524h			;get interrupt 24h vector
	int 21h
	mov their_24h.segm,es		;save it
	mov their_24h.offs,bx
	mov ah,25h			;then set it to IOERR routine
	mov dx,offset my_24
	int 21h
	pop es				;restore ES
	ret
ioset	endp
;
ioreset	proc near
	assume	ds:nothing
	mov ax,2524h			;restore interrupt 24h vector
	lds dx,their_24h
	int 21h
	mov ax,2523h			;restore interrupt 23h vector
	lds dx,their_23h
	int 21h
	mov ax,251Bh			;restore interrupt 1Bh vector
	lds dx,their_1Bh
	int 21h
	ret
ioreset	endp

csegend


	pubproc	start_back
	push	ds
	mov	ax,cs
	mov	ds,ax
	dscode
;
;Remember our psp.
;
	mov ah,51h			;get current PSP segment
	int 21h
	mov my_psp,bx			;save it

;
;Determine which version of DOS is running.
;
init3:
	mov ah,30h			;DOS function 30h
	int 21h
	mov dos_version,al		;major version number
;
;Get and save the address of the INDOS flag.
;
	mov ah,34h			;function 34h
	int 21h				;get address
	mov dos_segment,es		;save segment
	mov indos_offset,bx		;save offset
;
;Get and save the address of the critical error flag.
;
	mov ax,3E80h			;CMP opcode
	mov cx,2000h			;max search length
	mov di,bx			;start at INDOS address
init4:
	repne scasw			;do the search
	jcxz init5			;branch if search failed
	cmp byte ptr es:[di+5],0BCh	;verify this is it
	je found			;branch if it is
	jmp init4			;resume loop if it's not
init5:
	mov cx,2000h			;search again
	inc bx				;search odd addresses this time
	mov di,bx
init6:
	repne scasw			;look for the opcode
	jcxz notfound			;not found if loop expires
	cmp byte ptr es:[di+5],0BCh	;verify this is it
	je found
	jmp init6
notfound:
	pop	ds
	xor	ax,ax
	ret
found:
	mov ax,es:[di]			;get flag offset address
	mov errflag_offset,ax		;save it

;
;Save and replace all required interrupt vectors.
;
  if 0
	mov ax,3508h			;get interrupt 8 vector
	int 21h
	mov their_8h.offs,bx			;save it
	mov their_8h.segm,es
	mov ah,25h			;point it to the timer routine
	mov dx,offset timer
	int 21h
  endif

	mov ax,3510h			;get interrupt 10h vector
	int 21h
	mov their_10h.offs,bx			;save it
	mov their_10h.segm,es
	mov ah,25h			;point it to video
	mov dx,offset video
	int 21h

	mov ax,3513h			;get interrupt 13h vector
	int 21h
	mov their_13h.offs,bx			;save it
	mov their_13h.segm,es
	mov ah,25h			;point it to my_13
	mov dx,offset my_13
	int 21h

	mov ax,3528h			;get interrupt 28h vector
	int 21h
	mov their_28h.offs,bx			;save it
	mov their_28h.segm,es
	mov ah,25h			;point it to my_28
	mov dx,offset my_28
	int 21h

	mov ax,3521h			;get interrupt 21h vector
	int 21h
	mov their_21h.offs,bx			;save it
	mov their_21h.segm,es
	mov ah,25h			;point it to my_21
	mov dx,offset my_21
	int 21h

	pop	ds
	mov	ax,1
	ret
	pend	start_back


	pubproc	stop_back
	assume	ds:nothing
	push	ds
	mov ax,2521h			;restore interrupt 21h vector
	lds dx,their_21h
	int 21h
	mov ax,2510h			;restore interrupt 10h vector
	lds dx,their_10h
	int 21h
	mov ax,2513h			;restore interrupt 13h vector
	lds dx,their_13h
	int 21h
  if 0
	mov ax,2508h			;restore interrupt 8 vector
	lds dx,their_8h
	int 21h
  endif
	mov ax,2528h			;restore interrupt 28h vector
	lds dx,their_28h
	int 21h
	pop	ds
	mov dx,cs
	mov ax,offset tick_counter
	ret
	pend	stop_back

  if verify
cseg
check_stack:
	push	ax
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es

	mov	ds,cs:intds@
	mov	es,cs:intds@
	mov	cx,verify_len
	mov	si,offset verify_original
	mov	di,offset verify_copy
	repne	cmpsb
	je	check_stack_1
	mov	dx,offset verify_err
	mov	ah,9
	int	21h
check_stack_1:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	ax
	ret
csegend
  endif

	end

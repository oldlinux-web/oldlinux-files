;VooDoo init!

;This will setup the system into a special mode.  This code will crash if
; EMM386, Windoze and other PMODE software is loaded.  You must also
; enable the a20 thru XMS if himem.sys is loaded.

; once complete the following will happen in real mode.
;  - mov ax,[ebx] is legal now
;  - code will still have a 64K limit range (IP still used not EIP)

;This is presented to learn from - I really suggest not using this technique
;as it's old and crappy.  Gamez to use this : Ultima 7. This was the day
;I hated the PC, but things got better as DPMI and VCPI were introduced

.386p

vd_desc struc
lmt   dw  0
bsl   dw  0
bsm   db  0
typel db  0
typeh db  0
bsh   db  0
vd_desc ENDS

.code
	cli       ;No ints
	xor eax,eax
	mov ax,cs
	mov ds,ax
	shl eax,4
	mov ds:[oldcs.bsl],ax
	mov ds:[oldds.bsl],ax
	shr eax,16
	mov ds:[oldcs.bsm],al
	mov ds:[oldds.bsm],al

	mov eax,code32
	shl eax,4
	add dword ptr ds:gdt32[2],eax
	add dword ptr ds:idt32[2],eax

	mov ds:[scode32.bsl],ax
	mov ds:[sdata32.bsl],ax
	shr eax,16
	mov ds:[scode32.bsm],al
	mov ds:[sdata32.bsm],al
	mov ds:[scode32.bsh],ah
	mov ds:[sdata32.bsh],ah   ;All mem ptr are calculated

	lgdt  fword ptr ds:gdt32    ;Load the GDT

	mov eax,cr0
	or  al,1
	mov cr0,eax       ;Hop to Prot. mode
	db  0eah
	dw  main32,8      ;far jmp 08:main

realmode16:
	lidt  fword ptr ds:defidt   ;load the IDT

	mov eax,cr0
	and al,0feh
	mov cr0,eax       ;Kill the P-mode bit

	db  0eah
	dw  realmode,Code_start   ;Another far jmp


realmode:
	mov ebx,10000h      ;
	mov ax,[ebx]      ;Oh my god this shit works !!
;Jmp here to your own code !!! VERY IMPORTANT !
	mov ax,4c00h
	int 21h

defidt    dw  3ffh,0,0    ;The normal IDT
	ENDS

code32    segment para public use32
	assume  cs:code32,ds:code32


main32:
	lidt  fword ptr cs:idt32    ;The PROT MODE IDT
	mov ax,10h
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov ss,ax       ;Just loading some segs
	mov ax,30h
	mov gs,ax       ;This is the videoseg
	xor esp,esp
	mov esp,offset stackend   ;What could that be ???
	call  enablea20
; jmp gone        ;Testing exc6
exit:
	mov ax,20h
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax       ;Loading the segs back
	db  0eah
	dw  realmode16,0,18h    ;Far jmp 18:realmode16

gone:
		db  10 dup(0feh)

idt32   dw  187h,idt,0
gdt32   dw  224,dummy,0
dummy   vd_desc  <>
scode32   vd_desc  <0ffffh,0,0,10011010b,11001111b,0>
sdata32   vd_desc  <0ffffh,0,0,10010010b,11001111b,0>
oldcs   vd_desc  <0ffffh,0,0,10011010b,10000000b,0>
oldds   vd_desc  <0ffffh,0,0,10010010b,10000000b,0>
bios    vd_desc  <0ffffh,0,0,10010010b,11001111b,0>
vseg    vd_desc  <0ffffh,8000h,0bh,10010010b,11001111b,0>

idt   dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0
		dw  inter,8,8e00h,0

inter:
	jmp exit


enablea20:
	call  enablea201
	jnz short enablea20done
	mov al,0d1h
	out 64h,al
	call  enablea201
	jnz short enablea20done
	mov al,0dfh
	out 60h,al
enablea201:
	mov ecx,20000h
enablea201l:
	jmp short $+2
	in  al,64h
	test  al,2
	loopnz  enablea201l
enablea20done:
	ret
pile    db  400 dup(?)
stackend:
	ENDS
	END

comment *
   Purpose:
   To detect math coprocessor type (8087, 287, 387) and manufacturer
   (Intel, IIT)
*

        .model  tiny

        .data
        fnone   equ     0
        f8087   equ     1
        f80287  equ     2
        f80387  equ     3
        funk    equ     0FFh
        ndp_cw  dw      ?
        ndp_sw  dw      ?
        mNDPCW  dw      ?
        mndp    db      ?
        ndpmsg  db      "Math coprocessor found: $"
        manmsg  db      "Manufacturer: $"
        _87     db      "8087",13,10,"$"
        _287    db      "80287",13,10,"$"
        _387    db      "80387 or i486",13,10,"$"
        _funk   db      "What the hell kind of a copro is this?",13,10,"$"
        Intel   db      "Intel or clone",13,10,"$"
        IIT     db      "IIT or clone",13,10,"$"

        .code
        org     100h

start:

; The next two 80x87 instructions cannot carry the WAIT prefix,
; because there may not be an 80x87 for which to wait.  The WAIT is
; therefore emulated with a MOV CX,<value>! LOOP $ combination.

.8087
        mov     word ptr ndp_cw,0000H
        cli                     ;no interrupts during this test

        fninit                  ;initialize NDP
        mov     cx,2
        loop    $

        fnstcw  ndp_cw          ;store control word in ndp_cw
;        mov     cx,14h
;        loop    $

        sti
        mov     ax,ndp_cw       ;check for valid status word
        cmp     ah,3            ;is NDP present?
        je      short ndp_01    ;if 3, must be there
        mov     mNDP,fnone
        jmp     short ndp_done

ndp_01:
        cmp     ax,03FFH        ;check if 8087
        jne     short ndp_02
        mov     mNDP,f8087
        jmp     short ndp_04

ndp_02:
        call    iit_test

.286P
        cmp     ax,037FH        ;check if 286/387/486
        jne     short ndp_05    ;must be garbage

;detect 287 or 387

        fld1                    ;Load +1.0 onto NDP stack
        fldz                    ;Load +0.0 onto NDP stack
        fdiv                    ;do +1/0
        fld1                    ;Load +1.0 onto NDP stack
        fchs                    ;Change to -1.0
        fldz                    ;Load +0.0 onto NDP stack
        fdiv                    ;do -1/0
        fcom                    ;compare
        fstsw   ndp_sw
        mov     ax,ndp_sw
        and     ah,41H          ; C3, C0
        cmp     ah,40H          ; ST(0) = ST(1)
        jne     short ndp_03
        mov     mNDP,f80287
        jmp     short ndp_04

ndp_03:
        cmp     ah,01H          ; ST(0) < ST(1)
        jne     short ndp_05
        mov     mNDP,f80387

ndp_04:

.8087
        fstcw   mNDPCW          ;save status for INFOPLUS
        jmp     short ndp_done

ndp_05:
        mov     mNDP,funk

ndp_done:
        mov     ah, 9
        mov     dx, offset manmsg
        int     21h
        cmp     [mman], 1       ;IIT=1?
        jne     short intelman
        mov     ah, 9
        mov     dx, offset iit
        int     21h
        jmp     short type_msgs

intelman:
        mov     ah, 9
        mov     dx, offset intel
        int     21h

type_msgs:
        mov     ah, 9
        mov     dx, offset ndpmsg
        int     21h
        cmp     [mndp], 0FFh
        jne     short not_funk
        mov     ah, 9
        mov     dx, offset _funk
        int     21h
        jmp     short exit_prog

not_funk:
        cmp     [mndp], 3
        jne     short not_387
        mov     ah, 9
        mov     dx, offset _387
        int     21h
        jmp     short exit_prog

not_387:
        cmp     [mndp], 2
        jne     short not_287
        mov     ah, 9
        mov     dx, offset _287
        int     21h
        jmp     short exit_prog

not_287:
        mov     ah, 9
        mov     dx, offset _87
        int     21h
exit_prog:
        mov     al, [mndp]
        mov     ah, 4ch
        int     21h

iit_test        proc    near

        .data
        fsb0    equ     <dw 0E8DBh>     ;bank 0 opcode
        fsb1    equ     <dw 0EBDBh>     ;bank 1 opcode
        fsb2    equ     <dw 0EADBh>     ;bank 2 opcode
        f4x4    equ     <dw 0F1DBh>     ;4x4 mat transform opcode
        f0      dd      9.9999
        f1      dd      10.0
        mman    db      0       ;assume Intel=0 installed initially

        .code
        ;initialize two banks to zero
        wait
        fsb0            ;switch to bank 0, default on Intel
        finit
        fsb1            ;switch to bank 1
        finit
        ;store a 2.0 into bank 0 while placing a 1.0 into bank 1
        fsb0            ;switch to bank 0
        fld     [f0]    ;load value from [F2] into bank 0 stack
        fclex           ;clear all math copro exceptions

        fsb1            ;switch to bank 1, should fail on Intel
        fld     [f1]    ;load a 1 into bank 1 stack
        fclex

        fsb0
        fcom    [f0]    ;compare, should be false on Intel
        fclex

        push    ax
        .286p           ;FNSTSW AX only works on 287+
        fnstsw  ax      ;store stat word in AX
        sahf            ;transfer copro flags to CPU flags
        ja      short is_intel
        mov     [mman], 1       ;IIT=1, Intel=0 (default)

is_intel:
        finit           ;reset to original
        pop     ax
        ret
        endp
        end     start


iit_test        proc    near

        .data
        fsb0    equ     <dw 0E8DBh>     ;bank 0 opcode
        fsb1    equ     <dw 0EBDBh>     ;bank 1 opcode
        fsb2    equ     <dw 0EADBh>     ;bank 2 opcode
        f4x4    equ     <dw 0F1DBh>     ;4x4 mat transform opcode
        f0      dd      9.9999
        f1      dd      10.0
        mman    db      0       ;assume Intel=0 installed initially

        .code
        ;initialize two banks to zero
        wait
        fsb0            ;switch to bank 0, default on Intel
        finit
        fsb1            ;switch to bank 1
        finit
        ;store a 2.0 into bank 0 while placing a 1.0 into bank 1
        fsb0            ;switch to bank 0
        fld     [f0]    ;load value from [F2] into bank 0 stack
        fclex           ;clear all math copro exceptions

        fsb1            ;switch to bank 1, should fail on Intel
        fld     [f1]    ;load a 1 into bank 1 stack
        fclex

        fsb0
        fcom    [f0]    ;compare, should be false on Intel
        fclex

        push    ax
        .286p           ;FNSTSW AX only works on 287+
        fnstsw  ax      ;store stat word in AX
        sahf            ;transfer copro flags to CPU flags
        ja      short is_intel
        mov     [mman], 1       ;IIT=1, Intel=0 (default)

is_intel:
        finit           ;reset to original
        pop     ax
        ret
        endp
        end     start

; EOF COPRO.ASM

; ========================================================================== ;
; IDEHDC.asm                                                                 ;
;   Direct disk I/O module for IDE disk controllers.                         ;
;   Written by Alan Martin.                                                  ;
;   Note - this code doesn't use interrupts!                                 ;
; ========================================================================== ;
idehdc segment use16 'CODE'     ; Direct disk I/O code segment.
                                        ; Initialize segment.
        assume  cs:idehdc                       ; Initial segment assumptions.
        .386                                    ; Assume 386+ code is valid.

                                        ; Stack addressing equates.
_ax     equ     word ptr bp+001Ch               ; Saved AX value.
_al     equ     byte ptr bp+001Ch               ; Saved AL value.
_ah     equ     byte ptr bp+001Dh               ; Saved AH value.
_bx     equ     word ptr bp+0010h               ; Saved BX value.
_bl     equ     byte ptr bp+0010h               ; Saved BL value.
_bh     equ     byte ptr bp+0011h               ; Saved BH value.
_cx     equ     word ptr bp+0018h               ; Saved CX value.
_cl     equ     byte ptr bp+0018h               ; Saved CL value.
_ch     equ     byte ptr bp+0019h               ; Saved CH value.
_dx     equ     word ptr bp+0014h               ; Saved DX value.
_dl     equ     byte ptr bp+0014h               ; Saved DL value.
_dh     equ     byte ptr bp+0015h               ; Saved DH value.

delay macro x                           ; Long delay (for 400ns transition).
local   @loop                                   ; Local labels.
        push    cx                              ; Save CX.
        mov     cx,x                            ; Get repeat count.
 @loop: loop @loop                              ; Loop X times.
        pop     cx                              ; Restore CX.
endm                                            ; End of DELAY macro.

hdcwait macro                           ; Wait for HDC to finish commands.
local   @loop,@ok,@err                          ; Local labels.
        push    ax ecx dx                       ; Save registers.
        mov     ecx,00040000h                   ; 1s delay on 486DX-50.
        mov     dx,01F7h                        ; HDC status register.
        in      al,dx                           ; Read status.
        test    al,80h                          ; Is the HDC busy?
        jz      @ok                             ; If not, end immediately.
 @loop: in      al,dx                           ; Read status.
        delay   000Ah                           ; 50-100 clock delay.
        test    al,80h                          ; Is the HDC busy?
        jz      @ok                             ; If not, end loop.
        loopd   @loop                           ; Otherwise, continue.
        pop     dx ecx ax                       ; Restore registers.
        stc                                     ; Set CF.
        jmp     @err                            ; Exit with error.
 @ok:   pop     dx ecx ax                       ; Restore registers.
        clc                                     ; Clear CF.
 @err:
endm                                            ; End of HDCWAIT macro.

; ------------------------------------------ ;
; IOREAD - Read sectors through direct I/O.  ;
; Input.: AX=cylinder number                 ;
;         BH=head number;  BL=sector number  ;
;         CH=# of sectors; CL=drive (0 or 1) ;
;         DS:DX->buffer for sectors          ;
; Output: CF=0 (OK) or 1 (Error)             ;
;         AL=error code or 0 if no error.    ;
;         AH=device error code if applicable.;
; Errors: AL=0 - No error.                   ;
;            1 - Controller busy or absent.  ;
;            2 - Drive not ready.            ;
;            3 - Drive not ready for read.   ;
;            4 - Device error:               ;
;         AH, bit 0: Address mark not found. ;
;                 1: Track 0 not found.      ;
;                 2: Write fault.            ;
;                 4: Sector not found.       ;
;                 6: Error in data.          ;
;                 7: Sector marked bad.      ;
; Note: There may be errors if a read is     ;
;       across a cylinder (or sometimes even ;
;       a head) boundary.                    ;
; ------------------------------------------ ;
ioread  proc far                        ; Read sectors through direct I/O.
        pushad                                  ; Save all registers.
        mov     bp,sp                           ; Address the stack.
        push    ds es                           ; Save segments.
        in      al,0A1h                         ; Get PIC 2 mask.
        push    ax                              ; Save it.
        or      al,40h                          ; Disable IRQ 14.
        out     0A1h,al                         ; Set PIC 2 mask.
        hdcwait                                 ; Wait for HDC not busy.
        jnc     ir_ok0                          ; Continue if not busy.
        mov     al,01h                          ; Error 1: Controller busy...
        mov     [_al],al                        ;    .
        jmp     ir_err                          ; ...done.
 ir_ok0:mov     al,[_bh]                        ; Get head number.
        mov     ah,[_cl]                        ; Get drive number.
        and     ax,010Fh                        ; Mask out extra bits.
        shl     ah,04h                          ; Adjust AH.
        or      al,ah                           ; Combine data.
        or      al,0A0h                         ; Set 512 bytes + ECC.
        mov     dx,01F6h                        ; Write drive/head numbers...
        out     dx,al                           ; ...done.
        hdcwait                                 ; Wait for HDC not busy.
        jnc     ir_ok1                          ; Continue if not busy.
        mov     al,01h                          ; Error 1: Controller busy...
        mov     [_al],al                        ;    .
        jmp     ir_err                          ; ...done.
 ir_ok1:mov     ecx,000C0000h                   ; 3s delay.
        mov     dx,01F7h                        ; HDC status register.
 ir_l1: in      al,dx                           ; Read status.
        test    al,40h                          ; Drive ready?
        jnz     ir_ok2                          ; Continue if so.
        loopd   ir_l1                           ; Loop for 3s.
        mov     al,02h                          ; Error 2: Drive not ready...
        mov     [_al],al                        ;    .
        jmp     ir_err                          ; ...done.
 ir_ok2:test    al,10h                          ; Drive ready for read?
        jnz     ir_ok3                          ; Continue if so.
        loopd   ir_l1                           ; Loop for 3s.
        mov     al,03h                          ; Error 3: Cannot read data...
        mov     [_al],al                        ;    .
        jmp     ir_err                          ; ...done.
 ir_ok3:mov     al,10h                          ; Set to >8 heads...
        mov     dx,03F6h                        ;    .
        out     dx,ax                           ; ...done.
        mov     dx,01F2h                        ; Write read parameters...
        mov     al,[_ch]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_bl]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_al]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_ah]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_bh]                        ;    .
        mov     ah,[_cl]                        ;    .
        and     ax,010Fh                        ;    .
        shl     ah,04h                          ;    .
        or      al,ah                           ;    .
        or      al,0A0h                         ;    .
        out     dx,al                           ; ...done.
        mov     dx,01F1h                        ; Write Precompensation = 0...
        xor     al,al                           ;    .
        out     dx,al                           ; ...done.
        hdcwait                                 ; Wait for HDC not busy.
        jnc     ir_ok4                          ; Continue if not busy.
        mov     al,01h                          ; Error 1: Controller busy...
        mov     [_al],al                        ;    .
        jmp     ir_err                          ; ...done.
 ir_ok4:xor     cx,cx                           ; Get sector count...
        mov     cl,[_ch]                        ; ...done.
        push    ds                              ; Put DS in ES...
        pop     es                              ; ...done.
        mov     di,[_dx]                        ; Get offset.
        mov     dx,01F7h                        ; Send read command...
        mov     al,20h                          ;    .
        out     dx,al                           ; ...done.
 ir_l2: mov     dx,01F7h                        ; Get status port.
        delay   000Ah                           ; Delay for >400ns.
        in      al,dx                           ; Get status.
        test    al,80h                          ; Busy?
        jnz     ir_l2                           ; Loop if so.
        test    al,29h                          ; Loop if no change...
        jz      ir_l2                           ; ...done.
        test    al,08h                          ; Ready for data?
        jnz     ir_rda                          ; If so, read it.
        test    al,21h                          ; Error in command?
        jnz     ir_dev                          ; If so, return device error.
        jmp     ir_l2                           ; Continue loop.
 ir_rda:push    cx                              ; Save CX.
        mov     cx,0100h                        ; Repeat count.
        mov     dx,01F0h                        ; 16-bit transfer port.
        rep     insw                            ; Read data.
        pop     cx                              ; Restore CX.
        loop    ir_l2                           ; Loop until done.
        mov     al,12h                          ; Deactivate controller...
        mov     dx,03F6h                        ;    .
        out     dx,ax                           ; ...done.
        mov     al,00h                          ; No error - return 0...
        mov     [_al],al                        ; ...done.
        pop     ax                              ; Reset PIC 2 mask...
        out     0A1h,al                         ; ...done.
        clc                                     ; No error: CF=0.
        pop     es ds                           ; Restore segments.
        popad                                   ; Restore all registers.
        ret                                     ; Return (far).
 ir_dev:mov     al,04h                          ; Error 4: Device fault...
        mov     [_al],al                        ; ...done.
        mov     dx,01F1h                        ; Get error code...
        in      al,dx                           ;    .
        mov     [_ah],al                        ; ...done.
        mov     dx,01F6h                        ; Recalibrate head...
        mov     al,[_bh]                        ;    .
        mov     ah,[_cl]                        ;    .
        and     ax,010Fh                        ;    .
        shl     ah,04h                          ;    .
        or      al,ah                           ;    .
        or      al,0A0h                         ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,10h                          ;    .
        out     dx,al                           ; ...done.
        hdcwait                                 ; Wait for HDC not busy.
        mov     al,12h                          ; Deactivate controller...
        mov     dx,03F6h                        ;    .
        out     dx,ax                           ; ...done.
 ir_err:pop     ax                              ; Reset PIC 2 mask...
        out     0A1h,al                         ; ...done.
        stc                                     ; Error: CF=1.
        pop     es ds                           ; Restore segments.
        popad                                   ; Restore all registers.
        ret                                     ; Return (far).
ioread  endp                                    ; End of IOREAD procedure.

; ------------------------------------------ ;
; IOWRITE - Write sectors through direct I/O.;
; Input.: AX=cylinder number                 ;
;         BH=head number;  BL=sector number  ;
;         CH=# of sectors; CL=drive (0 or 1) ;
;         DS:DX->buffer for sectors          ;
; Output: CF=0 (OK) or 1 (Error)             ;
;         AL=error code or 0 if no error.    ;
;         AH=device error code if applicable.;
; Errors: AL=0 - No error.                   ;
;            1 - Controller busy or absent.  ;
;            2 - Drive not ready.            ;
;            3 - Drive not ready for write.  ;
;            4 - Device error:               ;
;         AH, bit 0: Address mark not found. ;
;                 1: Track 0 not found.      ;
;                 2: Write fault.            ;
;                 4: Sector not found.       ;
;                 6: Error in data.          ;
;                 7: Sector marked bad.      ;
; Note: There may be errors if a write is    ;
;       across a cylinder (or sometimes even ;
;       a head) boundary.                    ;
; ------------------------------------------ ;
iowrite proc far                        ; Write sectors through direct I/O.
        pushad                                  ; Save all registers.
        mov     bp,sp                           ; Address the stack.
        push    ds es                           ; Save segments.
        in      al,0A1h                         ; Get PIC 2 mask.
        push    ax                              ; Save it.
        or      al,40h                          ; Disable IRQ 14.
        out     0A1h,al                         ; Set PIC 2 mask.
        hdcwait                                 ; Wait for HDC not busy.
        jnc     iw_ok0                          ; Continue if not busy.
        mov     al,01h                          ; Error 1: Controller busy...
        mov     [_al],al                        ;    .
        jmp     iw_err                          ; ...done.
 iw_ok0:mov     al,[_bh]                        ; Get head number.
        mov     ah,[_cl]                        ; Get drive number.
        and     ax,010Fh                        ; Mask out extra bits.
        shl     ah,04h                          ; Adjust AH.
        or      al,ah                           ; Combine data.
        or      al,0A0h                         ; Set 512 bytes + ECC.
        mov     dx,01F6h                        ; Write drive/head numbers...
        out     dx,al                           ; ...done.
        hdcwait                                 ; Wait for HDC not busy.
        jnc     iw_ok1                          ; Continue if not busy.
        mov     al,01h                          ; Error 1: Controller busy...
        mov     [_al],al                        ;    .
        jmp     iw_err                          ; ...done.
 iw_ok1:mov     ecx,000C0000h                   ; 3s delay.
        mov     dx,01F7h                        ; HDC status register.
 iw_l1: in      al,dx                           ; Read status.
        test    al,40h                          ; Drive ready?
        jnz     iw_ok2                          ; Continue if so.
        loopd   iw_l1                           ; Loop for 3s.
        mov     al,02h                          ; Error 2: Drive not ready...
        mov     [_al],al                        ;    .
        jmp     iw_err                          ; ...done.
 iw_ok2:test    al,10h                          ; Drive ready for write?
        jnz     iw_ok3                          ; Continue if so.
        loopd   iw_l1                           ; Loop for 3s.
        mov     al,03h                          ; Error 3: Cannot write...
        mov     [_al],al                        ;    .
        jmp     iw_err                          ; ...done.
 iw_ok3:mov     al,10h                          ; Set to >8 heads...
        mov     dx,03F6h                        ;    .
        out     dx,ax                           ; ...done.
        mov     dx,01F2h                        ; Write write parameters...
        mov     al,[_ch]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_bl]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_al]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_ah]                        ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,[_bh]                        ;    .
        mov     ah,[_cl]                        ;    .
        and     ax,010Fh                        ;    .
        shl     ah,04h                          ;    .
        or      al,ah                           ;    .
        or      al,0A0h                         ;    .
        out     dx,al                           ; ...done.
        mov     dx,01F1h                        ; Write Precompensation = 0...
        xor     al,al                           ;    .
        out     dx,al                           ; ...done.
        hdcwait                                 ; Wait for HDC not busy.
        jnc     iw_ok4                          ; Continue if not busy.
        mov     al,01h                          ; Error 1: Controller busy...
        mov     [_al],al                        ;    .
        jmp     iw_err                          ; ...done.
 iw_ok4:xor     cx,cx                           ; Get sector count...
        mov     cl,[_ch]                        ; ...done.
        mov     si,[_dx]                        ; Get offset.
        mov     dx,01F7h                        ; Send write command...
        mov     al,30h                          ;    .
        out     dx,al                           ; ...done.
 iw_l2: mov     dx,01F7h                        ; Get status port.
        delay   000Ah                           ; Delay for >400ns.
        in      al,dx                           ; Get status.
        test    al,80h                          ; Busy?
        jnz     iw_l2                           ; Loop if so.
        test    al,29h                          ; Loop if no change...
        jz      iw_l2                           ; ...done.
        test    al,08h                          ; Ready for data?
        jnz     iw_wda                          ; If so, write it.
        test    al,21h                          ; Error in command?
        jnz     iw_dev                          ; If so, return device error.
        jmp     iw_l2                           ; Continue loop.
 iw_wda:push    cx                              ; Save CX.
        mov     cx,0100h                        ; Repeat count.
        mov     dx,01F0h                        ; 16-bit transfer port.
        rep     outsw                           ; Write data.
        pop     cx                              ; Restore CX.
        loop    iw_l2                           ; Loop until done.
        mov     al,12h                          ; Deactivate controller...
        mov     dx,03F6h                        ;    .
        out     dx,ax                           ; ...done.
        mov     al,00h                          ; No error - return 0...
        mov     [_al],al                        ; ...done.
        pop     ax                              ; Reset PIC 2 mask...
        out     0A1h,al                         ; ...done.
        clc                                     ; No error: CF=0.
        pop     es ds                           ; Restore segments.
        popad                                   ; Restore all registers.
        ret                                     ; Return (far).
 iw_dev:mov     al,04h                          ; Error 4: Device fault...
        mov     [_al],al                        ; ...done.
        mov     dx,01F1h                        ; Get error code...
        in      al,dx                           ;    .
        mov     [_ah],al                        ; ...done.
        mov     dx,01F6h                        ; Recalibrate head...
        mov     al,[_bh]                        ;    .
        mov     ah,[_cl]                        ;    .
        and     ax,010Fh                        ;    .
        shl     ah,04h                          ;    .
        or      al,ah                           ;    .
        or      al,0A0h                         ;    .
        out     dx,al                           ;    .
        inc     dx                              ;    .
        mov     al,10h                          ;    .
        out     dx,al                           ; ...done.
        hdcwait                                 ; Wait for HDC not busy.
        mov     al,12h                          ; Deactivate controller...
        mov     dx,03F6h                        ;    .
        out     dx,ax                           ; ...done.
 iw_err:pop     ax                              ; Reset PIC 2 mask...
        out     0A1h,al                         ; ...done.
        stc                                     ; Error: CF=1.
        pop     es ds                           ; Restore segments.
        popad                                   ; Restore all registers.
        ret                                     ; Return (far).
iowrite endp                                    ; End of IOWRITE procedure.

                                        ; Clean up segment.
        assume  nothing                         ; Remove segment assumptions.
idehdc ends                                     ; End of direct I/O segment.
; ========================================================================== ;

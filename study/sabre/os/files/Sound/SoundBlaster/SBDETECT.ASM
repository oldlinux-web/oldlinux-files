WAIT_TIME       EQU     000FFh
DSP_INTRQ_CMD   EQU     000F2h

DATA    SEGMENT WORD PUBLIC

        EXTRN   SBInt   : WORD;
        EXTRN   SBIrq   : WORD;

        ORG_INT2_ADDX   DD 0;
        ORG_INT3_ADDX   DD 0;
        ORG_INT5_ADDX   DD 0;
        ORG_INT7_ADDX   DD 0;

DATA    ENDS

CODE	SEGMENT WORD	PUBLIC
	ASSUME	CS:CODE,DS:DATA

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
; ResetDSP - reset the DSP
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

ResetDSP        PROC

                MOV    DX,SBInt
                ADD    DL,6            ; DX = DSP reset port 2x6h
                MOV    AL,1
                OUT    DX,AL           ; write 1, then wait 3 micro-seconds

                IN     AL,DX           ; read value from DSP
RDSP05:
                INC    AL
                JNZ    RDSP05          ; wait until AL = 0

                OUT    DX,AL           ; write 0

                MOV    CL,20H          ; set reset timeout value

RDSP10:
                CALL   ReadDSPTime     ; read from DSP
                CMP    AL,0AAH         ; if byte is 0AAh then
                JE     RDSP20          ;   DSP is reset, exit

                LOOP   RDSP10          ; not reset, try again

                MOV    AX,2            ; set I/O failure error
                JMP    SHORT RDSP90    ; exit;

RDSP20:
                XOR    AX,AX           ; clear AX, no error

RDSP90:
                OR     AX,AX           ; set flags for return

                RET

ResetDSP        ENDP

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
; ReadDSPTime   - read from DSP with timeout
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

ReadDSPTime     PROC

                PUSH   CX
                PUSH   DX

                MOV    DX,SBInt
                ADD    DL,0EH          ; DX = DSP data available port 2xEh

                MOV    CX,WAIT_TIME    ; load timeout value into CX

RDT10:
                IN     AL,DX           ; check to see if DSP is ready
                OR     AL,AL
                JS     RDT20           ; if sign set, then DSP read ready

                LOOP   RDT10           ; not ready, keep waiting

                STC                    ; set error, DSP read timeout
                JMP    SHORT RDT90     ; exit

RDT20:
                SUB    DL,4            ; DX = DSP read data port 2xAh
                IN     AL,DX           ; read byte from DSP
                CLC                    ; clear error

RDT90:
                POP    DX
                POP    CX

                RET

ReadDSPTime     ENDP

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
; CheckSBInt - detect DMA interrupt
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

PUBLIC          CheckSBInt

CheckSBInt      PROC   FAR

                push   bp
                mov    bp,sp

                MOV    AL,2                           ; setup end of DMA
                MOV    DX,OFFSET DUMMY_DMA_INT2       ; interrupts for all
                MOV    BX,OFFSET DATA:ORG_INT2_ADDX   ; possible IRQs
                CALL   SetupInterrupt                 ; (2, 3, 5, and 7)

                MOV    AL,3
                MOV    DX,OFFSET DUMMY_DMA_INT3
                MOV    BX,OFFSET DATA:ORG_INT3_ADDX
                CALL   SetupInterrupt

                MOV    AL,5
                MOV    DX,OFFSET DUMMY_DMA_INT5
                MOV    BX,OFFSET DATA:ORG_INT5_ADDX
                CALL   SetupInterrupt

                MOV    AL,7
                MOV    DX,OFFSET DUMMY_DMA_INT7
                MOV    BX,OFFSET DATA:ORG_INT7_ADDX
                CALL   SetupInterrupt

                MOV    SBIrq,0          ; reset current IRQ

                MOV    DX,SBInt
                ADD    DX,0CH           ; DX = DSP write port 2xCh
                MOV    AL,DSP_INTRQ_CMD ; AL = interrupt request command
                CALL   WriteDSP         ; write command to DSP

                XOR    AX,AX           ; assume success
                MOV    CX,WAIT_TIME    ; load timeout value into CX
                CLD

VI10:
                CMP    SBIrq,0         ; see if interrupt has been changed
                JNE    VI90            ; if so, continue

                LOOP   VI10            ; if not, keep waiting

                MOV    AX,0FFh         ; set DMA failure error

VI90:
                PUSH   AX              ; save result variable

                MOV    AL,2                          ; restore end of DMA
                MOV    BX,OFFSET DATA:ORG_INT2_ADDX  ; interrupts
                CALL   RestoreInterrupt

                MOV    AL,3
                MOV    BX,OFFSET DATA:ORG_INT3_ADDX
                CALL   RestoreInterrupt

                MOV    AL,5
                MOV    BX,OFFSET DATA:ORG_INT5_ADDX
                CALL   RestoreInterrupt

                MOV    AL,7
                MOV    BX,OFFSET DATA:ORG_INT7_ADDX
                CALL   RestoreInterrupt

                POP    AX              ; restore result

                OR     AX,AX

                mov    sp,bp
                pop    bp

                RET

CheckSBInt      ENDP

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
;  WriteDSP
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

WriteDSP        PROC

                PUSH   CX

                MOV    AH,AL           ; save value in AL for later

                MOV    CX,WAIT_TIME

WD10:
                IN     AL,DX           ; check to see if DSP is ready
                OR     AL,AL
                JNS    WD20            ; if sign set, DSP not write ready
                LOOP   WD10

WD20:
                MOV    AL,AH           ; restore AL
                OUT    DX,AL           ; send byte to DSP

                POP    CX

                RET

WriteDSP        ENDP

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
; SetupInterrupt
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

SetupInterrupt  PROC

                PUSH   BX
                PUSH   CX
                PUSH   DX

                CLI

                MOV    CL,AL           ; preserve interrupt number for use

                ADD    AL,8            ; calculate interrupt vector addx
                CBW
                SHL    AL,1
                SHL    AL,1
                MOV    DI,AX

                PUSH   ES              ; setup and preserve interrupt
                XOR    AX,AX
                MOV    ES,AX
                MOV    AX,ES:[DI]
                MOV    [BX],AX
                MOV    ES:[DI],DX

                MOV    AX,ES:[DI+2]
                MOV    [BX+2],AX
                MOV    ES:[DI+2],CS

                POP    ES

                MOV    AH,1            ; enable interrupt control mask-bit
                SHL    AH,CL
                NOT    AH

                IN     AL,21H
                AND    AL,AH
                OUT    21H,AL

                STI

                POP    DX
                POP    CX
                POP    BX

                RET

SetupInterrupt  ENDP


;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
; RestoreInterrupt
;
; ENTRY: AL = INTERRUPT NUM
;        BX = offset to stored addx
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

RestoreInterrupt PROC

                CLI

                MOV    CL,AL

                ADD    AL,8            ; calculate interrupt vector addx
                CBW
                SHL    AL,1
                SHL    AL,1
                MOV    DI,AX

                PUSH   ES              ; restore interrupt vector
                XOR    AX,AX
                MOV    ES,AX
                MOV    AX,[BX]
                MOV    ES:[DI],AX

                MOV    AX,[BX+2]
                MOV    ES:[DI+2],AX

                POP    ES

                MOV    AH,1
                SHL    AH,CL

                IN     AL,21H
                OR     AL,AH
                OUT    21H,AL

                STI

                RET

RestoreInterrupt ENDP

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様
;
; DUMMY INTERRUPTS - used for interrupt detection
;
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様

DUMMY_DMA_INT2  PROC   FAR

                PUSH   DX
                MOV    DX,2
                JMP    SHORT DUMMY_DMA_ISR

DUMMY_DMA_INT2  ENDP


DUMMY_DMA_INT3  PROC   FAR

                PUSH   DX
                MOV    DX,3
                JMP    SHORT DUMMY_DMA_ISR

DUMMY_DMA_INT3  ENDP


DUMMY_DMA_INT5  PROC   FAR

                PUSH   DX
                MOV    DX,5
                JMP    SHORT DUMMY_DMA_ISR

DUMMY_DMA_INT5  ENDP


DUMMY_DMA_INT7  PROC   FAR

                PUSH   DX
                MOV    DX,7

DUMMY_DMA_ISR:
                PUSH   AX
                PUSH   DS

                MOV    AX,DATA
                MOV    DS,AX

                MOV    SBIrq,DX        ; update interrupt variable with
                                       ; number of interrupt called
                MOV    DX,SBInt
                ADD    DX,0EH          ; DX = DSP data available port 2xEh
                IN     AL,DX           ; acknowledge DSP interrupt

                MOV    AL,20H          ; send EOI (end of interrupt) to
                OUT    20H,AL          ; interrupt controller port 20h

                POP    DS
                POP    AX
                POP    DX
                IRET                   ; interrupt return

DUMMY_DMA_INT7  ENDP

CODE	ENDS

	END

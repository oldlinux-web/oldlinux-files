;---------------------------------------------------------------------------
; SB_DSP.ASM -- Programmer's library for the Sound Blaster DSP interface
;---------------------------------------------------------------------------

                        IDEAL
                        MODEL small

                        DATASEG
SB_IO_Port  DW 0220h    ;These values are kept here to avoid add
DSP_Reset   DW 0226h    ;instructions in code that may be time-
DSP_RDData  DW 022Ah    ;sensitive.
DSP_Command DW 022Ch
DSP_RDAvail DW 022Eh

DSP_WRData EQU DSP_Command      ;Aliases for the same variable name
DSP_Status EQU DSP_Command

                        CODESEG

PUBLIC _dsp_reset,_dsp_voice,_set_SB_address,_dsp_dma_prepare,_dsp_time

MACRO await_DSP
LOCAL @@Wait_Ready
     push ax dx
@@Wait_Ready:
     mov  dx,[DSP_Status]
     in   al,dx
     and  al,128
      jnz @@Wait_Ready
     pop  dx ax
ENDM MACRO


MACRO ten_microsec_delay
LOCAL @@KT                 ;If you know of a better, more accurate,
     push cx               ;more reliable, just plain smarter, way
     mov  cx,20000         ;to do this, PLEASE TELL ME.
@@KT:                      ;
     nop                   ;(The idea is to kill 10 microseconds.)
     loop @@KT             ;
     pop  cx               ;email: heathh@cco.caltech.edu
ENDM ten_microsec_delay

;---------------------------------------------------------------------------
; void far dsp_time(int pacing)
;---------------------------------------------------------------------------
; pacing = 255 - (1,000,000 / frequency)
;---------------------------------------------------------------------------
PROC _dsp_time FAR
ARG delay:WORD
     push bp
     mov  bp,sp
     push ax dx

     await_DSP
     mov  dx,[DSP_Command]
     mov  al,040h
     out  dx,al

     await_DSP
     mov  dx,[DSP_Command]
     mov  ax,[delay]
     out  dx,al
     pop  dx ax
     pop  bp
     ret
ENDP _dsp_time


;---------------------------------------------------------------------------
; void far dsp_reset(void)
;---------------------------------------------------------------------------
PROC _dsp_reset FAR
     push ax dx

     mov  dx,[DSP_Reset]
     mov  al,1
     out  dx,al

     ten_microsec_delay

     mov  al,0
     out  dx,al

@@Wait_Ready:
     mov  dx,[DSP_RDAvail]
     in   al,dx
     and  al,128
       jz @@Wait_Ready
     mov  dx,[DSP_RDData]
     in   al,dx
     cmp  al,0AAh
      jne @@Wait_Ready

     mov  ax,165
     push ax
     call _dsp_time
     pop  ax
     pop  dx ax
     ret
ENDP _dsp_reset

;---------------------------------------------------------------------------
; void far dsp_dma_prepare(int Dir,int Length)
;---------------------------------------------------------------------------
; Dir = 0 for Microphone Input, 1 for Speaker Output
; Length = Length of data to be sampled/played
;---------------------------------------------------------------------------
PROC _dsp_dma_prepare FAR
ARG Dir:WORD,Len:WORD
     push bp
     mov  bp,sp
     push ax dx

     await_DSP
     mov  al,24h
     cmp  [Dir],0
       jz @@Is_Read
     mov  al,14h
@@Is_Read:
     mov  dx,[DSP_Command]
     out  dx,al
     await_DSP
     mov  ax,[Len]
     out  dx,al
     await_DSP
     mov  al,ah
     out  dx,al

     pop  dx ax
     pop  bp
     ret
ENDP _dsp_dma_prepare

;---------------------------------------------------------------------------
; void far dsp_voice(Activity)
;---------------------------------------------------------------------------
; Activity = 0 for silent, 1 for audible
;---------------------------------------------------------------------------
PROC _dsp_voice FAR
ARG Which:WORD
     push bp
     mov  bp,sp
     push dx ax

     await_DSP
     mov  ax,[Which]
     and  al,1                  ;Want a 1/0 parameter
     xor  al,1
     shl  al,1
     or   al,0D1h
     mov  dx,[DSP_Command]
     out  dx,al
     pop  ax dx
     pop  bp
     ret
ENDP _dsp_voice

;---------------------------------------------------------------------------
; void far set_SB_address(int base)
;---------------------------------------------------------------------------
; Sets base port of SoundBlaster that other functions refer to.
; Most likely value (and the default): 0x220
;---------------------------------------------------------------------------
PROC _set_SB_address FAR
ARG Port:WORD
     push bp
     mov  bp,sp
     push ax
     mov  ax,[Port]
     mov  [SB_IO_Port],ax
     add  ax,6
     mov  [DSP_Reset],ax
     add  ax,4
     mov  [DSP_RDData],ax
     add  ax,2
     mov  [DSP_Command],ax
     add  ax,2
     mov  [DSP_RDAvail],ax
     pop  ax
     pop  bp
     ret
ENDP _set_SB_address
END

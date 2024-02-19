* Author: Michael Schwingen
*         Kranichstraže 10
*         5042 Erftstadt
* purpose:
* Unixname installes itself resident in the GEMDOS trap vector and converts
* all '/'s in filenames to TOS-like '\'.
* Assembler: DevpacST 2.0
              opt o+

start:        bra     init

; List of GEMDOS-functions and address of filename on stack
; 57 (Dcreate)  : 8(SP)
; 58 (Ddelete)  : 8(SP)
; 59 (Dsetpath) : 8(SP)
; 60 (Fcreate)  : 8(SP)
; 61 (Fopen)    : 8(SP)
; 65 (Fdelete)  : 8(SP)
; 67 (Fattrib)  : 8(SP)
; 75 (Pexec)    : 10(SP) - 8(SP) = mode, changes only if 0/3 !
; 78 (Fsfirst)  : 8(SP)
; 86 (Frename)  : 10(SP),14(SP)

              DC.B "XBRAUn*x"
oldvec:       DC.L 0
new_vec:      move.w  (sp),d0         ; get SR
              lea     6(sp),a0
              btst    #$0D,d0         ; SUPER-mode ?
              bne.s   .new2           ; yes
              move    usp,a0          ; no, parameters are on userstack
.new2:        move.w  (a0),d0         ; get GEMDOS Opcode
              cmp.w   #57,d0          ; Dcreate() ?
              beq     func_type1
              cmp.w   #58,d0          ; Ddelete() ?
              beq     func_type1
              cmp.w   #59,d0          ; Dsetpath() ?
              beq     func_type1
              cmp.w   #60,d0          ; Fcreate() ?
              beq     func_type1
              cmp.w   #61,d0          ; Fopen() ?
              beq     func_type1
              cmp.w   #65,d0          ; Fdelete() ?
              beq     func_type1
              cmp.w   #67,d0          ; Fattrib() ?
              beq     func_type1
              cmp.w   #75,d0          ; Pexec() ?
              beq     func_pexec
              cmp.w   #78,d0          ; Fsfirst() ?
              beq     func_type1
              cmp.w   #86,d0          ; Frename() ?
              beq     func_frename
old_vec:      movea.l oldvec,a0
              jmp     (a0)
func_type1:   movea.l 2(a0),a0        ; ptr to filename
func1_1:      bsr     convert_name
              bra.s   old_vec
func_pexec:   move.w  2(a0),d0        ; mode: 0/3 = filename at 4(a0)
              cmp.w   #0,d0
              beq.s   .pexec2
              cmp.w   #3,d0
              bne.s   old_vec
.pexec2:      movea.l 4(a0),a0
              bra.s   func1_1
func_frename: move.l  a0,-(sp)
              movea.l 8(a0),a0
              bsr     convert_name
              movea.l (sp)+,a0
              bra.s   func_pexec

convert_name:
              move.b  (a0),d0
              beq     .convend
              cmp.b   #"/",d0
              bne     .convnext
              move.b  #"\",(a0)
.convnext:    addq.l  #1,a0
              bra.s   convert_name
.convend:     rts

res_len       EQU *-start
; everything after this point does not stay resident in memory


; install resident part
init:         pea     new_vec(pc)
              move.l  #$050021,-(sp)  ; Setexc(33,...)
              trap    #13
              addq.l  #8,sp
              move.l  d0,oldvec

              lea     inst_txt(pc),a0
              bsr     Cconws

              clr.w   -(sp)           ; Ret.-Code
              move.l  #256+res_len,-(sp)
              move.w  #49,-(sp)       ; Ptermres()
              trap    #1

Cconws:       move.l  a0,-(sp)
              move.w  #9,-(sp)
              trap    #1              ; Cconws
              addq.l  #6,sp
              rts

inst_txt:     DC.B 13,10,"Un*xName V1.0 ½ 1991 by Michael Schwingen installed.",13,10,0

								IDEAL
								ModeL large

;+--------------------------------------------------------------------------+
;|  IBM-PC(tm) compatible programmer's DMA library                          |
;+--------------------------------------------------------------------------+
;|  This assembly code defines 3 functions that are intended for use        |
;|  by C programmers in code that requires access to the DMA system.        |
;|                                                                          |
;|  The general sequence for using the DMA is:                              |
;|      int channel=1;                                                      |
;|      if (dma_reset(channel))                                             |
;|              abort();                                                    |
;|      if (dma_setup(channel,(char far *)My_Buffer,sizeof(My_Buffer),1))   |
;|              abort();                                                    |
;|      /* Insert "foreground" code here. */                                |
;|      while (dma_done(channel)!=-1) {                                     |
;|              if (dma_errno)                                              |
;|                      abort();                                            |
;|      }                                                                   |
;+--------------------------------------------------------------------------+
;| PUBLIC FUNCTIONS                                                         |
;| int far dma_reset(int Channel)                                           |
;| int far dma_setup(int Channel,char far *Buffer,unsigned Length,int Dir)  |
;| int far dma_done(int Channel)                                            |
;+--------------------------------------------------------------------------+
;| PUBLIC DATA                                                              |
;| int far dma_errno                                                        |
;| char far *dma_errlist[]                                                  |
;+--------------------------------------------------------------------------+

Status          EQU     08h     ;DMAC status port (read)     \  same port
Command         EQU     08h     ;DMAC command port (write)   /  (read/write)
;STATUS/COMMAND BYTE:   ("*" represents defaults)
;  [ 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 ]
;   Bit 0: Memory-to-memory transfer  0 => disable*
;                                     1 => enable
;       1: "Don't Care" if mem-to-mem disabled  (Bit 0==0)*
;          Channel 0 address hold     0 => disable
;                                     1 => enable
;       2: Controller enable          0 => enable*
;                                     1 => disable
;       3: "Don't Care" if mem-to-mem enabled (Bit 0==1)
;          Timing                     0 => Normal?
;                                     1 => Compressed?
;       4: Priority                   0 => Fixed?
;                                     1 => Rotating
;       5: "Don't care" if compressed timing (Bit 3==1)
;          Write selection            0 => Late
;                                     1 => Extended
;       6: DREQ sense active          0 => High
;                                     1 => Low
;       7: DACK sense active          0 => Low
;                                     1 => High

Request         EQU     09h     ;DMAC channel request (write-only)
;REQUEST BYTE:
;  [ 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 ]
;  \__________________/  |  \_____/
;       Don't care       |     |
;                        |     +------+  00 = Select channel 0
;                        |            |  01 = Select channel 1
;                        |            |  10 = Select channel 2
;                        |            +  11 = Select channel 3
;                        +---+ 0 = Reset request bit
;                            + 1 = Set request bit

DMA_Mask        EQU     0Ah     ;DMAC DMA_Mask (write-only)
Mode            EQU     0Bh     ;DMAC mode (read/write)


byte_ptr        EQU     00ch    ; byte pointer flip-flop

addr          EQU 000h  ; per-channel base address
count         EQU 001h  ; per-channel byte count

read_cmd        EQU     048h    ; read mode
write_cmd       EQU     044h    ; write mode
set_cmd         EQU     000h    ; DMA_Mask set
reset_cmd       EQU     004h    ; DMA_Mask reset

; dma controller page register table
; this table maps from channel number to the i/o port number of the
; page register for that channel
								DATASEG

page_table      DW  00087h  ; channel 0
					DW  00083h  ; channel 1
					DW  00081h  ; channel 2
						DW  00082h  ; channel 3

; "Extra" messages are for future compatability with the Virtual DMA
; specification.
DMA_E0          DB      0
DMA_E1          DB      "Region not in contiguous memory.",0
DMA_E2          DB      "Region crossed a physical alignment boundary.",0
DMA_E3          DB      "Unable to lock pages.",0
DMA_E4          DB      "No buffer available.",0
DMA_E5          DB      "Region too large for buffer.",0
DMA_E6          DB      "Buffer currently in use.",0
DMA_E7          DB      "Invalid memory region.",0
DMA_E8          DB      "Region was not locked.",0
DMA_E9          DB      "Number of physical pages greater than table length.",0
DMA_EA          DB      "Ivalid buffer ID.",0
DMA_EB          DB      "Copy out of buffer range.",0
DMA_EC          DB      "Invalid DMA channel number.",0
_dma_errlist DD DMA_E0, DMA_E1, DMA_E2, DMA_E3, DMA_E4, DMA_E5, DMA_E6, DMA_E7, DMA_E8, DMA_E9, DMA_EA, DMA_EB, DMA_EC
_dma_errno   DW 0

;char far *dma_errlist[]
;int _dma_errno
PUBLIC _dma_errlist,_dma_errno

								CODESEG
MACRO zero reg
			xor reg,reg
ENDM zero
			
PUBLIC _dma_setup,_dma_reset,_dma_done
;+---------------------------------------------------------------------------+
;| int far dma_setup(int Channel,char far *Buffer,unsigned Length,int Dir)   |
;| ------------------------------------------------------------------------- |
;| Channel = 0-3  !Channel 0 is often reserved for memory refresh!           |
;| Buffer  = Address of data to transfer                                     |
;| Length  = Length of data to transfer                                      |
;| Dir     = Direction to move bytes.  1 == Out to the BUS (TO the card)     |
;|                                     0 == In from the BUS and cards.       |
;| ------------------------------------------------------------------------- |
;| Returns: 0 if no errors (dma_errno == 0)                                  |
;|         -1 if errors occured (dma_errno set to indicate error.)           |
;+---------------------------------------------------------------------------+
PROC _dma_setup FAR
ARG Channel:WORD,Buffer:DWORD,Len:WORD,Dir:WORD
	push bp
	mov  bp,sp
				push bx cx dx si di
	pushf

				mov  [_dma_errno],0
;Convert seg:ofs Buffer to 20-bit physical address
;Assumes operating in 8086/real-Mode
				mov  bx,[WORD PTR Buffer]
				mov  ax,[WORD PTR Buffer+2]
	mov  cl,4
				rol  ax,cl
				mov  ch,al
				and  al,0F0h
				add  ax,bx
				adc  ch,0
				and  ch,0Fh
				mov  di,ax
; (ch << 16) + di == The physical buffer base.

;Calculate the port to receive this address
				mov  bx,[Channel]
				cmp  bx,3
				 jbe @@OkChannel
				mov  [_dma_errno],0Ch
				mov  ax,-1
				 jmp @@ExitPt
@@OkChannel:
				shl  bx,1
;bx == Port # Channel*2

;Determine which command byte will be written later
				cmp  [WORD PTR Dir],0
				 jnz SHORT @@Do_Read
				mov  al,write_cmd
				 jmp SHORT @@Do_Mode
@@Do_Read:
		mov  al,read_cmd
@@Do_Mode:
				push cx
				mov  cx,[Channel]
		add  al,cl
				zero ah
				mov  si,ax
				mov  ax,set_cmd
				add  al,cl
				pop  cx
				mov  cl,al
;si contains READ/WRITE command for DMA controller
;cl contains confirmation command for DMA controller

;-------------------------------------------------------------------------
; Calculations have been done ahead of time to minimize time with
; interrupts disabled.
;
; ch:di == physical base address
;
; cl == Confirmation command    (Tells DMA we're done bothering it.)
;
; bx == I/O port Channel*2      (This is where the address is written)
;
; si == Mode command for DMA
;-------------------------------------------------------------------------
				mov  ax,di              ;Let's check the address to see if we
				add  ax,[Len]           ;span a page boundary with our length
				 jnc @@BoundaryOk       ;Do we?
				mov  [_dma_errno],2     ; y: Error #2
				mov  ax,-1              ;    Return -1
				 jmp @@ExitPt           ;    See ya...
@@BoundaryOk:                   ; n: Continue with action
				cli                     ;Disable interrupts while mucking with DMA

;The "byte pointer" is also known as the LSB/MSB flip flop.
;By writing any value to it, the DMA controller registers are prepared
;to accept the address and length values LSB first.
				mov  dx,byte_ptr        ;Reset byte pointer Flip/flop
				out  dx,al              ;All we have to do is write to it

				mov  ax,di              ;ax=LSW of 20-bit address
				mov  dx,bx              ;dx=DMAC Base Address port
	out  dx,al              ;Store LSB
				mov  al,ah
				out  dx,al              ;Store next byte

				mov  al,ch              ;al=Page number
				mov  dx,[bx + OFFSET page_table]        ;dx=Port is the "Page index"
				out  dx,al              ;Store the page

;Write length to port Channel*2 + 1
				mov  ax,[Len]
				mov  dx,bx              ;dx=DMAC Base Adress port
				inc  dx                 ;dx=DMAC Count port (1 after Base address)
				out  dx,al              ;Write LSB of Length
				mov  al,ah
				out  dx,al              ;Write MSB

				mov  ax,si              ;Load pre-calculated mode
				mov  dx,Mode            ;dx=DMAC mode register
				out  dx,al              ;Write it to the DSP

				mov  dx,DMA_Mask        ;dx=DMAX DMA_Mask register
				mov  al,cl              ;al=pre-calulated DMA_Mask value
				out  dx,al              ;Write DMA_Mask
				mov  ax,0               ;Return with no error

@@ExitPt:                       ;Restore stack and return
				popf
				pop  di si dx cx bx
				pop  bp
				ret
ENDP _dma_setup                 

;+---------------------------------------------------------------------------+
;| int far dma_reset(int Channel)                                            |
;| ------------------------------------------------------------------------- |
;| Channel = 0-3                                                             |
;|         Resets the specified channel.                                     |
;| ------------------------------------------------------------------------- |
;| Returns 0 if Ok, -1 and sets dma_errno on error                           |
;+---------------------------------------------------------------------------+
PROC _dma_reset FAR
ARG Channel:Word
	push bp
				mov  bp,sp
				push dx
				mov  [_dma_errno],0
				cmp  [Channel],3
				 jbe @@OkChannel
				mov  [_dma_errno],0Ch
				mov  ax,-1
				 jmp @@Exit_Pt
@@OkChannel:
				mov  dx,DMA_Mask
				mov  ax,reset_cmd
				add  ax,[Channel]
				out  dx,al
				mov  ax,0
@@Exit_Pt:
				pop  dx
				pop  bp
				ret
ENDP _dma_reset

;+---------------------------------------------------------------------------+
;| int far dma_done(Channel)                                                 |
;| ------------------------------------------------------------------------- |
;| Channel = 0-4                                                             |
;| ------------------------------------------------------------------------- |
;| Returns: -1 if DMA transaction completed                                  |
;|         (Maybe it returns the number of bytes left to transfer?)          |
;| dma_errno == 0 if no error, otherwise equals error number                 |
;+---------------------------------------------------------------------------+
PROC _dma_done FAR
ARG Channel:Word
	push bp
				mov  bp,sp
				pushf
				push dx
				cmp  [Channel],3
				 jbe @@OkChannel
				mov  ax,-1
				mov  [_dma_errno],0Ch
				 jmp @@Exit_Pt
@@OkChannel:
				mov  dx,[Channel]
				shl  dx,1
				add  dx,count
				cli
				in   al,dx
				mov  ah,al
				in   al,dx
				xchg al,ah
@@Exit_Pt:
				pop  dx
				popf
				pop  bp
				ret
ENDP _dma_done
END

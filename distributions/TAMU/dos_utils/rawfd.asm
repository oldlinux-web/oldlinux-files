;*********************************************************************;
;*   rawfd.asm  - a raw floppy device driver for MS-LOSS             *;
;*                                                                   *;
;*   written by david safford (dave.safford@net.tamu.edu)            *;
;*       portions of this code were derived from:                    *;
;*           condrv.asm by Michael Tischer (overall skeleton)        *;
;*           drvload by Rick Knoblaugh     (exe based loading)       *;
;*           tapedriver by Brian Antoine   (raw device open trap)    *;
;*                                                                   *;
;*   Assembly:         TASM rawfd                                    *;
;*                     LINK rawfd                                    *;
;*                                                                   *;
;*   Usage:  add "device=rawfd.exe" to config.sys and reboot,        *;
;*           or load the driver directly with "rawfd".               *;
;*           This will create devices "rawfda" and "rawfdb" for      *;
;*           drives a and b respectively.  The driver gets initial   *;
;*           drive type information from bios.  All data sent to     *;
;*           the raw drives is simply streamed out to floppy one     *;
;*           track at a time.  If less than a track is written,      *;
;*           it is held indefinitely until more data is sent,        *;
;*           until the device is closed, or until the flush          *;
;*           ioctl is sent (by rerunning rawfd.exe).                 *;
;*           If the driver is already loaded, subsequent invocations *;
;*           will send any command line options to the driver        *;
;*           via ioctl. Command switches include 'f' (format), 'n'   *;
;*           (no format), and 'r' (reset).                           *;
;*                                                                   *;
;*********************************************************************;

code     segment
         assume cs:code,ds:code
         org 0                   

;=====================================================================
;Driver section

;---------------------------------------------------------------------;
; header rawfda

hdr_a    dw offset hdr_b         ;well documented (:-) way to link to       
         dw 0                    ;hdr_b
         dw 1110100000000000b    ;Driver attribute
         dw offset strata        ;Pointer to strategy routine
         dw offset intr          ;Pointer to interrupt routine
         db "RAWFDA  "           ;Raw Floppy driver

;---------------------------------------------------------------------;
; header rawfdb

hdr_b    dw -1,-1                ;end of driver chain
         dw 1110100000000000b    ;Driver attribute
         dw offset stratb        ;Pointer to strategy routine
         dw offset intr          ;Pointer to interrupt routine
         db "RAWFDB  "           ;Raw Floppy driver

;---------------------------------------------------------------------;
; driver function pointer table

fct_tab  dw offset init          ;Function  0: Initialization
         dw offset dummy         ;Function  1: Media check
         dw offset dummy         ;Function  2: Create BPB
         dw offset no_sup        ;Function  3: I/O control read 
         dw offset dummy         ;Function  4: Read  
         dw offset dummy         ;Function  5: Non-destructive read
         dw offset dummy         ;Function  6: Input status
         dw offset dummy         ;Function  7: Delete input buffer
         dw offset write         ;Function  8: Write 
         dw offset write         ;Function  9: Write & verify 
         dw offset dummy         ;Function 10: Output status
         dw offset flush         ;Function 11: Delete output buffer 
         dw offset ioc_wr        ;Function 12: I/O control write 
         dw offset open          ;Function 13: Open (Ver. 3.0 and up)
         dw offset close         ;Function 14: Close 
         dw offset dummy         ;Function 15: Changeable medium
         dw offset write         ;Function 16: Output until busy 

;---------------------------------------------------------------------;
; driver equates and data

cmd_fld         equ 2                 ;Offset command in req header
status          equ 3                 ;Offset status in req header
end_adr         equ 14                ;Offset driver end adr in req header
num_db          equ 18                ;Offset number in req header
b_adr           equ 14                ;Offset buf address in req header

NUM_SEC_4       equ 18                ;type 4 diskette (1.44M)
NUM_SEC_2       equ 15                ;type 2 diskette (1.2M)
BUF_SZ_4        equ 512*18            ;Size of track buf for type 4 (1.44)
BUF_SZ_2        equ 512*15            ;size of track buf for type 2 (1.2) 
num_cmd         equ 16                ;Subfunctions 0-16 are supported 


db_ptr          dw (?),(?)            ;Address of req header 
buf_start       dw (?)                ;offset to active track buffer
buf_end         dw (?)                ;offset to end of buffer
fmt             db 18*4 dup (?)       ;format track data buffer

num_cyl         db 80
num_head        db 2
num_sec_a       db NUM_SEC_4
buf_sz_a        dw BUF_SZ_4
num_sec_b       db NUM_SEC_2
buf_sz_b        dw BUF_SZ_2

do_format       db 1
cur_count       dw 0
cur_drive       db 0
cur_num_sec     db NUM_SEC_4
cur_buf_sz      dw BUF_SZ_4
cur_cyl         db 0
cur_head        db 0
tries           db 0
req_num         dw 0
req_dec         dw 0
req_off         dw 0
req_seg         dw 0
init_done       db 0

;---------------------------------------------------------------------;
; driver a strategy

strata   proc far                ;Strategy routine

         mov    cs:db_ptr,bx       ;Store address of req header in the 
         mov    cs:db_ptr+2,es     ;Variable DB_PTR 
         mov    cs:cur_drive,0     ;let writer know logical device
         mov    ah,cs:num_sec_a    ;update cur parameters
         mov    cs:cur_num_sec,ah
         mov    ax,cs:buf_sz_a
         mov    cs:cur_buf_sz,ax

         ret                     ;Return to caller 

strata   endp

;---------------------------------------------------------------------;
;driver b strategy

stratb   proc far                       ;Strategy routine

         mov    cs:db_ptr,bx            ;Store address of req header in
         mov    cs:db_ptr+2,es          ;the Variable DB_PTR 
         mov    cs:cur_drive,1          ;let writer know logical device
         mov    ah,cs:num_sec_b         ;update cur drive parameters
         mov    cs:cur_num_sec,ah
         mov    ax,cs:buf_sz_b
         mov    cs:cur_buf_sz,ax

         ret                            ;Return to caller 

stratb   endp

;---------------------------------------------------------------------;
; driver interrupt code

intr     proc   far                     ;Interrupt routine

         push   ax                      ;Push registers onto the stack 
         push   bx
         push   cx
         push   dx
         push   di
         push   si
         push   bp
         push   ds
         push   es
         pushf                          ;Push flag register onto the stack

         push   cs                      ;Set data segment register 
         pop    ds                      ;Code and data are identical 

         les    di,dword ptr db_ptr     ;Address of data req header to ES:DI
         mov    bl,es:[di+cmd_fld]      ;Get command code 
         cmp    bl,num_cmd              ;is command code permitted?
         jle    bc_ok                   ;YES --> bc_ok

         mov    ax,8003h                ;Code for "Unknown command"
         jmp    short intr_end          ;Return to caller 

bc_ok:   shl    bl,1                    ;Calculate pointer in jump table
         xor    bh,bh                   ;Clear BH 
         call   [fct_tab+bx]            ;Call function
         les    di,dword ptr cs:db_ptr  ;req header address to ES:DI

intr_end label near
         or     ax,0100h                ;Set ready bit 
         mov    es:[di+status],ax       ;Store everything in the status field 

         popf                           ;Restore flag register 
         pop    es                      ;Restore other registers 
         pop    ds
         pop    bp
         pop    si
         pop    di
         pop    dx
         pop    cx
         pop    bx
         pop    ax

         ret                            ;Return to caller 

intr     endp

;----------------------------------------------------------------------
; dummy (no error) routine

dummy    proc   near                    ;This routine does nothing 

         xor    ax,ax                   ;Clear busy bit 
         ret                            ;Return to caller 

dummy    endp

;----------------------------------------------------------------------
; dummy (returns error) routine

no_sup   proc   near                    ;for all functions
                                        ;which should not be called 
         mov    ax,8003h                ;Error: Command not recognized
         ret                            ;Return to caller 

no_sup   endp

;----------------------------------------------------------------------
;write: simply buffer chars until have a full track.

write    proc   near                   
         
         ;this is a pain in asm -- have to handle case of
         ;request larger, smaller, or equal to that available in buffer.

         ;move original request info to working variables
         mov    cx, es:[di+num_db]      ;Number of source characters 
         mov    cs:req_num, cx          ;working pointer to source data
         mov    ax, es:[di+b_adr]
         mov    cs:req_off, ax
         mov    ax, es:[di+b_adr+2]
         mov    cs:req_seg, ax

write_s: ; set up copy of data to buffer: want ds:si -> es:di
         ; for cx count, LIMITED by buf length and any current contents

         ;check that we have something to put in buffer
         mov    cx, cs:req_num
         jcxz   write_e

         ;calculate di (destination offset)
         mov    ax, cs:buf_start        ;get start of buf
         add    ax, cs:cur_count        ;add offset for already queued data
         mov    di,ax

         ;calculate cx (count for this loop)
         mov    ax, cs:cur_buf_sz       ;how big is buf
         sub    ax, cs:cur_count        ;minus queued data -> available
         cmp    ax, cx                  ;num left to write
         jnb    write_a                 ;fits within buf
         mov    cx,ax                   ;data too big, so limit to buf space
                                        ;cx now has num to write this time

write_a: mov    cs:req_dec,cx           ;save this decrement count
         mov    ds,cs:req_seg           ;point ds to source data
         mov    si,cs:req_off
         push   cs
         pop    es                      ;point es to dest buf
         cld                            ;Increment on movsb 
         rep    movsb                   ;do the move (ds:si -> es:di for cx)

         ; copy done, update cur_count, req_off, req_num
         mov    ax,cs:cur_count
         add    ax,cs:req_dec
         mov    cs:cur_count,ax
         
         mov    ax,cs:req_num
         sub    ax,cs:req_dec
         mov    cs:req_num,ax
         
         mov    ax,cs:req_off
         add    ax,cs:req_dec
         mov    cs:req_off,ax

         ;if buf full, write it and check for more
         mov    ax,cs:cur_count
         cmp    ax,cs:cur_buf_sz
         jb     write_e
         call   flush                   ;write the track
         cmp    ax,1
         jz     write_err
         jmp    write_s                 ;check for more source data

write_e: xor    ax,ax                   ;Everything O.K.
         ret                            ;Return to caller 

write_err:
         mov    ax,0ah                  ;send write error
         ret

write endp

;----------------------------------------------------------------------
; flush a track to diskette, with optional formatting

flush proc near                                    

         mov    cs:tries,0              ;reset error count
flush_s: mov    ah,cs:do_format         ;do we want to format track first?
         cmp    ah,1
         jnz    flush_w

         ; format the track first
         ; first set the media descriptor table (ddtp)
         mov    ah, 18h                 ;set media function
         mov    ch, cs:num_cyl
         dec    ch                      ;need max cyl number (79)
         mov    cl, cs:cur_num_sec
         mov    dl, cs:cur_drive
         int    13h                     ;do set media
         jc     ferror
         mov    ax,0
         mov    ds,ax
         mov    bx,1eh*4                ;bios ptr to ddtp
         mov    ds:[bx],di
         mov    ds:[bx+2],es

         ; set up fmt buffer         
         mov    ch,1                    ;sec number (1..18)
         mov    bx, offset cs:fmt
f_fmt:   mov    ah,cs:cur_cyl
         mov    cs:[bx],ah              ;cylinder
         inc    bx
         mov    ah,cs:cur_head
         mov    cs:[bx],ah              ;head
         inc    bx
         mov    cs:[bx],ch              ;sector
         inc    bx
         mov    ah,2
         mov    cs:[bx],ah              ;sector size 2 (512 bytes)
         inc    bx
         inc    ch
         mov    ah,ch
         cmp    ah,cs:cur_num_sec
         jna    f_fmt

         ;set up for format call
         push   cs
         pop    es
         mov    bx, offset cs:fmt       ;es:bx -> format buffer
         mov    dh,cs:cur_head          ;head
         mov    dl,cs:cur_drive         ;drive
         mov    ch,cs:cur_cyl           ;cylinder
         mov    al,cs:cur_num_sec       ;num sectors
         mov    ah,5                    ;format command
         int    13h                     ;do the format
         jc     ferror

flush_w: ;write the track
         mov    ah,3                    ;write command         
         mov    al,cs:cur_num_sec   
         mov    ch,cs:cur_cyl
         mov    cl,1                    ;start sector
         mov    dh,cs:cur_head
         mov    dl,cs:cur_drive
         push   cs
         pop    es
         mov    bx, cs:buf_start
         int    13h                     ;bios write track
         jnc    flush_c
          
ferror:  ;got error
         mov    ah,cs:tries
         inc    ah
         cmp    ah,3
         jz     flush_f                 ;3 errors, give up
         mov    cs:tries,ah
         mov    ah,0                    ;reset drive
         mov    dl,cs:cur_drive
         int    13h
         jmp    flush_s                 ;try again

flush_c:                                ;write complete, update pointers
         xor    ax,ax
         mov    cs:cur_count,ax         ;zero cur_count
         mov    ah,cs:cur_head      
         xor    ah,1                    ;bump head
         mov    cs:cur_head,ah
         cmp    ah,1                    ;head was 0, so don't bump cyl
         jz     flush_e
         mov    ah,cs:cur_cyl           ;head was 1 so bump cyl
         inc    ah
         cmp    ah,80
         jb     flush_d
         xor    ax,ax                   ;end of disk, zero cyl
flush_d: mov    cs:cur_cyl,ah

flush_e: 
         xor    ax,ax                   ;Everything O.K.
         ret                            ;Return to caller 

flush_f: mov    ax,1                    ;return error to write function
         ret

flush    endp

;----------------------------------------------------------------------
; driver open function

open     proc   near
         mov    cs:opened_flag,1
         mov    ax,cs:cur_count
         cmp    ax,0
         jz     open_a
         call   flush
open_a:         
         call   reset
         xor    ax,ax
         ret
open     endp

;----------------------------------------------------------------------
; driver close function

close    proc   near
         mov    cs:opened_flag,0
         mov    ax,cs:cur_count
         cmp    ax,0
         jz     close_a
         call   flush
close_a:
         call   reset
         xor    ax,ax
         ret
close    endp

;----------------------------------------------------------------------
; ioctl write driver function

ioc_wr   proc   near
         lds    si, es:[di+b_adr]       ;get ioc msg in ds:[si]
         mov    ah,ds:[si]              ;get first data char
         cmp    ah,'f'                  ;is 'f'ormat?
         jnz    ioc_wr_a
         mov    cs:do_format,1
         jmp    ioc_wr_e
ioc_wr_a:
         cmp    ah,'n'                  ;is 'n'o format ?
         jnz    ioc_wr_b
         mov    cs:do_format,0
         jmp    ioc_wr_e
ioc_wr_b:
         cmp    ah,'r'
         jnz    ioc_wr_e
         mov    ax,cs:cur_count
         cmp     ax,0
         jz     ioc_wr_c
         call   flush
ioc_wr_c:
         call   reset
ioc_wr_e:
         xor    ax,ax
         ret
ioc_wr   endp

;----------------------------------------------------------------------
; reset current write parameters

reset    proc near

         mov    cs:cur_count,0 
         mov    cs:cur_drive,0
         mov    cs:cur_cyl,0 
         mov    cs:cur_head,0

         ret
reset    endp

;----------------------------------------------------------------------
; int 21 handler to make all our opens in binary mode

orig_int_21     dd      ?                       ;Original INT 21 Vector
opened_flag     db      0

our_int_21      proc    far
                pushf                           ;Save entry flags
                cmp     ah,3Dh                  ;Is it an open request?
                jnz     not_open_req
                popf                            ;Restore entry flags

                sti                             ;Allow interrupts
                pushf                           ;After the iret
                cli                             ;Shut interrupts off
                call    cs:orig_int_21          ;While we Pass the request on

                pushf
                cli
                cmp     cs:opened_flag,0       ;Was it an open for us?
                jz      not_our_open
                mov     cs:opened_flag,0       ;Clear for next time

                push    ax
                mov     bx,ax                   ;Save the Handle
                mov     ax,4400h                ;Get Device Information
                pushf
                call    cs:orig_int_21
                mov     dh,0                    ;Setup
                or      dl,20h                  ;for RAW Mode
                mov     ax,4401h                ;Set Device Information
                pushf
                call    cs:orig_int_21
                pop     ax

not_our_open:   popf                            ;The Original Flags to return

                ret     2                       ;Return and discard flags

not_open_req:   popf                            ;Pop the saved flags
                jmp     cs:orig_int_21          ;Continue with original code
our_int_21      endp


;======================================================================
; driver initialization section 

;----------------------------------------------------------------------
; init - if loaded by config.sys

init     proc near      

         ;check if this is second call
         mov    ah,cs:init_done
         cmp    ah,0
         jnz    init_e

         ;on entry, es:di -> request header
         lds    si, es:[di+18]          ;get ds:si -> command line after '='
         mov    al,0                    ;counter to limit search

init_a:  mov    ah,ds:[si]              ;look for first switch character
         cmp    ah, ' '
         jz     init_b                  ;found a space?
         cmp    ah, 0                   ;found a null?
         jmp    init_f                  ;if so, nothing to do
         inc    si
         inc    al
         cmp    al,128                  ;we will look for 128 chars in tail
         jnz    init_a                  ;keep on looking
         jmp    init_f                  ;give up

init_b:  inc    si                      ;got a space
         mov    ah,ds:[si]              ;this is first arg
         cmp    ah,'N'                  ;dos makes the line all uppercase
         jnz    init_f                  ;if not 'N' then nothing to do
         mov    cs:do_format,0          ;got 'N', so reset do_format

init_f:  
         call   do_init                 ;method independent initialization
         call   patch_us_in             ;raw open checker

init_e:
         mov    cs:init_done,1
         ;set end address of driver -- keep init, as it is called twice
         les    di,dword ptr cs:db_ptr  ;req header address to ES:DI
         mov    ax,cs:buf_end
         mov    es:[di+end_adr],ax
         mov    es:[di+end_adr+2],cs                       
         xor    ax,ax     
         ret           
init     endp

;-------------------------------------------------------------------------
; init code common to driver or exe loading

do_init proc near
         ;get drive params
         ;default is drive a: 1.44, drive b: 1.2M
         mov    ah,8                    ;get drive type
         mov    dl,0                    ;drive a:
         int    13h
         cmp    bl,4                    ;is drive 0 type 4 (1.44M)?
         jz     do_init_a
         mov    cs:num_sec_a,NUM_SEC_2 ;not type 4 - then assume type 2
         mov    cs:buf_sz_a,BUF_SZ_2
do_init_a:  
         mov    ah,8                    ;get drive type
         mov    dl,1                    ;drive b:
         int    13h
         cmp    bl,2                    ;is drive b: type 2 (1.44M)?
         jz     do_init_b
         mov    cs:num_sec_a,NUM_SEC_4  ;not type 2 - then assume type 4
         mov    cs:buf_sz_a,BUF_SZ_4
do_init_b:
         ;setup track buf start and end offsets 
         mov    ax, offset cs:patch_end
         mov    cs:buf_start, ax
         add    ax,BUF_SZ_4
         mov    cs:buf_end,ax

         ;check if it crosses 64K DMA boundary
         mov    cl,4
         push   cs
         pop    dx
         mov    ax, cs:buf_start        ;compute buf_start's top nibble
         shr    ax,cl
         add    ax,dx
         and    ax,0f000h
         mov    bx,ax                   ;save buf_start's in bx
         mov    ax, cs:buf_end          ;compute buf_end's top nibble
         shr    ax,cl
         add    ax,dx
         and    ax,0f000h
         cmp    ax,bx                   ;compare them
         jz     do_init_e               ;same, so buf OK
         mov    ax,cs:buf_end
         mov    cs:buf_start,ax         ;first buf bad, so move to its end
         add    ax,BUF_SZ_4       
         mov    cs:buf_end,ax
do_init_e:
         ;add psp and good luck
         mov    ax,cs:buf_end
         add    ax,200h
         mov    cs:buf_end,ax
         ret

do_init endp

;---------------------------------------------------------------------
; initialization code to patch in our int 21 handler

vect_int_21     equ     word ptr 4 * 21h

patch_us_in     proc    near
                cli
                mov     ax,0                    ;Patch Ourselves into
                mov     es,ax                   ;the INT 21 Vector
                mov     ax,es:[vect_int_21]     ;Offset
                mov     word ptr cs:orig_int_21,ax
                mov     ax, offset cs:our_int_21
                mov     es:[vect_int_21],ax
                mov     ax,es:[vect_int_21+2]   ;Segment
                mov     word ptr cs:orig_int_21+2,ax
                mov     ax,cs
                mov     es:[vect_int_21+2],ax
                sti
                ret
patch_end: 
patch_us_in     endp

;======================================================================
; exe section:  load driver or send ioctl

; Equates

DOS_GET_VER     equ     30h
DOS_RELEASE_MEM equ     49h
DOS_TSR_FUNC    equ     31h
DOS_LIST_LISTS  equ     52h    ;undocumented call to get "list of lists"

; structs:
;
doub_word       struc
d_offset        dw      ?
d_segment       dw      ?
doub_word       ends

list_lists30    struc                   ;list of lists info (DOS 3.0)
dpb_ptr30       dd      ?               ;ptr 1st DOS DPB
file_tab30      dd      ?               ;ptr DOS file tables
clock_ptr30     dd      ?               ;ptr to CLOCK$ device
con_ptr30       dd      ?               ;ptr to CON device
num_blk30       db      ?               ;number block devices
max_byte30      dw      ?               ;max bytes/block
dsk_buf30       dd      ?               ;ptr 1st disk buffer
cds_ptr30       dd      ?               ;ptr to current disk struc
last_drv30      db      ?               ;LASTDRIVE value
strg_wrk30      dd      ?               ;STRING workspace area
srg_size30      dw      ?               ;size of STRING area
fcb_tab30       dd      ?               ;ptr to FCB table
fcb_y_30        dw      ?               ;y in FCBs=x,y
nul_dev30       db      18 dup(?)       ;NUL device header                                
list_lists30    ends 

list_lists31    struc                   ;list of lists info (DOS 3.1)
dpb_ptr31       dd      ?               ;ptr 1st DOS DPB
file_tab31      dd      ?               ;ptr DOS file tables
clock_ptr31     dd      ?               ;ptr to CLOCK$ device
con_ptr31       dd      ?               ;ptr to CON device
max_byte31      dw      ?               ;max bytes/block
dsk_buf31       dd      ?               ;ptr 1st disk buffer
cds_ptr31       dd      ?               ;ptr to current disk struc
fcb_tab31       dd      ?               ;ptr to FCB table
fcb_y_31        dw      ?               ;y in FCBs=x,y
num_blk31       db      ?               ;number block devices
last_drv31      db      ?               ;LASTDRIVE value
nul_dev31       db      18 dup(?)       ;NUL device header                                
num_join31      db      ?               ;number of JOINed drives
list_lists31    ends 

dev_header      struc
dev_chain       dd      ?
dev_attrib      dw      ?
dev_stratr      dw      ?
dev_intr        dw      ?
dev_num_units   db      ?               ;first byte of char name
dev_char_name   db      7 dup(?)        
dev_header      ends

ver_spec_off    struc                   ;version specific offsets
vcds_ptr        dw      ?               ;offset of CDS ptr
vdpb_ptr        dw      ?               ;offset of DPB ptr
vnul_dev_ptr    dw      ?               ;offset of NUL device header
vlast_drive     dw      ?               ;offset of LASTDRIVE
ver_spec_off    ends

; data:
;
dos_ver         dw      0               
nul_dev_ptr     dd      ?               ;pointer to NUL device

dos30_ver_off   ver_spec_off <cds_ptr30,dpb_ptr30,nul_dev30,last_drv30>
dos31_ver_off   ver_spec_off <cds_ptr31,dpb_ptr31,nul_dev31,last_drv31>

;----------------------------------------------------------------------
; entry point if run as exe

exe_start proc near
        call    do_args                 ;get first option char 
        call    do_ioc                  ;exits if drv already loaded
        call    do_init                 ;init driver
        call    patch_us_in
        call    get_list_ptr            ;find nul dev header
        call    add_to_chain            ;add rawfda and rawfdb to chain
        call    go_tsr                  ;leave driver resident
exe_start endp

;--------------------------------------------------------------
;set do_format and ioc_msg based on args

do_args  proc    near
         ;the exe program is just starting, so ds is still PSP

         ;set defaults
         mov    cs:do_format,1          ;default is format
         mov    cs:ioc_msg, byte ptr 'r'

         mov    si,82h
         mov    ah,ds:[si]
         cmp    ah,'n'
         jnz    do_args_a
         mov    cs:ioc_msg, byte ptr 'n'
         mov    cs:do_format,0
         ret

do_args_a: 
         cmp    ah,'f'
         jnz    do_args_b
         mov    cs:ioc_msg, byte ptr 'f'
         mov    cs:do_format,1
         ret
do_args_b:
         cmp    ah,'r'
         jnz    do_args_e
         mov    cs:ioc_msg, byte ptr 'r'

do_args_e:         
         ret
do_args  endp

;--------------------------------------------------------------
; see if driver is loaded, and if so, send command switch via ioctl

drv_name  db    'RAWFDA',0h
ioc_msg   db    'n'

do_ioc  proc    near

        ;try to open rawfda using file handle call
        push    cs
        pop     ds
        mov     al,2                    ;read/write access
        mov     ah,3dh                  ;open file handle
        lea     dx,drv_name             ;filename
        int     21h                     ;do it
        jc      do_ioc_e                ;failed, ret to load driver
        
        ;do ioctl write                 ;succeeded, so just send ioctl
        push    cs
        pop     ds
        mov     bx,ax                   ;handle ->bx
        mov     ah,44h                  ;ioctl
        mov     al,3                    ;write
        mov     cx,1                    ;length of write
        lea     dx,ioc_msg              ;msg buf
        int     21h                     ;doit

        ;simply exit now
        mov     ax,4c00h
        int     21h

do_ioc_e: 
        ret
do_ioc  endp

;--------------------------------------------------------------
;get_list_ptr - Use Undocumented DOS function 52h to 
;               retrieve pointer to NUL driver header.

get_list_ptr    proc    near
                mov     ax,3000h
                int     21h                         ; get dos version
                mov     cs:dos_ver,ax
                mov     ah, DOS_LIST_LISTS 
                int     21h                         ;get list of lists
                mov     di, offset cs:dos31_ver_off ;default to 3.1 >
                cmp     byte ptr cs:dos_ver, 4      ;maj ver 4 >?
                jae     get_list100
                cmp     dos_ver, 300h               ;is it 3.0?
                jne     get_list100
                mov     di, offset cs:dos30_ver_off ;offsets for 3.0
get_list100:
                mov     si, cs:[di].vnul_dev_ptr    ;offset to NUL ptr
                mov     ax, es                      ;get seg NUL dev
                mov     cs:nul_dev_ptr.d_segment, ax
                add     si, bx
                mov     cs:nul_dev_ptr.d_offset, si ;offset of NUL dev

get_list999:
                ret
get_list_ptr    endp

;--------------------------------------------------------------
;add_to_chain - Add driver to driver chain.                   
;                                                             

add_to_chain    proc    near

                lds     si, cs:nul_dev_ptr            ;point to NUL header
                mov     ax, [si].dev_chain.d_offset   ;ptr to next drvr
                mov     dx, [si].dev_chain.d_segment
                
                cli          
                ; point NUL -> hdr_a  (es:bx)
                mov     bx, offset cs:hdr_a
                mov     [si].dev_chain.d_offset, bx   ;put ours in list
                push    cs
                pop     es
                mov     [si].dev_chain.d_segment, es

                ; point hdr_a -> hdr_b (offset is already set)
                mov     es:[bx+2],es

                ; point hdr_b -> orig next dev (dx:ax)
                mov     bx, offset cs:hdr_b
                mov     es:[bx].dev_chain.d_offset, ax  ;link to 
                mov     es:[bx].dev_chain.d_segment, dx ;old 1st drvr
                sti
                ret
add_to_chain    endp

;--------------------------------------------------------------
;go_tsr - Free our environment and then terminate and stay resident

go_tsr          proc    near 
                
                mov     bx, cs:buf_end
                mov     cl,4
                shr     bx,cl
                mov     ah, DOS_RELEASE_MEM        
                push    bx
                int     21h

                pop     dx
                mov     ax, (DOS_TSR_FUNC SHL 8)        ;exit code 0      
                int     21h                             ;Go TSR
tail:           ret

go_tsr          endp        

;======================================================================

code     ends
         end exe_start

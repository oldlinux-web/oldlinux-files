# 1 "boot/boot.S"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "boot/boot.S"







BOOTSEG = 0x07c0
SYSSEG = 0x1000
SYSSIZE = 0x3000 # Max value may be 0x9000.
SYSEND = SYSSEG + SYSSIZE

# For 1.44M.
sectors = 18

.code16
.text
.global _start
_start:
        movw $BOOTSEG, %ax
        movw %ax, %ds
        movw %ax, %es

        cli
        movw %ax, %ss
        movw $0x8000, %sp # 0x8000 + 0x7c00 < 0x10000
        sti

        movb $3, %ah
        xorb %bh, %bh
        int $0x10

        movw $0x1301, %ax
        movw $0x0007, %bx
        movw $message, %bp
        movw $9, %cx
        int $0x10

        movw $SYSSEG, %ax
        movw %ax, %es

        call read_kernel
        call kill_motor

# Save the current cursor at address 0x7c00 + 510, 0x7c00 + 511.
        movb $3, %ah
        xorb %bh, %bh
        int $0x10
        movw %dx, (510)

# Now it's time to enter protected mode.'
        cli
        lidt idt_pdesc
        lgdt gdt_pdesc

# Enable a20.
        call kb_wait
        movb $0xd1, %al # command write.
        outb %al, $0x64
        call kb_wait
        movb $0xdf, %al # enable a20.
        outb %al, $0x60
        call kb_wait

# Assume all things is ok. Go on to initialize 8259.
        movb $0x11, %al # initialize sequence.
        outb %al, $0x20
        call delay
        outb %al, $0xa0
        call delay
        movb $0x20, %al # start of hardware interrupt (0x20).
        outb %al, $0x21
        call delay
        movb $0x28, %al # start of hardware interrupt (0x28).
        outb %al, $0xa1
        call delay
        movb $0x04, %al # set master.
        outb %al, $0x21
        call delay
        movb $0x02, %al # set slave.
        outb %al, $0xa1
        call delay
        movb $0x01, %al # 8086 mode.
        outb %al, $0x21
        call delay
        outb %al, $0xa1
        call delay
        movb $0xff, %al # mask all.
        outb %al, $0x21
        call delay
        outb %al, $0xa1

# Actually enter protected mode.
        movw $0x0001, %ax
        lmsw %ax
        .byte 0x66, 0xea
code386:.long 0x10000
        .word 8

kb_wait:
        .word 0x00eb, 0x00eb # Dealy
        inb $0x64, %al
        testb $0x02, %al # Keyboard buffer is full?
        jnz kb_wait # Yes, wait.
delay:
        ret

secnum: .byte sectors - 1

read_kernel:
        xorw %ax, %ax
        xorw %dx, %dx
        int $0x13

        xorw %bx, %bx
        movb $0x02, %ah
        movb secnum, %al # %al == number of sectors
        movw $0x0002, %cx # %ch == track number, %cl == sector number
        movw $0x0000, %dx # %dh == head number, %dl == drive number

repeat_read:
        call read_track

        xorb %ah, %ah # al == number of sectors which have been read
        shlw $9, %ax
        addw %ax, %bx # %bx = %bx + (just read)
        movw %bx, %ax # %ax = %bx

        orw %ax, %ax # Is %ax == 0, if it's true, we have read 64K'
        je inc_es # just have read 64K, only %ax == 0, set CF

        notw %ax # get the remaining size in one segment (64K).
        incw %ax # the same as 64K - %ax.

        shrw $9, %ax # unit from byte to 512 bytes.

        cmpw $sectors, %ax # could the remaining size hold one track ?
        jbe set_sector
        movw $sectors, %ax
        jmp set_sector

inc_es:
        call print_dot
        movw %es, %ax
        addw $0x1000, %ax
        cmpw $SYSEND, %ax
        jae end_read
        movw %ax, %es
        movw $sectors, %ax

# %al contain the max secnum next time.
# %es : %bx, points the address next time. %cx, %dx, not being changed after
# read_track.
set_sector:
        addb secnum, %cl # get the next sector to read
        decb %cl # get the number of sectors being read
                                # within this track.
        movb $sectors, secnum
        subb %cl, secnum # get remaining sectors in track.
        jne restore_cl # if remaining some, then not change head

# when reamining none, the set sector to 1, and change head
set_head_track:
        movb $sectors, secnum # change head, so set secnum to max
        movb $1, %cl # set sector
        addb %dh, %ch # if head == 1, increment track
        xorb $1, %dh # 0-->1, 1-->0
        jmp set_secnum

restore_cl:
        incb %cl

set_secnum:
        cmpb secnum, %al # get the less
        jb save_al
        movb secnum, %al
        jmp next
save_al:
        movb %al, secnum

next:
        movb $2, %ah
        jmp repeat_read

end_read:
        ret

print_dot:
        pushw %ax

        movw $0x0e2e, %ax
        int $0x10

        popw %ax
        ret

read_track:
        pushw %ax
        pushw %bx
        pushw %cx
        pushw %dx

        int $0x13 # all params have been prepared for.

        jc reset_fd

        popw %dx
        popw %cx
        popw %bx
        popw %ax
        ret

reset_fd:
        xorw %ax, %ax
        xorw %dx, %dx
        int $0x13

        popw %dx
        popw %cx
        popw %bx
        popw %ax
        jmp read_track

kill_motor:
        movw $0x3f2, %dx
        xorb %al, %al
        outb %al, %dx
        ret

gdt:
        .word 0, 0, 0, 0

        .word 0x07ff # limit == 8M (2048 * 4096)
        .word 0x0000 # base address = 0
        .word 0x9a00 # code read/exec
        .word 0x00c0 # unit = 4096, 386

        .word 0x07ff
        .word 0x0000
        .word 0x9200 # data read/write
        .word 0x00c0

idt_pdesc:
        .word 0
        .word 0, 0

gdt_pdesc:
        .word 8 * 3 - 1 # gdt limit = 24, 3 GDT entries
        .word gdt + 0x7c00, 0

message:
        .byte 0x0d, 0x0a
        .ascii "Loading"

.org 510
        .word 0xaa55

# 1 "boot/head.S"













.text
.globl _idt,_gdt,
.globl _swapper_pg_dir,_pg0
.globl _empty_bad_page
.globl _empty_bad_page_table
.globl _tmp_floppy_area,_floppy_track_buffer




startup_32:
	cld
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss _stack_start,%esp
	call setup_idt
	xorl %eax,%eax
1:	incl %eax		# check that A20 really IS enabled
	movl %eax,0x000000	# loop forever if it isn't
	cmpl %eax,0x100000
	je 1b

	movl %esp,%edi		# save stack pointer
	andl $0xfffffffc,%esp	# align stack to avoid AC fault
	pushfl			# push EFLAGS
	popl %eax		# get EFLAGS
	movl %eax,%ecx		# save original EFLAGS
	xorl $0x40000,%eax	# flip AC bit in EFLAGS
	pushl %eax		# copy to EFLAGS
	popfl			# set EFLAGS
	pushfl			# get new EFLAGS
	popl %eax		# put it in eax
	xorl %ecx,%eax		# check if AC bit is changed. zero is 486.
	jz 1f			# 486
	pushl %ecx		# restore original EFLAGS
	popfl
	movl %edi,%esp		# restore esp
	movl %cr0,%eax		# 386
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl $2,%eax		# set MP
	jmp 2f	






1:	pushl %ecx		# restore original EFLAGS
	popfl
	movl %edi,%esp		# restore esp
	movl %cr0,%eax		# 486
	andl $0x80000011,%eax	# Save PG,PE,ET
	orl $0x10022,%eax	# set NE and MP
2:	movl %eax,%cr0
	call check_x87
	jmp after_page_tables




check_x87:
	movl $0,_hard_math
	fninit
	fstsw %ax
	cmpb $0,%al
	je 1f
	movl %cr0,%eax		
	xorl $6,%eax		
	movl %eax,%cr0
	ret
.align 2
1:	movl $1,_hard_math
	.byte 0xDB,0xE4		
	ret












setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		
	movw $0x8E00,%dx	

	lea _idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	ret





.org 0x1000
_swapper_pg_dir:




.org 0x2000
_pg0:

.org 0x3000
_empty_bad_page:

.org 0x4000
_empty_bad_page_table:

.org 0x5000





_tmp_floppy_area:
	.fill 1024,1,0






_floppy_track_buffer:
	.fill 512*2*18,1,0

after_page_tables:
	call setup_paging
	lgdt gdt_descr
	lidt idt_descr
	ljmp $0x08,$1f
1:	movl $0x10,%eax		# reload all the segment registers
	mov %ax,%ds		# after changing gdt.
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss _stack_start,%esp
	pushl $0		# These are the parameters to main :-)
	pushl $0
	pushl $0
	cld			# gcc2 wants the direction flag cleared at all times
	call _start_kernel
L6:
	jmp L6			# main should never return here, but
				# just in case, we know what happens.


int_msg:
	.asciz "Unknown interrupt\n\r"
.align 2
ignore_int:
	cld
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	pushl $int_msg
	call _printk
	popl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret





























.align 2
setup_paging:
	movl $1024*2,%ecx		
	xorl %eax,%eax
	movl $_swapper_pg_dir,%edi	
	cld;rep;stosl

	movl $_pg0+7,_swapper_pg_dir		

	movl $_pg0+7,_swapper_pg_dir+3072	
	movl $_pg0+4092,%edi
	movl $0x03ff007,%eax		
	std
1:	stosl			
	subl $0x1000,%eax
	jge 1b
	cld
	movl $_swapper_pg_dir,%eax
	movl %eax,%cr3			
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		
	ret			




.align 4
.word 0
idt_descr:
	.word 256*8-1		# idt contains 256 entries
	.long 0xc0000000+_idt

.align 4
_idt:
	.fill 256,8,0		# idt is uninitialized




.align 4
.word 0
gdt_descr:
	.word 256*8-1
	.long 0xc0000000+_gdt





.align 4
_gdt:
	.quad 0x0000000000000000	
	.quad 0xc0c39a000000ffff	
	.quad 0xc0c392000000ffff	
	.quad 0x0000000000000000	
	.fill 252,8,0			

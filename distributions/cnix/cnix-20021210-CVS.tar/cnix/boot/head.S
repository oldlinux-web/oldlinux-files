/* head.S
 * This code will check a20, and move itself from 0x10000 to 0x100000. Then
 * setup idt table and gdt table, and set kernel page dir and kernel page table.
 * But only low 8M is paged. After have done all things, it jump to main. This
 * code still run under disabled interrupt.
 */
 
.text
.global idt, gdt, kp_dir
kp_dir:
.global on_journey
on_journey:
	movl	$0x10, %eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %fs
	movl	%eax, %gs

	xorl	%eax, %eax
check_a20:
	incl	%eax
	movl	%eax, 0x000000
	cmpl	%eax, 0x100000
	je	check_a20
	
do_move:
	cld	
	movl	$0x24000, %ecx		# max value is 0x90000 / 4
	movl	$0x10000, %esi
	movl	$0x100000, %edi
	rep
	movsl
	
	movl	$go_here, %eax
	jmp	*%eax
	
go_here:
	lss	stack_start, %esp
	
	call	setup_idt
	call	setup_gdt
	
	movl	$0x10, %eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %fs
	movl	%eax, %gs
	lss	stack_start, %esp

	# first we clear the screen
#	cld
#	movl	$0x07200720, %eax
#	movl	$0xb8000, %edi
#	movl	$0x1000, %ecx		# 4 x 0x1000 = 16K
#	rep
#	stosl

	# update the cursor position, row = 0, col = 1
#	movw	$0x0, 0x7dfe
#	movw	$0x0, 0x7c00 + 510

	# Then we will clear the bss region, set static var to 0
	cld
	movl	$__bss_start, %edi
	movl	$_end, %ecx
	subl	%edi, %ecx	# %ecx is the bss length
	xorl	%eax, %eax
	rep
	stosb

	movl 	%cr0, %eax		# check math chip
	andl	$0x80000011, %eax	# save PG, ET, PE
					# 31	4  3  2  1  0
					# PG... ET TS EM MP PE
	testl	$0x10, %eax		# ET is set if having 387
	jne	have_387
	orl	$0x04, %eax		# emulate by software
have_387:
	movl	%eax, %cr0
	
setup_env_for_main:
	pushl	$0			# params for main
	pushl	$0		
	pushl	$0			
	pushl	$hang			# ret addr for main
	pushl	$main			
	jmp	setup_paging
	
# idt descriptor
# 7-6             5 - 4	     3-2 1-0
# offset(31...16) Attributes sel offset(15...0)
setup_idt:
	lea 	ignore_int, %edx
	movl 	$0x00080000, %eax	# sel = 0x0008 == cs
	movw	%dx, %ax			
	movw	$0x8E00, %dx		# interrupt gate - dpl = 0, present
					# the high part in %edx contains ign...

	movl	$idt, %edi
	movl	$256, %ecx
rp_sidt:
	movl	%eax, (%edi)
	movl	%edx, 4(%edi)
	addl	$8, %edi
	decl 	%ecx
	jne	rp_sidt
	lidt	idt_pdesc
	ret

setup_gdt:
	lgdt	gdt_pdesc
	ret

	.align 2
idt_pdesc:
	.word 256 * 8 - 1		# idt contains 256 entries
	.long idt	

	.align 2
gdt_pdesc:
	.word 256 * 8 - 1		# But I think we must large it.
	.long gdt			# Or only use one ldt and tss 
					# in our system. 

# code in front of here will be ...
.org 0x1000
kp_0:

.org 0x2000
kp_1:

.org 0x3000
kp_2:

hang:
	jmp hang

	.align 2
ignore_int:
	incb 0xb8000 + 160		# put something on the screen
	movb $2, 0xb8000 + 161		# this would let us know something 
	iret				# happened

setup_paging:
	cld
	movl	$kp_dir, %edi
	xorl	%eax, %eax
	movl	$1024 * 3, %ecx
	rep
	stosl

	movl	$kp_0 + 7, kp_dir	
	movl	$kp_1 + 7, kp_dir + 4

	movl	$kp_1 + 4092, %edi
	movl	$0x007ff007, %eax	#  8M - 4096 + 7 (r/w u/s, p)
	std
fill_pg:
	stosl
	subl	$0x1000, %eax
	jge	fill_pg
	movl	$kp_dir, %eax
	movl	%eax, %cr3
	movl	%cr0, %eax
	orl	$0x80000000, %eax
	movl	%eax, %cr0
	ret
	
	.align 8
idt:	
	.fill 256, 8, 0

	.align 8
gdt:
	.word 0, 0, 0, 0
	
	# kernel cs 0x08
	.word 0x3fff	# base: 0, limit: 64M
	.word 0x0000
	.word 0x9a00
	.word 0x00c0

	# kernel ds 0x10
	.word 0x3fff	# base: 0, limit: 64M
	.word 0x0000
	.word 0x9200
	.word 0x00c0

	# user cs 0x1b
#	.word 0x3fff	# base: 64M, limit: 64M
#	.word 0x0000
#	.word 0xfa00	 
#	.word 0x04c0

	# user ds 0x23
#	.word 0x3fff	# base: 64M, limit: 64M
#	.word 0x0000
#	.word 0xf200
#	.word 0x04c0

	# only for testing shell, signal, will be deleted 
	
	# user cs 0x1b
	.word 0x3fff	# base: 0, limit: 64M
	.word 0x0000
	.word 0xfa00	 
	.word 0x00c0

	# user ds 0x23
	.word 0x3fff	# base: 0, limit: 64M
	.word 0x0000
	.word 0xf200
	.word 0x00c0

	# tss descriptor
	.word 0, 0, 0, 0

	# for ldt, one each process

	.fill 250, 8, 0	# not necessary to have so many

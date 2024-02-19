/*
 *  linux/kernel/asm.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * asm.s contains the low-level code for most hardware faults.
 * page_exception is handled by the mm, so that isn't here. This
 * file also handles (hopefully) fpu-exceptions due to TS-bit, as
 * the fpu must be properly saved/resored. This hasn't been tested.
 */

.globl _divide_error,_debug,_nmi,_int3,_overflow,_bounds,_invalid_op
.globl _double_fault,_coprocessor_segment_overrun
.globl _invalid_TSS,_segment_not_present,_stack_segment
.globl _general_protection,_coprocessor_error,_irq13,_reserved
.globl _alignment_check
.globl _page_fault

_divide_error:
	pushl $0 		# no error code
	pushl $_do_divide_error
error_code:
	push %fs
	push %es
	push %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	cld
	movl $-1, %eax
	xchgl %eax, 0x2c(%esp)	# orig_eax (get the error code. )
	xorl %ebx,%ebx		# zero ebx
	mov %gs,%bx		# get the lower order bits of gs
	xchgl %ebx, 0x28(%esp)	# get the address and save gs.
	pushl %eax		# push the error code
	lea 52(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%ebx
	addl $8,%esp
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop %ds
	pop %es
	pop %fs
	pop %gs
	addl $4,%esp
	iret

_debug:
	pushl $0
	pushl $_do_int3		# _do_debug
	jmp error_code

_nmi:
	pushl $0
	pushl $_do_nmi
	jmp error_code

_int3:
	pushl $0
	pushl $_do_int3
	jmp error_code

_overflow:
	pushl $0
	pushl $_do_overflow
	jmp error_code

_bounds:
	pushl $0
	pushl $_do_bounds
	jmp error_code

_invalid_op:
	pushl $0
	pushl $_do_invalid_op
	jmp error_code

_coprocessor_segment_overrun:
	pushl $0
	pushl $_do_coprocessor_segment_overrun
	jmp error_code

_reserved:
	pushl $0
	pushl $_do_reserved
	jmp error_code

_irq13:
	pushl %eax
	xorb %al,%al
	outb %al,$0xF0
	movb $0x20,%al
	outb %al,$0x20
	jmp 1f
1:	jmp 1f
1:	outb %al,$0xA0
	popl %eax
	jmp _coprocessor_error

_double_fault:
	pushl $_do_double_fault
	jmp error_code

_invalid_TSS:
	pushl $_do_invalid_TSS
	jmp error_code

_segment_not_present:
	pushl $_do_segment_not_present
	jmp error_code

_stack_segment:
	pushl $_do_stack_segment
	jmp error_code

_general_protection:
	pushl $_do_general_protection
	jmp error_code

_alignment_check:
	pushl $_do_alignment_check
	jmp error_code

_page_fault:
	pushl $_do_page_fault
	jmp error_code

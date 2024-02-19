/*
 *  linux/kernel/sys_call.S
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * sys_call.S  contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 * I changed all the .align's to 4 (16 byte alignment), as that's faster
 * on a 486.
 *
 * Stack layout in 'ret_from_system_call':
 * 	ptrace needs to have all regs on the stack.
 *	if the order here is changed, it needs to be 
 *	updated in fork.c:copy_process, signal.c:do_signal,
 *	ptrace.c and ptrace.h
 *
 *	 0(%esp) - %ebx
 *	 4(%esp) - %ecx
 *	 8(%esp) - %edx
 *       C(%esp) - %esi
 *	10(%esp) - %edi
 *	14(%esp) - %ebp
 *	18(%esp) - %eax
 *	1C(%esp) - %ds
 *	20(%esp) - %es
 *      24(%esp) - %fs
 *	28(%esp) - %gs
 *	2C(%esp) - orig_eax
 *	30(%esp) - %eip
 *	34(%esp) - %cs
 *	38(%esp) - %eflags
 *	3C(%esp) - %oldesp
 *	40(%esp) - %oldss
 */

EBX		= 0x00
ECX		= 0x04
EDX		= 0x08
ESI		= 0x0C
EDI		= 0x10
EBP		= 0x14
EAX		= 0x18
DS		= 0x1C
ES		= 0x20
FS		= 0x24
GS		= 0x28
ORIG_EAX	= 0x2C
EIP		= 0x30
CS		= 0x34
EFLAGS		= 0x38
OLDESP		= 0x3C
OLDSS		= 0x40

IF_MASK		= 0x00000200
NT_MASK		= 0x00004000
VM_MASK		= 0x00020000

/*
 * these are offsets into the task-struct.
 */
state		= 0
counter		= 4
priority	= 8
signal		= 12
sigaction	= 16		# MUST be 16 (=len of sigaction)
blocked		= (33*16)
saved_kernel_stack = ((33*16)+4)

/*
 * offsets within sigaction
 */
sa_handler	= 0
sa_mask		= 4
sa_flags	= 8
sa_restorer	= 12

ENOSYS = 38

.globl _system_call,_sys_execve
.globl _device_not_available, _coprocessor_error
.globl _divide_error,_debug,_nmi,_int3,_overflow,_bounds,_invalid_op
.globl _double_fault,_coprocessor_segment_overrun
.globl _invalid_TSS,_segment_not_present,_stack_segment
.globl _general_protection,_reserved
.globl _alignment_check,_page_fault
.globl ret_from_sys_call

#define SAVE_ALL \
	cld; \
	push %gs; \
	push %fs; \
	push %es; \
	push %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $0x10,%edx; \
	mov %dx,%ds; \
	mov %dx,%es; \
	movl $0x17,%edx; \
	mov %dx,%fs

.align 4
reschedule:
	pushl $ret_from_sys_call
	jmp _schedule
.align 4
_system_call:
	pushl %eax			# save orig_eax
	SAVE_ALL
	movl $-ENOSYS,EAX(%esp)
	cmpl _NR_syscalls,%eax
	jae ret_from_sys_call
	call _sys_call_table(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
	.align 4,0x90
ret_from_sys_call:
	movl EFLAGS(%esp),%eax		# check VM86 flag: CS/SS are
	testl $VM_MASK,%eax		# different then
	jne 4f
	cmpw $0x0f,CS(%esp)		# was old code segment supervisor ?
	jne 2f
	cmpw $0x17,OLDSS(%esp)		# was stack segment = 0x17 ?
	jne 2f
4:	orl $IF_MASK,%eax		# these just try to make sure
	andl $~NT_MASK,%eax		# the program doesn't do anything
	movl %eax,EFLAGS(%esp)		# stupid
1:	cmpl $0,_need_resched
	jne reschedule
	movl _current,%eax
	cmpl _task,%eax			# task[0] cannot have signals
	je 2f
	cmpl $0,state(%eax)		# state
	jne reschedule
	cmpl $0,counter(%eax)		# counter
	je reschedule
	movl signal(%eax),%ebx
	movl blocked(%eax),%ecx
	notl %ecx
	andl %ebx,%ecx
	bsfl %ecx,%ecx
	je 2f
	btrl %ecx,%ebx
	incl %ecx
	movl %ebx,signal(%eax)
	movl %esp,%ebx
	testl $VM_MASK,EFLAGS(%esp)
	je 3f
	pushl %ebx
	pushl %ecx
	call _save_v86_state
	popl %ecx
	movl %eax,%ebx
	movl %eax,%esp
3:	pushl %ebx
	pushl %ecx
	call _do_signal
	popl %ecx
	popl %ebx
	testl %eax, %eax
	jne 1b			# see if we need to switch tasks, or do more signals
2:	popl %ebx
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
	addl $4,%esp 		# skip the orig_eax
	iret

.align 4
_sys_execve:
	lea (EIP+4)(%esp),%eax  # don't forget about the return address.
	pushl %eax
	call _do_execve
	addl $4,%esp
	ret

.align 4
_divide_error:
	pushl $0 		# no error code
	pushl $_do_divide_error
.align 4,0x90
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
	xchgl %eax, ORIG_EAX(%esp)	# orig_eax (get the error code. )
	xorl %ebx,%ebx			# zero ebx
	mov %gs,%bx			# get the lower order bits of gs
	xchgl %ebx, GS(%esp)		# get the address and save gs.
	pushl %eax			# push the error code
	lea 52(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	movl $0x17,%edx
	mov %dx,%fs
	call *%ebx
	addl $8,%esp
	jmp ret_from_sys_call

.align 4
_coprocessor_error:
	pushl $0
	pushl $_do_coprocessor_error
	jmp error_code

.align 4
_device_not_available:
	pushl $-1		# mark this as an int
	SAVE_ALL
	pushl $ret_from_sys_call
	clts				# clear TS so that we can use math
	movl %cr0,%eax
	testl $0x4,%eax			# EM (math emulation bit)
	je _math_state_restore
	pushl $0		# temporary storage for ORIG_EIP
	call _math_emulate
	addl $4,%esp
	ret

.align 4
_debug:
	pushl $0
	pushl $_do_debug
	jmp error_code

.align 4
_nmi:
	pushl $0
	pushl $_do_nmi
	jmp error_code

.align 4
_int3:
	pushl $0
	pushl $_do_int3
	jmp error_code

.align 4
_overflow:
	pushl $0
	pushl $_do_overflow
	jmp error_code

.align 4
_bounds:
	pushl $0
	pushl $_do_bounds
	jmp error_code

.align 4
_invalid_op:
	pushl $0
	pushl $_do_invalid_op
	jmp error_code

.align 4
_coprocessor_segment_overrun:
	pushl $0
	pushl $_do_coprocessor_segment_overrun
	jmp error_code

.align 4
_reserved:
	pushl $0
	pushl $_do_reserved
	jmp error_code

.align 4
_double_fault:
	pushl $_do_double_fault
	jmp error_code

.align 4
_invalid_TSS:
	pushl $_do_invalid_TSS
	jmp error_code

.align 4
_segment_not_present:
	pushl $_do_segment_not_present
	jmp error_code

.align 4
_stack_segment:
	pushl $_do_stack_segment
	jmp error_code

.align 4
_general_protection:
	pushl $_do_general_protection
	jmp error_code

.align 4
_alignment_check:
	pushl $_do_alignment_check
	jmp error_code

.align 4
_page_fault:
	pushl $_do_page_fault
	jmp error_code

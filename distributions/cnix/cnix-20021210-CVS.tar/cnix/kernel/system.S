/* #include <errno.h> */


need_sched = 0

nr_syscall = 256
ENOSYS = 38
EFLAGS	=	48
CS	=	44
EIP	=	40
INDEX	=	36
ES	=	32
DS	=	28
EAX	=	24
EBP	=	20
EDI	=	16
ESI 	=	12
EDX	=	8
ECX	=	4
EBX	=	0

#define SAVE_SPOT	\
	cld;		\
	pushl	%es;	\
	pushl	%ds;	\
	pushl	%eax;	\
	pushl	%ebp;	\
	pushl	%edi;	\
	pushl	%esi;	\
	pushl	%edx;	\
	pushl	%ecx;	\
	pushl	%ebx;	\
	movl	$0x10, %edx;	\
	movl	%edx, %ds;	\
	movl	%edx, %es;	\
	
#define RESTORE_SPOT	\
	popl	%ebx;	\
	popl	%ecx;	\
	popl	%edx;	\
	popl	%esi;	\
	popl	%edi;	\
	popl	%ebp;	\
	popl	%eax;	\
	popl	%ds;	\
	popl	%es;	\

.global system_call

.global hw_int0, hw_int1, hw_int2, hw_int3, hw_int4, hw_int5, hw_int6, hw_int7
.global hw_int8, hw_int9, hw_int10, hw_int11, hw_int12, hw_int13, hw_int14, 
.global hw_int15

.global divide_err, debug, nmi, int3, overflow, bounds, invalid_op
.global device_not_avl, double_fault, coprocessor_segment_overrun, invalid_tss
.global segment_not_present, stack_segment, general_protection, page_fault
.global coprocessor_fault

.global ret_from_syscall
	
	.align 2
bad_syscall:
	movl	$-ENOSYS, EAX(%esp)
	jmp	ret_from_syscall
	
	.align 2
system_call:
	pushl	%eax
	SAVE_SPOT	
	cmpl	$nr_syscall - 1, %eax
	ja	bad_syscall
	call	*syscall_table(, %eax, 4)
	movl	%eax, EAX(%esp)

ret_from_syscall:

/* But I think not necessary to check, because ret_from_syscall, must return
 * to user state. :-) But when call from kernel, ...
 */

/*	movl	CS(%esp), %eax
	testl	$0x03, %eax
	je	restore_all
*/

ret_with_reschedule:
	movl	current, %ebx
	cmpl	$0, need_sched(%ebx)
	jne	reschedule

	/* do something to check signal, if have, then jmp do_with_signal */

restore_all:
	RESTORE_SPOT	
	addl	$4, %esp
	iret

reschedule:
	call	schedule
	
	/* why not ret_from_intr, the same to ret_from_syscall */

	jmp	ret_from_syscall
	
ret_from_intr:
	/* assume nested interrupt will not happen */
	/* movl	CS(%esp), %eax
	testl	$0x03, %eax */
	jne	ret_with_reschedule
	jmp	restore_all
	
	.align 2
hw_int0:
	pushl	$0
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int1:
	pushl	$1
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int2:
	pushl	$2
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int3:
	pushl	$3
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int4:
	pushl	$4
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int5:
	pushl	$5
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int6:
	pushl	$6
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int7:
	pushl	$7
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int8:
	pushl	$8
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int9:
	pushl	$9
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int10:
	pushl	$10
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int11:
	pushl	$11
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int12:
	pushl	$12
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int13:
	pushl	$13
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int14:
	pushl	$14
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr

	.align 2
hw_int15:
	pushl	$15
	SAVE_SPOT
	call	do_with_irq
	jmp ret_from_intr
	
	.align 2
divide_err:
	pushl	$0
	pushl	$0
	jmp	execption
	
	.align 2
debug:
	pushl	$0
	pushl	$1
	jmp	execption
	
	.align 2
nmi:
	pushl	$0
	pushl	$2
	jmp	execption
	
	.align 2
int3:
	pushl	$0
	pushl	$3
	jmp	execption
	
	.align 2
overflow:
	pushl	$0
	pushl	$4
	jmp	execption

	.align 2
bounds:
	pushl	$0
	pushl	$5
	jmp	execption

	.align 2
invalid_op:
	pushl	$0
	pushl	$6
	jmp	execption
	
	.align 2
device_not_avl:
	pushl	$0
	pushl	$7
	jmp	execption
	
	.align 2
double_fault:
	pushl	$8
	jmp	execption
	
	.align 2
coprocessor_segment_overrun:
	pushl	$0
	pushl	$9
	jmp	execption
	
	.align 2
invalid_tss:
	pushl	$10
	jmp	execption
	
	.align 2
segment_not_present:
	pushl	$11
	jmp	execption
	
	.align 2
stack_segment:
	pushl	$12
	jmp	execption
	
	.align 2
general_protection:
	pushl	$13
	jmp	execption
	
	.align 2
page_fault:	
	pushl	$14
	jmp	execption
	
	.align 2
coprocessor_fault:
	pushl	$0
	pushl	$15
	jmp	execption
	

execption:
	cld		
	pushl	%ds
	pushl	%eax
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	movl	$0x10, %eax
	movl	%eax, %ds
	call	do_with_execption
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	popl	%ebp
	popl	%eax
	popl	%ds
	addl	$8, %esp
	iret

.data
.global syscall_table
syscall_table:
	.long	sys_null_call
	.long	sys_null_call
	.long	sys_fork
	.rept	64
		.long	sys_null_call
	.endr
	.long	sys_printf
	.rept nr_syscall - 68
		.long	sys_null_call
	.endr

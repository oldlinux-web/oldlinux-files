/*
 *  linux/kernel/traps.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'Traps.c' handles hardware traps and faults after we have saved some
 * state in 'asm.s'. Currently mostly a debugging-aid, will be extended
 * to mainly kill the offending process (probably by giving it a signal,
 * but possibly by killing it outright if necessary).
 */
#include <linux/head.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>

#include <asm/system.h>
#include <asm/segment.h>
#include <asm/io.h>

#define get_seg_byte(seg,addr) ({ \
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define get_seg_long(seg,addr) ({ \
register unsigned long __res; \
__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define _fs() ({ \
register unsigned short __res; \
__asm__("mov %%fs,%%ax":"=a" (__res):); \
__res;})

void page_exception(void);

void divide_error(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
void page_fault(void);
void coprocessor_error(void);
void reserved(void);
void alignment_check(void);

/*static*/ void die_if_kernel(char * str,long esp_ptr,long nr)
{
	long * esp = (long *) esp_ptr;
	int i;

	if ((esp[2] & VM_MASK) || ((0xffff & esp[1]) == 0xf))
		return;
	printk("%s: %04x\n\r",str,nr&0xffff);
	printk("EIP:    %04x:%p\nEFLAGS: %p\n", 0xffff & esp[1],esp[0],esp[2]);
	printk("fs: %04x\n",_fs());
	printk("base: %p, limit: %p\n",get_base(current->ldt[1]),get_limit(0x17));
	str(i);
	printk("Pid: %d, process nr: %d\n\r",current->pid,0xffff & i);
	for(i=0;i<10;i++)
		printk("%02x ",0xff & get_seg_byte(esp[1],(i+(char *)esp[0])));
	printk("\n\r");
	do_exit(SIGSEGV);
}

void do_double_fault(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("double fault",esp,error_code);
}

void do_general_protection(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("general protection",esp,error_code);
}

void do_alignment_check(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("alignment check",esp,error_code);
}

void do_divide_error(long esp, long error_code)
{
	send_sig(SIGFPE, current, 1);
	die_if_kernel("divide error",esp,error_code);
}

void do_int3(long esp, long error_code)
{
	send_sig(SIGTRAP, current, 1);
	die_if_kernel("int3",esp,error_code);
}

void do_nmi(long esp, long error_code)
{
	printk("Uhhuh. NMI received. Dazed and confused, but trying to continue\n");
}

void do_debug(long esp, long error_code)
{
	send_sig(SIGTRAP, current, 1);
	die_if_kernel("debug",esp,error_code);
}

void do_overflow(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("overflow",esp,error_code);
}

void do_bounds(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("bounds",esp,error_code);
}

void do_invalid_op(long esp, long error_code)
{
	send_sig(SIGILL, current, 1);
	die_if_kernel("invalid operand",esp,error_code);
}

void do_device_not_available(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("device not available",esp,error_code);
}

void do_coprocessor_segment_overrun(long esp, long error_code)
{
	send_sig(SIGFPE, last_task_used_math, 1);
	die_if_kernel("coprocessor segment overrun",esp,error_code);
}

void do_invalid_TSS(long esp,long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("invalid TSS",esp,error_code);
}

void do_segment_not_present(long esp,long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("segment not present",esp,error_code);
}

void do_stack_segment(long esp,long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("stack segment",esp,error_code);
}

void do_coprocessor_error(long esp, long error_code)
{
	send_sig(SIGFPE, last_task_used_math, 1);
	__asm__("fnclex");
}

void do_reserved(long esp, long error_code)
{
	send_sig(SIGSEGV, current, 1);
	die_if_kernel("reserved (15,17-47) error",esp,error_code);
}

void trap_init(void)
{
	int i;

	set_trap_gate(0,&divide_error);
	set_trap_gate(1,&debug);
	set_trap_gate(2,&nmi);
	set_system_gate(3,&int3);	/* int3-5 can be called from all */
	set_system_gate(4,&overflow);
	set_system_gate(5,&bounds);
	set_trap_gate(6,&invalid_op);
	set_trap_gate(7,&device_not_available);
	set_trap_gate(8,&double_fault);
	set_trap_gate(9,&coprocessor_segment_overrun);
	set_trap_gate(10,&invalid_TSS);
	set_trap_gate(11,&segment_not_present);
	set_trap_gate(12,&stack_segment);
	set_trap_gate(13,&general_protection);
	set_trap_gate(14,&page_fault);
	set_trap_gate(15,&reserved);
	set_trap_gate(16,&coprocessor_error);
	set_trap_gate(17,&alignment_check);
	for (i=18;i<48;i++)
		set_trap_gate(i,&reserved);
}

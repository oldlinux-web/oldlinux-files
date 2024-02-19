#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

/*
 *	linux/include/asm/irq.h
 *
 *	(C) 1992 Linus Torvalds
 */
 
#define SAVE_ALL \
	"cld\n\t" \
	"push %gs\n\t" \
	"push %fs\n\t" \
	"push %es\n\t" \
	"push %ds\n\t" \
	"pushl %eax\n\t" \
	"pushl %ebp\n\t" \
	"pushl %edi\n\t" \
	"pushl %esi\n\t" \
	"pushl %edx\n\t" \
	"pushl %ecx\n\t" \
	"pushl %ebx\n\t" \
	"movl $0x10,%edx\n\t" \
	"mov %dx,%ds\n\t" \
	"mov %dx,%es\n\t" \
	"movl $0x17,%edx\n\t" \
	"mov %dx,%fs\n\t"

/*
 * SAVE_MOST/RESTORE_MOST is used for the faster version of IRQ handlers,
 * installed by using the SA_INTERRUPT flag. These kinds of IRQ's don't
 * call the routines that do signal handling etc on return, and can have
 * more relaxed register-saving etc. They are also atomic, and are thus
 * suited for small, fast interrupts like the serial lines or the harddisk
 * drivers, which don't actually need signal handling etc.
 *
 * Also note that we actually save only those registers that are used in
 * C subroutines (%eax, %edx and %ecx), so if you do something weird,
 * you're on your own. The only segments that are saved (not counting the
 * automatic stack and code segment handling) are %ds and %es, and they
 * point to kernel space. No messing around with %fs here.
 */
#define SAVE_MOST \
	"cld\n\t" \
	"push %es\n\t" \
	"push %ds\n\t" \
	"pushl %eax\n\t" \
	"pushl %edx\n\t" \
	"pushl %ecx\n\t" \
	"movl $0x10,%edx\n\t" \
	"mov %dx,%ds\n\t" \
	"mov %dx,%es\n\t"

#define RESTORE_MOST \
	"popl %ecx\n\t" \
	"popl %edx\n\t" \
	"popl %eax\n\t" \
	"pop %ds\n\t" \
	"pop %es\n\t" \
	"iret"

#define ACK_FIRST(mask) \
	"inb $0x21,%al\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\torb $" #mask ",%al\n\t" \
	"outb %al,$0x21\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\tmovb $0x20,%al\n\t" \
	"outb %al,$0x20\n\t"

#define ACK_SECOND(mask) \
	"inb $0xA1,%al\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\torb $" #mask ",%al\n\t" \
	"outb %al,$0xA1\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\tmovb $0x20,%al\n\t" \
	"outb %al,$0xA0\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\toutb %al,$0x20\n\t"

#define UNBLK_FIRST(mask) \
	"inb $0x21,%al\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\tandb $~(" #mask "),%al\n\t" \
	"outb %al,$0x21\n\t"

#define UNBLK_SECOND(mask) \
	"inb $0xA1,%al\n\t" \
	"jmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:\tandb $~(" #mask "),%al\n\t" \
	"outb %al,$0xA1\n\t"

#define IRQ_NAME2(nr) nr##_interrupt()
#define IRQ_NAME(nr) IRQ_NAME2(IRQ##nr)
#define FAST_IRQ_NAME(nr) IRQ_NAME2(fast_IRQ##nr)
#define BAD_IRQ_NAME(nr) IRQ_NAME2(bad_IRQ##nr)
	
#define BUILD_IRQ(chip,nr,mask) \
void IRQ_NAME(nr); \
void FAST_IRQ_NAME(nr); \
void BAD_IRQ_NAME(nr); \
__asm__( \
"\n.align 2\n" \
"_IRQ" #nr "_interrupt:\n\t" \
	"pushl $-"#nr"-2\n\t" \
	SAVE_ALL \
	ACK_##chip(mask) \
	"sti\n\t" \
	"movl %esp,%ebx\n\t" \
	"pushl %ebx\n\t" \
	"pushl $" #nr "\n\t" \
	"call _do_IRQ\n\t" \
	"addl $8,%esp\n\t" \
	"testl %eax,%eax\n\t" \
	"jne ret_from_sys_call\n\t" \
	"cli\n\t" \
	UNBLK_##chip(mask) \
	"jmp ret_from_sys_call\n" \
"\n.align 2\n" \
"_fast_IRQ" #nr "_interrupt:\n\t" \
	SAVE_MOST \
	ACK_##chip(mask) \
	"pushl $" #nr "\n\t" \
	"call _do_fast_IRQ\n\t" \
	"addl $4,%esp\n\t" \
	"testl %eax,%eax\n\t" \
	"jne 2f\n\t" \
	"cli\n\t" \
	UNBLK_##chip(mask) \
	"\n2:\t" \
	RESTORE_MOST \
"\n\n.align 2\n" \
"_bad_IRQ" #nr "_interrupt:\n\t" \
	"pushl %eax\n\t" \
	ACK_##chip(mask) \
	"popl %eax\n\t" \
	"iret");

#endif

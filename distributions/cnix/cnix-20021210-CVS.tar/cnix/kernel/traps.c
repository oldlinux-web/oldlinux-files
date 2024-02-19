#include <asm/regs.h>
#include <asm/system.h>
#include <cnix/head.h>

extern void divide_err(void);
extern void debug(void);
extern void nmi(void);
extern void int3(void);
extern void overflow(void);
extern void bounds(void);
extern void invalid_op(void);
extern void device_not_avl(void);
extern void double_fault(void);
extern void coprocessor_segment_overrun(void);
extern void invalid_tss(void);
extern void segment_not_present(void);
extern void stack_segment(void);
extern void general_protection(void);
extern void page_fault(void);
extern void coprocessor_fault(void);
extern void system_call(void);

static void reserved(void)
{
	panic("error: idt is reserved.\n");
}

void traps_init(void)
{
	int index;
	
	set_trap_gate(0, divide_err);
	set_trap_gate(1, debug);
	set_trap_gate(2, nmi);
	/* ...... */
	set_trap_gate(3, int3);
	set_trap_gate(4, overflow);
	set_trap_gate(5, bounds);

	set_trap_gate(6, invalid_op);
	set_trap_gate(7, device_not_avl);
	set_trap_gate(8, double_fault);
	set_trap_gate(9, coprocessor_segment_overrun);
	set_trap_gate(10, invalid_tss);
	set_trap_gate(11, segment_not_present);
	set_trap_gate(12, stack_segment);
	set_trap_gate(13, general_protection);
	set_trap_gate(14, page_fault);
	set_trap_gate(15, coprocessor_fault);

	for(index = 16; index < 32; index++)
		set_trap_gate(index, reserved);
	
	/* when call, does it auto-cli */
	set_system_gate(0x80, &system_call);	
}

void do_with_execption(struct regs_t regs)
{
	if(regs.es == 14)
		panic("page_fault\n");

	panic("execption %d err_code %x cs: %x eip: %x\n", regs.es, regs.index,
			regs.cs, regs.eip);
}

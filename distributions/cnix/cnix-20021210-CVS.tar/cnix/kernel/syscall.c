#include <errno.h>
#include <stdarg.h>
#include <asm/regs.h>

static char buf[1024];

int sys_fork(struct regs_t regs)
{
	int i, ret;

	i = find_empty_process();
	if(i < 0)
		return i;
	
	ret = copy_process(i, regs);
	
	if(ret < 0)
		return ret;

	return i;
}

int sys_printf(const char * string)
{
	puts(string);
	return 0;
}

void sys_null_call(void)
{
	printk("calling sys_null_call...\n");
}

#include <unistd.h>
#include <const.h>
#include <stdarg.h>
#include <asm/io.h>
#include <asm/regs.h>
#include <asm/system.h>
#include <cnix/sched.h>
#include <cnix/cfile.h>
#include <string.h>

/* Do you understand why to use volatile */
extern void vga_init(void);
extern void intr_init(void);
extern void traps_init(void);
extern void timer_init(void);
extern void ide_init(void);
extern void mem_init(void);
extern void buf_init(void);
extern void fs_init(void);
extern void sched_init(void);
extern void sleep_on(struct wait_queue **p);
extern void wake_up(struct wait_queue **p); 

int errno;	
_syscall0(int, fork)

int main(void)		
{	
	/* Note!!! traps_init and intr_init must be executed first. */
	traps_init();
	intr_init();

	/* Now it's ok to enable interrupt. */
	sti();	
	
	vga_init();
	puts("Cnix beta version!\n");
	/*for sample alloc memory,need fix!*/
	cnix_malloc_init();
	mem_init();
	timer_init();
	/* I think ide will  use delay function which will be supported by 
	 * timer, so after being laid timer_init.
	 */
	ide_init();
	sched_init();
	fs_init();

	if(!fork()){
		shell_init();
	}	

	while(1);
}

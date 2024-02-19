/*
 *  linux/init/main.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <stdarg.h>

#include <asm/system.h>
#include <asm/io.h>

#include <linux/mktime.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/head.h>
#include <linux/unistd.h>
#include <linux/string.h>

extern unsigned long * prof_buffer;
extern unsigned long prof_len;
extern int end;
extern char *linux_banner;

/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */
static inline _syscall0(int,idle)
static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall1(int,setup,void *,BIOS)
static inline _syscall0(int,sync)
static inline _syscall0(pid_t,setsid)
static inline _syscall3(int,write,int,fd,const char *,buf,off_t,count)
static inline _syscall1(int,dup,int,fd)
static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp)
static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
static inline _syscall1(int,close,int,fd)
static inline _syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)

static inline pid_t wait(int * wait_stat)
{
	return waitpid(-1,wait_stat,0);
}

static char printbuf[1024];

extern int vsprintf();
extern void init(void);
extern void init_IRQ(void);
extern long blk_dev_init(long,long);
extern long chr_dev_init(long,long);
extern void floppy_init(void);
extern void sock_init(void);
extern long rd_init(long mem_start, int length);
extern long kernel_mktime(struct mktime * time);
extern unsigned long simple_strtoul(const char *cp,char **endp,unsigned int
    base);

#ifdef CONFIG_SCSI
extern unsigned long scsi_dev_init(unsigned long, unsigned long);
#endif

/*
 * This is set up by the setup-routine at boot-time
 */
#define EXT_MEM_K (*(unsigned short *)0x90002)
#define DRIVE_INFO (*(struct drive_info *)0x90080)
#define SCREEN_INFO (*(struct screen_info *)0x90000)
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)
#define AUX_DEVICE_INFO (*(unsigned char *)0x901FF)

/*
 * Boot command-line arguments
 */
#define MAX_INIT_ARGS 8
#define MAX_INIT_ENVS 8
#define CL_MAGIC_ADDR (*(unsigned short *) 0x90020)
#define CL_MAGIC 0xa33f
#define CL_BASE_ADDR ((char *) 0x90000)
#define CL_OFFSET (*(unsigned short *) 0x90022)

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void time_init(void)
{
	struct mktime time;

	do {
		time.sec = CMOS_READ(0);
		time.min = CMOS_READ(2);
		time.hour = CMOS_READ(4);
		time.day = CMOS_READ(7);
		time.mon = CMOS_READ(8);
		time.year = CMOS_READ(9);
	} while (time.sec != CMOS_READ(0));
	BCD_TO_BIN(time.sec);
	BCD_TO_BIN(time.min);
	BCD_TO_BIN(time.hour);
	BCD_TO_BIN(time.day);
	BCD_TO_BIN(time.mon);
	BCD_TO_BIN(time.year);
	time.mon--;
	startup_time = kernel_mktime(&time);
}

static unsigned long memory_start = 0; /* After mem_init, stores the */
				       /* amount of free user memory */
static unsigned long memory_end = 0;
static unsigned long low_memory_start = 0;

static char * argv_init[MAX_INIT_ARGS+2] = { "/bin/init", NULL, };
static char * envp_init[MAX_INIT_ENVS+2] = { "HOME=/", "TERM=console", NULL, };

static char * argv_rc[] = { "/bin/sh", NULL };
static char * envp_rc[] = { "HOME=/", "TERM=console", NULL };

static char * argv[] = { "-/bin/sh",NULL };
static char * envp[] = { "HOME=/usr/root", "TERM=console", NULL };

struct drive_info { char dummy[32]; } drive_info;
struct screen_info screen_info;

unsigned char aux_device_present;

static char command_line[80] = { 0, };

/*
 * This is a simple kernel command line parsing function: it parses
 * the command line, and fills in the arguments/environment to init
 * as appropriate. Any cmd-line option is taken to be an environment
 * variable if it contains the character '='.
 *
 *
 * This routine also checks for options meant for the kernel - currently
 * only the "root=XXXX" option is recognized. These options are not given
 * to init - they are for internal kernel use only.
 */
static void parse_options(char *line)
{
	char *next;
	int args, envs;

	if (!*line)
		return;
	args = 0;
	envs = 1;	/* TERM is set to 'console' by default */
	next = line;
	while (line = next) {
		if (next = strchr(line,' '))
			*next++ = 0;
		/*
		 * check for kernel options first..
		 */
		if (!strncmp(line,"root=",5)) {
			ROOT_DEV = simple_strtoul(line+5,NULL,16);
			continue;
		}
		/*
		 * Then check if it's an environment variable or
		 * an option.
		 */	
		if (strchr(line,'=')) {
			if (envs >= MAX_INIT_ENVS)
				break;
			envp_init[++envs] = line;
		} else {
			if (args >= MAX_INIT_ARGS)
				break;
			argv_init[++args] = line;
		}
	}
	argv_init[args+1] = NULL;
	envp_init[envs+1] = NULL;
}

void start_kernel(void)
{
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
 	ROOT_DEV = ORIG_ROOT_DEV;
 	drive_info = DRIVE_INFO;
 	screen_info = SCREEN_INFO;
	aux_device_present = AUX_DEVICE_INFO;
	memory_end = (1<<20) + (EXT_MEM_K<<10);
	memory_end &= 0xfffff000;
#ifdef MAX_16M
	if (memory_end > 16*1024*1024)
		memory_end = 16*1024*1024;
#endif
	memory_start = 1024*1024;
	low_memory_start = (unsigned long) &end;
	low_memory_start += 0xfff;
	low_memory_start &= 0xfffff000;
	memory_start = paging_init(memory_start,memory_end);
	if (CL_MAGIC_ADDR == CL_MAGIC)
		strcpy(command_line,CL_BASE_ADDR+CL_OFFSET);
	trap_init();
	init_IRQ();
	sched_init();
	parse_options(command_line);
#ifdef PROFILE_SHIFT
	prof_buffer = (unsigned long *) memory_start;
	prof_len = (unsigned long) &end;
	prof_len >>= PROFILE_SHIFT;
	memory_start += prof_len * sizeof(unsigned long);
#endif
	memory_start = chr_dev_init(memory_start,memory_end);
	memory_start = blk_dev_init(memory_start,memory_end);
#ifdef CONFIG_SCSI
	memory_start = scsi_dev_init(memory_start,memory_end);
#endif
	mem_init(low_memory_start,memory_start,memory_end);
	buffer_init();
	inode_init();
	time_init();
	floppy_init();
	sock_init();
	sti();
	move_to_user_mode();
	if (!fork())		/* we count on this going ok */
		init();
/*
 * task[0] is meant to be used as an "idle" task: it may not sleep, but
 * it might do some general things like count free pages or it could be
 * used to implement a reasonable LRU algorithm for the paging routines:
 * anything that can be useful, but shouldn't take time from the real
 * processes.
 *
 * Right now task[0] just does a infinite idle loop.
 */
	for(;;)
		idle();
}

static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

void init(void)
{
	int pid,i;

	setup((void *) &drive_info);
	(void) open("/dev/tty1",O_RDWR,0);
	(void) dup(0);
	(void) dup(0);

	printf(linux_banner);
	execve("/etc/init",argv_init,envp_init);
	execve("/bin/init",argv_init,envp_init);
	/* if this fails, fall through to original stuff */

	if (!(pid=fork())) {
		close(0);
		if (open("/etc/rc",O_RDONLY,0))
			_exit(1);
		execve("/bin/sh",argv_rc,envp_rc);
		_exit(2);
	}
	if (pid>0)
		while (pid != wait(&i))
			/* nothing */;
	while (1) {
		if ((pid=fork())<0) {
			printf("Fork failed in init\r\n");
			continue;
		}
		if (!pid) {
			close(0);close(1);close(2);
			setsid();
			(void) open("/dev/tty1",O_RDWR,0);
			(void) dup(0);
			(void) dup(0);
			_exit(execve("/bin/sh",argv,envp));
		}
		while (1)
			if (pid == wait(&i))
				break;
		printf("\n\rchild %d died with code %04x\n\r",pid,i);
		sync();
	}
	_exit(0);	/* NOTE! _exit, not exit() */
}

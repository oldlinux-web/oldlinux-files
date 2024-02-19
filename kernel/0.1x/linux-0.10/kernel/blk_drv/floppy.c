/*
 *  linux/kernel/floppy.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * As with hd.c, all routines within this file can (and will) be called
 * by interrupts, so extreme caution is needed. A hardware interrupt
 * handler may not sleep, or a kernel panic will happen. Thus I cannot
 * call "floppy-on" directly, but have to set a special timer interrupt
 * etc.
 *
 * Also, I'm not certain this works on more than 1 floppy. Bugs may
 * abund.
 */

#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/fdreg.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>

#define MAJOR_NR 2
#include "blk.h"

static void reset_floppy(void);
static void seek_interrupt(void);
static void rw_interrupt(void);

extern unsigned char current_DOR;
extern unsigned char selected;

#define immoutb_p(val,port) \
__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (val)),"i" (port))

#define TYPE(x) ((x)>>2)
#define DRIVE(x) ((x)&0x03)
/*
 * Note that MAX_ERRORS=10 doesn't imply that we retry every bad read
 * max 10 times - some types of errors increase the errorcount by 2,
 * so we might actually retry only 6-7 times before giving up.
 */
#define MAX_ERRORS 10

/*
 * globals used by 'result()'
 */
#define MAX_REPLIES 7
static unsigned char reply_buffer[MAX_REPLIES];
#define ST0 (reply_buffer[0])
#define ST1 (reply_buffer[1])
#define ST2 (reply_buffer[2])
#define ST3 (reply_buffer[3])

/*
 * This struct defines the different floppy types. Unlike minix
 * linux doesn't have a "search for right type"-type, as the code
 * for that is convoluted and weird.
 *
 * The 'stretch' tells if the tracks need to be boubled for some
 * types (ie 360kB diskette in 1.2MB drive etc). Others should
 * be self-explanatory.
 */
static struct floppy_struct {
	int size, sect, head, track, stretch;
	unsigned char gap,rate,spec1;
} floppy_type[] = {
	{    0, 0,0, 0,0,0x00,0x00,0x00 },	/* no testing */
	{  720, 9,2,40,0,0x2A,0x02,0xDF },	/* 360kB PC diskettes */
	{ 2400,15,2,80,0,0x1B,0x00,0xDF },	/* 1.2 MB AT-diskettes */
	{  720, 9,2,40,1,0x2A,0x02,0xDF },	/* 360kB in 720kB drive */
	{ 1440, 9,2,80,0,0x2A,0x02,0xDF },	/* 3.5" 720kB diskette */
	{  720, 9,2,40,1,0x23,0x01,0xDF },	/* 360kB in 1.2MB drive */
	{ 1440, 9,2,80,0,0x23,0x01,0xDF },	/* 720kB in 1.2MB drive */
	{ 2880,18,2,80,0,0x1B,0x00,0xCF },	/* 1.44MB diskette */
};
/*
 * Rate is 0 for 500kb/s, 2 for 300kbps, 1 for 250kbps
 * Spec1 is 0xSH, where S is stepping rate (F=1ms, E=2ms, D=3ms etc),
 * H is head unload time (1=16ms, 2=32ms, etc)
 *
 * Spec2 is (HLD<<1 | ND), where HLD is head load time (1=2ms, 2=4 ms etc)
 * and ND is set means no DMA. Hardcoded to 6 (HLD=6ms, use DMA).
 */

extern void floppy_interrupt(void);
extern char tmp_floppy_area[1024];

/*
 * These are global variables, as that's the easiest way to give
 * information to interrupts. They are the data used for the current
 * request.
 */
static int cur_spec1 = -1;
static int cur_rate = -1;
static struct floppy_struct * floppy = floppy_type;
static unsigned char current_drive = 0;
static unsigned char sector = 0;
static unsigned char head = 0;
static unsigned char track = 0;
static unsigned char seek_track = 0;
static unsigned char command = 0;

/*
 * floppy-change is never called from an interrupt, so we can relax a bit
 * here.
 */
int floppy_change(unsigned int nr)
{
	floppy_on(nr);
	floppy_select(nr);
	if (inb(FD_DIR) & 0x80) {
		floppy_off(nr);
		return 1;
	}
	floppy_off(nr);
	return 0;
}

#define copy_buffer(from,to) \
__asm__("cld ; rep ; movsl" \
	::"c" (BLOCK_SIZE/4),"S" ((long)(from)),"D" ((long)(to)) \
	:"cx","di","si")

static void setup_DMA(void)
{
	long addr = (long) CURRENT->buffer;

	if (addr >= 0x100000) {
		addr = (long) tmp_floppy_area;
		if (command == FD_WRITE)
			copy_buffer(CURRENT->buffer,tmp_floppy_area);
	}
/* mask DMA 2 */
	immoutb_p(4|2,10);
/* output command byte. I don't know why, but everyone (minix, */
/* sanches & canton) output this twice, first to 12 then to 11 */
 	__asm__("outb %%al,$12\n\tjmp 1f\n1:\tjmp 1f\n1:\t"
	"outb %%al,$11\n\tjmp 1f\n1:\tjmp 1f\n1:"::
	"a" ((char) ((command == FD_READ)?DMA_READ:DMA_WRITE)));
/* 8 low bits of addr */
	immoutb_p(addr,4);
	addr >>= 8;
/* bits 8-15 of addr */
	immoutb_p(addr,4);
	addr >>= 8;
/* bits 16-19 of addr */
	immoutb_p(addr,0x81);
/* low 8 bits of count-1 (1024-1=0x3ff) */
	immoutb_p(0xff,5);
/* high 8 bits of count-1 */
	immoutb_p(3,5);
/* activate DMA 2 */
	immoutb_p(0|2,10);
}

static void output_byte(char byte)
{
	int counter;
	unsigned char status;

	for(counter = 0 ; counter < 10000 ; counter++) {
		status = inb(FD_STATUS) & (STATUS_READY | STATUS_DIR);
		if (status == STATUS_READY) {
			outb(byte,FD_DATA);
			return;
		}
	}
	printk("Unable to send byte to FDC\n\r");
}

static int result(void)
{
	int i = 0, counter, status;

	for (counter = 0 ; counter < 10000 ; counter++) {
		status = inb(FD_STATUS)&(STATUS_DIR|STATUS_READY|STATUS_BUSY);
		if (status == STATUS_READY)
			return i;
		if (status == (STATUS_DIR|STATUS_READY|STATUS_BUSY)) {
			if (i >= MAX_REPLIES)
				break;
			reply_buffer[i++] = inb(FD_DATA);
		}
	}
	printk("Getstatus times out\n\r");
	return -1;
}

/*
 * This is the routine called after every seek (or recalibrate) interrupt
 * from the floppy controller. Note that the "unexpected interrupt" routine
 * also does a recalibrate, but doesn't come here.
 */
static void seek_interrupt(void)
{
/* sense drive status */
	output_byte(FD_SENSEI);
	if (result() != 2 || (ST0 & 0xF8) != 0x20) {
		CURRENT->errors++;
		if (CURRENT->errors > MAX_ERRORS) {
			floppy_deselect(current_drive);
			end_request(0);
			reset_floppy();
			return;
		}
		output_byte(FD_RECALIBRATE);
		output_byte(head<<2 | current_drive);
		return;
	}
/* are we on the right track? */
	if (ST1 != seek_track) {
		CURRENT->errors++;
		if (CURRENT->errors > MAX_ERRORS) {
			floppy_deselect(current_drive);
			end_request(0);
			reset_floppy();
			return;
		}
		output_byte(FD_SEEK);
		output_byte(head<<2 | current_drive);
		output_byte(seek_track);
		return;
	}
/* yes - set up DMA and read/write command */
	setup_DMA();
	do_floppy = rw_interrupt;
	output_byte(command);
	output_byte(head<<2 | current_drive);
	output_byte(track);
	output_byte(head);
	output_byte(sector);
	output_byte(2);		/* sector size = 512 */
	output_byte(floppy->sect);
	output_byte(floppy->gap);
	output_byte(0xFF);	/* sector size (0xff when n!=0 ?) */
}

/*
 * Ok, this interrupt is called after a DMA read/write has succeeded,
 * so we check the results, and copy any buffers.
 */
static void rw_interrupt(void)
{
	if (result() != 7 || (ST0 & 0xf8) || (ST1 & 0xbf) ||
	    (ST2 & 0x73)) {
		CURRENT->errors++;
		if (CURRENT->errors > MAX_ERRORS || (ST1 & 0x02)) {
			if (ST1 & 0x02)
				printk("Drive %d is write protected\n\r",
				current_drive);
			floppy_deselect(current_drive);
			end_request(0);
			do_fd_request();
			return;
		}
		do_floppy = seek_interrupt;
		output_byte(FD_RECALIBRATE);
		output_byte(head<<2 | current_drive);
		return;
	}
	if (command == FD_READ && (long)(CURRENT->buffer) >= 0x100000)
		copy_buffer(tmp_floppy_area,CURRENT->buffer);
	floppy_deselect(current_drive);
	end_request(1);
	do_fd_request();
}

/*
 * This routine is called when everything should be correctly set up
 * for the transfer (ie floppy motor is on and the correct floppy is
 * selected).
 */
static void transfer(void)
{
	if (cur_spec1 != floppy->spec1) {
		cur_spec1 = floppy->spec1;
		output_byte(FD_SPECIFY);
		output_byte(cur_spec1);		/* hut etc */
		output_byte(6);			/* Head load time =6ms, DMA */
	}
	if (cur_rate != floppy->rate)
		outb_p(cur_rate = floppy->rate,FD_DCR);
	do_floppy = seek_interrupt;
	if (seek_track) {
		output_byte(FD_SEEK);
		output_byte(head<<2 | current_drive);
		output_byte(seek_track);
	} else {
		output_byte(FD_RECALIBRATE);
		output_byte(head<<2 | current_drive);
	}
}

/*
 * Special case - used after a unexpected interrupt (or reset)
 */
static void recal_interrupt(void)
{
	do_floppy = NULL;
	output_byte(FD_SENSEI);
	if (result()!=2 || (ST0 & 0xE0) == 0x60) {
		reset_floppy();
		return;
	}
	do_fd_request();
}

void unexpected_floppy_interrupt(void)
{
	output_byte(FD_SENSEI);
	if (result()!=2 || (ST0 & 0xE0) == 0x60) {
		reset_floppy();
		return;
	}
	do_floppy = recal_interrupt;
	output_byte(FD_RECALIBRATE);
	output_byte(head<<2 | current_drive);
}

static void reset_interrupt(void)
{
	output_byte(FD_SENSEI);
	(void) result();
	do_floppy = recal_interrupt;
	output_byte(FD_RECALIBRATE);
	output_byte(head<<2 | current_drive);
}

static void reset_floppy(void)
{
	printk("Reset-floppy called\n\r");
	do_floppy = reset_interrupt;
	outb_p(0,FD_DOR);
	outb(current_DOR,FD_DOR);
}

static void floppy_on_interrupt(void)
{
/* We cannot do a floppy-select, as that might sleep. We just force it */
	selected = 1;
	current_DOR &= 0xFC;
	current_DOR |= current_drive;
	transfer();
}

void do_fd_request(void)
{
	unsigned int block;

	INIT_REQUEST;
	floppy = (MINOR(CURRENT->dev)>>2) + floppy_type;
	current_drive = CURRENT_DEV;
	block = CURRENT->sector;
	if (block+2 > floppy->size) {
		end_request(0);
		goto repeat;
	}
	sector = block % floppy->sect;
	block /= floppy->sect;
	head = block % floppy->head;
	track = block / floppy->head;
	seek_track = track << floppy->stretch;
	sector++;
	if (CURRENT->cmd == READ)
		command = FD_READ;
	else if (CURRENT->cmd == WRITE)
		command = FD_WRITE;
	else
		panic("do_fd_request: unknown command");
	add_timer(ticks_to_floppy_on(current_drive),&floppy_on_interrupt);
}

void floppy_init(void)
{
	blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
	set_intr_gate(0x26,&floppy_interrupt);
	outb(inb_p(0x21)&~0x40,0x21);
}

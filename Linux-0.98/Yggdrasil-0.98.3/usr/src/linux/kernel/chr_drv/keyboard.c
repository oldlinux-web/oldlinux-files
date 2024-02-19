/*
 * linux/kernel/chr_drv/keyboard.c
 *
 * Keyboard driver for Linux v0.96 using Latin-1.
 *
 * Written for linux by Johan Myreen as a translation from
 * the assembly version by Linus (with diacriticals added)
 */

#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/tty.h>
#include <linux/mm.h>
#include <linux/ptrace.h>

/*
 * The default IO slowdown is doing 'inb()'s from 0x61, which should be
 * safe. But as that is the keyboard controller chip address, we do our
 * slowdowns here by doing short jumps: the keyboard controller should
 * be able to keep up
 */
#define REALLY_SLOW_IO
#define SLOW_IO_BY_JUMPING
#include <asm/io.h>
#include <asm/system.h>

#define LSHIFT		0x01
#define RSHIFT		0x02
#define LCTRL		0x04
#define RCTRL		0x08
#define ALT		0x10
#define ALTGR		0x20
#define CAPS		0x40
#define CAPSDOWN	0x80

#define SCRLED		0x01
#define NUMLED		0x02
#define CAPSLED		0x04

#define NO_META_BIT 0x80

#ifndef KBD_DEFLOCK
#define KBD_DEFLOCK NUMLED
#endif

unsigned char kapplic = 0;
unsigned char ckmode = 0;
unsigned char krepeat = 1;
unsigned char kmode = 0;
unsigned char default_kleds = KBD_DEFLOCK;
unsigned char kleds = KBD_DEFLOCK;
unsigned char ke0 = 0;
unsigned char kraw = 0;
unsigned char kbd_flags = KBDFLAGS;
unsigned char lfnlmode = 0;

extern void do_keyboard_interrupt(void);
extern void ctrl_alt_del(void);
extern void change_console(unsigned int new_console);
extern struct tty_queue *table_list[];

typedef void (*fptr)(int);

static unsigned char old_leds = NUMLED;
static int diacr = -1;
static int npadch = 0;
fptr key_table[];

static void put_queue(int);
void set_leds(void);
static void applkey(int);
static void cur(int);
static void kb_wait(void), kb_ack(void);
static unsigned int handle_diacr(unsigned int);

static struct pt_regs * pt_regs;

void keyboard_interrupt(int int_pt_regs)
{
	static unsigned char rep = 0xff, repke0 = 0;
	unsigned char scancode, x;
	struct tty_struct * tty = TTY_TABLE(0);

	pt_regs = (struct pt_regs *) int_pt_regs;
	scancode=inb_p(0x60);
	x=inb_p(0x61);
	outb_p(x|0x80, 0x61);
	outb_p(x&0x7f, 0x61);
	outb(0x20, 0x20);
	sti();

	if (kraw) {
		put_queue(scancode);
		do_keyboard_interrupt();
		return;
	}
	if (scancode == 0xe0) {
		ke0 = 1;
		return;
	}
	if (scancode == 0xe1) {
		ke0 = 2;
		return;
	}
	/*
	 *  The keyboard maintains its own internal caps lock and num lock
	 *  statuses. In caps lock mode E0 AA precedes make code and E0 2A
	 *  follows break code. In num lock mode, E0 2A precedes make
	 *  code and E0 AA follows break code. We do our own book-keeping,
	 *  so we will just ignore these.
	 */
	if (ke0 == 1 && (scancode == 0x2a || scancode == 0xaa)) {
		ke0 = 0;
		return;
	}
	/*
	 *  Repeat a key only if the input buffers are empty or the
	 *  characters get echoed locally. This makes key repeat usable
	 *  with slow applications and unders heavy loads.
	 */
	if (rep == 0xff) {
		if (scancode < 0x80) {
			rep = scancode;
			repke0 = ke0;
		}
	} else if (ke0 == repke0 && (scancode & 0x7f) == rep) {
		if (scancode & 0x80)
			rep = 0xff;
		else if (!(krepeat && tty &&
			   (L_ECHO(tty) ||
			    (EMPTY(&tty->secondary) &&
			     EMPTY(&tty->read_q))))) {
			ke0 = 0;
			return;
		}
	}
	key_table[scancode](scancode);
	do_keyboard_interrupt();
	ke0 = 0;
}

static void put_queue(int ch)
{
	struct tty_queue *qp;
	struct tty_struct *tty = TTY_TABLE(0);
	unsigned long new_head;

	wake_up(&keypress_wait);
	if (!tty)
		return;
	qp = &tty->read_q;

	qp->buf[qp->head]=ch;
	if ((new_head=(qp->head+1)&(TTY_BUF_SIZE-1)) != qp->tail)
		qp->head=new_head;
	wake_up(&qp->proc_list);
}

static void puts_queue(char *cp)
{
	struct tty_queue *qp;
	struct tty_struct *tty = TTY_TABLE(0);
	unsigned long new_head;
	char ch;

	wake_up(&keypress_wait);
	if (!tty)
		return;
	qp = &tty->read_q;

	while (ch=*cp++) {
		qp->buf[qp->head]=ch;
		if ((new_head=(qp->head+1)&(TTY_BUF_SIZE-1))
				 != qp->tail)
			qp->head=new_head;
	}
	wake_up(&qp->proc_list);
}

static void ctrl(int sc)
{
	if (ke0)
		kmode|=RCTRL;
	else
		kmode|=LCTRL;
}

static void alt(int sc)
{
	if (ke0)
		kmode|=ALTGR;
	else
		kmode|=ALT;
}

static void unctrl(int sc)
{
	if (ke0)
		kmode&=(~RCTRL);
	else
		kmode&=(~LCTRL);
}

static void unalt(int sc)
{
	if (ke0)
		kmode&=(~ALTGR);
	else {
		kmode&=(~ALT);
		if (npadch != 0) {
			put_queue(npadch);
			npadch=0;
		}
	}
}

static void lshift(int sc)
{
	kmode|=LSHIFT;
}

static void unlshift(int sc)
{
	kmode&=(~LSHIFT);
}

static void rshift(int sc)
{
	kmode|=RSHIFT;
}

static void unrshift(int sc)
{
	kmode&=(~RSHIFT);
}

static void caps(int sc)
{
	if (!(kmode & CAPSDOWN)) {
		kleds ^= CAPSLED;
		kmode ^= CAPS;
		kmode |= CAPSDOWN;
		set_leds();
	}
}

void set_leds(void)
{
	if (kleds != old_leds) {
		old_leds = kleds;
		kb_wait();
		outb(0xed, 0x60);	/* set leds command */
		kb_ack();
		kb_wait();
		outb(kleds, 0x60);
		kb_ack();
	}
}

static void uncaps(int sc)
{
	kmode &= ~CAPSDOWN;
}

static void show_ptregs(void)
{
	printk("\nEIP: %04x:%08x",0xffff & pt_regs->cs,pt_regs->eip);
	if (pt_regs->cs & 3)
		printk(" ESP: %04x:%08x",0xffff & pt_regs->cs,pt_regs->eip);
	printk(" EFLAGS: %08x",pt_regs->eflags);
	printk("\nEAX: %08x EBX: %08x ECX: %08x EDX: %08x",
		pt_regs->orig_eax,pt_regs->ebx,pt_regs->ecx,pt_regs->edx);
	printk("\nESI: %08x EDI: %08x EBP: %08x",
		pt_regs->esi, pt_regs->edi, pt_regs->ebp);
	printk(" DS: %04x ES: %04x FS: %04x GS: %04x\n",
		0xffff & pt_regs->ds,0xffff & pt_regs->es,
		0xffff & pt_regs->fs,0xffff & pt_regs->gs);
}

static void scroll(int sc)
{
	if (kmode & (LSHIFT | RSHIFT))
		show_mem();
	else if (kmode & (ALT | ALTGR))
		show_ptregs();
	else if (kmode & (LCTRL | RCTRL))
		show_state();
	kleds ^= SCRLED;
	set_leds();
}

static void num(int sc)
{
	if (kapplic)
		applkey(0x50);
	else {
		kleds ^= NUMLED;
		set_leds();
	}
}

static void applkey(int key)
{
	char buf[] = { 0x1b, 0x4f, 0x00, 0x00 };

	buf[2] = key;
	puts_queue(buf);
}

#if defined KBD_FINNISH

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '+', '\'',  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '}',    0,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  '|',
	'{',    0,    0, '\'',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!', '\"',  '#',  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  ']',  '^',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L', '\\',
	'[',    0,    0,  '*',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  163,  '$',    0,    0,
	'{',   '[',  ']', '}', '\\',    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_FINNISH_LATIN1

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '+',  180,  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  229,  168,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  246,
	228,  167,    0, '\'',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!',  '"',  '#',  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  197,  '^',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  214,
	196,  189,    0,  '*',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  163,  '$',    0,    0,
	'{',  '[',  ']',  '}', '\\',    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_US

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '-',  '=',  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '[',  ']',   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	'\'', '`',    0, '\\',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '/',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!',  '@',  '#',  '$',  '%',  '^',
	'&',  '*',  '(',  ')',  '_',  '+',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  '{',  '}',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
	'"',  '~',  '0',  '|',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  '<',  '>',  '?',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',    0,  '$',    0,    0,
	'{',   '[',  ']', '}', '\\',    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_UK

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '-',  '=',  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '[',  ']',   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	'\'', '`',    0,  '#',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '/',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0, '\\',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!',  '"',  163,  '$',  '%',  '^',
	'&',  '*',  '(',  ')',  '_',  '+',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  '{',  '}',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
	'@',  '~',  '0',  '~',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  '<',  '>',  '?',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',    0,  '$',    0,    0,
	'{',   '[',  ']', '}', '\\',    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_GR

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', '\\', '\'',  127,    9,
	'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
	'o',  'p',  '@',  '+',   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  '[',
	']',  '^',    0,  '#',  'y',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!',  '"',  '#',  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
	'O',  'P', '\\',  '*',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  '{',
	'}',  '~',    0, '\'',  'Y',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',    0,  '$',    0,    0,
	'{',   '[',  ']', '}', '\\',    0,    0,    0,
	'@',    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_GR_LATIN1

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', 223,  180,  127,    9,
	'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
	'o',  'p',  252,  '+',   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l', 246,
	228,   94,    0,  '#',  'y',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!',  '"',  167,  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
	'O',  'P',  220,  '*',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  214,
	196,  176,    0, '\'',  'Y',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  178,  179,  '$',    0,    0,
	'{',   '[',  ']', '}', '\\',    0,    0,    0,
	'@',    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  181,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_FR

static unsigned char key_map[] = {
	  0,   27,  '&',  '{',  '"', '\'',  '(',  '-',
	'}',  '_',  '/',  '@',  ')',  '=',  127,    9,
	'a',  'z',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '^',  '$',   13,    0,  'q',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  'm',
	'|',  '`',    0,   42,  'w',  'x',  'c',  'v',
	'b',  'n',  ',',  ';',  ':',  '!',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  ']',  '+',  127,    9,
	'A',  'Z',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  '<',  '>',   13,    0,  'Q',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  'M',
	'%',  '~',    0,  '#',  'W',  'X',  'C',  'V',
	'B',  'N',  '?',  '.',  '/', '\\',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '~',  '#',  '{',  '[',  '|',
	'`', '\\',   '^',  '@', ']',  '}',    0,    0,
	'@',    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_FR_LATIN1

static unsigned char key_map[] = {
	  0,   27,  '&',  233,  '"', '\'',  '(',  '-',
	232,  '_',  231,  224,  ')',  '=',  127,    9,
	'a',  'z',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '^',  '$',   13,    0,  'q',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  'm',
	249,  178,    0,   42,  'w',  'x',  'c',  'v',
	'b',  'n',  ',',  ';',  ':',  '!',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  176,  '+',  127,    9,
	'A',  'Z',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  168,  163,   13,    0,  'Q',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  'M',
	'%',    0,    0,  181,  'W',  'X',  'C',  'V',
	'B',  'N',  '?',  '.',  '/',  167,    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '~',  '#',  '{',  '[',  '|',
	'`', '\\',   '^',  '@', ']',  '}',    0,    0,
	'@',    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  164,   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_DK

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '+', '\'',  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  229,    0,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  230,
	162,    0,    0, '\'',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!', '\"',  '#',  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  197,  '^',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  198,
	165,    0,    0,  '*',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  163,  '$',    0,    0,
	'{',   '[',  ']', '}',    0,  '|',    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '\\',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_DK_LATIN1

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '+',  180,  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  229,  168,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  230,
	162,  189,    0, '\'',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!', '\"',  '#',  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  197,  '^',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  198,
	165,  167,    0,  '*',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  163,  '$',    0,    0,
	'{',   '[',  ']', '}',    0,  '|',    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0, '\\',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_DVORAK

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', '\\',  '=',  127,    9,
	'\'', ',',  '.',  'p',  'y',  'f',  'g',  'c',
	'r',  'l',  '/',  ']',   13,    0,  'a',  'o',
	'e',  'u',  'i',  'd',  'h',  't',  'n',  's',
	'-',  '`',    0,  '[',  ';',  'q',  'j',  'k',
	'x',  'b',  'm',  'w',  'v',  'z',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!',  '@',  '#',  '$',  '%',  '^',
	'&',  '*',  '(',  ')',  '|',  '+',  127,    9,
	'"',  '<',  '>',  'P',  'Y',  'F',  'G',  'C',
	'R',  'L',  '?',  '}',   13,    0,  'A',  'O',
	'E',  'U',  'I',  'D',  'H',  'T',  'N',  'S',
	'_',  '~',    0,  '{',  ':',  'Q',  'J',  'K',
	'X',  'B',  'M',  'W',  'V',  'Z',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',    0,  '$',    0,    0,
	'{',   '[',  ']', '}', '\\',    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '|',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_SG

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', '\'',  '^',  127,    9,
	'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
	'o',  'p',    0,    0,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',    0,
	  0,    0,    0,  '$',  'y',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '+',  '"',  '*',    0,  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
	'O',  'P',    0,  '!',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',    0,
	  0,    0,    0,    0,  'Y',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  '#',    0,    0,    0,
	'|',    0,    0,    0, '\'',  '~',    0,    0,
	'@',    0,    0,    0,    0,    0,    0,    0,
	  0,    0,   '[',  ']',  13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	'{',    0,    0,  '}',    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0, '\\',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_SG_LATIN1

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', '\'',  '^',  127,    9,
	'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
	'o',  'p',  252,    0,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  246,
	228,  167,    0,  '$',  'y',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '+',  '"',  '*',  231,  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
	'O',  'P',  220,  '!',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  214,
	196,  176,    0,  163,  'Y',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  '#',    0,    0,  172,
	'|',  162,    0,    0, '\'',  '~',    0,    0,
	'@',    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '[',  ']',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,  233,
	'{',    0,    0,  '}',    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0, '\\',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_NO

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0',  '+', '\\',  127,    9,
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	'o',  'p',  '}',  '~',   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  '|',
	'{',  '|',    0, '\'',  'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char shift_map[] = {
	  0,   27,  '!', '\"',  '#',  '$',  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
	'O',  'P',  ']',  '^',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L', '\\',
	'[',    0,    0,  '*',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

static unsigned char alt_map[] = {
	  0,    0,    0,  '@',    0,  '$',    0,    0,
	'{',   '[',  ']', '}',    0, '\'',    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,  '~',   13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_SF

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', '\'',  '^',  127,    9,
	'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
	'o',  'p',    0,    0,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',    0,
	  0,    0,   0,   '$',  'y',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };
static unsigned char shift_map[] = {
	  0,   27,  '+',  '"',  '*',    0,  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
	'O',  'P',    0,  '!',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',    0,
	  0,    0,    0,    0,  'Y',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };
static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  '#',    0,    0,    0,
	'|',    0,    0,    0,  '\'', '~',    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,   '[',  ']',  13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	 '{',   0,    0,   '}',   0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '\\',   0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };

#elif defined KBD_SF_LATIN1

static unsigned char key_map[] = {
	  0,   27,  '1',  '2',  '3',  '4',  '5',  '6',
	'7',  '8',  '9',  '0', '\'',  '^',  127,    9,
	'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
	'o',  'p',  232,  168,   13,    0,  'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  233,
	224,  167,    0,  '$',  'y',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '-',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '<',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };
static unsigned char shift_map[] = {
	  0,   27,  '+',  '"',  '*',  231,  '%',  '&',
	'/',  '(',  ')',  '=',  '?',  '`',  127,    9,
	'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
	'O',  'P',  252,  '!',   13,    0,  'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  246,
	228,  176,    0,  163,  'Y',  'X',  'C',  'V',
	'B',  'N',  'M',  ';',  ':',  '_',    0,  '*',
	  0,   32,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,  '-',    0,    0,    0,  '+',    0,
	  0,    0,    0,    0,    0,    0,  '>',    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };
static unsigned char alt_map[] = {
	  0,    0,    0,  '@',  '#',    0,    0,  172,
	'|',   162,   0,    0,  180,  '~',    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,   '[',  ']',  13,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	 '{',   0,    0,   '}',   0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0,    0,    0,    0,    0,    0,  '\\',   0,
	  0,    0,    0,    0,    0,    0,    0,    0,
	  0 };
#else
#error "KBD-type not defined"
#endif

static void do_self(int sc)
{
	unsigned char ch;

	if (kmode & ALTGR)
		ch = alt_map[sc];
	else if (kmode & (LSHIFT | RSHIFT | LCTRL | RCTRL))
		ch = shift_map[sc];
	else
		ch = key_map[sc];

	if (ch == 0)
		return;

	if ((ch = handle_diacr(ch)) == 0)
		return;

	if (kmode & (LCTRL | RCTRL | CAPS))	/* ctrl or caps */
		if ((ch >= 'a' && ch <= 'z') || (ch >= 224 && ch <= 254))
			ch -= 32;
	if (kmode & (LCTRL | RCTRL))		/* ctrl */
		ch &= 0x1f;

	if (kmode & ALT)
		if (kbd_flags & NO_META_BIT) {
			put_queue('\033');
			put_queue(ch);
		} else
			put_queue(ch|0x80);
	else
		put_queue(ch);
}

unsigned char accent_table[5][64] = {
	" \300BCD\310FGH\314JKLMN\322PQRST\331VWXYZ[\\]^_"
	"`\340bcd\350fgh\354jklmn\362pqrst\371vwxyz{|}~",   /* accent grave */

	" \301BCD\311FGH\315JKLMN\323PQRST\332VWX\335Z[\\]^_"
	"`\341bcd\351fgh\355jklmn\363pqrst\372vwxyz{|}~",   /* accent acute */

	" \302BCD\312FGH\316JKLMN\324PQRST\333VWXYZ[\\]^_"
	"`\342bcd\352fgh\356jklmn\364pqrst\373vwxyz{|}~",   /* circumflex */

	" \303BCDEFGHIJKLMN\325PQRSTUVWXYZ[\\]^_"
	"`\343bcdefghijklm\361\365pqrstuvwxyz{|}~",	    /* tilde */

	" \304BCD\313FGH\316JKLMN\326PQRST\334VWXYZ[\\]^_"
	"`\344bcd\353fgh\357jklmn\366pqrst\374vwx\377z{|}~" /* dieresis */
};

/*
 * Check if dead key pressed. If so, check if same key pressed twice;
 * in that case return the char, otherwise store char and return 0.
 * If dead key not pressed, check if accented character pending. If
 * not: return the char, otherwise check if char is a space. If it is
 * a space return the diacritical. Else combine char with diacritical
 * mark and return.
 */

unsigned int handle_diacr(unsigned int ch)
{
	static unsigned char diacr_table[] =
		{'`', 180, '^', '~', 168, 0};		/* Must end with 0 */
	int i;

	for(i=0; diacr_table[i]; i++)
		if (ch==diacr_table[i] && ((1<<i)&kbd_flags)) {
			if (diacr == i) {
				diacr=-1;
				return ch;		/* pressed twice */
			} else {
				diacr=i;		/* key is dead */
				return 0;
			}
		}
	if (diacr == -1)
		return ch;
	else if (ch == ' ') {
		ch=diacr_table[diacr];
		diacr=-1;
		return ch;
	} else if (ch<64 || ch>122) {
		diacr=-1;
		return ch;
	} else {
		ch=accent_table[diacr][ch-64];
		diacr=-1;
		return ch;
	}
}

#if defined KBD_FR || defined KBD_US || defined KBD_UK || defined KBD_FR_LATIN1
static unsigned char num_table[] = "789-456+1230.";
#else
static unsigned char num_table[] = "789-456+1230,";
#endif

static unsigned char cur_table[] = "1A5-DGC+4B623";
static unsigned int pad_table[] = { 7,8,9,0,4,5,6,0,1,2,3,0,0 };

/*
    Keypad /			35	B7	Q
    Keypad *  (PrtSc)		37	B7	R
    Keypad NumLock		45	??	P
    Keypad 7  (Home)		47	C7	w
    Keypad 8  (Up arrow)	48	C8	x
    Keypad 9  (PgUp)		49	C9	y
    Keypad -			4A	CA	S
    Keypad 4  (Left arrow)	4B	CB	t
    Keypad 5			4C	CC	u
    Keypad 6  (Right arrow)	4D	CD	v
    Keypad +			4E	CE	l
    Keypad 1  (End)		4F	CF	q
    Keypad 2  (Down arrow)	50	D0	r
    Keypad 3  (PgDn)		51	D1	s
    Keypad 0  (Ins)		52	D2	p
    Keypad .  (Del)		53	D3	n
*/

static unsigned char appl_table[] = "wxyStuvlqrspn";

/*
  Set up keyboard to generate DEC VT200 F-keys.
  DEC F1  - F5  not implemented (DEC HOLD, LOCAL PRINT, SETUP, SW SESS, BREAK)
  DEC F6  - F10 are mapped to F6 - F10
  DEC F11 - F20 are mapped to Shift-F1 - Shift-F10
  DEC HELP and DEC DO are mapped to F11, F12 or Shift- F11, F12.
  Regular (?) Linux F1-F5 remain the same.
*/

static char *func_table[2][12] = { /* DEC F1 - F10 */ {
	"\033[[A",  "\033[[B",  "\033[[C",  "\033[[D",
	"\033[[E",  "\033[17~", "\033[18~", "\033[19~",
	"\033[20~", "\033[21~", "\033[28~", "\033[29~"
}, /* DEC F11 - F20 */ {
	"\033[23~", "\033[24~", "\033[25~", "\033[26~",
	"\033[28~", "\033[29~", "\033[31~", "\033[32~",
	"\033[33~", "\033[34~", "\033[28~", "\033[29~"
}};

static void cursor(int sc)
{
	if (sc < 0x47 || sc > 0x53)
		return;
	sc-=0x47;
	if (sc == 12 && (kmode&(LCTRL|RCTRL)) && (kmode&(ALT|ALTGR))) {
		ctrl_alt_del();
		return;
	}
	if (ke0 == 1) {
		cur(sc);
		return;
	}

	if ((kmode&ALT) && sc!=12) {			/* Alt-numpad */
		npadch=npadch*10+pad_table[sc];
		return;
	}

	if (kapplic && !(kmode&(LSHIFT|RSHIFT))) {	/* shift forces cursor */
		applkey(appl_table[sc]);
		return;
	}

	if (kleds&NUMLED) {
		put_queue(num_table[sc]);
	} else
		cur(sc);
}

static void cur(int sc)
{
	char buf[] = { 0x1b, '[', 0, 0, 0 };		/* must not be static */

	buf[2]=cur_table[sc];
	if (buf[2] < '9')
		buf[3]='~';
	else
		if ((buf[2] >= 'A' && buf[2] <= 'D') ? ckmode : kapplic)
			buf[1]='O';
	puts_queue(buf);
}

static void func(int sc)
{
	if (sc < 0x3b)
		return;
	sc-=0x3b;
	if (sc > 9) {
		sc-=18;
		if (sc < 10 || sc > 11)
			return;
	}
	if (kmode&ALT)
		change_console(sc);
	else
		if (kmode & ( LSHIFT | RSHIFT))		/* DEC F11 - F20 */
			puts_queue(func_table[1][sc]);
		else					/* DEC F1 - F10 */
			puts_queue(func_table[0][sc]);
}

static void slash(int sc)
{
	if (ke0 != 1)
		do_self(sc);
	else if (kapplic)
		applkey('Q');
	else
		put_queue('/');
}

static void star(int sc)
{
	if (kapplic)
		applkey('R');
	else
		do_self(sc);
}

static void enter(int sc)
{
	if (ke0 == 1 && kapplic)
		applkey('M');
	else {
		put_queue(13);
		if (lfnlmode)
			put_queue(10);
	}
}

static void minus(int sc)
{
	if (kapplic)
		applkey('S');
	else
		do_self(sc);
}

static void plus(int sc)
{
	if (kapplic)
		applkey('l');
	else
		do_self(sc);
}

static void none(int sc)
{
}

/*
 * kb_wait waits for the keyboard controller buffer to empty.
 */

static void kb_wait(void)
{
	int i;

	for (i=0; i<0x10000; i++)
		if ((inb(0x64)&0x02) == 0)
			break;
}

/*
 * kb_ack waits for 0xfa to appear in port 0x60
 *
 * Suggested by Bruce Evans
 * Added by Niels Skou Olsen [NSO]
 * April 21, 1992
 *
 * Heavily inspired by kb_wait :-)
 * I don't know how much waiting actually is required,
 * but this seems to work
 */

void kb_ack(void)
{
	int i;

	for(i=0; i<0x10000; i++)
		if (inb(0x60) == 0xfa)
			break;
}

long no_idt[2] = {0, 0};

/*
 * This routine reboots the machine by asking the keyboard
 * controller to pulse the reset-line low. We try that for a while,
 * and if it doesn't work, we do some other stupid things.
 */
void hard_reset_now(void)
{
	int i;
	extern unsigned long pg0[1024];

	sti();
/* rebooting needs to touch the page at absolute addr 0 */
	pg0[0] = 7;
	for (;;) {
		for (i=0; i<100; i++) {
			kb_wait();
			*((unsigned short *)0x472) = 0x1234;
			outb(0xfe,0x64);	 /* pulse reset low */
		}
		__asm__("\tlidt _no_idt"::);
	}
}

static fptr key_table[] = {
	none,do_self,do_self,do_self,		/* 00-03 s0 esc 1 2 */
	do_self,do_self,do_self,do_self,	/* 04-07 3 4 5 6 */
	do_self,do_self,do_self,do_self,	/* 08-0B 7 8 9 0 */
	do_self,do_self,do_self,do_self,	/* 0C-0F + ' bs tab */
	do_self,do_self,do_self,do_self,	/* 10-13 q w e r */
	do_self,do_self,do_self,do_self,	/* 14-17 t y u i */
	do_self,do_self,do_self,do_self,	/* 18-1B o p } ^ */
	enter,ctrl,do_self,do_self,		/* 1C-1F enter ctrl a s */
	do_self,do_self,do_self,do_self,	/* 20-23 d f g h */
	do_self,do_self,do_self,do_self,	/* 24-27 j k l | */
	do_self,do_self,lshift,do_self,		/* 28-2B { para lshift , */
	do_self,do_self,do_self,do_self,	/* 2C-2F z x c v */
	do_self,do_self,do_self,do_self,	/* 30-33 b n m , */
	do_self,slash,rshift,star,		/* 34-37 . - rshift * */
	alt,do_self,caps,func,			/* 38-3B alt sp caps f1 */
	func,func,func,func,			/* 3C-3F f2 f3 f4 f5 */
	func,func,func,func,			/* 40-43 f6 f7 f8 f9 */
	func,num,scroll,cursor,			/* 44-47 f10 num scr home */
	cursor,cursor,minus,cursor,		/* 48-4B up pgup - left */
	cursor,cursor,plus,cursor,		/* 4C-4F n5 right + end */
	cursor,cursor,cursor,cursor,		/* 50-53 dn pgdn ins del */
	none,none,do_self,func,			/* 54-57 sysreq ? < f11 */
	func,none,none,none,			/* 58-5B f12 ? ? ? */
	none,none,none,none,			/* 5C-5F ? ? ? ? */
	none,none,none,none,			/* 60-63 ? ? ? ? */
	none,none,none,none,			/* 64-67 ? ? ? ? */
	none,none,none,none,			/* 68-6B ? ? ? ? */
	none,none,none,none,			/* 6C-6F ? ? ? ? */
	none,none,none,none,			/* 70-73 ? ? ? ? */
	none,none,none,none,			/* 74-77 ? ? ? ? */
	none,none,none,none,			/* 78-7B ? ? ? ? */
	none,none,none,none,			/* 7C-7F ? ? ? ? */
	none,none,none,none,			/* 80-83 ? br br br */
	none,none,none,none,			/* 84-87 br br br br */
	none,none,none,none,			/* 88-8B br br br br */
	none,none,none,none,			/* 8C-8F br br br br */
	none,none,none,none,			/* 90-93 br br br br */
	none,none,none,none,			/* 94-97 br br br br */
	none,none,none,none,			/* 98-9B br br br br */
	none,unctrl,none,none,			/* 9C-9F br unctrl br br */
	none,none,none,none,			/* A0-A3 br br br br */
	none,none,none,none,			/* A4-A7 br br br br */
	none,none,unlshift,none,		/* A8-AB br br unlshift br */
	none,none,none,none,			/* AC-AF br br br br */
	none,none,none,none,			/* B0-B3 br br br br */
	none,none,unrshift,none,		/* B4-B7 br br unrshift br */
	unalt,none,uncaps,none,			/* B8-BB unalt br uncaps br */
	none,none,none,none,			/* BC-BF br br br br */
	none,none,none,none,			/* C0-C3 br br br br */
	none,none,none,none,			/* C4-C7 br br br br */
	none,none,none,none,			/* C8-CB br br br br */
	none,none,none,none,			/* CC-CF br br br br */
	none,none,none,none,			/* D0-D3 br br br br */
	none,none,none,none,			/* D4-D7 br br br br */
	none,none,none,none,			/* D8-DB br ? ? ? */
	none,none,none,none,			/* DC-DF ? ? ? ? */
	none,none,none,none,			/* E0-E3 e0 e1 ? ? */
	none,none,none,none,			/* E4-E7 ? ? ? ? */
	none,none,none,none,			/* E8-EB ? ? ? ? */
	none,none,none,none,			/* EC-EF ? ? ? ? */
	none,none,none,none,			/* F0-F3 ? ? ? ? */
	none,none,none,none,			/* F4-F7 ? ? ? ? */
	none,none,none,none,			/* F8-FB ? ? ? ? */
	none,none,none,none			/* FC-FF ? ? ? ? */
};

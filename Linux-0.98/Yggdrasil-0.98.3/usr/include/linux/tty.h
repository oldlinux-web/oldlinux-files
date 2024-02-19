#ifndef _LINUX_TTY_H
#define _LINUX_TTY_H

/*
 * 'tty.h' defines some structures used by tty_io.c and some defines.
 */

#include <linux/termios.h>

#include <asm/system.h>

#define NR_CONSOLES	8

/*
 * These are set up by the setup-routine at boot-time:
 */

struct screen_info {
	unsigned char  orig_x;
	unsigned char  orig_y;
	unsigned char  unused1[2];
	unsigned short orig_video_page;
	unsigned char  orig_video_mode;
	unsigned char  orig_video_cols;
	unsigned short orig_video_ega_ax;
	unsigned short orig_video_ega_bx;
	unsigned short orig_video_ega_cx;
	unsigned char  orig_video_lines;
};

extern struct screen_info screen_info;

#define ORIG_X			(screen_info.orig_x)
#define ORIG_Y			(screen_info.orig_y)
#define ORIG_VIDEO_PAGE		(screen_info.orig_video_page)
#define ORIG_VIDEO_MODE		(screen_info.orig_video_mode)
#define ORIG_VIDEO_COLS 	(screen_info.orig_video_cols)
#define ORIG_VIDEO_EGA_AX	(screen_info.orig_video_ega_ax)
#define ORIG_VIDEO_EGA_BX	(screen_info.orig_video_ega_bx)
#define ORIG_VIDEO_EGA_CX	(screen_info.orig_video_ega_cx)
#define ORIG_VIDEO_LINES	(screen_info.orig_video_lines)

#define VIDEO_TYPE_MDA		0x10	/* Monochrome Text Display	*/
#define VIDEO_TYPE_CGA		0x11	/* CGA Display 			*/
#define VIDEO_TYPE_EGAM		0x20	/* EGA/VGA in Monochrome Mode	*/
#define VIDEO_TYPE_EGAC		0x21	/* EGA/VGA in Color Mode	*/

/*
 * This character is the same as _POSIX_VDISABLE: it cannot be used as
 * a c_cc[] character, but indicates that a particular special character
 * isn't in use (eg VINTR ahs no character etc)
 */
#define __DISABLED_CHAR '\0'

/*
 * See comment for the tty_struct structure before changing
 * TTY_BUF_SIZE.  Actually, there should be different sized tty_queue
 * structures for different purposes.  1024 bytes for the transmit
 * queue is way overkill.  TYT, 9/14/92
 */
#define TTY_BUF_SIZE 1024	/* Must be a power of 2 */

struct tty_queue {
	unsigned long data;
	unsigned long head;
	unsigned long tail;
	struct wait_queue * proc_list;
	unsigned char buf[TTY_BUF_SIZE];
};

struct serial_struct {
	int	type;
	int	line;
	int	port;
	int	irq;
	int	flags;
	int	xmit_fifo_size;
	int	custom_divisor;
	int	reserved[8];
};

/*
 * These are the supported serial types.
 */
#define PORT_UNKNOWN	0
#define PORT_8250	1
#define PORT_16450	2
#define PORT_16550	3
#define PORT_16550A	4

/*
 * Definitions for async_struct (and serial_struct) flags field
 */
#define ASYNC_NOSCRATCH	0x0001	/* 16XXX UART with no scratch register */
#define ASYNC_FOURPORT  0x0002	/* Set OU1, OUT2 per AST Fourport settings */
#define ASYNC_SAK	0x0004	/* Secure Attention Key (Orange book) */

#define ASYNC_SPD_MASK	0x0030
#define ASYNC_SPD_HI	0x0010	/* Use 56000 instead of 38400 bps */
#define ASYNC_SPD_VHI	0x0020  /* Use 115200 instead of 38400 bps */
#define ASYNC_SPD_CUST	0x0030  /* Use user-specified divisor */

#define IS_A_CONSOLE(min)	(((min) & 0xC0) == 0x00)
#define IS_A_SERIAL(min)	(((min) & 0xC0) == 0x40)
#define IS_A_PTY(min)		((min) & 0x80)
#define IS_A_PTY_MASTER(min)	(((min) & 0xC0) == 0x80)
#define IS_A_PTY_SLAVE(min)	(((min) & 0xC0) == 0xC0)
#define PTY_OTHER(min)		((min) ^ 0x40)

#define SL_TO_DEV(line)		((line) | 0x40)
#define DEV_TO_SL(min)		((min) & 0x3F)

#define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
#define DEC(a) ((a) = ((a)-1) & (TTY_BUF_SIZE-1))
#define EMPTY(a) ((a)->head == (a)->tail)
#define LEFT(a) (((a)->tail-(a)->head-1)&(TTY_BUF_SIZE-1))
#define LAST(a) ((a)->buf[(TTY_BUF_SIZE-1)&((a)->head-1)])
#define FULL(a) (!LEFT(a))
#define CHARS(a) (((a)->head-(a)->tail)&(TTY_BUF_SIZE-1))

extern void put_tty_queue(char c, struct tty_queue * queue);
extern int get_tty_queue(struct tty_queue * queue);

#define INTR_CHAR(tty) ((tty)->termios->c_cc[VINTR])
#define QUIT_CHAR(tty) ((tty)->termios->c_cc[VQUIT])
#define ERASE_CHAR(tty) ((tty)->termios->c_cc[VERASE])
#define KILL_CHAR(tty) ((tty)->termios->c_cc[VKILL])
#define EOF_CHAR(tty) ((tty)->termios->c_cc[VEOF])
#define START_CHAR(tty) ((tty)->termios->c_cc[VSTART])
#define STOP_CHAR(tty) ((tty)->termios->c_cc[VSTOP])
#define SUSPEND_CHAR(tty) ((tty)->termios->c_cc[VSUSP])

#define _L_FLAG(tty,f)	((tty)->termios->c_lflag & f)
#define _I_FLAG(tty,f)	((tty)->termios->c_iflag & f)
#define _O_FLAG(tty,f)	((tty)->termios->c_oflag & f)

#define L_CANON(tty)	_L_FLAG((tty),ICANON)
#define L_ISIG(tty)	_L_FLAG((tty),ISIG)
#define L_ECHO(tty)	_L_FLAG((tty),ECHO)
#define L_ECHOE(tty)	_L_FLAG((tty),ECHOE)
#define L_ECHOK(tty)	_L_FLAG((tty),ECHOK)
#define L_ECHONL(tty)	_L_FLAG((tty),ECHONL)
#define L_ECHOCTL(tty)	_L_FLAG((tty),ECHOCTL)
#define L_ECHOKE(tty)	_L_FLAG((tty),ECHOKE)
#define L_TOSTOP(tty)	_L_FLAG((tty),TOSTOP)

#define I_UCLC(tty)	_I_FLAG((tty),IUCLC)
#define I_NLCR(tty)	_I_FLAG((tty),INLCR)
#define I_CRNL(tty)	_I_FLAG((tty),ICRNL)
#define I_NOCR(tty)	_I_FLAG((tty),IGNCR)
#define I_IXON(tty)	_I_FLAG((tty),IXON)
#define I_IXANY(tty)	_I_FLAG((tty),IXANY)
#define I_STRP(tty)	_I_FLAG((tty),ISTRIP)

#define O_POST(tty)	_O_FLAG((tty),OPOST)
#define O_NLCR(tty)	_O_FLAG((tty),ONLCR)
#define O_CRNL(tty)	_O_FLAG((tty),OCRNL)
#define O_NLRET(tty)	_O_FLAG((tty),ONLRET)
#define O_LCUC(tty)	_O_FLAG((tty),OLCUC)

#define C_SPEED(tty)	((tty)->termios->c_cflag & CBAUD)
#define C_HUP(tty)	(C_SPEED((tty)) == B0)

/*
 * Where all of the state associated with a tty is kept while the tty
 * is open.  Since the termios state should be kept even if the tty
 * has been closed --- for things like the baud rate, etc --- it is
 * not stored here, but rather a pointer to the real state is stored
 * here.  Possible the winsize structure should have the same
 * treatment, but (1) the default 80x24 is usually right and (2) it's
 * most often used by a windowing system, which will set the correct
 * size each time the window is created or resized anyway.
 * IMPORTANT: since this structure is dynamically allocated, it must
 * be no larger than 4096 bytes.  Changing TTY_BUF_SIZE will change
 * the size of this structure, and it needs to be done with care.
 * 						- TYT, 9/14/92
 */
struct tty_struct {
	struct termios *termios;
	int pgrp;
	int session;
	unsigned char stopped:1, status_changed:1, packet:1;
	unsigned char ctrl_status;
	short line;
	int flags;
	int count;
	struct winsize winsize;
	int  (*open)(struct tty_struct * tty, struct file * filp);
	void (*close)(struct tty_struct * tty, struct file * filp);
	void (*write)(struct tty_struct * tty);
	int  (*ioctl)(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned int arg);
	void (*throttle)(struct tty_struct * tty, int status);
	struct tty_struct *link;
	struct tty_queue read_q;
	struct tty_queue write_q;
	struct tty_queue secondary;
	};

/*
 * These are the different types of thottle status which can be sent
 * to the low-level tty driver.  The tty_io.c layer is responsible for
 * notifying the low-level tty driver of the following conditions:
 * secondary queue full, secondary queue available, and read queue
 * available.  The low-level driver must send the read queue full
 * command to itself, if it is interested in that condition.
 *
 * Note that the low-level tty driver may elect to ignore one or both
 * of these conditions; normally, however, it will use ^S/^Q or some
 * sort of hardware flow control to regulate the input to try to avoid
 * overflow.  While the low-level driver is responsible for all
 * receiving flow control, note that the ^S/^Q handling (but not
 * hardware flow control) is handled by the upper layer, in
 * copy_to_cooked.  
 */
#define TTY_THROTTLE_SQ_FULL	1
#define TTY_THROTTLE_SQ_AVAIL	2
#define TTY_THROTTLE_RQ_FULL	3
#define TTY_THROTTLE_RQ_AVAIL	4

/*
 * This defines the low- and high-watermarks for the various conditions.
 * Again, the low-level driver is free to ignore any of these, and has
 * to implement RQ_THREHOLD_LW for itself if it wants it.
 */
#define SQ_THRESHOLD_LW	16
#define SQ_THRESHOLD_HW 768
#define RQ_THRESHOLD_LW 16
#define RQ_THRESHOLD_HW 768

/*
 * These bits are used in the flags field of the tty structure.
 * 
 * So that interrupts won't be able to mess up the queues,
 * copy_to_cooked must be atomic with repect to itself, as must
 * tty->write.  Thus, you must use the inline functions set_bit() and
 * clear_bit() to make things atomic.
 */
#define TTY_WRITE_BUSY 0
#define TTY_READ_BUSY 1
#define TTY_CR_PENDING 2
#define TTY_SQ_THROTTLED 3
#define TTY_RQ_THROTTLED 4

#define TTY_WRITE_FLUSH(tty) tty_write_flush((tty))
#define TTY_READ_FLUSH(tty) tty_read_flush((tty))

extern void tty_write_flush(struct tty_struct *);
extern void tty_read_flush(struct tty_struct *);

extern struct tty_struct *tty_table[];
extern struct tty_struct * redirect;
extern int fg_console;
extern unsigned long video_num_columns;
extern unsigned long video_num_lines;
extern struct wait_queue * keypress_wait;

#define TTY_TABLE_IDX(nr)	((nr) ? (nr) : (fg_console+1))
#define TTY_TABLE(nr) 		(tty_table[TTY_TABLE_IDX(nr)])

/*	intr=^C		quit=^|		erase=del	kill=^U
	eof=^D		vtime=\0	vmin=\1		sxtc=\0
	start=^Q	stop=^S		susp=^Z		eol=\0
	reprint=^R	discard=^U	werase=^W	lnext=^V
	eol2=\0
*/
#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"

extern long rs_init(long);
extern long lp_init(long);
extern long con_init(long);
extern long tty_init(long);

extern void flush_input(struct tty_struct * tty);
extern void flush_output(struct tty_struct * tty);
extern void wait_until_sent(struct tty_struct * tty);
extern void copy_to_cooked(struct tty_struct * tty);

extern int tty_ioctl(struct inode *, struct file *, unsigned int, unsigned int);
extern int is_orphaned_pgrp(int pgrp);
extern int is_ignored(int sig);
extern int tty_signal(int sig, struct tty_struct *tty);
extern int kill_pg(int pgrp, int sig, int priv);
extern void do_SAK(struct tty_struct *tty);

/* tty write functions */

extern void rs_write(struct tty_struct * tty);
extern void con_write(struct tty_struct * tty);

/* serial.c */

extern int  rs_open(struct tty_struct * tty, struct file * filp);
extern void change_speed(unsigned int line);

/* pty.c */

extern int  pty_open(struct tty_struct * tty, struct file * filp);

/* console.c */

extern int con_open(struct tty_struct * tty, struct file * filp);
extern void update_screen(int new_console);
extern void blank_screen(void);
extern void unblank_screen(void);

/* vt.c */

extern int vt_ioctl(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned int arg);

#endif

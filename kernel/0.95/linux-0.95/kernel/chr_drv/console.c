/*
 *  linux/kernel/console.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *	console.c
 *
 * This module implements the console io functions
 *	'void con_init(void)'
 *	'void con_write(struct tty_queue * queue)'
 * Hopefully this will be a rather complete VT102 implementation.
 *
 * Beeping thanks to John T Kohl.
 * 
 * Virtual Consoles, Screen Blanking, Screen Dumping, Color, Graphics
 *   Chars, and VT100 enhancements by Peter MacDonald.
 */

/*
 *  NOTE!!! We sometimes disable and enable interrupts for a short while
 * (to put a word in video IO), but this will work even for keyboard
 * interrupts. We know interrupts aren't enabled when getting a keyboard
 * interrupt, as we use trap-gates. Hopefully all is well.
 */

/*
 * Code to check for different video-cards mostly by Galen Hunt,
 * <g-hunt@ee.utah.edu>
 */

#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/config.h>
#include <linux/kernel.h>

#include <asm/io.h>
#include <asm/system.h>
#include <asm/segment.h>

#include <string.h>
#include <errno.h>

#define DEF_TERMIOS \
(struct termios) { \
	ICRNL, \
	OPOST | ONLCR, \
	0, \
	IXON | ISIG | ICANON | ECHO | ECHOCTL | ECHOKE, \
	0, \
	INIT_C_CC \
}

static void blank_screen(void);
static void unblank_screen(void);

/*
 * These are set up by the setup-routine at boot-time:
 */

#define ORIG_X			(*(unsigned char *)0x90000)
#define ORIG_Y			(*(unsigned char *)0x90001)
#define ORIG_VIDEO_PAGE		(*(unsigned short *)0x90004)
#define ORIG_VIDEO_MODE		((*(unsigned short *)0x90006) & 0xff)
#define ORIG_VIDEO_COLS 	(((*(unsigned short *)0x90006) & 0xff00) >> 8)
#define ORIG_VIDEO_LINES	((*(unsigned short *)0x9000e) & 0xff)
#define ORIG_VIDEO_EGA_AX	(*(unsigned short *)0x90008)
#define ORIG_VIDEO_EGA_BX	(*(unsigned short *)0x9000a)
#define ORIG_VIDEO_EGA_CX	(*(unsigned short *)0x9000c)

#define VIDEO_TYPE_MDA		0x10	/* Monochrome Text Display	*/
#define VIDEO_TYPE_CGA		0x11	/* CGA Display 			*/
#define VIDEO_TYPE_EGAM		0x20	/* EGA/VGA in Monochrome Mode	*/
#define VIDEO_TYPE_EGAC		0x21	/* EGA/VGA in Color Mode	*/

#define NPAR 16

int NR_CONSOLES = 0;

extern void keyboard_interrupt(void);
extern void set_leds(void);
extern unsigned char kapplic;
extern unsigned char kleds;
extern unsigned char kmode;

unsigned long	video_num_columns;		/* Number of text columns	*/
unsigned long	video_num_lines;		/* Number of test lines		*/

static unsigned char	video_type;		/* Type of display being used	*/
static unsigned long	video_mem_base;		/* Base of video memory		*/
static unsigned long	video_mem_term;		/* End of video memory		*/
static unsigned long	video_size_row;		/* Bytes per row		*/
static unsigned char	video_page;		/* Initial video page		*/
static unsigned short	video_port_reg;		/* Video register select port	*/
static unsigned short	video_port_val;		/* Video register value port	*/
static int can_do_colour = 0;

static struct {
	unsigned short	vc_video_erase_char;	
	unsigned char	vc_attr;
	unsigned char	vc_def_attr;
	int		vc_bold_attr;
	unsigned long	vc_ques;
	unsigned long	vc_state;
	char *	 	vc_restate;
	unsigned long	vc_checkin;
	unsigned long	vc_origin;		/* Used for EGA/VGA fast scroll	*/
	unsigned long	vc_scr_end;		/* Used for EGA/VGA fast scroll	*/
	unsigned long	vc_pos;
	unsigned long	vc_x,vc_y;
	unsigned long	vc_top,vc_bottom;
	unsigned long	vc_npar,vc_par[NPAR];
	unsigned long	vc_video_mem_start;	/* Start of video RAM		*/
	unsigned long	vc_video_mem_end;	/* End of video RAM (sort of)	*/
	unsigned int	vc_saved_x;
	unsigned int	vc_saved_y;
	unsigned int	vc_iscolor;
	unsigned char	vc_kbdapplic;
	unsigned char   vc_kbdleds;
	unsigned char	vc_kbdmode;
	char *		vc_translate;
} vc_cons [MAX_CONSOLES];

#define MEM_BUFFER_SIZE (2*80*50*8) 

unsigned short *vc_scrbuf[MAX_CONSOLES];
unsigned short vc_scrmembuf[MEM_BUFFER_SIZE/2];
static int console_blanked = 0;

#define origin		(vc_cons[currcons].vc_origin)
#define scr_end		(vc_cons[currcons].vc_scr_end)
#define pos		(vc_cons[currcons].vc_pos)
#define top		(vc_cons[currcons].vc_top)
#define bottom		(vc_cons[currcons].vc_bottom)
#define x		(vc_cons[currcons].vc_x)
#define y		(vc_cons[currcons].vc_y)
#define state		(vc_cons[currcons].vc_state)
#define restate		(vc_cons[currcons].vc_restate)
#define checkin		(vc_cons[currcons].vc_checkin)
#define npar		(vc_cons[currcons].vc_npar)
#define par		(vc_cons[currcons].vc_par)
#define ques		(vc_cons[currcons].vc_ques)
#define attr		(vc_cons[currcons].vc_attr)
#define saved_x		(vc_cons[currcons].vc_saved_x)
#define saved_y		(vc_cons[currcons].vc_saved_y)
#define translate	(vc_cons[currcons].vc_translate)
#define video_mem_start	(vc_cons[currcons].vc_video_mem_start)
#define video_mem_end	(vc_cons[currcons].vc_video_mem_end)
#define def_attr	(vc_cons[currcons].vc_def_attr)
#define video_erase_char  (vc_cons[currcons].vc_video_erase_char)	
#define iscolor		(vc_cons[currcons].vc_iscolor)
#define kbdapplic	(vc_cons[currcons].vc_kbdapplic)
#define kbdmode		(vc_cons[currcons].vc_kbdmode)
#define kbdleds		(vc_cons[currcons].vc_kbdleds)

int blankinterval = 5*60*HZ;
static int screen_size = 0;

static void sysbeep(void);

/*
 * this is what the terminal answers to a ESC-Z or csi0c
 * query (= vt100 response).
 */
#define RESPONSE "\033[?1;2c"

static char * translations[] = {
/* normal 7-bit ascii */
	" !\"#$%&'()*+,-./0123456789:;<=>?"
	"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	"`abcdefghijklmnopqrstuvwxyz{|}~ ",
/* vt100 graphics */
	" !\"#$%&'()*+,-./\333123456789:;<=>?"
	"@ABCDEFGH\017JKLMNOPQRSTUVWXYZ[\\]^ "
	/* '   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
	"\004\261\007\007\007\007\370\361\040\007\331\277\332\300\305\007"
	/* p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~     */
	"\007\304\007\007\303\264\301\302\263\007\007\007\007\007\234 "
	
      /*"\004\261\007\007\007\007\370\361\007\007\275\267\326\323\327\304"
        "\304\304\304\304\307\266\320\322\272\363\362\343\\007\234\007 " */
};

#define NORM_TRANS (translations[0])
#define GRAF_TRANS (translations[1])

/* NOTE! gotoxy thinks x==video_num_columns is ok */
static inline void gotoxy(int currcons, int new_x,unsigned int new_y)
{
	if (new_x > video_num_columns || new_y >= video_num_lines)
		return;
	x = new_x;
	y = new_y;
	pos = origin + y*video_size_row + (x<<1);
}

static inline void set_origin(int currcons)
{
	if (video_type != VIDEO_TYPE_EGAC && video_type != VIDEO_TYPE_EGAM)
		return;
	if (currcons != fg_console)
		return;
	cli();
	outb_p(12, video_port_reg);
	outb_p(0xff&((origin-video_mem_base)>>9), video_port_val);
	outb_p(13, video_port_reg);
	outb_p(0xff&((origin-video_mem_base)>>1), video_port_val);
	sti();
}

static void scrup(int currcons)
{
	unsigned int oldbottom, oldtop;

	oldbottom = bottom;
	oldtop = top;
	if (y < top) {
		top = 0;
		bottom = y + 1;
	} else if (y > bottom) {
		bottom = video_num_lines;
		top = y;
	}
	if (top > video_num_lines)
		top = 0;
	if (bottom > video_num_lines)
		bottom = video_num_lines;
	if (bottom <= top) {
		bottom = oldbottom;
		top = oldtop;
		return;
	}
	if (video_type == VIDEO_TYPE_EGAC || video_type == VIDEO_TYPE_EGAM)
	{
		if (!top && bottom == video_num_lines) {
			origin += video_size_row;
			pos += video_size_row;
			scr_end += video_size_row;
			if (scr_end > video_mem_end) {
				__asm__("cld\n\t"
					"rep\n\t"
					"movsl\n\t"
					"movl _video_num_columns,%1\n\t"
					"rep\n\t"
					"stosw"
					::"a" (video_erase_char),
					"c" ((video_num_lines-1)*video_num_columns>>1),
					"D" (video_mem_start),
					"S" (origin)
					:"cx","di","si");
				scr_end -= origin-video_mem_start;
				pos -= origin-video_mem_start;
				origin = video_mem_start;
			} else {
				__asm__("cld\n\t"
					"rep\n\t"
					"stosw"
					::"a" (video_erase_char),
					"c" (video_num_columns),
					"D" (scr_end-video_size_row)
					:"cx","di");
			}
			set_origin(currcons);
		} else {
			__asm__("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl _video_num_columns,%%ecx\n\t"
				"rep\n\t"
				"stosw"
				::"a" (video_erase_char),
				"c" ((bottom-top-1)*video_num_columns>>1),
				"D" (origin+video_size_row*top),
				"S" (origin+video_size_row*(top+1))
				:"cx","di","si");
		}
	}
	else		/* Not EGA/VGA */
	{
		__asm__("cld\n\t"
			"rep\n\t"
			"movsl\n\t"
			"movl _video_num_columns,%%ecx\n\t"
			"rep\n\t"
			"stosw"
			::"a" (video_erase_char),
			"c" ((bottom-top-1)*video_num_columns>>1),
			"D" (origin+video_size_row*top),
			"S" (origin+video_size_row*(top+1))
			:"cx","di","si");
	}
	bottom = oldbottom;
	top = oldtop;
}

static void scrdown(int currcons)
{
	unsigned int oldbottom, oldtop;

	oldbottom = bottom;
	oldtop = top;
	if (y < top) {
		top = 0;
		bottom = y + 1;
	} else if (y > bottom) {
		bottom = video_num_lines;
		top = y;
	}
	if (top > video_num_lines)
		top = 0;
	if (bottom > video_num_lines)
		bottom = video_num_lines;
	if (bottom<=top) {
		bottom = oldbottom;
		top = oldtop;
		return;
	}
	__asm__("std\n\t"
		"rep\n\t"
		"movsl\n\t"
		"addl $2,%%edi\n\t"	/* %edi has been decremented by 4 */
		"movl _video_num_columns,%%ecx\n\t"
		"rep\n\t"
		"stosw\n\t"
		"cld"
		::"a" (video_erase_char),
		"c" ((bottom-top-1)*video_num_columns>>1),
		"D" (origin+video_size_row*bottom-4),
		"S" (origin+video_size_row*(bottom-1)-4)
		:"ax","cx","di","si");
	bottom = oldbottom;
	top = oldtop;
}

static void lf(int currcons)
{
	if (y+1<bottom) {
		y++;
		pos += video_size_row;
		return;
	} else 
		scrup(currcons);
}

static void ri(int currcons)
{
	if (y>top) {
		y--;
		pos -= video_size_row;
		return;
	} else
		scrdown(currcons);
}

static void cr(int currcons)
{
	pos -= x<<1;
	x=0;
}

static void del(int currcons)
{
	if (x) {
		pos -= 2;
		x--;
		*(unsigned short *)pos = video_erase_char;
	}
}

static void csi_J(int currcons, int vpar)
{
	long count;
	long start;

	switch (vpar) {
		case 0:	/* erase from cursor to end of display */
			count = (scr_end-pos)>>1;
			start = pos;
			break;
		case 1:	/* erase from start to cursor */
			count = (pos-origin)>>1;
			start = origin;
			break;
		case 2: /* erase whole display */
			count = video_num_columns * video_num_lines;
			start = origin;
			break;
		default:
			return;
	}
	__asm__("cld\n\t"
		"rep\n\t"
		"stosw\n\t"
		::"c" (count),
		"D" (start),"a" (video_erase_char)
		:"cx","di");
}

static void csi_K(int currcons, int vpar)
{
	long count;
	long start;

	switch (vpar) {
		case 0:	/* erase from cursor to end of line */
			if (x>=video_num_columns)
				return;
			count = video_num_columns-x;
			start = pos;
			break;
		case 1:	/* erase from start of line to cursor */
			start = pos - (x<<1);
			count = (x<video_num_columns)?x:video_num_columns;
			break;
		case 2: /* erase whole line */
			start = pos - (x<<1);
			count = video_num_columns;
			break;
		default:
			return;
	}
	__asm__("cld\n\t"
		"rep\n\t"
		"stosw\n\t"
		::"c" (count),
		"D" (start),"a" (video_erase_char)
		:"cx","di");
}

void csi_m(int currcons )
{
	int i;
	static int conv_table[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

	for (i=0;i<=npar;i++)
		switch (par[i]) {
			case 0: attr=def_attr;break;  /* default */
			case 1: attr=(iscolor?attr|0x08:attr|0x0f);break;  /* bold */
			/*case 4: attr=attr|0x01;break;*/  /* underline */
			case 4: /* bold */ 
			  if (!iscolor)
			    attr |= 0x01;
			  else
			  { /* check if forground == background */
			    if (vc_cons[currcons].vc_bold_attr != -1)
			      attr = (vc_cons[currcons].vc_bold_attr&0x0f)|(0xf0&(attr));
			    else
			    { short newattr = (attr&0xf0)|(0xf&(~attr));
			      attr = ((newattr&0xf)==((attr>>4)&0xf)? 
			        (attr&0xf0)|(((attr&0xf)+1)%0xf):
			        newattr);
			    }    
			  }
			  break;
			case 5: attr=attr|0x80;break;  /* blinking */
			case 7: attr=(attr<<4)|(attr>>4);break;  /* negative */
			case 22: attr=attr&0xf7;break; /* not bold */ 
			case 24: attr=attr&0xfe;break;  /* not underline */
			case 25: attr=attr&0x7f;break;  /* not blinking */
			case 27: attr=def_attr;break; /* positive image */
			case 39: attr=(attr & 0xf0)|(def_attr & 0x0f); break;
			case 49: attr=(attr & 0x0f)|(def_attr & 0xf0); break;
			default:
			  if (!can_do_colour)
			    break;
			  iscolor = 1;
			  if ((par[i]>=30) && (par[i]<=37))
			    attr = (attr & 0xf0) | conv_table[par[i]-30];
			  else  /* Background color */
			    if ((par[i]>=40) && (par[i]<=47))
			      attr = (attr & 0x0f) | (conv_table[par[i]-40]<<4);
			    else
				break;
		}
}

static inline void set_cursor(int currcons)
{
	if (currcons != fg_console)
		return;
	cli();
	outb_p(14, video_port_reg);
	outb_p(0xff&((pos-video_mem_base)>>9), video_port_val);
	outb_p(15, video_port_reg);
	outb_p(0xff&((pos-video_mem_base)>>1), video_port_val);
	sti();
}

static inline void hide_cursor(int currcons)
{
	outb_p(14, video_port_reg);
	outb_p(0xff&((scr_end-video_mem_base)>>9), video_port_val);
	outb_p(15, video_port_reg);
	outb_p(0xff&((scr_end-video_mem_base)>>1), video_port_val);
}

static void respond(int currcons, struct tty_struct * tty)
{
	char * p = RESPONSE;

	cli();
	while (*p) {
		PUTCH(*p,tty->read_q);
		p++;
	}
	sti();
	copy_to_cooked(tty);
}

static void insert_char(int currcons)
{
	int i=x;
	unsigned short tmp, old = video_erase_char;
	unsigned short * p = (unsigned short *) pos;

	while (i++<video_num_columns) {
		tmp=*p;
		*p=old;
		old=tmp;
		p++;
	}
}

static void insert_line(int currcons)
{
	int oldtop,oldbottom;

	oldtop=top;
	oldbottom=bottom;
	top=y;
	bottom = video_num_lines;
	scrdown(currcons);
	top=oldtop;
	bottom=oldbottom;
}

static void delete_char(int currcons)
{
	int i;
	unsigned short * p = (unsigned short *) pos;

	if (x>=video_num_columns)
		return;
	i = x;
	while (++i < video_num_columns) {
		*p = *(p+1);
		p++;
	}
	*p = video_erase_char;
}

static void delete_line(int currcons)
{
	int oldtop,oldbottom;

	oldtop=top;
	oldbottom=bottom;
	top=y;
	bottom = video_num_lines;
	scrup(currcons);
	top=oldtop;
	bottom=oldbottom;
}

static void csi_at(int currcons, unsigned int nr)
{
	if (nr > video_num_columns)
		nr = video_num_columns;
	else if (!nr)
		nr = 1;
	while (nr--)
		insert_char(currcons);
}

static void csi_L(int currcons, unsigned int nr)
{
	if (nr > video_num_lines)
		nr = video_num_lines;
	else if (!nr)
		nr = 1;
	while (nr--)
		insert_line(currcons);
}

static void csi_P(int currcons, unsigned int nr)
{
	if (nr > video_num_columns)
		nr = video_num_columns;
	else if (!nr)
		nr = 1;
	while (nr--)
		delete_char(currcons);
}

static void csi_M(int currcons, unsigned int nr)
{
	if (nr > video_num_lines)
		nr = video_num_lines;
	else if (!nr)
		nr=1;
	while (nr--)
		delete_line(currcons);
}

static void save_cur(int currcons)
{
	saved_x=x;
	saved_y=y;
}

static void restore_cur(int currcons)
{
	gotoxy(currcons,saved_x, saved_y);
}


enum { ESnormal, ESesc, ESsquare, ESgetpars, ESgotpars, ESfunckey, 
	ESsetterm, ESsetgraph, ESgraph, ESgresc, ESignore };

void con_write(struct tty_struct * tty)
{
	int nr;
	char c;
	int currcons;

	wake_up(&tty->write_q->proc_list);
	currcons = tty - tty_table;
	if ((currcons>=MAX_CONSOLES) || (currcons<0)) {
		printk("con_write: illegal tty\n\r");
		return;
	}
 	nr = CHARS(tty->write_q);
	while (nr--) {
		if (tty->stopped)
			break;
		GETCH(tty->write_q,c);
		if (c == 24 || c == 26)
			state = ESnormal;
		switch(state) {
			case ESnormal:
				if (c>31 && c<127) {
					if (x>=video_num_columns) {
						x -= video_num_columns;
						pos -= video_size_row;
						lf(currcons);
					}
					*(char *) pos = translate[c-32];
					pos++;
					*(char *) pos = attr;
					pos++;
					x++;
				} else if (c==27)
					state = ESesc;
				else if (c==10 || c==11 || c==12)
					lf(currcons);
				else if (c==13)
					cr(currcons);
				else if (c==127)
					del(currcons);
				else if (c==8) {
					if (x) {
						x--;
						pos -= 2;
					}
				} else if (c==9) {
					c = 8-(x&7);
					x += c;
					pos += c<<1;
					if (x>video_num_columns) {
						x -= video_num_columns;
						pos -= video_size_row;
						lf(currcons);
					}
					c=9;
				} else if (c==7)
					sysbeep();
			  	else if (c == 14) {
					checkin = 1;
				  	translate = restate;
			  	} else if (c == 15) {
					translate = NORM_TRANS;
					checkin = 0;
			  	}
				break;
			case ESesc:
				state = ESnormal;
				switch (c) {
				  case '[':
					state = ESsquare;
					break;
				  case 'E':
					gotoxy(currcons,0,y+1);
					break;
				  case 'M':
					ri(currcons);
					break;
				  case 'D':
					lf(currcons);
					break;
				  case 'Z':
					respond(currcons,tty);
					break;
				  case '7':
					save_cur(currcons);
					break;
				  case '8':
					restore_cur(currcons);
					break;
				  case '(':
				  case ')':
				    	state = ESsetgraph;		
					break;
				  case 'P':
				    	state = ESsetterm;  
				    	break;
				  case '#':
				  	state = -1;
				  	break;  	
				  case 'c':
					tty->termios = DEF_TERMIOS;
				  	state = ESnormal;
				  	restate = NORM_TRANS;
					checkin = 0;
					top = 0;
					bottom = video_num_lines;
					translate = NORM_TRANS;
				  case '>':  /* Numeric keypad */
				 	kbdapplic = 0;
				 	if (currcons == fg_console)
				 		kapplic = 0;
				 	break;
				  case '=':  /* Appl. keypad */
				 	kbdapplic = 1;
				 	if (currcons == fg_console)
				 		kapplic = 1;
				 	break;
				}	
				break;
			case ESsquare:
				for(npar = 0 ; npar < NPAR ; npar++)
					par[npar]=0;
				npar = 0;
				state = ESgetpars;
				if (c == '[') { /* Function key */
					state=ESfunckey;
					break;
				}  
				if (ques=(c=='?'))
					break;
			case ESgetpars:
				if (c==';' && npar<NPAR-1) {
					npar++;
					break;
				} else if (c>='0' && c<='9') {
					par[npar]=10*par[npar]+c-'0';
					break;
				} else state=ESgotpars;
			case ESgotpars:
				state = ESnormal;
				if (ques) {
					ques = 0;
					break;
				}  
				switch(c) {
					case 'G': case '`':
						if (par[0]) par[0]--;
						gotoxy(currcons,par[0],y);
						break;
					case 'A':
						if (!par[0]) par[0]++;
						gotoxy(currcons,x,y-par[0]);
						break;
					case 'B': case 'e':
						if (!par[0]) par[0]++;
						gotoxy(currcons,x,y+par[0]);
						break;
					case 'C': case 'a':
						if (!par[0]) par[0]++;
						gotoxy(currcons,x+par[0],y);
						break;
					case 'D':
						if (!par[0]) par[0]++;
						gotoxy(currcons,x-par[0],y);
						break;
					case 'E':
						if (!par[0]) par[0]++;
						gotoxy(currcons,0,y+par[0]);
						break;
					case 'F':
						if (!par[0]) par[0]++;
						gotoxy(currcons,0,y-par[0]);
						break;
					case 'd':
						if (par[0]) par[0]--;
						gotoxy(currcons,x,par[0]);
						break;
					case 'H': case 'f':
						if (par[0]) par[0]--;
						if (par[1]) par[1]--;
						gotoxy(currcons,par[1],par[0]);
						break;
					case 'J':
						csi_J(currcons,par[0]);
						break;
					case 'K':
						csi_K(currcons,par[0]);
						break;
					case 'L':
						csi_L(currcons,par[0]);
						break;
					case 'M':
						csi_M(currcons,par[0]);
						break;
					case 'P':
						csi_P(currcons,par[0]);
						break;
					case '@':
						csi_at(currcons,par[0]);
						break;
					case 'm':
						csi_m(currcons);
						break;
					case 'r':
						if (par[0]) par[0]--;
						if (!par[1]) par[1] = video_num_lines;
						if (par[0] < par[1] &&
						    par[1] <= video_num_lines) {
							top=par[0];
							bottom=par[1];
						}
						break;
					case 's':
						save_cur(currcons);
						break;
					case 'u':
						restore_cur(currcons);
						break;
					case 'l': /* blank interval */
					case 'b': /* bold attribute */
						  if (!((npar >= 2) &&
						  ((par[1]-13) == par[0]) && 
						  ((par[2]-17) == par[0]))) 
						    break;
						if ((c=='l') && (par[0]<=60)) {
						  blankinterval = HZ*60*par[0];
						}
						if (c=='b')
						  vc_cons[currcons].vc_bold_attr
						    = par[0];
				}
				break;
			case ESfunckey:
				state = ESnormal;
				break;
			case ESsetterm:  /* Setterm functions. */
				state = ESnormal;
				if (c == 'S') {
					def_attr = attr;
					video_erase_char = (video_erase_char&0x0ff) | (def_attr<<8);
				} else if (c == 'L')
					/*linewrap on*/;
				else if (c == 'l')
					/*linewrap off*/;
				break;
			case ESsetgraph:
				if (c == '0') { 
					if (checkin)
						translate = GRAF_TRANS;
					restate = GRAF_TRANS;
				} else if (c == 'B')
					translate = restate = NORM_TRANS;
				state = ESnormal;
				break;
			default:
				state = ESnormal;
		}
	}
	set_cursor(currcons);
	timer_active &= ~(1<<BLANK_TIMER);
	if (console_blanked) {
		timer_table[BLANK_TIMER].expires = 0;
		timer_active |= 1<<BLANK_TIMER;
	} else if (blankinterval) {
		timer_table[BLANK_TIMER].expires = jiffies + blankinterval;
		timer_active |= 1<<BLANK_TIMER;
	}
}

void do_keyboard_interrupt(void)
{
	copy_to_cooked(TTY_TABLE(0));
	timer_active &= ~(1<<BLANK_TIMER);
	if (console_blanked) {
		timer_table[BLANK_TIMER].expires = 0;
		timer_active |= 1<<BLANK_TIMER;
	} else if (blankinterval) {
		timer_table[BLANK_TIMER].expires = jiffies + blankinterval;
		timer_active |= 1<<BLANK_TIMER;
	}
}	

void * memsetw(void * s,unsigned short c,int count)
{
__asm__("cld\n\t"
	"rep\n\t"
	"stosw"
	::"a" (c),"D" (s),"c" (count)
	:"cx","di");
return s;
}

/*
 *  void con_init(void);
 *
 * This routine initalizes console interrupts, and does nothing
 * else. If you want the screen to clear, call tty_write with
 * the appropriate escape-sequece.
 *
 * Reads the information preserved by setup.s to determine the current display
 * type and sets everything accordingly.
 */
void con_init(void)
{
	register unsigned char a;
	char *display_desc = "????";
	char *display_ptr;
	int currcons = 0;
	long base, term;
	long video_memory;
	long saveterm, savebase;

	video_num_columns = ORIG_VIDEO_COLS;
	video_size_row = video_num_columns * 2;
	video_num_lines = ORIG_VIDEO_LINES;
	video_page = ORIG_VIDEO_PAGE;
	video_erase_char = 0x0720;
	timer_table[BLANK_TIMER].fn = blank_screen;
	timer_table[BLANK_TIMER].expires = 0;
	if (blankinterval) {
		timer_table[BLANK_TIMER].expires = jiffies+blankinterval;
		timer_active |= 1<<BLANK_TIMER;
	}
	
	if (ORIG_VIDEO_MODE == 7)	/* Is this a monochrome display? */
	{
		video_mem_base = 0xb0000;
		video_port_reg = 0x3b4;
		video_port_val = 0x3b5;
		if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
		{
			video_type = VIDEO_TYPE_EGAM;
			video_mem_term = 0xb8000;
			display_desc = "EGAm";
		}
		else
		{
			video_type = VIDEO_TYPE_MDA;
			video_mem_term = 0xb2000;
			display_desc = "*MDA";
		}
	}
	else				/* If not, it is color. */
	{
		can_do_colour = 1;
		video_mem_base = 0xb8000;
		video_port_reg	= 0x3d4;
		video_port_val	= 0x3d5;
		if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
		{
			video_type = VIDEO_TYPE_EGAC;
			video_mem_term = 0xc0000;
			display_desc = "EGAc";
		}
		else
		{
			video_type = VIDEO_TYPE_CGA;
			video_mem_term = 0xba000;
			display_desc = "*CGA";
		}
	}
	
	/* Let the user known what kind of display driver we are using */
	
	display_ptr = ((char *)video_mem_base) + video_size_row - 8;
	while (*display_desc)
	{
		*display_ptr++ = *display_desc++;
		display_ptr++;
	}
	
	savebase = video_mem_base;
	saveterm = video_mem_term;
	memsetw(vc_scrmembuf,video_erase_char,MEM_BUFFER_SIZE/2);
	video_mem_base = (long)vc_scrmembuf;
	video_mem_term = (long)&(vc_scrmembuf[MEM_BUFFER_SIZE/2]);
	video_memory = video_mem_term - video_mem_base;
	screen_size = (video_num_lines * video_size_row);
	NR_CONSOLES = video_memory / screen_size;
	if (NR_CONSOLES > MAX_CONSOLES)
		NR_CONSOLES = MAX_CONSOLES;
	if (!NR_CONSOLES)
		NR_CONSOLES = 1;
	video_memory = screen_size;
	
	/* Initialize the variables used for scrolling (mostly EGA/VGA)	*/
	
	base = origin = video_mem_start = video_mem_base;
	term = video_mem_end = base + video_memory;
	scr_end	= video_mem_start + screen_size;
	top	= 0;
	bottom	= video_num_lines;
  	attr = 0x07;
  	def_attr = 0x07;
        restate = NORM_TRANS;
        state = ESnormal;
        checkin = 0;
	ques = 0;
	iscolor = 0;
	translate = NORM_TRANS;
	kbdleds = 2;
	kbdmode = 0;
	kbdapplic = 0;
        vc_cons[0].vc_bold_attr = -1;

	gotoxy(currcons,ORIG_X,ORIG_Y);
  	for (currcons = 1; currcons<NR_CONSOLES; currcons++) {
		vc_cons[currcons] = vc_cons[0];
		origin = video_mem_start = (base += video_memory);
		scr_end = origin + video_num_lines * video_size_row;
		video_mem_end = (term += video_memory);
		gotoxy(currcons,0,0);
	}
  	for (currcons = 0; currcons<NR_CONSOLES; currcons++) 
		vc_scrbuf[currcons] = (unsigned short *)origin;
	currcons = 0;	
	video_mem_base = savebase;
	video_mem_term = saveterm;
	
	video_mem_start = video_mem_base;
	video_mem_end = video_mem_term;
	origin	= video_mem_start;
	scr_end	= video_mem_start + video_num_lines * video_size_row;
	top	= 0;
	bottom	= video_num_lines; 
	pos=origin + y*video_size_row + (x<<1);
	update_screen(fg_console);
	set_trap_gate(0x21,&keyboard_interrupt);
	outb_p(inb_p(0x21)&0xfd,0x21);
	a=inb_p(0x61);
	outb_p(a|0x80,0x61);
	outb_p(a,0x61);
}

void kbdsave(int new_console)
{
	int currcons = fg_console;
	kbdmode = kmode;
	kbdleds = kleds;
	kbdapplic = kapplic;
	currcons = new_console;
	kmode = (kmode & 0x3F) | (kbdmode & 0xC0);
	kleds = kbdleds;
	kapplic = kbdapplic;
	set_leds();
}

static void get_scrmem(int currcons)
{
	memcpy((void *)vc_scrbuf[fg_console],(void *)origin, screen_size);
	video_mem_start = (unsigned long)vc_scrbuf[fg_console];
  	origin 	= video_mem_start;
	scr_end = video_mem_end = video_mem_start+screen_size;
  	top = 0;
  	pos = origin + y*video_size_row + (x<<1);
  	bottom	= video_num_lines;
}

static void set_scrmem(int currcons)
{
	video_mem_start = video_mem_base;
	video_mem_end = video_mem_term;
	origin	= video_mem_start;
	scr_end	= video_mem_start + screen_size;
	top	= 0;
	bottom	= video_num_lines; 
	pos = origin + y*video_size_row + (x<<1);
	memcpy((void *)video_mem_base, (void *)vc_scrbuf[fg_console], screen_size);
}

void blank_screen(void)
{
	timer_table[BLANK_TIMER].fn = unblank_screen;
	get_scrmem(fg_console);
	hide_cursor(fg_console);
	console_blanked = 1;
	memsetw((void *)video_mem_base, 0x0020, video_mem_term-video_mem_base );
}

void unblank_screen(void)
{
	timer_table[BLANK_TIMER].fn = blank_screen;
	if (blankinterval) {
		timer_table[BLANK_TIMER].expires = jiffies + blankinterval;
		timer_active |= 1<<BLANK_TIMER;
	}
	console_blanked = 0;
	set_scrmem(fg_console);
	set_origin(fg_console);
	set_cursor(fg_console);
}

void update_screen(int new_console)
{
	static int lock = 0;

	if (new_console == fg_console || lock)
		return;
	lock = 1;
	kbdsave(new_console);
	get_scrmem(fg_console); 
	fg_console = new_console;
	set_scrmem(fg_console); 
	set_origin(fg_console);
	set_cursor(new_console);
	lock = 0;
}

/* from bsd-net-2: */

static void sysbeepstop(void)
{
	/* disable counter 2 */
	outb(inb_p(0x61)&0xFC, 0x61);
}

static void sysbeep(void)
{
	/* enable counter 2 */
	outb_p(inb_p(0x61)|3, 0x61);
	/* set command for counter 2, 2 byte write */
	outb_p(0xB6, 0x43);
	/* send 0x637 for 750 HZ */
	outb_p(0x37, 0x42);
	outb(0x06, 0x42);
	/* 1/8 second */
	timer_table[BEEP_TIMER].expires = jiffies + HZ/8;
	timer_table[BEEP_TIMER].fn = sysbeepstop;
	timer_active |= 1<<BEEP_TIMER;
}

int do_screendump(int arg)
{
	char *sptr, *buf = (char *)arg;
	int currcons, l;

	verify_area(buf,2+video_num_columns*video_num_lines);
	currcons = get_fs_byte(buf+1);
	if ((currcons<0) || (currcons>NR_CONSOLES))
		return -EIO;
	put_fs_byte((char)(video_num_lines),buf++);	
	put_fs_byte((char)(video_num_columns),buf++);
	currcons = (currcons ? currcons-1 : fg_console);
	sptr = (char *) origin;
	for (l=video_num_lines*video_num_columns; l>0 ; l--, sptr++)
		put_fs_byte(*sptr++,buf++);	
	return(0);
}

void console_print(const char * b)
{
	int currcons = fg_console;
	char c;

	if (currcons<0 || currcons>=NR_CONSOLES)
		currcons = 0;
	while (c = *(b++)) {
		if (c == 10) {
			cr(currcons);
			lf(currcons);
			continue;
		}
		if (c == 13) {
			cr(currcons);
			continue;
		}
		if (x>=video_num_columns) {
			x -= video_num_columns;
			pos -= video_size_row;
			lf(currcons);
		}
		*(char *) pos = c;
		pos++;
		*(char *) pos = attr;
		pos++;
		x++;
	}
	set_cursor(currcons);
}

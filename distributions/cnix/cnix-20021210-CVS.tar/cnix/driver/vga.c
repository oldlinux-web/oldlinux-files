#include <string.h>
#include <vga.h>
#include <tty.h>
#include <asm/io.h>
#include <asm/system.h>
#include <cnix/wait.h>

#define VGA_CRT_INX	0x3d4
#define VGA_CRT_DATA 	0x3d5

#define VID_MEM		0xb8000
#define lines		25
#define cols		80
extern kbd_interrupt();

unsigned char *vidmem;
static int pos;
static int cursor_x,cursor_y;

struct tty_queue read_q = { 0,0,""} ;/* head ,tail ,buf */


struct wait_queue * wait;

static inline void gotoxy(unsigned int x, unsigned  int y);

static void scroll_up()
{
	int i;
	unsigned char * vidptr;

	memcpy ( VID_MEM, VID_MEM + cols * 2, ( lines - 1 ) * cols * 2 );

	vidptr = (unsigned char *)(VID_MEM + (lines - 1) * cols * 2);
	for (i = 0; i < cols * 2; i += 2)
		vidptr[i] = ' ';
}

static inline void gotoxy(unsigned int x,unsigned int y)
{
	pos =( cols * y +x) * 2;

	cli();
	outb(14, VGA_CRT_INX);
	outb(0xff & (pos >> 9), VGA_CRT_DATA);
	outb(15, VGA_CRT_INX);
	outb(0xff & (pos >> 1), VGA_CRT_DATA);
	sti();

	cursor_x = x;
	cursor_y = y;
}


void puts(char *s)
{
	unsigned char c;
	int x, y;

	x = cursor_x;
	y = cursor_y;

	while ( ( c = *s++ ) != '\0' ) {
		if ( c == '\n' ) {
			x = 0;
			if ( ++y >= lines ) { /* has gone to bottom of screen */
				scroll_up();
				y--;
			}
		} else {
			vidmem [ ( x + cols * y ) * 2 ] = c; 
			if ( ++x >= cols ) {
				x = 0;
				if ( ++y >= lines ) {
					scroll_up();
					y--;
				}
			}
		}
	}
	cursor_x = x;
	cursor_y = y;
	gotoxy(x,y);
}

void putchar(char  c)
{
	int x,y;
	
	x = cursor_x;
	y = cursor_y;

	if ( c == '\n' ) { /* print next line */
		x = 0;
		if ( ++y >= lines ) { /* has gone to bottom of screen */
			scroll_up();
			y--;
		}
	} else {
		vidmem [ ( x + cols * y ) * 2 ] = c; 
		if ( ++x >= cols ) {
			x = 0;
			if ( ++y >= lines ) {
				scroll_up();
				y--;
			}
		}
	}

	cursor_x = x;
	cursor_y = y;
	gotoxy(x,y);
}

void delchar()
{
	int x,y;
	
	x = cursor_x;
	y = cursor_y;
	
	if(x==0){
	    if (y==0);
	    else { x =80; y--;}
	}else x--;
        
	vidmem [ ( x + cols * y ) * 2 ] = 0x20; 

	cursor_x = x;
	cursor_y = y;
	gotoxy(x,y);
}

unsigned char getchar(void)
{	
	unsigned char ch;
	
	if(read_q.tail == read_q.head)
		sleep_on(&wait);		

	ch = read_q.buf[read_q.tail];
	read_q.tail = (read_q.tail + 1) & (SIZE - 1);

	return ch;
}

void do_tty_interrupt(int channel)
{
	if(read_q.tail != read_q.head)
		if(wait != NULL)
			wakeup(&wait);
}

void vga_init()
{
	int x, y;
	unsigned char val;

	x = *(unsigned char *)(0x7c00+510);
	y = *(unsigned char *)(0x7c00+511);

	x = 0, y++;
	gotoxy(x,y);
	vidmem = (char *)VID_MEM;

	wait = NULL;

	put_irq_handler(0x1,&kbd_interrupt);
}

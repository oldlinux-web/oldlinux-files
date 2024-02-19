/* VGAlib version 1.0 - (c) 1992 Tommy Frandsen 		   */
/*								   */
/* This library is free software; you can redistribute it and/or   */
/* modify it without any restrictions. This library is distributed */
/* in the hope that it will be useful, but without any warranty;   */
/* without even the implied warranty of merchantability or fitness */
/* for a particular purpose.					   */

#include "vga.h"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/kd.h>
#include <linux/mm.h>
#undef free
#include <stdlib.h>

#define GRAPH_BASE 0xA0000
#define GRAPH_SIZE 0x10000
#define FONT_BASE  0xA0000
#define FONT_SIZE  0x2000
#define NULL_SIZE  0x1000

/* VGA index register ports */
#define CRT_IC  0x3D4   /* CRT Controller Index - color emulation */
#define CRT_IM  0x3B4   /* CRT Controller Index - mono emulation */
#define ATT_IW  0x3C0   /* Attribute Controller Index & Data Write Register */
#define GRA_I   0x3CE   /* Graphics Controller Index */
#define SEQ_I   0x3C4   /* Sequencer Index */
#define PEL_IW  0x3C8   /* PEL Write Index */
#define PEL_IR  0x3C7   /* PEL Read Index */

/* VGA data register ports */
#define CRT_DC  0x3D5   /* CRT Controller Data Register - color emulation */
#define CRT_DM  0x3B5   /* CRT Controller Data Register - mono emulation */
#define ATT_R   0x3C1   /* Attribute Controller Data Read Register */
#define GRA_D   0x3CF   /* Graphics Controller Data Register */
#define SEQ_D   0x3C5   /* Sequencer Data Register */
#define MIS_R   0x3CC   /* Misc Output Read Register */
#define MIS_W   0x3C2   /* Misc Output Write Register */
#define IS1_RC  0x3DA   /* Input Status Register 1 - color emulation */
#define IS1_RM  0x3BA   /* Input Status Register 1 - mono emulation */
#define PEL_D   0x3C9   /* PEL Data Register */

/* VGA indexes max counts */
#define CRT_C   24      /* 24 CRT Controller Registers */
#define ATT_C   21      /* 21 Attribute Controller Registers */
#define GRA_C   9       /* 9  Graphics Controller Registers */
#define SEQ_C   5       /* 5  Sequencer Registers */
#define MIS_C   1       /* 1  Misc Output Register */
 
/* VGA registers saving indexes */
#define CRT     0               /* CRT Controller Registers start */
#define ATT     CRT+CRT_C       /* Attribute Controller Registers start */
#define GRA     ATT+ATT_C       /* Graphics Controller Registers start */
#define SEQ     GRA+GRA_C       /* Sequencer Registers */
#define MIS     SEQ+SEQ_C       /* General Registers */
#define END     MIS+MIS_C       /* last */

#define ABS(a) (((a)<0) ? -(a) : (a))

/* variables used to shift between monchrome and color emulation */
static int CRT_I;		/* current CRT index register address */
static int CRT_D;		/* current CRT data register address */
static int IS1_R;		/* current input status register address */
static int color_text;		/* true if color text emulation */


/* graphics mode information */
struct info {
    int xdim;
    int ydim;
    int colors;
    int xbytes;
};


/* BIOS mode 0Dh - 320x200x16 */
static char g320x200x16_regs[60] = {
  0x2D,0x27,0x28,0x90,0x2B,0x80,0xBF,0x1F,0x00,0xC0,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x14,0x00,0x96,0xB9,0xE3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x01,0x00,0x0F,0x00,0x00, 
  0x00,0x0F,0x00,0x20,0x00,0x00,0x05,0x0F,0xFF, 
  0x03,0x09,0x0F,0x00,0x06, 
  0x63
};
static struct info g320x200x16_info = { 320, 200, 16, 40 };


/* BIOS mode 0Eh - 640x200x16 */
static char g640x200x16_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0xC0,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xE3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x01,0x00,0x0F,0x00,0x00, 
  0x00,0x0F,0x00,0x20,0x00,0x00,0x05,0x0F,0xFF, 
  0x03,0x01,0x0F,0x00,0x06, 
  0x63
};
static struct info g640x200x16_info = { 640, 200, 16, 80 };


/* BIOS mode 10h - 640x350x16 */
static char g640x350x16_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x40,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x83,0x85,0x5D,0x28,0x0F,0x63,0xBA,0xE3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x01,0x00,0x0F,0x00,0x00, 
  0x00,0x0F,0x00,0x20,0x00,0x00,0x05,0x0F,0xFF, 
  0x03,0x01,0x0F,0x00,0x06, 
  0xA3
};
static struct info g640x350x16_info = { 640, 350, 16, 80 };


/* BIOS mode 12h - 640x480x16 */
static char g640x480x16_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x54,0x80,0x0B,0x3E,0x00,0x40,0x00,0x00,
  0x00,0x00,0x00,0x00,0xEA,0x8C,0xDF,0x28,0x00,0xE7,0x04,0xE3,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
  0x0C,0x0D,0x0E,0x0F,0x01,0x00,0x0F,0x00,0x00,
  0x00,0x0F,0x00,0x20,0x00,0x00,0x05,0x0F,0xFF,
  0x03,0x01,0x0F,0x00,0x06,
  0xE3
};
static struct info g640x480x16_info = { 640, 480, 16, 80 };


/* BIOS mode 13h - 320x200x256 */
static char g320x200x256_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x41,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x28,0x40,0x96,0xB9,0xA3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x41,0x00,0x0F,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF, 
  0x03,0x01,0x0F,0x00,0x0E, 
  0x63
};
static struct info g320x200x256_info = { 320, 200, 256, 320 };


/* non-BIOS mode - 320x240x256 */
static char g320x240x256_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x54,0x80,0x0D,0x3E,0x00,0x41,0x00,0x00, 
  0x00,0x00,0x00,0x00,0xEA,0xAC,0xDF,0x28,0x00,0xE7,0x06,0xE3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x41,0x00,0x0F,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF, 
  0x03,0x01,0x0F,0x00,0x06, 
  0xE3
};
static struct info g320x240x256_info = { 320, 240, 256, 80 };


/* non-BIOS mode - 320x400x256 */
static char g320x400x256_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x40,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xE3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x41,0x00,0x0F,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF, 
  0x03,0x01,0x0F,0x00,0x06, 
  0x63
};
static struct info g320x400x256_info = { 320, 400, 256, 80 };


/* non-BIOS mode - 360x480x256 */
static char g360x480x256_regs[60] = {
  0x6B,0x59,0x5A,0x8E,0x5E,0x8A,0x0D,0x3E,0x00,0x40,0x00,0x00, 
  0x00,0x00,0x00,0x00,0xEA,0xAC,0xDF,0x2D,0x00,0xE7,0x06,0xE3, 
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
  0x0C,0x0D,0x0E,0x0F,0x41,0x00,0x0F,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF, 
  0x03,0x01,0x0F,0x00,0x06, 
  0xE7
};
static struct info g360x480x256_info = { 360, 480, 256, 90 };


/* default palette values */
static char default_red[256]   
             = { 0, 0, 0, 0,42,42,42,42,21,21,21,21,63,63,63,63,
		 0, 5, 8,11,14,17,20,24,28,32,36,40,45,50,56,63,
		 0,16,31,47,63,63,63,63,63,63,63,63,63,47,31,16,
		 0, 0, 0, 0, 0, 0, 0, 0,31,39,47,55,63,63,63,63,
		63,63,63,63,63,55,47,39,31,31,31,31,31,31,31,31,
		45,49,54,58,63,63,63,63,63,63,63,63,63,58,54,49,
		45,45,45,45,45,45,45,45, 0, 7,14,21,28,28,28,28,
		28,28,28,28,28,21,14, 7, 0, 0, 0, 0, 0, 0, 0, 0,
		14,17,21,24,28,28,28,28,28,28,28,28,28,24,21,17,
		14,14,14,14,14,14,14,14,20,22,24,26,28,28,28,28,
		28,28,28,28,28,26,24,22,20,20,20,20,20,20,20,20,
		 0, 4, 8,12,16,16,16,16,16,16,16,16,16,12, 8, 4,
		 0, 0, 0, 0, 0, 0, 0, 0, 8,10,12,14,16,16,16,16,
		16,16,16,16,16,14,12,10, 8, 8, 8, 8, 8, 8, 8, 8,
		11,12,13,15,16,16,16,16,16,16,16,16,16,15,13,12,
		11,11,11,11,11,11,11,11, 0, 0, 0, 0, 0, 0, 0, 0};
static char default_green[256] 
	     = { 0, 0,42,42, 0, 0,21,42,21,21,63,63,21,21,63,63,
		 0, 5, 8,11,14,17,20,24,28,32,36,40,45,50,56,63,
		 0, 0, 0, 0, 0, 0, 0, 0, 0,16,31,47,63,63,63,63,
		63,63,63,63,63,47,31,16,31,31,31,31,31,31,31,31,
		31,39,47,55,63,63,63,63,63,63,63,63,63,55,47,39,
		45,45,45,45,45,45,45,45,45,49,54,58,63,63,63,63,
		63,63,63,63,63,58,54,49, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 7,14,21,29,28,28,28,28,28,28,28,28,21,14, 7,
		14,14,14,14,14,14,14,14,14,17,21,24,28,28,28,28,
		28,28,28,28,28,24,21,17,20,20,20,20,20,20,20,20,
		20,22,24,26,28,28,28,28,28,28,28,28,28,26,24,22,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 8,12,16,16,16,16,
		16,16,16,16,16,12, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8,
		 8,10,12,14,16,16,16,16,16,16,16,16,16,14,12,10,
		11,11,11,11,11,11,11,11,11,12,13,15,16,16,16,16,
		16,16,16,16,16,15,13,12, 0, 0, 0, 0, 0, 0, 0, 0};
static char default_blue[256]  
             = { 0,42, 0,42, 0,42, 0,42,21,63,21,63,21,63,21,63,
		 0, 5, 8,11,14,17,20,24,28,32,36,40,45,50,56,63,
		63,63,63,63,63,47,31,16, 0, 0, 0, 0, 0, 0, 0, 0,
		 0,16,31,47,63,63,63,63,63,63,63,63,63,55,47,39,
		31,31,31,31,31,31,31,31,31,39,47,55,63,63,63,63,
		63,63,63,63,63,58,54,49,45,45,45,45,45,45,45,45,
		45,49,54,58,63,63,63,63,28,28,28,28,28,21,14, 7,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,14,21,28,28,28,28,
		28,28,28,28,28,24,21,17,14,14,14,14,14,14,14,14,
		14,17,21,24,28,28,28,28,28,28,28,28,28,26,24,22,
		20,20,20,20,20,20,20,20,20,22,24,26,28,28,28,28,
		16,16,16,16,16,12, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 4, 8,12,16,16,16,16,16,16,16,16,16,14,12,10,
		 8, 8, 8, 8, 8, 8, 8, 8, 8,10,12,14,16,16,16,16,
		16,16,16,16,16,15,13,12,11,11,11,11,11,11,11,11,
		11,12,13,15,16,16,16,16, 0, 0, 0, 0, 0, 0, 0, 0};


/* used to decompose color value into bits (for fast scanline drawing) */
union bits {
    struct {
        char bit3;
        char bit2;
        char bit1;
        char bit0;
    } b;
    unsigned int i;
};

/* color decompositions */
static union bits color16[16] = {{0,0,0,0},
			         {0,0,0,1},
			      	 {0,0,1,0},
			         {0,0,1,1},
			         {0,1,0,0},
			         {0,1,0,1},
			         {0,1,1,0},
			         {0,1,1,1},
			         {1,0,0,0},
			         {1,0,0,1},
			         {1,0,1,0},
			         {1,0,1,1},
			         {1,1,0,0},
			         {1,1,0,1},
			         {1,1,1,0},
			         {1,1,1,1}};

/* display plane buffers (for fast scanline drawing) */
static char plane0[256];
static char plane1[256];
static char plane2[256];
static char plane3[256];


static char text_regs[60];   /* VGA registers for saved text mode */   

/* saved text mode palette values */ 
static char text_red[256];  
static char text_green[256];  
static char text_blue[256];  

/* saved graphics mode palette values */ 
static char graph_red[256];  
static char graph_green[256];  
static char graph_blue[256];  

static int         prv_mode  = TEXT;     /* previous video mode      */
static int         cur_mode  = TEXT;     /* current video mode       */
static int         flip_mode = TEXT;     /* flipped video mode       */
static struct info cur_info;             /* current video parameters */
static int         cur_color;            /* current color            */

static int initialized = 0;   /* flag: initialize() called ?  */
static int flip        = 0;   /* flag: executing vga_flip() ? */

static int   tty0_fd;    /* /dev/tty0 file descriptor 		     */
static int   mem_fd;     /* /dev/mem file descriptor		     */
static FILE* console;    /* console input stream		     */
static char* graph_mem;  /* dummy buffer for mmapping grahics memory */

static char* graph_buf = NULL;  /* saves graphics data during flip */

static char font_buf1[FONT_SIZE];  /* saved font data - plane 2 */
static char font_buf2[FONT_SIZE];  /* saved font data - plane 3 */
static char null_buf[NULL_SIZE];   /* used to speed up clear */

static struct termios text_termio;  /* text mode termio parameters     */
static struct termios graph_termio; /* graphics mode termio parameters */

int flipchar = '\x1b';   /* flip character - initially  ESCAPE */


static void inline port_out(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1"
		::"a" ((char) value),"d" ((unsigned short) port));
}


static unsigned char inline port_in(unsigned short port)
{
	unsigned char _v;
__asm__ volatile ("inb %1,%0"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}


static void set_graphtermio()
{
        /* set graphics mode termio parameters */
        ioctl(0, TCSETSW, &graph_termio);
}


static void set_texttermio()
{
    /* restore text mode termio parameters */
    ioctl(0, TCSETSW, &text_termio);
}


static void disable_interrupt()
{
    struct termios cur_termio;

    ioctl(0, TCGETS, &cur_termio);
    cur_termio.c_lflag &= ~ISIG;
    ioctl(0, TCSETSW, &cur_termio);
}


static void enable_interrupt()
{
    struct termios cur_termio;

    ioctl(0, TCGETS, &cur_termio);
    cur_termio.c_lflag |= ISIG;
    ioctl(0, TCSETSW, &cur_termio);
}


static int set_regs(char regs[])
{
    int i;

    /* disable video */
    port_in(IS1_R);	        
    port_out(0x00, ATT_IW);     
  
    /* update misc output register */
    port_out(regs[MIS], MIS_W);         

    /* synchronous reset on */
    port_out(0x00,SEQ_I); 
    port_out(0x01,SEQ_D);	        
  
    /* write sequencer registers */
    for (i = 1; i < SEQ_C; i++) {       
	port_out(i, SEQ_I); 
	port_out(regs[SEQ+i], SEQ_D); 
    }

    /* synchronous reset off */
    port_out(0x00, SEQ_I); 
    port_out(0x03, SEQ_D);	        
  
    /* deprotect CRT registers 0-7 */
    port_out(0x11, CRT_I);		  
    port_out(port_in(CRT_D)&0x7F, CRT_D);   
  
    /* write CRT registers */
    for (i = 0; i < CRT_C; i++) {       
	port_out(i, CRT_I); 
	port_out(regs[CRT+i], CRT_D); 
    }

    /* write graphics controller registers */
    for (i = 0; i < GRA_C; i++) {       
	port_out(i, GRA_I); 
	port_out(regs[GRA+i], GRA_D); 
    }
     
    /* write attribute controller registers */
    for (i = 0; i < ATT_C; i++) {       
	port_in(IS1_R);   /* reset flip-flop */
	port_out(i, ATT_IW);
	port_out(regs[ATT+i],ATT_IW);
    }
}


static void interrupt_handler(int value) 
{
    /* handle SIGINT - restore text mode and exit */
    if (cur_mode != TEXT) 
	vga_setmode(TEXT);
    set_texttermio();
    exit(-1);

}


static void initialize()
{
    int  i, j;

    /* save text mode termio parameters */
    ioctl(0, TCGETS, &text_termio);

    graph_termio = text_termio;

    /* change termio parameters to allow our own I/O processing */
    graph_termio.c_iflag &= ~(BRKINT|PARMRK|INPCK|IUCLC|IXON|IXOFF);
    graph_termio.c_iflag |=  (IGNBRK|IGNPAR);

    graph_termio.c_oflag &= ~(ONOCR);

    graph_termio.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|NOFLSH);
    graph_termio.c_lflag |= (ISIG);   /* enable interrupt */

    graph_termio.c_cc[VMIN] = 1;
    graph_termio.c_cc[VTIME] = 0;
    graph_termio.c_cc[VSUSP] = 0;   /* disable suspend */

    disable_interrupt();

    /* get I/O permissions for VGA registers */
    if (ioperm(CRT_IC, 1, 1)) {
	printf("VGAlib: can't get I/O permissions \n");
	exit (-1);
    }
    ioperm(CRT_IM,  1, 1);
    ioperm(ATT_IW, 1, 1);
    ioperm(GRA_I,  1, 1);
    ioperm(SEQ_I,  1, 1);
    ioperm(PEL_IW, 1, 1);
    ioperm(PEL_IR, 1, 1);
    ioperm(CRT_DC,  1, 1);
    ioperm(CRT_DM,  1, 1);
    ioperm(ATT_R,  1, 1);
    ioperm(GRA_D,  1, 1);
    ioperm(SEQ_D,  1, 1);
    ioperm(MIS_R,  1, 1);
    ioperm(MIS_W,  1, 1);
    ioperm(IS1_RC,  1, 1);
    ioperm(IS1_RM,  1, 1);
    ioperm(PEL_D,  1, 1);

    /* color or monochrome text emulation? */
    color_text = port_in(MIS_R)&0x01;

    /* chose registers for color/monochrome emulation */
    if (color_text) {
	CRT_I = CRT_IC;
	CRT_D = CRT_DC;
	IS1_R = IS1_RC;
    } else {
	CRT_I = CRT_IM;
	CRT_D = CRT_DM;
	IS1_R = IS1_RM;
    }

    /* open /dev/tty0 - current virtual console */
    if ((tty0_fd = open("/dev/tty0", O_RDONLY) ) < 0) {
	printf("VGAlib: can't open /dev/tty0 \n");
	exit (-1);
    }
    console = fdopen(tty0_fd, "r"); 

    /* disable text output to console */
    ioctl(tty0_fd, KDSETMODE, KD_GRAPHICS);
    
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR) ) < 0) {
	printf("VGAlib: can't open /dev/mem \n");
	exit (-1);
    }

    /* mmap graphics memory */
    if ((graph_mem = malloc(GRAPH_SIZE + (PAGE_SIZE-1))) == NULL) {
	printf("VGAlib: allocation error \n");
	exit (-1);
    }
    if ((unsigned long)graph_mem % PAGE_SIZE)
	graph_mem += PAGE_SIZE - ((unsigned long)graph_mem % PAGE_SIZE);
    graph_mem = (unsigned char *)mmap(
	(caddr_t)graph_mem, 
	GRAPH_SIZE,
	PROT_READ|PROT_WRITE,
	MAP_SHARED|MAP_FIXED,
	mem_fd, 
	GRAPH_BASE
    );
    if ((long)graph_mem < 0) {
	printf("VGAlib: mmap error \n");
	exit (-1);
    }

    /* disable video */
    port_in(IS1_R);	        
    port_out(0x00, ATT_IW);     
  
    /* save text mode palette - first select palette index 0 */
    port_out(0, PEL_IR); 

    /* read RGB components - index is autoincremented */
    for(i = 0; i < 256; i++) {
	for(j = 0; j < 10; j++) ;   /* delay (minimum 240ns) */ 
	text_red[i] = port_in(PEL_D);
	for(j = 0; j < 10; j++) ;   /* delay (minimum 240ns) */ 
	text_green[i] = port_in(PEL_D);
	for(j = 0; j < 10; j++) ;   /* delay (minimum 240ns) */ 
	text_blue[i] = port_in(PEL_D);
    }

    /* save text mode VGA registers */
    for (i = 0; i < CRT_C; i++) {
	 port_out(i, CRT_I); 
	 text_regs[CRT+i] = port_in(CRT_D); 
    }
    for (i = 0; i < ATT_C; i++) {
      	 port_in(IS1_R);
         port_out(i, ATT_IW); 
         text_regs[ATT+i] = port_in(ATT_R); 
    }
    for (i = 0; i < GRA_C; i++) {
       	 port_out(i, GRA_I); 
       	 text_regs[GRA+i] = port_in(GRA_D); 
    }
    for (i = 0; i < SEQ_C; i++) {
       	 port_out(i, SEQ_I); 
       	 text_regs[SEQ+i] = port_in(SEQ_D); 
    }
    text_regs[MIS] = port_in(MIS_R); 

    /* shift to color emulation */
    CRT_I = CRT_IC;
    CRT_D = CRT_DC;
    IS1_R = IS1_RC;
    port_out(port_in(MIS_R)|0x01, MIS_W); 

    /* save font data - first select a 16 color graphics mode */
    set_regs(g640x480x16_regs);

    /* save font data in plane 2 */
    port_out(0x04, GRA_I); 
    port_out(0x02, GRA_D); 
    memcpy(font_buf1, graph_mem, FONT_SIZE);

    /* save font data in plane 3 */
    port_out(0x04, GRA_I); 
    port_out(0x03, GRA_D); 
    memcpy(font_buf2, graph_mem, FONT_SIZE);

    /* initialize buffer used when clearing in 256 color modes */
    for(i = 0; i < NULL_SIZE; i++)
        null_buf[i] = 0;

    initialized = 1;

    /* do our own interrupt handling */
    (void) signal(SIGINT, interrupt_handler);
}


int vga_setmode(int mode) 
{
    int i;

    if (!initialized)
        initialize();

    disable_interrupt();

    prv_mode = cur_mode;
    cur_mode = mode;

    /* disable video */
    port_in(IS1_R); 		
    port_out(0x00, ATT_IW);	
  
    if (mode == TEXT) {    
        /* restore font data - first select a 16 color graphics mode */
        set_regs(g640x480x16_regs);

	/* disable Set/Reset Register */
    	port_out(0x01, GRA_I ); 
    	port_out(0x00, GRA_D );   

        /* restore font data in plane 2 - necessary for all VGA's */
    	port_out(0x02, SEQ_I ); 
    	port_out(0x04, SEQ_D );   
	memcpy(graph_mem, font_buf1, FONT_SIZE);

        /* restore font data in plane 3 - necessary for Trident VGA's */
    	port_out(0x02, SEQ_I ); 
    	port_out(0x08, SEQ_D );   
	memcpy(graph_mem, font_buf2, FONT_SIZE);

        /* change register adresses if monochrome text mode */
        if (!color_text) {
            CRT_I = CRT_IM;
            CRT_D = CRT_DM;
            IS1_R = IS1_RM;
            port_out(port_in(MIS_R)&0xFE, MIS_W); 
        }

	/* restore text mode VGA registers */
    	set_regs(text_regs);

        /* restore saved palette */
        for(i = 0; i < 256; i++)
            vga_setpalette(
                i, text_red[i], text_green[i], text_blue[i]
            );

	/* enable text output - restores the screen contents */ 
        ioctl(tty0_fd, KDSETMODE, KD_TEXT);

        /* enable video */
        port_in(IS1_R); 
        port_out(0x20, ATT_IW); 
  
        /* restore text mode termio */
        set_texttermio();

    } else {
	/* disable text output */
        ioctl(tty0_fd, KDSETMODE, KD_GRAPHICS);

        /* shift to color emulation */
        CRT_I = CRT_IC;
        CRT_D = CRT_DC;
        IS1_R = IS1_RC;
        port_out(port_in(MIS_R)|0x01, MIS_W); 

        switch (mode) {
            case G320x200x16:
                set_regs(g320x200x16_regs);
                cur_info = g320x200x16_info;
                break;
            case G640x200x16:
                set_regs(g640x200x16_regs);
                cur_info = g640x200x16_info;
                break;
            case G640x350x16:
                set_regs(g640x350x16_regs);
                cur_info = g640x350x16_info;
                break;
            case G640x480x16:
                set_regs(g640x480x16_regs);
                cur_info = g640x480x16_info;
                break;
	    case G320x200x256:
                set_regs(g320x200x256_regs);
                cur_info = g320x200x256_info;
                break;
	    case G320x240x256:
                set_regs(g320x240x256_regs);
                cur_info = g320x240x256_info;
                break;
	    case G320x400x256:
                set_regs(g320x400x256_regs);
                cur_info = g320x400x256_info;
                break;
	    case G360x480x256:
                set_regs(g360x480x256_regs);
                cur_info = g360x480x256_info;
                break;
        }

        if (!flip) {
            /* set default palette */
            for(i = 0; i < 256; i++)
                vga_setpalette(
                    i, default_red[i], default_green[i], default_blue[i]
                );
 
            /* clear screen (sets current color to 15) */
            vga_clear();
        }    

        /* enable video */
        port_in(IS1_R); 
        port_out(0x20, ATT_IW); 
  
        /* set graphics mode termio */
        set_graphtermio();
    }

    return 0;  
}


int vga_clear()
{
    int i;

    vga_screenoff();

    switch (cur_mode) {
        case G320x200x16:
        case G640x200x16:
        case G640x350x16:
        case G640x480x16:
            vga_setcolor(0);

	    /* write to all bits */
    	    port_out(0x08, GRA_I ); 
            port_out(0xFF, GRA_D );   

    	    /* write dummy values to clear video memory */
            for(i = 0; i < 16; i++)
		memcpy(graph_mem + i*NULL_SIZE, null_buf, NULL_SIZE);

            break;
	case G320x200x256:
	case G320x240x256:
	case G320x400x256:
	case G360x480x256:
            /* write to all planes */ 
            port_out(0x02, SEQ_I ); 
            port_out(0x0F, SEQ_D );
   
    	    /* clear video memory */
            for(i = 0; i < 16; i++)
		memcpy(graph_mem + i*NULL_SIZE, null_buf, NULL_SIZE);

            break;
    }

    vga_setcolor(15);
        
    vga_screenon();

    return 0;
}


int inline vga_setcolor(int color)
{
    switch (cur_mode) {
        case G320x200x16:
        case G640x200x16:
        case G640x350x16:
        case G640x480x16:
            /* update set/reset register */
	    port_out(0x00, GRA_I ); 
	    port_out(color, GRA_D );   
            break;
	case G320x200x256:
	case G320x240x256:
	case G320x400x256:
	case G360x480x256:
	    cur_color = color;
            break;
    }
        
    return 0;
}


int vga_setpalette(int index, int red, int green, int blue)
{
    int i;

    /* select palette register */
    port_out(index, PEL_IW); 

    /* write RGB components */
    for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */ 
    port_out(red, PEL_D);
    for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */ 
    port_out(green, PEL_D);
    for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */ 
    port_out(blue, PEL_D);

    return 0;
}


int vga_getpalette(int index, int *red, int *green, int *blue)
{
    int i;

    /* select palette register */
    port_out(index, PEL_IR);

    /* read RGB components */
    for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
    *red = (int) port_in(PEL_D);
    for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
    *green = (int) port_in(PEL_D);
    for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
    *blue = (int) port_in(PEL_D);

    return 0;
}


int vga_setpalvec(int start, int num, int *pal)
{
    int i, j;

    /* select palette register */
    port_out(start, PEL_IW);

    for(j = 0; j < num; j++) {
        for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
        port_out(*(pal++), PEL_D);
        for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
        port_out(*(pal++), PEL_D);
        for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
        port_out(*(pal++), PEL_D);
    }

    return j;
}


int vga_getpalvec(int start, int num, int *pal)
{
    int i, j;

    /* select palette register */
    port_out(start, PEL_IR);

    for(j = 0; j < num; j++) {
        for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
        *(pal++) = (int) port_in(PEL_D);
        for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
        *(pal++) = (int) port_in(PEL_D);
        for(i = 0; i < 10; i++) ;   /* delay (minimum 240ns) */
        *(pal++) = (int) port_in(PEL_D);
    }

    return i;
}


int inline vga_drawpixel(int x, int y)
{
    unsigned long offset;

    switch (cur_mode) {
        case G320x200x16:
        case G640x200x16:
        case G640x350x16:
        case G640x480x16:
	    /* select bit */
	    port_out(8, GRA_I);
	    port_out(0x80 >> (x & 7), GRA_D);   

            /* read into latch and write dummy back */
	    offset = y*cur_info.xbytes + (x>>3);   
            graph_mem[offset] = graph_mem[offset];   
            break;
	case G320x200x256:
            /* write color to pixel */
	    graph_mem[y*320 + x] = cur_color;  
            break;
	case G320x240x256:
	case G320x400x256:
	case G360x480x256:
	    /* select plane */ 
	    port_out(0x02, SEQ_I); 
    	    port_out(1 << (x & 3), SEQ_D);   

            /* write color to pixel */
	    graph_mem[y*cur_info.xbytes + (x>>2)] = cur_color;  
            break;
    }
        
    return 0;
}


int vga_drawscanline(int line, char* colors)
{
    switch (cur_mode) {
        case G320x200x16:
        case G640x200x16:
        case G640x350x16:
        case G640x480x16:
	    {
       		int i, j, k, first, last;
		union bits bytes;
                char* address;

                k = 0;
                for(i = 0; i < cur_info.xdim; i += 8) {
                    bytes.i = 0;
                    first = i;
                    last  = i+8;
                    for(j = first; j < last; j++)
                       bytes.i = (bytes.i<<1) | color16[colors[j]].i;
		    plane0[k]   = bytes.b.bit0;
		    plane1[k]   = bytes.b.bit1;
		    plane2[k]   = bytes.b.bit2;
		    plane3[k++] = bytes.b.bit3;
                }

                address = graph_mem + line*cur_info.xbytes;

		/* disable Set/Reset Register */
	    	port_out(0x01, GRA_I ); 
    		port_out(0x00, GRA_D ); 

		/* write to all bits */
	        port_out(0x08, GRA_I ); 
    		port_out(0xFF, GRA_D );   

		/* select map mask register */
	    	port_out(0x02, SEQ_I ); 

                /* write plane 0 */
    		port_out(0x01, SEQ_D ); 
	        memcpy(address, plane0, cur_info.xbytes);

                /* write plane 1 */
    		port_out(0x02, SEQ_D ); 
	        memcpy(address, plane1, cur_info.xbytes);

                /* write plane 2 */
    		port_out(0x04, SEQ_D ); 
	        memcpy(address, plane2, cur_info.xbytes);

                /* write plane 3 */
    		port_out(0x08, SEQ_D ); 
	        memcpy(address, plane3, cur_info.xbytes);

                /* restore map mask register */
    		port_out(0x0F, SEQ_D ); 
  
		/* enable Set/Reset Register */
	    	port_out(0x01, GRA_I ); 
    		port_out(0x0F, GRA_D );   
            }
            break;
	case G320x200x256:
            /* linear addressing - easy and fast */
	    memcpy(graph_mem + line*cur_info.xbytes, colors, cur_info.xbytes);
            break;
	case G320x240x256:
	case G320x400x256:
	case G360x480x256: 
            {
                int first, last, offset, pixel, plane;

	        /* select map mask register */ 
	        port_out(0x02, SEQ_I); 

                for(plane = 0; plane < 4; plane++) {
                    /* select plane */
    	            port_out(1 << plane, SEQ_D);   

                    pixel = plane;
                    first = cur_info.xbytes * line;
                    last  = cur_info.xbytes * (line+1);
                    for(offset = first; offset < last; offset++) {
		        graph_mem[offset] = colors[pixel];  
                        pixel += 4;
                    }
	        }
            }
            break;
    }
        
    return 0;
}


int vga_drawline(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int ax = ABS(dx) << 1;
    int ay = ABS(dy) << 1;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
 
    int x  = x1;
    int y  = y1;

    if (ax > ay) {
        int d = ay - (ax >> 1);
        while (x != x2) {
	    vga_drawpixel(x, y);

            if (d > 0 || d == 0 && sx == 1) {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    } else {
        int d = ax - (ay >> 1);
        while (y != y2) {
	    vga_drawpixel(x, y);

            if (d > 0 || d == 0 && sy == 1) {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
    vga_drawpixel(x, y);
     
    return 0;
}


int vga_screenoff()
{
    /* turn off screen for faster VGA memory acces */
    port_out(0x01, SEQ_I); 		  
    port_out(port_in(SEQ_D)|0x20, SEQ_D);   

    return 0;
}


int vga_screenon()
{
    /* turn screen back on */
    port_out(0x01, SEQ_I); 		  
    port_out(port_in(SEQ_D)&0xDF, SEQ_D);   

    return 0;
}


int vga_getxdim()
{
    return cur_info.xdim;
}


int vga_getydim()
{
    return cur_info.ydim;
}


int vga_getcolors()
{
    return cur_info.colors;
}


int vga_getch()
{
    char c;

    if (cur_mode == TEXT)
	return -1;

    read(tty0_fd, &c, 1);
    while(c == flipchar) {
	vga_flip();

	set_graphtermio();
        read(tty0_fd, &c, 1);

	vga_flip();
        read(tty0_fd, &c, 1);
    }

    return c;
}


int vga_setflipchar(int c)
{
    flipchar = c;

    return 0;
}


int vga_flip()
{
    int i, j;

    if (cur_mode == TEXT && flip_mode == TEXT)
	return -1;

    flip = 1;

    disable_interrupt();

    /* disable video */
    port_in(IS1_R); 		
    port_out(0x00, ATT_IW);	
  
    if (cur_mode == TEXT) {
	/* disable text output */
        ioctl(tty0_fd, KDSETMODE, KD_GRAPHICS);

        /* restore all four planes - first select a 16 color graphics mode */
        set_regs(g640x480x16_regs);

	/* disable Set/Reset Register */
    	port_out(0x01, GRA_I ); 
    	port_out(0x00, GRA_D );   

        for(i = 0; i < 4; i++) {
            /* restore plane i */
    	    port_out(0x02, SEQ_I ); 
    	    port_out(1<<i, SEQ_D );   
            memcpy(graph_mem, graph_buf + i*GRAPH_SIZE, GRAPH_SIZE);
	}

        free(graph_buf);

        /* restore saved palette */
        for(i = 0; i < 256; i++)
            vga_setpalette(
                i, graph_red[i], graph_green[i], graph_blue[i]
            );

        vga_setmode(flip_mode);

	flip_mode = TEXT;
    } else {
	/* save all four planes - first select a 16 color graphics mode */
        set_regs(g640x480x16_regs);

	/* allocate memory for saved graphics data - only if necessary */
        if ((graph_buf = malloc(4*GRAPH_SIZE)) == NULL) {
            printf("vga_flip: allocation error \n");
            exit (-1);
        }

        for(i = 0; i < 4; i++) {
            /* save plane i */
            port_out(0x04, GRA_I); 
            port_out(   i, GRA_D); 
            memcpy(graph_buf + i*GRAPH_SIZE, graph_mem, GRAPH_SIZE);
	}

    	/* save graphics mode palette - first select palette index 0 */
    	port_out(0, PEL_IR); 

    	/* read RGB components - index is autoincremented */
    	for(i = 0; i < 256; i++) {
            for(j = 0; j < 10; j++) ;   /* delay (minimum 240ns) */ 
            graph_red[i] = port_in(PEL_D);
    	    for(j = 0; j < 10; j++) ;   /* delay (minimum 240ns) */ 
            graph_green[i] = port_in(PEL_D);
    	    for(j = 0; j < 10; j++) ;   /* delay (minimum 240ns) */ 
            graph_blue[i] = port_in(PEL_D);
        }

        flip_mode = cur_mode;

	vga_setmode(TEXT);
    }

    flip = 0;

    return 0;
}

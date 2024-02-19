#include <tty.h> 
#include <asm/io.h>
#include <cnix/sched.h>

#define DEBUG 0

static unsigned char e0;
static unsigned char mode = 0;
static unsigned char leds;

static unsigned char shift_map[] = {
  0,27,
  '!','@','#','$','%','^','&','*','(',')','_','+',
  127,9
  ,'Q','W','E','R','T','Y','U','I','O','P','{','}',
  10,0,
  'A','S','D','F','G','H','J','K','L',':','"',
  0,0
  ,'|','Z','X','C','V','B','N','M','<','>','?',
  0,'\'','*',0,32,		/* 36-39 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 3A-49 */
  '-',0,0,0,'+',	/* 4A-4E */
  0,0,0,0,0,0,0	/* 4F-55 */
  ,'>',
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  
};
static unsigned char key_map[] = {
  0,27,
  '1','2','3','4','5','6','7','8','9','0','-','=',
  127,9,
  'q','w','e','r','t','y','u','i','o','p','[',
  0,10,0,
  'a','s','d','f','g','h','j','k','l',';','\'',
  0,0,
  '\\','z','x','c','v','b','n','m',',','.','/',
  0,'*',0,32,		/* 36-39 */
  16,1,0	,	/* 3A-49 */
  '-',0,0,0,'+',	/* 4A-4E */
  0,0,0,0,0,0,0,	/* 4F-55 */
  '<',
  10,1,0
};



static unsigned char alt_map[] = {
  0,0,
  0,'@',0,'$',0,0,'{','[',']','}','\\',0,
  0,0,
  0,0,0,0,0,0,0,0,0,0,0,
  '~',10,0,
  0,0,0,0,0,0,0,0,0,0,0,
  0,0,
  0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,		/* 36-39 */
  16,1,0,		/* 3A-49 */
  0,0,0,0,0,		/* 4A-4E */
  0,0,0,0,0,0,0,	/* 4F-55 */
  '|',
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


static long func_table[] = {
  0x415b5b1b,0x425b5b1b,0x435b5b1b,0x445b5b1b,
  0x455b5b1b,0x465b5b1b,0x475b5b1b,0x485b5b1b,
  0x495b5b1b,0x4a5b5b1b,0x4b5b5b1b,0x4c5b5b1b
};

static int do_self(int scancode);
static int uncaps(int scancode);
static int caps(int scancode);
static int lshift(int scancode);
static int rshift(int scancode);
static int unctrl(int scancode);
static int ctrl(int scancode);
static int none(int scancode);
static int num(int scancode);
static int minus(int scancode);
static int cursor(int scancode);

static int func(int scancode);
static int scroll(int scancode);
static int alt(int scancode);
static int unalt(int scancode);

static int unlshift(int scancode);
static int unrshift(int scancode);
static int setleds();
static int put_queue(long);
static int kbd_wait();

int reboot();

/* extern struct tty_quenue * table_list[6];  */
extern struct tty_queue read_q;
extern int do_tty_interrupt(int);
#define SIZE  64

typedef int (*keyfn_ptr)();
static keyfn_ptr key_tab[] = {
  none,do_self,do_self,do_self	/* 00-03 s0 esc 1 2 */
  , do_self,do_self,do_self,do_self	/* 04-07 3 4 5 6 */
  , do_self,do_self,do_self,do_self	/* 08-0B 7 8 9 0 */
  , do_self,do_self,do_self,do_self	/* 0C-0F + ' bs tab */
  , do_self,do_self,do_self,do_self	/* 10-13 q w e r */
  , do_self,do_self,do_self,do_self	/* 14-17 t y u i */
  , do_self,do_self,do_self,do_self	/* 18-1B o p } ^ */
  , do_self,ctrl,do_self,do_self	/* 1C-1F enter ctrl a s */
  , do_self,do_self,do_self,do_self	/* 20-23 d f g h */
  , do_self,do_self,do_self,do_self	/* 24-27 j k l | */
  , do_self,do_self,lshift,do_self	/* 28-2B { para lshift , */
  , do_self,do_self,do_self,do_self	/* 2C-2F z x c v */
  , do_self,do_self,do_self,do_self	/* 30-33 b n m , */
  , do_self,minus,rshift,do_self	/* 34-37 . - rshift * */
  , alt,do_self,caps,func		/* 38-3B alt sp caps f1 */
  , func,func,func,func		/* 3C-3F f2 f3 f4 f5 */
  , func,func,func,func		/* 40-43 f6 f7 f8 f9 */
  , func,num,scroll,cursor		/* 44-47 f10 num scr home */
  , cursor,cursor,do_self,cursor	/* 48-4B up pgup - left */
  , cursor,cursor,do_self,cursor	/* 4C-4F n5 right + end */
  , cursor,cursor,cursor,cursor	/* 50-53 dn pgdn ins del */
  , none,none,do_self,func		/* 54-57 sysreq ? < f11 */
  , func,none,none,none		/* 58-5B f12 ? ? ? */
  , none,none,none,none		/* 5C-5F ? ? ? ? */
  , none,none,none,none		/* 60-63 ? ? ? ? */
  , none,none,none,none		/* 64-67 ? ? ? ? */
  , none,none,none,none		/* 68-6B ? ? ? ? */
  , none,none,none,none		/* 6C-6F ? ? ? ? */
  , none,none,none,none		/* 70-73 ? ? ? ? */
  , none,none,none,none		/* 74-77 ? ? ? ? */
  , none,none,none,none		/* 78-7B ? ? ? ? */
  , none,none,none,none		/* 7C-7F ? ? ? ? */
  , none,none,none,none		/* 80-83 ? br br br */
  , none,none,none,none		/* 84-87 br br br br */
  , none,none,none,none		/* 88-8B br br br br */
  , none,none,none,none		/* 8C-8F br br br br */
  , none,none,none,none		/* 90-93 br br br br */
  , none,none,none,none		/* 94-97 br br br br */
  , none,none,none,none		/* 98-9B br br br br */
  , none,unctrl,none,none		/* 9C-9F br unctrl br br */
  , none,none,none,none		/* A0-A3 br br br br */
  , none,none,none,none		/* A4-A7 br br br br */
  , none,none,unlshift,none		/* A8-AB br br unlshift br */
  , none,none,none,none		/* AC-AF br br br br */
  , none,none,none,none		/* B0-B3 br br br br */
  , none,none,unrshift,none		/* B4-B7 br br unrshift br */
  , unalt,none,uncaps,none		/* B8-BB unalt br uncaps br */
  , none,none,none,none		/* BC-BF br br br br */
  , none,none,none,none		/* C0-C3 br br br br */
  , none,none,none,none		/* C4-C7 br br br br */
  , none,none,none,none		/* C8-CB br br br br */
  , none,none,none,none		/* CC-CF br br br br */
  , none,none,none,none		/* D0-D3 br br br br */
  , none,none,none,none		/* D4-D7 br br br br */
  , none,none,none,none		/* D8-DB br ? ? ? */
  , none,none,none,none		/* DC-DF ? ? ? ? */
  , none,none,none,none		/* E0-E3 e0 e1 ? ? */
  , none,none,none,none		/* E4-E7 ? ? ? ? */
  , none,none,none,none		/* E8-EB ? ? ? ? */
  , none,none,none,none		/* EC-EF ? ? ? ? */
  , none,none,none,none		/* F0-F3 ? ? ? ? */
  , none,none,none,none		/* F4-F7 ? ? ? ? */
  , none,none,none,none		/* F8-FB ? ? ? ? */
  , none,none,none,none		/* FC-FF ? ? ? ? */
} ;


static unsigned char num_tab[] = { '7','8','9',' ','4','5','6',' ','1','2','3','0'};
static unsigned char cur_tab[] =  "HA5 DGC YB623" ;

static int code ;

int kbd_interrupt()
{
	code = inb_p(0x60);
#if DEBUG
	printk("%x ",code);
#endif
	if ( code == 0xe0 )   
		e0 = 1;
	else   if ( code == 0xe1 )
		e0 = 2; 
	else {
		keyfn_ptr funp;
		funp = key_tab[code];
		funp(code) ;
		e0  = 0;
	}
	outb_p((inb_p(0x61))|0x80,0x61);
	outb_p((inb_p(0x61))&0x7f,0x61);

	do_tty_interrupt(0);
	
	return 0; /* avoid warning */
}

int do_self(int scancode)
{

	char *map;
	unsigned char ch;
	int shftflg = 0 ; /* shift key is not pressed */

	if (mode & 0x20 ) /* alt */
		map = alt_map;
	else if ( (mode & 0x3) ){ /* shift ^ caps */ 
		map = shift_map;
		shftflg = 1;
	}
	else map = key_map;

	ch = map[scancode & 0xff];
#if DEBUG
	printk("%c\t",ch);
#endif
	if ( ch == 0) return 0;

	if ((mode & 0x4c)){ /* ctrl or  caps */
		if ((ch <= 'z' && ch >= 'a' )) /* caps lower*/
			ch -= 0x20 ; /* to upper */
		if (mode & 0xc) /* ctrl */	
			ch -= 0x40;        /* to ctrl ascii */

	}
	if (mode & 0x10) 
		ch |= 0x80 ; /* alt */

	ch &= 0xff;
	put_queue(ch);
	return 0; /* avoid warning */
}


int alt(int scancode)
{
	if ( e0)  /* right ctrl */
		mode |= 0x20;
	else 
		mode |= 0x10;
	return 0;
}

int unalt(int scancode)
{
	if (e0)
		mode &= 0xdf;
	else 
		mode &= 0xef;
	return 0;
}
int ctrl(int scancode)
{
	if ( e0)
		mode |= 0x8;
	else 
		mode |= 0x4;
	return 0;
}

int unctrl(int scancode)
{
	if ( e0)
		mode &= 0xF7;
	else
		mode &= 0xFB;
	  return 0;
}

int lshift(int scancode)
{
	mode |= 0x1;
	  return 0;
}

int rshift(int scancode)
{
	mode |= 0x2;
	return 0;
}

int unlshift(int scancode)
{
	mode &= 0xfe;
	return 0;
}

int unrshift(int scancode)
{
	mode &= 0xfd;
	return 0;
}

int caps(int scancode)
{
	if (!(mode & 0x80)){
		leds ^= 0x4;
		mode ^= 0x40;
		mode |= 0x80;
		setleds();
	}
	return 0;
}

int uncaps(int scancode)
{
	mode &= 0x7f;
	return 0;
}

int srcoll(int scancode)
{
	leds |= 0x1;
	setleds();
	return 0;
}

int num(int scancode)
{
	leds ^= 0x2;
	setleds();
	return 0;
}

int cursor(int scancode)
{
	int i;

	i =  code - 0x47 ; /* cursor start with 71 */

	if ( i < 0 || i > 12) return 0;

	if ( i == 12 && (mode & 0xc ) && (mode & 0x30)) /* alt + ctrl + del */
		reboot();

	if ((e0 == 1 && !(leds & 0x2))|| (mode &0x3)){ /* num is not on and extends code or  */
		int ch;
		ch = cur_tab[i];

		if ( ch <= '9' ) ch ='~';

		ch = ch<<16|0x5b1b; /* arrow direction */

		put_queue(ch);
	} else {  /* leds is on  , is num . this is not sure !!!! */
		int ch;
		ch = num_tab[i];
		put_queue(ch);
	}
	return 0;
}

int func(int scancode)
{

	int i = scancode - 59;     /* function key starts from 59 */

	if ( i >= 0 && i <= 11) {  /* 12 function keys */

		long ch ;
		ch = func_table[i];
		put_queue(ch);
	}
	return 0;
}


int minus (int scancode)
{
	if ( e0 != 1)
		do_self(scancode);
	else 
		put_queue((long)'/');
	return 0;
}

int setleds()
{
	kbd_wait();
	outb(0xed,0x60);
	kbd_wait();
	outb(leds,0x60);
	return 0;
}

int kbd_wait()
{
	while (inb_p(0x64) & 0x2)  /* kbd in buffer is full */
		/* do nothing */ ;
	return 0;
}

int reboot()
{
	kbd_wait();
	outb(0xfc,0x64);   /* reset system*/

	while (1)   ;
	return 0;
}

int none(int scancode)
{
	return 0;
}

int scroll(int scancode)
{
	leds ^= 0x1;
	setleds();
	return 0;
}

static int put_queue(long ch)
{

	struct tty_queue * q = &read_q;

	while(ch){
		(q->buf)[q->head] = ch &0xff;
#if DEBUG
		printk("%c", ch&0xff);
#endif
		q->head = (q->head +1)&(SIZE -1);

		if ( q->head == q->tail )  /* buf is full */
			return 0;
		ch >>= 8;
	}

	/*
	  if ((long)q->proc_list != '\0') 
	  (q->proc_list)->state = 0;
	*/ 
	return 0;
}

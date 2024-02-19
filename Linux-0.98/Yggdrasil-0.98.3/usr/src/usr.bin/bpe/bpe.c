/***************************************************************************

Version History:

Ver.No	Comment						By
===========================================================================
1.0 	first version (seems to do things right)   	andy@mssx
1.1	some bugs fixed (tks to Maarten)		andy@mssx.uucp
							maart@cs.vu.nl
1.2	works with NOKEYPAD in all cases		davidsen@crdos1.uucp
	make + and - move in 2 line increments
	add 'L' look for hex byte
	end edit with ^E, ^C gives signal in BSD
	/ remembers the last search string, can repeat
1.3	integrate 1.2 (davidsen) and 1.2 (andy/jon)	davidsen@crdos1.uucp
	find hex string code added, author		jon@joblab

BUG REPORTS:
============
	- The offset count in the first column is wrong,
	  except for the first line; it's 0x10 too high.
								(fixed)
	- The test in disp() if a char is printable, fails
	  for chars >= 0177.
								(fixed)

	- Help message for 'H' incorrect			(fixed)


I declare this program as freeware, i.e. you may duplicate it, give it
to your friends, and transfer it to any machine you like, as long as
you do not change or delete the build in copyright message.

	Andreas Pleschutznig
	Teichhofweg 2
	8044 Graz
	Austria 

Comments and bug reports to:
	andy@mssx	(mcvax!tuvie!mssx!andy)


*****************************************************************************/

#include <stdio.h>
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

/* this is needed for MS-DOS compilation */
#ifndef	O_BINARY
#define O_BINARY	0
#endif

#define		CTRL(c)		((c) & 037)
#define		DEL		'\177'

#ifdef NOKEYPAD
#define		KEY_LEFT	CTRL('H')
#define		KEY_DOWN	CTRL('J')
#define		KEY_UP		CTRL('K')
#define		KEY_RIGHT	CTRL('L')
#ifndef O_RDWR
#define		O_RDWR		2
#endif
#ifndef cbreak
#define		cbreak()	crmode()
#endif
#define		beep()		putchar(7)
#endif

#define	BELL	0x07
#define ASCX	63
#define ASCY	6
#define HEXY	6
#define HEXX	12

int     path;                   /* path to file to patch */
long	filpos;			/* position in file */
unsigned char secbuf[256];	/* sector read buffer */

int     donix();                /* default signal handling routine */
char	filename[60];
int	length;			/* length of read sector */

main(argc,argv)
int argc;
char	**argv;
{
	if (argc != 2) {
		fprintf(stderr,"Usage: %s filename\n",argv[0]);
		exit(1);
	}
	if (( path = open(argv[1],O_RDWR|O_BINARY)) == -1) {
		fprintf(stderr,"%s: Can't open '%s'\n",argv[0],argv[1]);
		exit(1);
	}
	sprintf(filename,"%s",argv[1]);
	initscr();
	refresh();
	signal(SIGINT,donix);
#ifdef	SIGQUIT
	signal(SIGQUIT,donix);
#endif	/* no QUIT in MS-DOS */
	cbreak();                       /* set single char input */
	noecho();
#ifndef NOKEYPAD
	keypad(stdscr,TRUE);
#endif
	filpos = 0;			/* set global position to 0 */
	length = 0;
	command();
	clear();
	refresh();
	endwin();
	close(path);
}

command()
{
	int inval;

	header("BPE Version 1.3",filename,"(C) 1988 MSS Graz");
	inval = 0;
	while ((inval != 'q') && (inval != 'Q')) {
		move(2,0);
		mvprintw(2,0,"COMMAND : ");
		refresh();
		inval = getch();
		switch (inval) {
			case 'q':
			case 'Q':
				break;
			case 'h':
			case 'H':
				find_hex();
				dump();
				break;
			case '?':
				help();
				break;
			case 'f':
			case 'F':
			case '/':
				find_string();
				dump();
				break;
			case '+':
				filpos += 32;
				dump();
				break;
			case 'n':
			case 'N':
				filpos += 256;
				dump();
				break;
			case '-':
				filpos -= 32;
				if (filpos < 0)
					filpos = 0;
				dump();
				break;
			case 'p':
			case 'P':
				filpos -= 256;
				if (filpos < 0)
					filpos = 0;
				dump();
				break;
			case 'D':
			case 'd':
				dump();
				break;
			case 's':
			case 'S':
				set();
				dump();
				break;
			case 'e':
				edit_ascii();
				break;
			case 'E':
				edit_hex();
				break;
			case 'w':
			case 'W':
				wrsec();
				break;
			default:
				werr("Invalid Command !");
		}
	}
}

edit_ascii()
{
	int inval = 0;
	int cury,curx;

	if (length == 0)
		length = dump();
	move(2,0);
	clrtoeol();
#ifdef NOKEYPAD
	printw("Left ^H - down ^J - up ^K - right ^L - end editing with ^E");
#else
	printw("End editing with ^E");
#endif
	curx = cury = 0;
	while (inval != CTRL('E')) {
		move(ASCY+cury,ASCX+curx);
		refresh();
		inval = getch();
		switch (inval) {
			case KEY_UP:
				if (cury)
					cury--;
				else
					beep();
				break;
			case KEY_DOWN:
				if (cury < 15)
					cury++;
				else
					beep();
				break;
			case KEY_RIGHT:
				if (curx < 15)
					curx++;
				else
					beep();
				break;
			case KEY_LEFT:
				if (curx)
					curx--;
				else
					beep();
				break;
			default:
				if ((inval >= 0x20) && (inval <= 0x7e)) {
					secbuf[cury*16+curx] =inval;
					curx++;
					if (curx > 15) {
						curx=0;
						cury++;
						}
					if (cury > 15)
						cury = 0;
					disp(length);
					}
				break;
		}
	}
	move(2,0);
	clrtoeol();
}

gethex(cury,curx)
int	cury,curx;
{
	int val;
	int inlen;
	int value;
	char *hexvals = "0123456789ABCDEF";
	char *strchr(), *wkptr;

	inlen = 0;
	while (inlen < 2) {
		val = getch();
		if (val > 0xff) return(val);
		if (islower(val)) val = toupper(val);
		wkptr = strchr(hexvals, val);
		if (wkptr == NULL) return(val|0x2000);
		else val = wkptr - hexvals;

		switch (inlen) {
		case 0:
			value = val << 4;
			secbuf[cury*16+curx] = value;
			disp(length);
			move(HEXY+cury,HEXX+curx*3+1);
			refresh();
			break;
		case 1:
			value += val ;
			break;
		}
		inlen++;
	}
	return(value);
}

edit_hex()
{
	int inval = 0;
	int cury,curx;

	if (length == 0)
		length = dump();
	move(2,0);
	clrtoeol();
#ifdef NOKEYPAD
	printw("Left ^H - down ^J - up ^K - right ^L - end editing with ^E");
#else
	printw("End editing with ^E");
#endif
	curx = cury = 0;
	while (inval != -1) {
		move(HEXY+cury,HEXX+curx*3);
		refresh();
		inval = gethex(cury,curx);
		if (inval > 0xff) {
			/* this is control information */
			if (inval > 0x1fff)
				inval &= 0xff;
			switch (inval) {
			case KEY_UP:
				if (cury)
					cury--;
				else
					beep();
				break;
			case KEY_DOWN:
				if (cury < 15)
					cury++;
				else
					beep();
				break;
			case KEY_RIGHT:
				if (curx < 15)
					curx++;
				else
					beep();
				break;
			case KEY_LEFT:
				if (curx)
					curx--;
				else
					beep();
				break;
			case CTRL('E'):
				inval = -1;
				break;
			}
		}
		else {
			secbuf[cury*16+curx] =inval;
			curx++;
			if (curx > 15) {
				curx=0;
				cury++;
			}
			if (cury > 15)
				cury = 0;
			disp(length);
		}
	}
	move(2,0);
	clrtoeol();
}

find_string()
{
	int 	stlen;
	char 	string[60];
static	char	laststring[60];
static	int	re_search = 0, old_filpos;
	int	found;
	int 	searchpos;

	move(2,0);
	clrtoeol();
	printw("String to search : ");
	refresh();
	echo();
	getstr(string);
	if (strlen(string) == 0) {
		if (strlen(laststring) > 0)
			strcpy(string, laststring);
		else {
			beep();
			return;
		}
	}
	else {
		strcpy(laststring, string);
	}
	noecho();
	move(2,0);
	clrtoeol();
	printw("Searching for '%s'",string);
	found = 0;
	searchpos = (filpos == old_filpos ? re_search : 0);
	stlen = strlen(string);
	while (found == 0) {
		while ((256 - searchpos) >= stlen) {
			if (testchar(secbuf+searchpos,string,stlen))
				searchpos++;
			else {
				filpos += searchpos;
				old_filpos = filpos;
#ifdef	CLINES
				if (filpos >= 16*CLINES) {
					filpos -= 16*CLINES;
				}
				else {
					filpos = 0;
				}
#endif	/* context lines */
#ifdef	ALLIGN
				filpos &= ~0xf;
#endif	/* allign */
				re_search = old_filpos - filpos + 1;
				old_filpos = filpos;
				found = 1;
				break;
			}
		}
		if (found == 0) {
			filpos += searchpos;
			searchpos = 0;
		}
		if (rdsec() == 0) {
			found = 1;	
		}
		refresh();
	}
	move(2, 0);
	clrtoeol();
}

testchar(buffer,string,length)
char	*buffer;
char	*string;
int	length;
{
	register int i;
	
	i = 0;
	while ( i < length) {
		if (buffer[i] != string[i])
			break;
		i++;
	}
	if ( i == length)
		return(0);
	return(1);
}

set()
{
	echo();
	move(2,0);
	clrtoeol();
	printw("New File Position : ");
	refresh();
	scanw("%lx",&filpos);
	move(2,0);
	clrtoeol();
	noecho();
}

disp(length)
int	length;
{
	int	i, j, c;

	/* output headings adjusted for the starting position */
	mvprintw(4,0, " ADDRESS   ");
	for (i = 0, j = filpos & 0x0f; i < 16; ++i) {
		printw(" 0%c", "0123456789ABCDEF"[j]);
		j = (j + 1) % 16;
	}
	printw("      ASCII");

	mvprintw(5,0, "%s%s",
		"=======================================",
		"========================================");

	for ( i = 0; i < 16; i++) {
		mvprintw(ASCY+i,0,"%08lX",filpos+i*16);
		for (j = 0; j < 16; j++) {
			if (( i*16 + j ) >= length) {
				clrtoeol();
				goto Disp1;
			}
			mvprintw(ASCY+i,HEXX+j*3,"%02X",secbuf[i*16+j] & 0xFF);
		}
Disp1:
		for (j = 0; j < 16; j++) {
			if (( i*16 + j ) >= length) {
				clrtobot();
				goto Disp2;
			}
			if (' ' <= (c = secbuf[i * 16 + j]) && c < DEL)
				mvprintw(ASCY+i,ASCX+j,"%c", c);
			else
				mvprintw(ASCY+i,ASCX+j,".");
		}
	}
Disp2:
	refresh();
}


dump()
{
	int	i,j;

	length = rdsec();
	disp(length);
	return(length);
}

rdsec()
{
	mvprintw(2,55,"Rel. Position : %08lX",filpos);
	refresh();
	lseek(path,filpos,0);
	length = read(path,secbuf,256);
	return(length);
}

wrsec()
{
	lseek(path,filpos,0);
	write(path,secbuf,length);
}

help()
{
	WINDOW	*win;

	win = newwin(0,0,0,0);
	wclear(win);
	mvwprintw(win,3,10,"Valid Commands are :");
	mvwprintw(win,5,15,"D - Dump one page from current file position");
	mvwprintw(win,6,15,"S - Set current file pointer");
	mvwprintw(win,7,15,
		"F - Find string in file (beginning from curr. position)");
	mvwprintw(win,8,15,
		"H - locate hex bytes in file (beginning from curr. position)");
	mvwprintw(win,9,15,"N - Display next sector");
	mvwprintw(win,10,15,"P - Display previous sector");
	mvwprintw(win,11,15,"+ - Scroll forward 2 lines");
	mvwprintw(win,12,15,"- - Scroll back 2 lines");
	mvwprintw(win,13,15,"e - Edit ASCII portion of file");
	mvwprintw(win,14,15,"E - Edit binary portion of file");
	mvwprintw(win,15,15,"W - Write modified sector back to disk");
	mvwprintw(win,16,15,"Q - Quit Program");
	mvwprintw(win,18,20,"Continue with any char.");
	wrefresh(win);
	getch();
	delwin(win);
	touchwin(stdscr);
	refresh();
}

werr(errstr)
char    *errstr;

{
	beep();
	move(LINES-1,0);
	printw("%s",errstr);
	refresh();
	sleep(2);
	move(LINES-1,0);
	clrtoeol();
	refresh();
}

	

header(left,mid,right)
char    *left;
char    *mid;
char    *right;

{
	mvprintw(0,0,"%s",left);
	mvprintw(0,79-strlen(right),"%s",right);
	mvprintw(0,40-strlen(mid)/2,"%s",mid);
}

donix(sig)
int sig;

{
	signal(sig,donix);
}


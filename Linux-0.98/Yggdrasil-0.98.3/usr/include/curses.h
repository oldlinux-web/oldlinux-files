/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)curses.h	5.9 (Berkeley) 7/1/90
 */

#ifndef WINDOW
#ifdef linux
#define __POSIX_TERMIOS
#include	<termcap.h>
#endif
#include	<stdio.h>
 
#ifdef __POSIX_TERMIOS
#include <termios.h>
#else
#define USE_OLD_TTY
#include	<sys/ioctl.h>
#undef USE_OLD_TTY
#endif /* __POSIX_TERMIOS */

#define	bool	char
#define	reg	register

#define	TRUE	(1)
#define	FALSE	(0)
#define	ERR	(0)
#define	OK	(1)

#define	_ENDLINE	001
#define	_FULLWIN	002
#define	_SCROLLWIN	004
#define	_FLUSH		010
#define	_FULLLINE	020
#define	_IDLINE		040
#define	_STANDOUT	0200
#define	_NOCHANGE	-1

#ifdef __POSIX_TERMIOS
typedef	struct termios	SGTTY;
#else
typedef	struct sgttyb	SGTTY;
#endif
/*
 * Capabilities from termcap
 */

/* If HZ is defined again, cpp should complain. */
#undef HZ
#define	HZ	HZ

extern bool     AM, BS, CA, DA, DB, EO, HC, HZ, IN, MI, MS, NC, NS, OS, UL,
		XB, XN, XT, XS, XX;
extern char	*AL, *BC, *BT, *CD, *CE, *CL, *CM, *CR, *CS, *DC, *DL,
		*DM, *DO, *ED, *EI, *K0, *K1, *K2, *K3, *K4, *K5, *K6,
		*K7, *K8, *K9, *HO, *IC, *IM, *IP, *KD, *KE, *KH, *KL,
		*KR, *KS, *KU, *LL, *MA, *ND, *NL, *RC, *SC, *SE, *SF,
		*SO, *SR, *TA, *TE, *TI, *UC, *UE, *UP, *US, *VB, *VS,
		*VE, *AL_PARM, *DL_PARM, *UP_PARM, *DOWN_PARM,
		*LEFT_PARM, *RIGHT_PARM;
extern char	PC;

/*
 * From the tty modes...
 */

extern bool	GT, NONL, UPPERCASE, normtty, _pfast;

struct _win_st {
	short		_cury, _curx;
	short		_maxy, _maxx;
	short		_begy, _begx;
	short		_flags;
	short		_ch_off;
	bool		_clear;
	bool		_leave;
	bool		_scroll;
	char		**_y;
	short		*_firstch;
	short		*_lastch;
	struct _win_st	*_nextp, *_orig;
};

#define	WINDOW	struct _win_st

extern bool	My_term, _echoit, _rawmode, _endwin;

extern char	*Def_term, ttytype[];

extern int	LINES, COLS, _tty_ch;
#ifdef __POSIX_TERMIOS
extern tcflag_t _res_iflg, _res_lflg;
#else
extern int	_res_flg;
#endif

extern SGTTY	_tty;

extern WINDOW	*stdscr, *curscr;

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC__) || defined(__cplusplus)

extern int box (WINDOW *, char, char);
extern int delwin(WINDOW *);
extern int endwin(void);
extern char *getcap(char *);
extern int gettmode(void);
extern void idlok(WINDOW *, int);
extern WINDOW * initscr(void);
#if 0
extern char * longname(void);
#endif
extern int mvcur(int, int, int, int);
extern int mvprintw(int, int, const char *, ...);
extern int mvscanw(int, int, const char *, ...);
extern int mvwin(WINDOW *, int, int);
extern int mvwprintw(WINDOW *, int, int, const char *, ...);
extern int mvwscanw(WINDOW *, int, int, const char *, ...);
extern WINDOW * newwin(int, int, int, int);
extern int overlay(WINDOW *, WINDOW *);
extern int overwrite(WINDOW *, WINDOW *);
extern int printw(const char *, ...);
extern int scanw(const char *, ...);
extern int scroll(WINDOW *);
extern int setterm(const char *);
extern WINDOW * subwin(WINDOW *, int, int, int, int);
extern int touchline(WINDOW *, int, int, int);
extern int touchwin(WINDOW *);
extern int waddbytes(WINDOW *, const char *, int);
extern int waddch(WINDOW *, char);
extern int waddstr(WINDOW *, const char *);
extern int wclear(WINDOW *);
extern int wclrtobot(WINDOW *);
extern int wclrtoeol(WINDOW *);
extern int wdelch(WINDOW *);
extern int wdeleteln(WINDOW *);
extern int werase(WINDOW *);
extern int wgetch(WINDOW *);        /* because it can return KEY_*, for 
					 instance */
extern int wgetstr(WINDOW *, char *);
extern int winsch(WINDOW *, char);
extern int winsertln(WINDOW *);
extern int wmove(WINDOW *, int, int);
extern int wprintw(WINDOW *, const char *, ...);
extern int wrefresh(WINDOW *);
extern int wscanw(WINDOW *, const char *, ...);
extern char * wstandend(WINDOW *);
extern char * wstandout(WINDOW *);

#else /* not __STDC__ || __cplusplus */

extern WINDOW * initscr();
extern WINDOW * newwin();
extern WINDOW * subwin();
#if 0
extern char	*longname();
#endif
extern int wgetch(); /* because it can return KEY_*, for instance. */

#endif /* __STDC__ || __cplusplus */

#ifdef __cplusplus
}
#endif

/*
 *	Define VOID to stop lint from generating "null effect"
 * comments.
 */
#ifdef lint
int	__void__;
#define	VOID(x)	(__void__ = (int) (x))
#else
#define	VOID(x)	(x)
#endif

/*
 * psuedo functions for standard screen
 */
#define	addch(ch)	VOID(waddch(stdscr, ch))
#define	getch()		VOID(wgetch(stdscr))
#define	addbytes(da,co)	VOID(waddbytes(stdscr, da,co))
#define	addstr(str)	VOID(waddbytes(stdscr, str, strlen(str)))
#define	getstr(str)	VOID(wgetstr(stdscr, str))
#define	move(y, x)	VOID(wmove(stdscr, y, x))
#define	clear()		VOID(wclear(stdscr))
#define	erase()		VOID(werase(stdscr))
#define	clrtobot()	VOID(wclrtobot(stdscr))
#define	clrtoeol()	VOID(wclrtoeol(stdscr))
#define	insertln()	VOID(winsertln(stdscr))
#define	deleteln()	VOID(wdeleteln(stdscr))
#define	refresh()	VOID(wrefresh(stdscr))
#define	inch()		VOID(winch(stdscr))
#define	insch(c)	VOID(winsch(stdscr,c))
#define	delch()		VOID(wdelch(stdscr))
#define	standout()	VOID(wstandout(stdscr))
#define	standend()	VOID(wstandend(stdscr))

/*
 * mv functions
 */
#define	mvwaddch(win,y,x,ch)	VOID(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define	mvwaddbytes(win,y,x,da,co) \
		VOID(wmove(win,y,x)==ERR?ERR:waddbytes(win,da,co))
#define	mvwaddstr(win,y,x,str) \
		VOID(wmove(win,y,x)==ERR?ERR:waddbytes(win,str,strlen(str)))
#define mvwgetstr(win,y,x,str)  VOID(wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define	mvwinch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : winch(win))
#define	mvwdelch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : wdelch(win))
#define	mvwinsch(win,y,x,c)	VOID(wmove(win,y,x) == ERR ? ERR:winsch(win,c))
#define	mvaddch(y,x,ch)		mvwaddch(stdscr,y,x,ch)
#define	mvgetch(y,x)		mvwgetch(stdscr,y,x)
#define	mvaddbytes(y,x,da,co)	mvwaddbytes(stdscr,y,x,da,co)
#define	mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define mvgetstr(y,x,str)       mvwgetstr(stdscr,y,x,str)
#define	mvinch(y,x)		mvwinch(stdscr,y,x)
#define	mvdelch(y,x)		mvwdelch(stdscr,y,x)
#define	mvinsch(y,x,c)		mvwinsch(stdscr,y,x,c)

/*
 * psuedo functions
 */

#define	clearok(win,bf)	 (win->_clear = bf)
#define	leaveok(win,bf)	 (win->_leave = bf)
#define	scrollok(win,bf) (win->_scroll = bf)
#define flushok(win,bf)	 (bf ? (win->_flags |= _FLUSH):(win->_flags &= ~_FLUSH))
#define	getyx(win,y,x)	 y = win->_cury, x = win->_curx
#define	winch(win)	 (win->_y[win->_cury][win->_curx] & 0177)

#ifdef __POSIX_TERMIOS
#define raw()	 (_tty.c_lflag&=~(ICANON|ISIG),\
	_pfast=_rawmode=TRUE, tcsetattr(_tty_ch, TCSANOW, &_tty))

#define noraw()	 (_tty.c_lflag|=ISIG,_rawmode=FALSE,\
	_pfast=(_tty.c_iflag&ICRNL),tcsetattr(_tty_ch, TCSANOW, &_tty))

/* cbreak is like raw, but without turning off signals. */

#define cbreak() (_tty.c_lflag&=~ICANON, \
	_rawmode = TRUE, tcsetattr(_tty_ch, TCSANOW, &_tty))

#define nocbreak() (_tty.c_lflag |= ICANON, _tty.c_lflag &= ~ISIG, \
	_rawmode=FALSE, tcsetattr(_tty_ch, TCSANOW, &_tty))

#define crmode() cbreak()	/* backwards compatability */
#define nocrmode() nocbreak()	/* backwards compatability */

#define echo()	 (_tty.c_lflag |= ECHO, _echoit = TRUE, \
	tcsetattr(_tty_ch, TCSADRAIN, &_tty))
#define noecho() (_tty.c_lflag &= ~ECHO, _echoit = FALSE, \
	tcsetattr(_tty_ch, TCSADRAIN, &_tty))
#define nl()	 (_tty.c_iflag |= ICRNL,_pfast = _rawmode, \
	_tty.c_oflag |= ONLCR, \
	tcsetattr(_tty_ch, TCSANOW, &_tty))
#define nonl()	 (_tty.c_iflag &= ~ICRNL, _pfast = TRUE, \
	_tty.c_oflag &= ~ONLCR, \
	tcsetattr(_tty_ch, TCSANOW, &_tty))
#define	savetty() ((void) tcgetattr(_tty_ch, &_tty), \
	_res_iflg = _tty.c_iflag, _res_lflg = _tty.c_lflag)
#define	resetty() (_tty.c_iflag = _res_iflg, _tty.c_lflag = _res_lflg,\
	_echoit = ((_res_lflg & ECHO) == ECHO), \
	_rawmode = ((_res_lflg & (ISIG|ICANON)) != 0), \
	_pfast = ((_res_iflg & ICRNL) ? _rawmode : TRUE), \
	(void) tcsetattr(_tty_ch, TCSADRAIN, &_tty))

#define	erasechar()	(_tty.c_cc[VERASE])
#define	killchar()	(_tty.c_cc[VKILL])
#define baudrate()	(cfgetospeed(&_tty))
#else
#define raw()	 (_tty.sg_flags|=RAW, _pfast=_rawmode=TRUE, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define noraw()	 (_tty.sg_flags&=~RAW,_rawmode=FALSE,\
	_pfast=!(_tty.sg_flags&CRMOD),ioctl(_tty_ch, TIOCSETP, &_tty))
#define cbreak() (_tty.sg_flags |= CBREAK, _rawmode = TRUE, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define nocbreak() (_tty.sg_flags &= ~CBREAK,_rawmode=FALSE, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define crmode() cbreak()	/* backwards compatability */
#define nocrmode() nocbreak()	/* backwards compatability */
#define echo()	 (_tty.sg_flags |= ECHO, _echoit = TRUE, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define noecho() (_tty.sg_flags &= ~ECHO, _echoit = FALSE, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define nl()	 (_tty.sg_flags |= CRMOD,_pfast = _rawmode, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define nonl()	 (_tty.sg_flags &= ~CRMOD, _pfast = TRUE, \
	ioctl(_tty_ch, TIOCSETP, &_tty))
#define	savetty() ((void) ioctl(_tty_ch, TIOCGETP, &_tty), \
	_res_flg = _tty.sg_flags)
#define	resetty() (_tty.sg_flags = _res_flg, \
	_echoit = ((_res_flg & ECHO) == ECHO), \
	_rawmode = ((_res_flg & (CBREAK|RAW)) != 0), \
	_pfast = ((_res_flg & CRMOD) ? _rawmode : TRUE), \
	(void) ioctl(_tty_ch, TIOCSETP, &_tty))

#define	erasechar()	(_tty.sg_erase)
#define	killchar()	(_tty.sg_kill)
#define baudrate()	(_tty.sg_ospeed)
#endif

/*
 * Used to be in unctrl.h.
 */
#define	unctrl(c)	_unctrl[(c) & 0177]
extern char *_unctrl[];
#endif

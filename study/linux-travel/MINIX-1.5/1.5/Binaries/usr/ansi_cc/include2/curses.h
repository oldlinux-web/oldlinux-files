#ifndef _CURSES_H
#define _CURSES_H

#include <ansi.h>

/* Lots of junk here, not packaged right. */
extern char termcap[];
extern char tc[];
extern char *ttytype;
extern char *arp;
extern char *cp;

extern char *cl;
extern char *cm;
extern char *so;
extern char *se;

extern char /* nscrn[ROWS][COLS], cscrn[ROWS][COLS], */ row, col, mode;
extern char str[];

_PROTOTYPE( void addstr, (char *_s)					);
_PROTOTYPE( void clear, (void)						);
_PROTOTYPE( void clrtobot, (void)					);
_PROTOTYPE( void clrtoeol, (void)					);
_PROTOTYPE( void endwin, (void)						);
_PROTOTYPE( void fatal, (char *_s)					);
_PROTOTYPE( char inch, (void)						);
_PROTOTYPE( void initscr, (void)					);
_PROTOTYPE( void move, (int _y, int _x)					);
/* WRONG, next is varargs. */
_PROTOTYPE( void printw, (char *_fmt, char *_a1, char *_a2, char *_a3,
			  char *_a4, char *_a5)				);
_PROTOTYPE( void outc, (int _c)						);
_PROTOTYPE( void refresh, (void)					);
_PROTOTYPE( void standend, (void)					);
_PROTOTYPE( void standout, (void)					);
_PROTOTYPE( void touchwin, (void)					);

#endif /* _CURSES_H */

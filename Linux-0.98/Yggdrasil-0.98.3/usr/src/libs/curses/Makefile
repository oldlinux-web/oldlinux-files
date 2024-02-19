#	@(#)Makefile	5.10 (Berkeley) 6/24/90

LIB=	curses
SRCS=	addbytes.c addch.c addstr.c box.c clear.c clrtobot.c clrtoeol.c \
	cr_put.c cr_tty.c curses.c delch.c deleteln.c delwin.c endwin.c \
	erase.c fullname.c getch.c getstr.c idlok.c id_subwins.c initscr.c \
	insch.c insertln.c longname.c move.c mvprintw.c mvscanw.c mvwin.c \
	newwin.c overlay.c overwrite.c printw.c putchar.c refresh.c scanw.c \
	scroll.c toucholap.c standout.c touchwin.c tstp.c unctrl.c
MAN3=	curses.0

CFLAGS+=-DPOSIX
OBJS+=	${SRCS:R:S/$/.o/g}
all:	libcurses.a

# need to alter default rules in /usr/share/mk/bsd.lib.mk; we don't have
# lorder or tsort.
lib${LIB}.a:: ${OBJS}
	@echo building standard ${LIB} library
	@rm -f lib${LIB}.a
	@${AR} cTq lib${LIB}.a ${OBJS} ${LDADD}
	ranlib lib${LIB}.a


beforeinstall:
	-cd ${.CURDIR}; cmp -s curses.h ${DESTDIR}/usr/include/curses.h || \
	    install -c -o ${BINOWN} -g ${BINGRP} -m 444 curses.h \
	    ${DESTDIR}/usr/include

.include <bsd.prog.mk>

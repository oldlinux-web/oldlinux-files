#
# Copyright (c) 1988 Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation,
# advertising materials, and other materials related to such
# distribution and use acknowledge that the software was developed
# by the University of California, Berkeley.  The name of the
# University may not be used to endorse or promote products derived
# from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#	@(#)Makefile	5.5 (Berkeley) 6/29/88
#
CFLAGS=
LIBC=	/usr/lib/libc.a
SRCS=	more.c regex.c
OBJS=	more.o regex.o

all: more

more:	${OBJS} ${LIBC}
	${CC} -o $@ ${CFLAGS} ${OBJS}

regex.o:  regex.c
	${CC} -c ${CFLAGS} -DREGEX regex.c

clean: FRC
	rm -f ${OBJS} core more

depend: FRC
	mkdep ${CFLAGS} ${SRCS}

install: FRC
	install -s -o bin -g bin -m 755 more ${DESTDIR}/usr/ucb/more
	rm -f ${DESTDIR}/usr/ucb/page
	ln ${DESTDIR}/usr/ucb/more ${DESTDIR}/usr/ucb/page
	install -c -o bin -g bin -m 644 more.help ${DESTDIR}/usr/lib/more.help

lint: FRC
	lint ${CFLAGS} ${SRCS}

tags: FRC
	ctags ${SRCS}

FRC:

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

more.o: more.c /usr/include/stdio.h /usr/include/sys/param.h
more.o: /usr/include/sys/types.h /usr/include/signal.h
more.o: /usr/include/ctype.h
more.o: /usr/include/signal.h /usr/include/errno.h
more.o: /usr/include/termios.h
more.o: /usr/include/setjmp.h /usr/include/sys/stat.h
more.o: /usr/include/sys/file.h /usr/include/a.out.h
more.o: /usr/include/varargs.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

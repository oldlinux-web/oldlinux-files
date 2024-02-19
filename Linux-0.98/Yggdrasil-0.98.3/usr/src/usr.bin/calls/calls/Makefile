# Makefile for calls	$Id: Makefile,v 2.1 87/03/11 13:23:31 ksb Exp $
#			Kevin S Braunsdorf, PUCC

BIN=	${DESTDIR}/usr/local/bin

I=/usr/include
S=/usr/include/sys
L=

CC=gcc -DMACRO_BSTRING
INCLUDE=
DEBUG=	-O
CDEFS= 
CFLAGS=	${DEBUG} ${CDEFS} ${INCLUDE}

HDR=	main.h scan.h getopt.h
SRC=	main.c scan.c getopt.c
OBJ=	main.o scan.o getopt.o
SOURCE=	Makefile ${HDR} ${SRC}

all: calls

calls: ${OBJ}
	${CC} -o $@ ${CFLAGS} ${OBJ}

clean: FRC
	rm -f Makefile.bak calls *.o a.out core errs tags

depend: ${SRC} ${HDR} FRC
	mkdep ${CDEFS} ${INCLUDE} ${SRC}

install: all FRC
	install -c -s -m 751 -o binary -g system calls ${BIN}

lint: ${SRC} ${HDR} FRC
	lint -hnx ${CDEFS} ${INCLUDE} ${SRC}

print: source FRC
	lpr -J'calls source' ${SOURCE}

source: ${SOURCE}

spotless: clean
	rcsclean ${SOURCE}

tags: ${SRC} ${HDR}
	ctags -t ${SRC} ${HDR}

${SOURCE}:
	co $@

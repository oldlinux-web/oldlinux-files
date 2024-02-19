# Makefile -- Makefile for NET-2 utilties ported to Linux
# Created: Sun Jun 14 13:43:28 1992
# Revised: Sat Jul  4 16:32:22 1992 by root
# Copyright 1992 Rickard E. Faith (faith@cs.unc.edu)
#
# $Log$
#

#STATIC= -static
CC= gcc
CFLAGS= -O -I. $(STATIC) -s

MAN1= cal.1 col.1 colcrt.1 colrm.1 column.1 false.1 hexdump.1 od.1 \
	strings.1 true.1 ul.1 uuencode.1 whereis.1 
MAN5= uuencode.5
MAN6= banner.6

NEEDS_GETOPT= banner cal col column strings uuencode
OTHER= colcrt colrm false true whereis
BINS= $(NEEDS_GETOPT) $(OTHER) hexdump ul

all: $(BINS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(NEEDS_GETOPT): getopt.o
	$(CC) $(CFLAGS) $@.c -o $@ getopt.o

$(OTHER):
	$(CC) $(CFLAGS) $@.c -o $@

ul: ul.c getopt.o
	$(CC) $(CFLAGS) -o ul ul.c getopt.o -ltermcap

hexdump: hexdump.c hexdump.h getopt.o conv.o display.o hexsyntax.o \
	odsyntax.o parse.o
	$(CC) $(CFLAGS) -o hexdump hexdump.c getopt.o conv.o display.o \
		hexsyntax.o odsyntax.o parse.o

install: all
	install -m 755 $(BINS) /usr/bin
	rm -f /usr/bin/od
	ln /usr/bin/hexdump /usr/bin/od
	rm -f /usr/bin/uudecode
	ln /usr/bin/uuencode /usr/bin/uudecode
	install -m 644 $(MAN1) /usr/man/man1
	install -m 644 $(MAN5) /usr/man/man5
	install -m 644 $(MAN6) /usr/man/man6
	
clean:
	-rm -f $(BINS) *.o

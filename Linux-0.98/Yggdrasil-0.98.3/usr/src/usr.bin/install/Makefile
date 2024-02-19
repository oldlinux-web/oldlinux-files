# Makefile for install(1)
#
# Use "make" or "make install" to compile, "make installit" to install.
#
# RCSid: $Id: Makefile,v 1.2 1992/01/01 16:01:49 yocs Exp $
#
# $Log: Makefile,v $
# Revision 1.2  1992/01/01  16:01:49  yocs
# Cleaned out make rules. Let's hope make doesn't get confused.
#
# Revision 1.1  1991/04/09  22:54:54  yocs
# Initial revision
#
#
# Change to your favourite compiler:
CC	= cc -c68
#
# You could add -DDEBUG here, but I don't think you'll need it ;-)
CFLAGS	=
#
# Specify here where you want install(1) to put install(1) :-)
BINDIR	= /usr/local/bin

#
all:	install.c
	$(CC) $(CFLAGS) -o install install.c
	@chmem =8192 install >/dev/null 2>&1

install:	all
	install -s -m 555 -o bin install $(BINDIR)

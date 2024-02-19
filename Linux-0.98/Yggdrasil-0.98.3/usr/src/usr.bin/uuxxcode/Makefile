CC=gcc
CFLAGS=-O -DSYSV
all: uuencode uudecode

uuencode: uuencode.c
	$(CC) -o uuencode $(CFLAGS) uuencode.c

uudecode: uudecode.c
	$(CC) -o uudecode $(CFLAGS) uudecode.c

install: uuencode uudecode
	install uuencode /usr/bin
	install uudecode /usr/bin
#	install uuencode.1 /usr/man/man1

#	XXX uuencode.1 produces a garbled man page for some reason.




# setserial.mk - makefile for setserial - rick sladkey

CC = gcc
CFLAGS = -static -nostdinc -I/usr/src/linux/include -I/usr/include

PROGS= setserial
TARGETS= $(PROGS)

all: $(TARGETS) 

setserial: setserial.c
	$(CC) $(CFLAGS) setserial.c -o setserial

install:
	install $(PROGS) /usr/bin
clean:
	rm *.o $(TARGETS)

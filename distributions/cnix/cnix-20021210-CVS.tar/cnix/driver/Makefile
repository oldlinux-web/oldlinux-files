AS = as
LD = ld
CC = gcc
CFLAGS = -Wall -O2 -fstrength-reduce -ofomit-frame-pointer \
	-finline-functions -nostdinc -I../include

OBJS = timer.o keyboard.o ide.o vga.o

all:	driver.o

driver.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)

timer.o:	timer.c
	$(CC) $(CFLAGS) -c $< -o $@

keryboard.o:	keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@

ide.o:	ide.c
	$(CC) $(CFLAGS) -c $< -o $@

vga.o:	vga.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f *~
	rm -f .*.swp

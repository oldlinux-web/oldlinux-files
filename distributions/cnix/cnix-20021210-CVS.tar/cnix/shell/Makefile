LD = ld
CC = gcc
CFLAGS = -Wall -O2 -fstrength-reduce -ofomit-frame-pointer \
	-finline-functions -nostdinc -I../include

OBJS = command.o

all:	shell.o

shell.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	rm -f *~
	rm -f .*.swp

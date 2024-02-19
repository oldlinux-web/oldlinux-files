AR = ar
AS = as
LD = ld
CC = gcc
CFLAGS = -Wall -O2 -fstrength-reduce -ofomit-frame-pointer \
	-finline-functions -nostdinc -I../include

OBJS = write.o

all: lib.a

lib.a:	$(OBJS)
	$(AR) rcs lib.a $(OBJS)

write.o:	write.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.a
	rm -f *~
	rm -f .*.swp

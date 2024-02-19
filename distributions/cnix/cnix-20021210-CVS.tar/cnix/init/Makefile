AR	=ar
AS	=as
LD	=ld
LDFLAGS	=-s -x
CC	=gcc
CFLAGS	=-Wall -O2 -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -I../include

OBJS  = main.o

main.o:	main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f *~
	rm -f .*.swp

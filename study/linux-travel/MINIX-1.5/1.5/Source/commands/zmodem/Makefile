all: rz sz

CFLAGS= -DV7 -D_MINIX -D_POSIX_SOURCE -DLCASE=0

rz: rz.c rbsb.c zm.c zmodem.h
	$(CC) $(CFLAGS) -o rz rz.c
	chmem =10000 rz

sz: sz.c rbsb.c zm.c zmodem.h
	$(CC) $(CFLAGS) -o sz sz.c
	chmem =10000 sz

clean:	
	@rm -f *.bak *.s rz sz

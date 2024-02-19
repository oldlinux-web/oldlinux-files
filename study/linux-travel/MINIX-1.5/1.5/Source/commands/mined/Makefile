CFLAGS=-F -D_POSIX_SOURCE -D_MINIX
l=/lib

mined:	mined1.s mined2.s mined.h
	cc -i -o mined  mined1.s mined2.s


clean:	
	@rm -f *.bak *.s mined

CFLAGS=	-Wall -Wno-parentheses -O6 -fstrength-reduce
LDFLAGS=-s
bins=	fdisk
all:	$(bins)
fdisk:	fdisk.o
install:all
	cp $(bins) /bin
	(cd /bin;chown bin $(bins);chgrp bin $(bins);chmod 555 $(bins))
#	cp $(usrbins) /usr/bin
#	(cd /usr/bin;chown bin $(usrbins);chgrp bin $(usrbins); \
#		chmod 555 $(usrbins))
clean:
	rm -f *.o $(execs)

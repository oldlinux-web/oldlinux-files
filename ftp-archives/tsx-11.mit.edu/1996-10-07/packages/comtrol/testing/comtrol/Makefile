CFLAGS = -m486 -O2 -D__KERNEL__ -DLINUX -Wall -I/usr/src/linux/include
LDFLAGS = -s -N 
CC=gcc

OBJS= linux.o rckt.o
MODOBJS= mod_linux.o rckt.o

all: rocket.o maybe_modules setrocket

maybe_modules:
	@if [ -f /usr/include/linux/modversions.h ]; then \
		echo "Building modrocket" ; \
		make mod_rocket.o ; \
	fi

setrocket: setrocket.c rocket.h
	$(CC) -O2 setrocket.c -o setrocket

install:
	@if [ ! -d /lib/modules ]; then mkdir -p /lib/modules; fi
	cp mod_rocket.o rocket.o /lib/modules
	@if [ ! -d /etc/rc.d ]; then mkdir -p /etc/rc.d; fi
	cp rc.rocket /etc/rc.d
	cp setrocket /bin/setrocket
	@strip /bin/setrocket
	@chmod 755 /bin/setrocket

clean:
	rm -f $(OBJS) $(MODOBJS) *~ setrocket setrocket.o

really-clean: clean
	rm -f *.o setrocket comtrol.tar.gz

distribution:
	make really-clean
	make all
	rm -f $(OBJS) $(MODOBJS) *~
	make tar-file

tar-file:
	tar --create --file - --verbose --exclude comtrol/RCS \
		--exclude comtrol/examples --exclude comtrol/comtrol.tar.gz \
		--exclude comtrol/dist \
		-C .. comtrol | gzip -9 > comtrol.tar.gz

rocket.o: $(OBJS)
	ld -r -o rocket.o $(OBJS)
	strip -x rocket.o

mod_rocket.o: $(MODOBJS)
	ld -r -o mod_rocket.o $(MODOBJS)
	strip -x mod_rocket.o

linux.o: linux.c linux.h rckt.c rcktldef.h rckt.h version.h \
	/usr/src/linux/include/linux/version.h

mod_linux.o: linux.c linux.h rckt.c rcktldef.h rckt.h version.h \
	/usr/src/linux/include/linux/version.h
	$(CC) $(CFLAGS) -DMODVERSIONS -o mod_linux.o -c linux.c 

rckt.o: rckt.c rcktldef.h rckt.h


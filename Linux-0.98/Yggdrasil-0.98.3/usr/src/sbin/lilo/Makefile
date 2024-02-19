CC=cc
CPP=$(CC) -E
AS86=as86 -0 -a
LD86=ld86 -0

CFLAGS=-Wall `if [ -f /usr/include/linux/genhd.h ]; then echo -DNEW_INCLUDES; fi`
LDFLAGS=

OBJS=lilo.o map.o geometry.o boot.o device.o common.o bsect.o

.SUFFIXES:	.img .b

all:		lilo boot.b chain.b dparam.com activate

.c.o:
		$(CC) -c $(CFLAGS) $*.c

.s.o:
		$(AS86) -o $*.o $*.s

.o.img:
		$(LD86) -s -o $*.img $*.o

.img.b:
		dd if=$*.img of=$*.b bs=32 skip=1

activate:	activate.c
		$(CC) -o activate activate.c $(LDFLAGS)

dparam.com:	dparam.img
		dd if=dparam.img of=dparam.com bs=288 skip=1

lilo:		$(OBJS)
		$(CC) -o lilo $(LDFLAGS) $(OBJS)

boot.b:		first.b second.b
		(dd if=first.b bs=512 conv=sync; dd if=second.b) >boot.b

first.s:	first.S lilo.h lilo
		$(CPP) `./lilo -X` first.S -o first.s

second.s:	second.S lilo.h lilo
		$(CPP) `./lilo -X` second.S -o second.s

chain.s:	chain.S lilo.h
		$(CPP) chain.S -o chain.s

first.img:	first.o
second.img:	second.o
chain.img:	chain.o
dparam.img:	dparam.o
first.b:	first.img
second.b:	second.img
chain.b:	chain.img

install:
		if [ ! -d /etc/lilo ]; then mkdir /etc/lilo; fi
		if [ -f /etc/lilo/boot.b ]; then \
		  mv /etc/lilo/boot.b /etc/lilo/boot.old; fi
		if [ -f /etc/lilo/chain.b ]; then \
		  mv /etc/lilo/chain.b /etc/lilo/chain.old; fi
		cp lilo boot.b chain.b /etc/lilo
		if [ ! -f /etc/lilo/disktab ]; then cp disktab /etc/lilo; fi
		if [ -f /etc/lilo/install ]; then echo; \
		  echo "Please don't forget to run /etc/lilo/install to\\c"; \
		  echo " update the map file."; fi
		
dep:
		sed '/\#\#\# Dependencies/q' <Makefile >tmp_make
		(for n in *.c; do $(CPP) -MM $$n; done) >>tmp_make
		mv tmp_make Makefile

clean:
		-rm *.o *.img first.s second.s chain.s tmp_make first.b second.b

spotless:	clean
		-rm lilo boot.b chain.b

### Dependencies
boot.o : boot.c common.h lilo.h geometry.h map.h boot.h 
bsect.o : bsect.c config.h common.h lilo.h device.h map.h geometry.h boot.h \
  bsect.h 
common.o : common.c common.h lilo.h 
device.o : device.c config.h common.h lilo.h device.h 
geometry.o : geometry.c config.h lilo.h common.h device.h geometry.h 
lilo.o : lilo.c config.h common.h lilo.h geometry.h bsect.h 
map.o : map.c lilo.h common.h geometry.h map.h 

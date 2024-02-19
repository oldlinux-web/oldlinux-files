CC=		gcc
CFLAGS=		-O -Wall
LDFLAGS=	-s -static
PROGS=		efsck mkefs

all:	$(PROGS)

clean:
	rm -f $(PROGS) *.o core tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	for i in *.c; do $(CPP) -M $$i; done >>tmp_make
	cp tmp_make Makefile

install:
	install $(PROGS) /usr/bin

### Dependencies:

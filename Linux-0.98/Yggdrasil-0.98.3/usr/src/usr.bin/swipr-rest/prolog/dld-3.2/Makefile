CC=		gcc
#CC=		cc
CFLAGS=		-O
MAN=		/usr/local/man/man3

OBJ=		dld.o dld_find_executable.o

all:		libdld.a

libdld.a:	$(OBJ)
		rm -f $@
		ar ru $@ $(OBJ)
		ranlib $@
		
install:	all
		install -c libdld.a /usr/local/lib
		ranlib /usr/local/lib/libdld.a
		install -c dld.h /usr/local/include
		install -c doc/dld.3 $(MAN)
		install -c doc/dld_find_executable.3 $(MAN)

clean:
		rm $(OBJ) libdld.a

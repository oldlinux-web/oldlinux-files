# This is Makefile of inet lib for Linux
#
# Host root directory, maybe /usr for Linux
HOSTROOTDIR=/net/gecko/users/hlu/local
HOSTROOTDIR=/local
# Where we are now.
SRCDIR=$(HOME)/Linux/src/libc-linux
SRCDIR=/usr2/linux/src/libc-linux
# Host root directory, maybe /usr for Linux
TARGETROOTDIR=$(HOME)/Linux/usr
TARGETROOTDIR=/usr2/linux/usr
# Host bin direcory
HOSTBINDIR=$(HOSTROOTDIR)/bin
# For Linux, that should be /usr/bin
CROSSBINDIR=$(HOST_LIB_DIR)
CROSSBINDIR=/usr2/linux/cross/bin

HOST_MACHINE=decstation-3100
TARGET_MACHINE=i386-linux

VERSION=`awk '{printf "%s", substr ($$2, 1, length ($$2)); }' $(SRCDIR)/VERSION`

HOST_LIB_DIR =$(HOSTROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
HOST_SHARED_DIR=$(HOST_LIB_DIR)/shared
TARGE_LIB_TDIR=$(TARGETROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
TARGET_SHARED_DIR =$(TARGET_LIB_DIR)/shared

MV	=/bin/mv

AR      =$(CROSSBINDIR)/ar
AS      =$(CROSSBINDIR)/as
LD      =$(CROSSBINDIR)/ld
NM      =$(CROSSBINDIR)/nm
RANLIB  =$(CROSSBINDIR)/ranlib

RANLIB  =echo

LDFLAGS =-r -x

LIB=../libc.a
#LIB=../libinet.a

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX
INET_FLAGS=-D_POSIX_SOURCE
CC= $(HOSTBINDIR)/gcc -V $(VERSION) -b i386-linux
CFLAGS = -Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
INC= -I../soft
CPP=$(CC) -E $(INC) $(XCFLAGS)
RM=/bin/rm

.SUFFIXES:
.SUFFIXES: .S .o .c .s .i

.s.o :
	$(CC) $(CFLAGS) $(INET_FLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.S.o :
	$(CC) $(CFLAGS) $(INET_FLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.o :
	$(CC) $(CFLAGS) $(INET_FLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.s :
	$(CC) $(CFLAGS) $(INET_FLAGS) -S $<

.c.i :
	$(CC) $(CFLAGS) $(INET_FLAGS) -E $< -o $*.i

OBJS1 = gethstnmad.o getnetbyad.o getnetbynm.o getnetent.o \
	getproto.o getprtent.o getprtname.o getservent.o \
	getsrvbynm.o getsrvbypt.o herror.o inet_addr.o \
	inet_lnaof.o inet_mkadr.o inet_net.o inet_netof.o \
	inet_ntoa.o rcmd.o res_comp.o res_debug.o res_init.o \
	res_mkqry.o res_query.o res_send.o rexec.o ruserpass.o \
	sethostent.o

OBJS2 = gethstnmad.o getnetbyad.o getnetbynm.o getnetent.o \
	getproto.o getprtent.o getprtname.o getservent.o \
	getsrvbynm.o getsrvbypt.o herror.o inet_addr.o \
	inet_lnaof.o inet_mkadr.o inet_net.o inet_netof.o \
	inet_ntoa.o 

OBJS = $(OBJS1)

library: $(OBJS)
	$(AR) uvc $(LIB) $(OBJS)
	sync

obj: $(OBJS)

clean:
	$(RM) -f core *.o *.a tmp_make foo
	-for i in *.c;do \
	  if [ -f $$i ]; then \
	    $(RM) -f `basename $$i .c`.s; \
	  fi; \
	done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
	$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:

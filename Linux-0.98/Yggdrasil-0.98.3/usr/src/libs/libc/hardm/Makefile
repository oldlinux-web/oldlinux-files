# This is Makefile of the math lib for Linux
#
# Host root directory, maybe /usr for Linux
HOSTROOTDIR=/net/gecko/users/hlu/local
HOSTROOTDIR=/local
# Where we are now.
SRCDIR=$(HOME)/Linux/src/libc-linux
SRCDIR=/usr2/linux/src/libc-linux
# Host root directory, maybe /usr for Linux
TARGETROOTDIR=$(HOME)/Linux/usr
# Host bin direcory
HOSTBINDIR=$(HOSTROOTDIR)/bin
# For Linux, that should be /usr/bin
CROSSBINDIR=/usr2/linux/cross/bin
CROSSBINDIR=$(HOST_LIB_DIR)

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

LIB=../libhard.a

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX
CC= $(HOSTBINDIR)/gcc -V $(VERSION) -b i386-linux
CFLAGS = -Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
INC= -I../soft
CPP=$(CC) -E $(INC) $(XCFLAGS)
LIBM_ADDR=3c70000
RM=/bin/rm

.SUFFIXES:
.SUFFIXES: .S .o .c .s

.s.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.S.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.s :
	$(CC) $(CFLAGS) -S $<

OBJS = asin.o atan.o atanh.o drem.o erf.o exp.o fabs.o floor.o fmod.o \
	frexp.o hypot.o j0.o j1.o jn.o lgamma.o log.o \
	sin.o sinh.o sqrt.o tan.o tanh.o #  aux.o

ASMS = asin.s atan.s atanh.s drem.s erf.s exp.s fabs.s floor.s fmod.s \
	frexp.s hypot.s j0.s j1.s jn.s lgamma.s log.s \
	sin.s sinh.s sqrt.s tan.s tanh.s # aux.s

library: $(OBJS)
	$(AR) uvc $(LIB) $(OBJS)
	sync

asm: $(ASMS)

obj: $(OBJS)

foo: obj
	$(LD) -T $(LIBM_ADDR) -o $@ *.o -L$(HOST_SHARED_DIR) -lc

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

#
# Makefile for iostream
#
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

LIB=../../libc.a

MFLAGS=CC="$(CC)" AR="$(AR)" LD="$(LD)" CPP="$(CPP)" AS="$(AS)" \
	RANLIB="$(RANLIB)" INC="$(INC)" RM="$(RM)" \
	CFLAGS="$(CFLAGS)" XCFLAGS="$(XCFLAGS)" \
	MINUS_G= XTRAFLAGS="$(XCFLAGS)" AR_FLAGS="$(AR_FLAGS)" \
	LDFLAGS="$(LDFLAGS)" MV="$(MV)" DEBUG="$(DEBUG)" \
	$(DUMMY)


XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX
CC= $(HOSTBINDIR)/gcc -V $(VERSION) -b i386-linux
CFLAGS = -Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
INC= -I../soft
CPP=$(CC) -E $(INC) $(XCFLAGS)
RM=/bin/rm
NOSTDINC=-nostdinc++

DIRS=stdio

.SUFFIXES:
.SUFFIXES: .S .C .o .c .s .i

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

.c.i :
	$(CC) $(CFLAGS) -E $< -o $*.i

.C.o:
	$(CC) $(CFLAGS) $(NOSTDINC) -I../../iostream $(WRAP_C_INCLUDES) -c $(XTRAFLAGS) $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.C.s:
	$(CC) $(CFLAGS) $(NOSTDINC) -I../../iostream $(WRAP_C_INCLUDES) -S $(XTRAFLAGS) $<


.C.i:
	$(CC) $(CFLAGS) $(NOSTDINC) -I../../iostream $(WRAP_C_INCLUDES) -E $(XTRAFLAGS) $< -o $*.i

OBJS = stdstrbufs2.o
ASMS = stdstrbufs2.s

library: lib

lib: $(OBJS)

library_: lib
	for i in $(DIRS); \
	do \
		echo making $$i; \
		(cd $$i; make library $(MFLAGS)); \
	done

lib_: $(OBJS)
	$(AR) uvc $(LIB) $(OBJS)
	sync

asm: $(ASMS)
	for i in $(DIRS); \
	do \
		echo making asm in $$i; \
		(cd $$i; make asm $(MFLAGS)); \
	done

clean:
	$(RM) -f core *.o *.a tmp_make foo
	-for i in *.c;do \
	  if [ -f $$i ]; then \
	    $(RM) -f `basename $$i .c`.s; \
	  fi; \
	done
	for i in $(DIRS); \
	do \
		echo cleaning $$i; \
		(cd $$i; make clean $(MFLAGS)); \
	done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
	$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:

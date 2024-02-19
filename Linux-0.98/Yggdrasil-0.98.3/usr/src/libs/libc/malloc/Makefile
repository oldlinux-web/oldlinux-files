#
# Makefile for malloc library functions
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

LIB=../libc.a
LIBMCHECK=../libmcheck.a

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX
CC= $(HOSTBINDIR)/gcc -V $(VERSION) -b i386-linux
MALLOC_FLAGS=-DNO_FIX_MALLOC
CFLAGS = -Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
INC= -I../soft
CPP=$(CC) -E $(INC) $(XCFLAGS)
RM=/bin/rm

.SUFFIXES:
.SUFFIXES: .S .o .c .s .i

.s.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.S.o :
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.o :
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.s :
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) -S $<

.c.i :
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) -E $< -o $*.i

OBJS  = malloc.o free.o realloc.o calloc.o morecore.o \
	valloc.o memalign.o mcheck.o mtrace.o mstats.o cfree.o

all: library libmcheck

libmcheck: mcheck-init.o
	$(AR) uvc $(LIBMCHECK) mcheck-init.o
	sync

library: $(OBJS)
	$(AR) uvc $(LIB) $(OBJS)
	sync

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

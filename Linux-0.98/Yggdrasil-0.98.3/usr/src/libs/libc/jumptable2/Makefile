#
# Makefile for the global data
#

VERSION=`awk '{printf "%s", substr ($$2, 1, length ($$2)); }' $(SRCDIR)/VERSION`

# Host root directory, maybe /usr for Linux
#HOSTROOTDIR=/net/gecko/users/hlu/local
#HOSTROOTDIR=/local
HOSTROOTDIR=/usr
# Where we are now.
#SRCDIR=$(HOME)/Linux/src/libc-linux
#SRCDIR=/usr2/linux/src/libc-linux
SRCDIR=$(HOME)/libc-linux
# Host root directory, maybe /usr for Linux
TARGETROOTDIR=$(HOME)/Linux/usr
TARGETROOTDIR=/usr2/linux/usr2
TARGETROOT1DIR=/usr2/linux/usr3
TARGETROOT2DIR=/usr2/linux/usr6
# Host bin direcory
HOSTBINDIR=$(HOSTROOTDIR)/bin
# For Linux, that should be /usr/bin
CROSSBINDIR=$(HOST_LIB_DIR)
CROSSBINDIR=/usr2/linux/cross/bin
CROSSBINDIR=/local/lib/gcc-lib/i386-linux/$(VERSION)
CROSSBINDIR=/usr/bin

HOST_MACHINE=decstation-3100
HOST_MACHINE=i386-att-sysv
TARGET_MACHINE=i386-linux

MKDIR=mkdir -p


HOST_LIB_DIR =$(HOSTROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
HOST_SHARED_DIR =$(HOST_LIB_DIR)/shared
TARGET_LIB_DIR =$(TARGETROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
TARGET_LIB1_DIR =$(TARGETROOT1DIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
TARGET_LIB2_DIR =$(TARGETROOT2DIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
TARGET_SHARED_DIR =$(TARGET_LIB_DIR)/shared

MV	=/bin/mv

AR	=$(CROSSBINDIR)/ar
AR_FLAGS=ucv
AS	=$(CROSSBINDIR)/as
LD	=$(CROSSBINDIR)/ld
NM	=$(CROSSBINDIR)/nm
REALRANLIB	=$(CROSSBINDIR)/ranlib

RANLIB	=echo
LDFLAGS	=-r -x

CC	=$(HOSTBINDIR)/gcc -V $(VERSION) -b $(TARGET_MACHINE)
INC	=-I. -I../soft
DEBUG   =-g
GCFLAGS	=-Wall -O2 $(DEBUG) $(INC) $(XCFLAGS)
PGCFLAGS=-Wall -O2 -pg $(INC) $(XCFLAGS)
SCFLAGS	=-Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
CPP	=$(CC) -E $(INC) $(XCFLAGS)
RM	=/bin/rm

CFLAGS=$(SCFLAGS)

DOSUBDIRS=lib

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX

DIRS	=iostream

MFLAGS=CC="$(CC)" AR="$(AR)" LD="$(LD)" CPP="$(CPP)" AS="$(AS)" \
	RANLIB="$(RANLIB)" INC="$(INC)" RM="$(RM)" \
	CFLAGS="$(CFLAGS)" XCFLAGS="$(XCFLAGS)" \
	MINUS_G= XTRAFLAGS="$(XCFLAGS)" AR_FLAGS="$(AR_FLAGS)" \
	LDFLAGS="$(LDFLAGS)" MV="$(MV)" DEBUG="$(DEBUG)" \
	$(DUMMY)

LIBC_DATA2S=iostream/stdstrbufs2.o

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
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.s :
	$(CC) $(CFLAGS) -S $<

.c.i :
	$(CC) $(CFLAGS) -E $< -o $*.i

lib:
	make dosubdirs DOSUBDIRS=library

dep:
	make dosubdirs DOSUBDIRS=dep

asm:
	make dosubdirs DOSUBDIRS=asm

data: libc.data2.o 

text: libc.text2.o

libm.data2.o: $(LIBM_DATA2S)
	$(LD) -x -r -o $@ $?

libc.data2.o: $(LIBC_DATA2S)
	$(LD) -x -r -o $@ $?

dosubdirs:
	for i in $(DIRS); \
	do \
		echo making $(DOSUBDIRS) in $$i; \
		(cd $$i; make $(DOSUBDIRS) $(MFLAGS)); \
	done

clean:
	$(RM) -f $(LIBS)
	for i in $(DIRS); \
	do \
		echo cleaning $$i; \
		(cd $$i; make clean $(MFLAGS)); \
	done

#
# Makefile for the Linux C library
#

VERSION=`awk '{printf "%s", substr ($$2, 1, length ($$2)); }' $(SRCDIR)/VERSION`

HOME=/home/adam
# Host root directory, maybe /usr for Linux
#HOSTROOTDIR=/net/gecko/users/hlu/local
HOSTROOTDIR=/usr
# Where we are now.
#SRCDIR=$(HOME)/Linux/src/libc-linux
#SRCDIR=/usr2/linux/src/libc-linux
SRCDIR=$(HOME)/libc-linux
# Host root directory, maybe /usr for Linux
#TARGETROOTDIR=$(HOME)/Linux/usr
TARGETROOTDIR=$(HOME)

# normal static libraries.
#TARGETROOTDIR=/usr2/linux/disk2/usr

# libc_p.a, libg.a
#TARGETROOT1DIR=/usr2/linux/disk3/usr
TARGETROOT1DIR=$(HOME)

# libg.a compiled with -g1
#TARGETROOT2DIR=/usr2/linux/disk6/usr
TARGETROOT2DIR=$(HOME)

# Host bin direcory
HOSTBINDIR=$(HOSTROOTDIR)/bin
# For Linux, that should be /usr/bin
#CROSSBINDIR=$(HOST_LIB_DIR)
#CROSSBINDIR=/local/lib/gcc-lib/i386-linux/$(VERSION)
CROSSBINDIR=/usr/bin

HOST_MACHINE=decstation-3100
HOST_MACHINE=i386-att-sysv
TARGET_MACHINE=i386-linux

MKDIR=mkdir -p


HOST_LIB_DIR =$(HOSTROOTDIR)/lib/gcc-lib/$(TARGET_MACHINE)/$(VERSION)
HOST_SHARED_DIR =$(HOST_LIB_DIR)/shared

# normal static libraries.
TARGET_LIB_DIR =$(TARGETROOTDIR)/lib

# libc_p.a, libg.a
TARGET_LIB1_DIR =$(TARGETROOT1DIR)/lib

# libg.a compiled with -g1
TARGET_LIB2_DIR =$(TARGETROOT2DIR)/lib

MV	=/bin/mv

AR	=$(CROSSBINDIR)/ar
AR_FLAGS=ucv
AS	=$(CROSSBINDIR)/as
LD	=$(CROSSBINDIR)/ld
NM	=$(CROSSBINDIR)/nm
REALRANLIB	=$(CROSSBINDIR)/ranlib

RANLIB	=echo
LDFLAGS	=-r -x

#CC	=$(HOSTBINDIR)/gcc -V $(VERSION) -b $(TARGET_MACHINE)
CC	=gcc -V $(VERSION) -b $(TARGET_MACHINE)
INC	=-I. -I../soft -I../misc
DEBUG   =-g
GCFLAGS	=-Wall -O2 $(DEBUG) $(INC) $(XCFLAGS)
PGCFLAGS=-Wall -O2 -pg $(INC) $(XCFLAGS)
SCFLAGS	=-Wall -O2 -fomit-frame-pointer $(INC) $(XCFLAGS)
CPP	=$(CC) -E $(INC) $(XCFLAGS)
RM	=/bin/rm

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX # -D__USE_BSD_SIGNAL

LIBS	= libc.a libhard.a libtermcap.a libsoft.a libcurses.a \
	libmcheck.a libgmon.a libdbm.a # libinet.a

PGLIBS	=libc.a

GDIRS	=bsd ctype cvt dirent iostream grp locale \
	login malloc misc mntent posix pwd regex setjmp signal socket \
	stdlib string syscall time ufc inet jumptable1 ftw

PGDIRS	=bsd ctype cvt dirent iostream grp locale \
	login malloc misc mntent posix pwd regex setjmp signal socket \
	stdlib string syscall time ufc inet jumptable1 ftw

CDIRS	=bsd ctype curses cvt dirent hardm iostream grp locale \
	login malloc misc mntent posix pwd regex setjmp signal socket \
	softm stdlib string syscall termcap time ufc \
	gdbm gmon inet jumptable1 ftw

MFLAGS=CC="$(CC)" AR="$(AR)" LD="$(LD)" CPP="$(CPP)" AS="$(AS)" \
	RANLIB="$(RANLIB)" INC="$(INC)" RM="$(RM)" \
	CFLAGS="$(CFLAGS)" XCFLAGS="$(XCFLAGS)" \
	MINUS_G= XTRAFLAGS="$(XCFLAGS)" AR_FLAGS="$(AR_FLAGS)" \
	LDFLAGS="$(LDFLAGS)" MV="$(MV)" DEBUG="$(DEBUG)" \
	$(DUMMY)

all: static

debug:
	make lib CFLAGS="$(GCFLAGS)" DIRS="$(GDIRS)" \
		MV=echo LD=echo DEBUG=-g LDFLAGS=

profile:
	make lib CFLAGS="$(PGCFLAGS)" DIRS="$(PGDIRS)" DEBUG=-pg

static:
	make lib CFLAGS="$(SCFLAGS)" DIRS="$(CDIRS)" DEBUG=

jump:
	make lib CFLAGS="$(SCFLAGS) -DJUMP_TABLE" DIRS="$(CDIRS)" DEBUG=

lib:
	for i in $(DIRS); \
	do \
		echo making $$i; \
		(cd $$i; make $(MFLAGS)); \
	done
	$(RANLIB) $(LIBS)


dep:
	for i in $(DIRS); \
	do \
		echo making depend $$i; \
		(cd $$i; make dep $(MFLAGS)); \
	done

clean:
	$(RM) -f $(LIBS)
	for i in $(CDIRS); \
	do \
		echo cleaning $$i; \
		(cd $$i; make clean $(MFLAGS)); \
	done

install:
	@echo which version? debug, static, or profiling.

install.profile: libc.a
	-$(AR) -d libc.a __.SYMDEF > /dev/null
	$(REALRANLIB) libc.a
	mv libc.a libc_p.a
	make install.target LIBS=libc_p.a TARGET_LIB_DIR=$(TARGET_LIB1_DIR)

install.static: $(LIBS)
	make install.lib LIBS="$(LIBS)"

install.debug: libc.a
	-$(AR) -d libc.a __.SYMDEF > /dev/null
	$(REALRANLIB) libc.a
	mv libc.a libg.a
	make install.target LIBS=libg.a TARGET_LIB_DIR=$(TARGET_LIB1_DIR)

install.debug1: libc.a
	-$(AR) -d libc.a __.SYMDEF > /dev/null
	$(REALRANLIB) libc.a
	mv libc.a libg.a
	make install.target LIBS=libg.a TARGET_LIB_DIR=$(TARGET_LIB2_DIR)

install.lib: install.host install.target

install.host:
	-for l in $(LIBS); do \
	  $(AR) -d $$l __.SYMDEF > /dev/null; \
	done
	$(REALRANLIB) $(LIBS)
	-if [ ! -d $(HOST_LIB_DIR) ]; then $(MKDIR) $(HOST_LIB_DIR); fi
	cp $(LIBS) $(HOST_LIB_DIR)

install.target:
	-if [ ! -d $(TARGET_LIB_DIR) ]; then $(MKDIR) $(TARGET_LIB_DIR); fi
	mv $(LIBS) $(TARGET_LIB_DIR)

# This is Makefile of ufc functions for Linux
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

XCFLAGS = -D_GNU_SOURCE -DSTDC_HEADERS -DUSG -DDIRENT -DSYSV \
	-DUSE_BSD_REGEX
CC= $(HOSTBINDIR)/gcc -V $(VERSION) -b i386-linux
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
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.c.s :
	$(CC) $(CFLAGS) -S $<

.c.i :
	$(CC) $(CFLAGS) -E $< -o $*.i

CRYPT = crypt.o

LIBUFC=../libc.a
CRYPTOBJ = crypt_util.o $(CRYPT)

all: library

#
# Sample programs for debugging/testing/benchmarking
#

# Simple test for correct operation
ufc: ufc.c library
	$(CC) $(CFLAGS) ufc.c libufc.a -o ufc

tests: cert ufc speedc speedf
	./cert < ./cert.data
	./ufc 1
	./speedc
	./speedf

# Benchmark UFC fcrypt the crypt(3) in libc.a
speedf: library  speeds.c
	$(CC) $(CFLAGS) -DFCRYPT speeds.c libufc.a -o speedf
speedc: library speeds.c
	$(CC) $(CFLAGS)          speeds.c          -o speedc

# DES validation suite
cert: library cert.c
	$(CC) $(CFLAGS) cert.c libufc.a -o cert

#

library : $(CRYPTOBJ)
	$(AR) ucv $(LIBUFC) $(CRYPTOBJ)

crypt_util.o: crypt_util.c ufc-crypt.h

#
# Crypt functions
#

# Semiportable
crypt.o: crypt.c ufc-crypt.h

# Sun3 
crypt.sun3.o: crypt.sun3.S
	./S2o crypt.sun3.S crypt.sun3.o

# HP 9000 series 300
crypt.s300.o: crypt.s300.S
	./S2o crypt.s300.S crypt.s300.o

# SUN SPARC architecture
crypt.sparc.o: crypt.sparc.S
	./S2o crypt.sparc.S crypt.sparc.o

clean:
	/bin/rm -f *.o core *~ ufc libufc.a speedf speedc tmp.s mon.out cert \
          a.out Part*

kit:
	makekit COPYING README Makefile S2o crypt.c speeds.c *.S crypt_util.c \
		ufc.c cert.c cert.data patchlevel.h ufc-crypt.h

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

# Makefile for GNU binary-file utilities
# select a set of CFLAGS and PROGS, below, depending on the system type

GNUCC = gcc -O

# for BSD systems
CFLAGS = -g
# Don't add robotussin; it won't compile on BSD or GNU systems.
# objdump also won't compile on most systems (trouble with N_DATADDR).
PROGS = gprof ld size nm strip ar ranlib

# for USG systems using COFF_ENCAPSULATE
# also, you will want to make the target libc.a (but it takes a long time)
# Note that you should leave a copy of `ar' in this directory
# after you install it, since `ranlib' will try to run it from here.
#CFLAGS = -g -DUSG -DCOFF_ENCAPSULATE -DPORTAR -DNON_NATIVE -Dnounderscore
#PROGS = ld size nm strip ar robotussin objdump ranlib
#LIBS = -lPW

all: $(PROGS)

ld: ld.o
# LIBS is used here since ld needs to use alloca.
# Alternatively, compile it with GNU C--then the compiler handles alloca.
	$(CC) -o ld ld.o $(LIBS)

size: size.o
	$(CC) -o size size.o

nm: nm.o
	$(CC) -o nm nm.o

strip: strip.o
	$(CC) -o strip strip.o

ar: ar.o
	$(CC) -o ar ar.o

gprof: gprof.o
	$(GNUCC) -o gprof gprof.o
gprof.o: gprof.c gmon.h
	$(GNUCC) -c $(CFLAGS) gprof.c

ranlib: ranlib.o
	$(CC) -o ranlib ranlib.o
ranlib.o: ranlib.c
	$(CC) -c -g -DAR_PROG=\"`pwd`/ar\" ranlib.c

# Robotussin is NOT part of `all'.
robotussin: robotussin.o
	$(CC) -o robotussin robotussin.o

libc.a: robotussin libconvert
	libconvert
	rm -rf tmp

clean:
	-rm -f *.o core
	-rm -f gprof ar nm size strip ld robotussin objdump ranlib

dist:
	-rm -rf binutils
	mkdir binutils
	-ln * binutils
	(cd binutils; rm -f GNUmakefile *~ "#*" log* binutils.tar*; make clean)
	tar cof binutils.tar binutils
	compress < binutils.tar > binutils.tar.Z
	rm -rf binutils

# CKUMNX.MAK, Version 2.11, 29 January 1988
#
# -- Makefile to build C-Kermit for Minix.  This is a separate file
# because Minix uses .o instead of .o for its object files. This
# means that not one line of the file is the same as the standard
# Unix makefile, so it seems silly to clutter the common file.

###########################################################################
#
#  Compile and Link variables:
#
LNKFLAGS=
CC= cc
CC2= cc
CFLAGS=-DV7 -DMINIX -O
#
###########################################################################
#
# Dependencies Section:
#

wermit: ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o ckcfns.o \
		 ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o
	$(CC2) $(LNKFLAGS) -o wermit ckcmai.o ckutio.o \
		 ckufio.o ckcfns.o \
		 ckcfn2.o ckcpro.o ckucmd.o ckuus2.o ckuus3.o ckuusr.o \
		 ckucon.o ckudia.o ckuscr.o

ckcmai.o: ckcmai.c ckcker.h ckcdeb.h ckcsym.h

ckuusr.o: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus2.o: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus3.o: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckucmd.o: ckucmd.c ckucmd.h ckcdeb.h

ckcpro.o: ckcpro.c ckcker.h ckcdeb.h

# I'm using the distributed copy of ckcpro.c.  Unfortunately
# wart won't compile.  If you need to change ckcpro.w, you'll have
# to find a way to reduce the number of strings in ckwart.c
#
# ckcpro.c: ckcpro.w wart
#	./wart ckcpro.w ckcpro.c

ckcfns.o: ckcfns.c ckcker.h ckcdeb.h ckcsym.h

ckcfn2.o: ckcfn2.c ckcker.h ckcdeb.h ckcsym.h

ckufio.o: ckufio.c ckcker.h ckcdeb.h

ckutio.o: ckutio.c ckcdeb.h

ckucon.o: ckucon.c ckcker.h ckcdeb.h

wart: ckwart.o
	$(CC) $(LNKFLAGS) -o wart ckwart.o

ckwart.o: ckwart.c

ckudia.o: ckudia.c ckcker.h ckcdeb.h ckucmd.h

ckuscr.o: ckuscr.c ckcker.h ckcdeb.h

#Clean up intermediate and object files
clean:
	-rm -f ckcmai.o ckucmd.o ckuusr.o ckuus2.o ckuus3.o ckcpro.o \
	ckcfns.o ckcfn2.o ckucon.o ckutio.o ckufio.o ckudia.o ckuscr.o \
	ckwart.o ckcpro.c

#Run Lint on this mess for the BSD version.
lint:
	-lint -x -DBSD4 -DDEBUG -DTLOG ck[cu]*.[hc] > ck.lint.bsd4

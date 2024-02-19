# CKUMNX.MAK, Version 2.11, 29 January 1988
#
# -- Makefile to build C-Kermit for Minix.  This is a separate file
# because Minix uses .s instead of .o for its object files. This
# means that not one line of the file is the same as the standard
# Unix makefile, so it seems silly to clutter the common file.

###########################################################################
#
#  Compile and Link variables:
#
LNKFLAGS= -i -T.
CC= cc
CC2= cc
CFLAGS=-DV7 -DMINIX -i -D_MINIX -D_POSIX_SOURCE
#
###########################################################################
#
# Dependencies Section:
#

wermit: ckcmai.s ckucmd.s ckuusr.s ckuus2.s ckuus3.s ckcpro.s ckcfns.s \
		 ckcfn2.s ckucon.s ckutio.s ckufio.s ckudia.s ckuscr.s
	$(CC2) $(LNKFLAGS) -o wermit ckcmai.s ckutio.s \
		 ckufio.s ckcfns.s \
		 ckcfn2.s ckcpro.s ckucmd.s ckuus2.s ckuus3.s ckuusr.s \
		 ckucon.s ckudia.s ckuscr.s

ckcmai.s: ckcmai.c ckcker.h ckcdeb.h ckcsym.h

ckuusr.s: ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus2.s: ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckuus3.s: ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h

ckucmd.s: ckucmd.c ckucmd.h ckcdeb.h

ckcpro.s: ckcpro.c ckcker.h ckcdeb.h

# I'm using the distributed copy of ckcpro.c.  Unfortunately
# wart won't compile.  If you need to change ckcpro.w, you'll have
# to find a way to reduce the number of strings in ckwart.c
#
# ckcpro.c: ckcpro.w wart
#	./wart ckcpro.w ckcpro.c

ckcfns.s: ckcfns.c ckcker.h ckcdeb.h ckcsym.h

ckcfn2.s: ckcfn2.c ckcker.h ckcdeb.h ckcsym.h

ckufio.s: ckufio.c ckcker.h ckcdeb.h

ckutio.s: ckutio.c ckcdeb.h

ckucon.s: ckucon.c ckcker.h ckcdeb.h

wart: ckwart.s
	$(CC) $(LNKFLAGS) -o wart ckwart.s

ckwart.s: ckwart.c

ckudia.s: ckudia.c ckcker.h ckcdeb.h ckucmd.h

ckuscr.s: ckuscr.c ckcker.h ckcdeb.h

#Clean up intermediate and object files
clean:
	@rm -f *.s *.bak wermit

#Run Lint on this mess for the BSD version.
lint:
	-lint -x -DBSD4 -DDEBUG -DTLOG ck[cu]*.[hc] > ck.lint.bsd4

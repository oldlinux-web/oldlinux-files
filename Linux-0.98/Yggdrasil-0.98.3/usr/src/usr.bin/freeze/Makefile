SHELL         = /bin/sh

DEST          = /usr/bin
MANDEST       = /usr/man/man1
SEC           = 1

HDRS          = bitio.h\
		compat.h\
		freeze.h\
		huf.h\
		lz.h\
		patchlevel.h

CC            = gcc
# or proprietary compiler, if it's better than that

# bsd:
#	CFLAGS=-O -fstrength-reduce -DBSD -DUTIMES -DBITS=16 -DFAST -DSIGTYPE=void
# sysv:
#	CC=cc
#	CFLAGS=-O -DBITS=16 -DFAST -DSIGTYPE=void
#	LIBS=-lc_s
# linux:
CFLAGS=-O2 -DBITS=16 -DFAST -DSIGTYPE=void
# x286:
# 	CC=cc
#	CFLAGS=-Ox -Mc2 -DBITS=16 -DFAST -DSIGTYPE=int
# 	MANDEST=/usr/man/man.C
#	SEC=C
# generic:
#	CC=cc
# 	CFLAGS=-O -DBITS=16 -DFAST -DSIGTYPE=int
# sun4:
# 	CC=cc
#	CFLAGS=-O4 -DBSD -DSUN4 -DSIGTYPE=void -DUTIMES -DBITS=16 -DFAST

OPTIONS       = 

LINTFLAGS     = -DBITS=15 -DSIGTYPE=void -DCOMPAT -DDEBUG\
		-DGATHER_STAT -x -DFAST

MAKEFILE      = Makefile

OBJS          = bitio.o\
		debug.o\
		decode.o\
		default.o\
		encode.o\
		freeze.o\
		huf.o\
		lz.o

PROGRAM       = freeze

CATMAN        = freeze.man statist.man

MAN           = freeze.1 statist.1

SRCS          = bitio.c\
		debug.c\
		decode.c\
		default.c\
		encode.c\
		freeze.c\
		huf.c\
		lz.c

.SUFFIXES:       .1 .man

.1.man:
		nroff -man < $< > $@

.c.o:
		$(CC) -c $(CFLAGS) $(OPTIONS) $<

prog:            $(PROGRAM) statist

man:            $(CATMAN)

lint:           $(SRCS)
		lint $(LINTFLAGS) $(SRCS) > lint.out

$(PROGRAM):     $(OBJS) 
		$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)

statist: statist.o lz.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o statist statist.o lz.o $(LIBS)

clobber:        clean
		rm -f $(PROGRAM) statist *.man

clean:;         rm -f *.o *.b .,* core *.out

install:        $(DEST)/$(PROGRAM) $(DEST)/statist $(MANDEST)/freeze.$(SEC) $(MANDEST)/statist.$(SEC) $(MANDEST)/ftar.$(SEC) /etc/magic $(DEST)/zf $(DEST)/ftar

patch:;         rm -f patch.out
		-for i in ../distribution/* ; do \
		(diff -c $$i `basename $$i` >> patch.out); \
		done

$(DEST)/$(PROGRAM): $(PROGRAM)
		cp $(PROGRAM) $@
		chmod 755 $@
		-strip $@
		-mcs -d $@
		-ln -f $@ $(DEST)/melt
		-ln -f $@ $(DEST)/fcat

$(DEST)/statist: statist
		cp statist $@
		chmod 755 $@
		-strip $@
		-mcs -d $@

$(MANDEST)/freeze.$(SEC): freeze.1
		cp freeze.1 $@
		chmod +r $@
		-ln -f $@ $(MANDEST)/melt.$(SEC)
		-ln -f $@ $(MANDEST)/fcat.$(SEC)

$(MANDEST)/statist.$(SEC): statist.1
		cp statist.1 $@
		chmod +r $@

$(MANDEST)/ftar.$(SEC): ftar.1
		cp ftar.1 $@
		chmod +r $@
		-ln -f $@ $(MANDEST)/ztar.$(SEC)

/etc/magic: magic.patch
		patch -p0 <magic.patch

$(DEST)/zf: zf
		cp -p zf $@
		chmod 755 $@

$(DEST)/ftar: ftar
		cp -p ftar $@
		chmod 755 $@
		-ln -f $@ $(DEST)/ztar

###
bitio.o: freeze.h compat.h bitio.h
debug.o: freeze.h compat.h huf.h bitio.h
decode.o: freeze.h compat.h huf.h bitio.h
default.o: freeze.h compat.h
encode.o: freeze.h compat.h lz.h huf.h bitio.h
freeze.o: freeze.h compat.h lz.h huf.h patchlevel.h
huf.o: freeze.h compat.h huf.h bitio.h
lz.o: freeze.h compat.h lz.h
statist.o: freeze.h compat.h lz.h

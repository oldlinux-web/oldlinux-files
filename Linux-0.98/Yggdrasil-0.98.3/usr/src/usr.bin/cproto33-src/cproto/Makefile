# $Id: makefile.uni 3.4 92/04/11 19:27:21 cthuang Exp $
#
# UNIX makefile for C prototype generator

# Define BSD for BSD, otherwise System V is assumed.
#DEFINES = -DBSD

LEX = flex
YACC = bison
CFLAGS = $(DEFINES) -O2

DIST1 = README CHANGES cproto.man
DIST2 = cproto.1 Makefile.bc Makefile.msc Makefile.uni lex.l grammar.y
DIST3 = config.h cproto.h patchlev.h semantic.h symbol.h
DIST4 = cproto.c popen.c semantic.c strstr.c symbol.c

OBJECTS = cproto.o semantic.o symbol.o y.tab.o

all: cproto cproto.man

cproto: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

y.tab.c: grammar.y
	$(YACC) grammar.y
	mv grammar.tab.c y.tab.c

lex.yy.c: lex.l
	$(LEX) lex.l

cproto.man: cproto.1
	nroff -man cproto.1 >$@

clean:
	rm -f *.o *.bak *.log cproto # lex.yy.c y.tab.c

ci:
	ci -u3 $(DIST2) $(DIST3) $(DIST4)

shar:
	shar $(DIST1) $(DIST2) >cproto.sh1
	shar $(DIST3) $(DIST4) >cproto.sh2

depend:
	makedepend $(DIST4) y.tab.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

cproto.o: config.h cproto.h symbol.h
semantic.o: config.h cproto.h symbol.h semantic.h
strstr.o: config.h
symbol.o: config.h cproto.h symbol.h
y.tab.o: config.h cproto.h symbol.h semantic.h lex.yy.c

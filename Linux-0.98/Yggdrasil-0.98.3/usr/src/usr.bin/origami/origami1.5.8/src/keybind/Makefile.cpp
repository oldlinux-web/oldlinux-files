/* cpp-makefile for Origami
   Use the following symbols to generate a makefile for your system by
   using /usr/lib/cpp -DSYMBOL Makefile.cpp >Makefile

   MINIX    - MINIX 1.5.10
     INTEL  - Intel Version
     MC68K  - Motorola 68k Version

   SUNOS    - SUN 3/4 workstations

   The following CFLAGS are supported:

   SHORT_NAMES  for only 7 significant characters in indentifiers
*/

#ifdef MINIX
#ifdef INTEL
CC=		cc
CFLAGS=		-O -DSHORT_NAMES -D_MINIX -D_POSIX_SOURCE
LDFLAGS=	-i -T/usr/tmp
LIBS=		
O=		s
#endif
#ifdef MC68K
/* Warning: This will probably not work! */
CC=		cc
CFLAGS=		-O
LDFLAGS=	-T/usr/tmp
LIBS=		
O=		o
#endif
#endif

#ifdef SUNOS
CC=		cc
CFLAGS=		-I.. -O -D_POSIX_SOURCE
LDFLAGS=	-s
LIBS=		-ltermcap
O=		o
#endif

HDRS=		../common/rcformat.h ../common/keys.h keybind.h \
		short.h

SRCS=		codehelp.c keybind.c keytab.c messages.c name.c parsecond.c \
		parsemac.c parser.c scanner.c fileio.c

OBJS=		codehelp.$(O) keybind.$(O) keytab.$(O) messages.$(O) \
		name.$(O) parsecond.$(O) parsemac.$(O) parser.$(O) scanner.$(O)

keybind:        $(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
		
$(OBJS):	$(HDRS)

clean:
		rm -f core keybind $(OBJS)

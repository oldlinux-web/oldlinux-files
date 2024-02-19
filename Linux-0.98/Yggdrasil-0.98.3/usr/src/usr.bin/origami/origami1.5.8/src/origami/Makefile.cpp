/* cpp-makefile for Origami
   Use the following symbols to generate a makefile for your system by
   using /usr/lib/cpp -DSYMBOL Makefile.cpp >Makefile

   MINIX    - MINIX 1.5.10
     INTEL  - Intel Version
     MC68K  - Motorola 68k Version

   SUNOS    - SUN 3/4 workstations

   The following CFLAGS are supported:

   SHORT_NAMES  for only 7 significant characters in indentifiers
   NO_MSG       to avoid the copyright message with sleep at startup.  Only for
                personal use!
   NOLTCHARS    if your system does not support ltchars
*/

#ifdef MINIX
#ifdef INTEL
CC=		cc
CFLAGS=		-O -DSHORT_NAMES -DNO_MSG -DNOLTCHARS -D_MINIX -D_POSIX_SOURCE
LDFLAGS=	-i -T/usr/tmp
LIBS=		
O=		s
#endif
#ifdef MC68K
/* Warning: This will probably not work! */
CC=		cc
CFLAGS=		-O -DNO_MSG -DNOLTCHARS -D_POSIX_SOURCE
LDFLAGS=	-T/usr/tmp
LIBS=		
O=		o
#endif
#endif

#ifdef SUNOS
CC=		cc
CFLAGS=		-O -I.. -DNO_MSG -D_POSIX_SOURCE
LDFLAGS=	-s
LIBS=		-ltermcap
O=		o
STRERROR=	strerror.$(O)
#endif

HDRS=		../common/keys.h macros.h origami.h short.h

SRCS=		display.c fieldedit.c finds.c folding.c main.c \
		foldfiling.c gettoken.c initialise.c keyboard.c keytab.c \
		messages.c misc.c oriedt.c loop.c prompts.c screen.c \
		signals.c string.c readfolds.c writefolds.c

OBJS=		display.$(O) fieldedit.$(O) finds.$(O) main.$(O) \
		folding.$(O) foldfiling.$(O) gettoken.$(O) initialise.$(O) \
		keyboard.$(O) keytab.$(O) messages.$(O) misc.$(O) oriedt.$(O) \
		loop.$(O) prompts.$(O) screen.$(O) signals.$(O) string.$(O) \
		readfolds.$(O) writefolds.$(O) $(STRERROR)

origami:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJS):	$(HDRS)

hostbegin.$(O):	../common/rcformat.h

clean:
		rm -f core origami $(OBJS)

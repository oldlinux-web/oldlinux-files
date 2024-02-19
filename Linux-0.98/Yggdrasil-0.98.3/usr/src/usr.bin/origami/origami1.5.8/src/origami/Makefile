# 1 "Makefile.lin"
 

















# 35 "Makefile.lin"



CC=		cc
# was CFLAGS= -O .........
CFLAGS=		-O3 -I.. -DLINUX -DNO_MSG -D_POSIX_SOURCE
LDFLAGS=	-s
LIBS=		-ltermcap
O=		o
STRERROR=	strerror.$(O)


HDRS=		../common/keys.h macros.h origami.h short.h

SRCS=		display.c fieldedit.c finds.c folding.c main.c 		foldfiling.c gettoken.c initialise.c linuxkeys.c keytab.c 		messages.c misc.c oriedt.c loop.c prompts.c screen.c 		signals.c string.c readfolds.c writefolds.c




OBJS=		display.$(O) fieldedit.$(O) finds.$(O) main.$(O) 		folding.$(O) foldfiling.$(O) gettoken.$(O) initialise.$(O) 		linuxkeys.$(O) keytab.$(O) messages.$(O) misc.$(O) oriedt.$(O) 		loop.$(O) prompts.$(O) screen.$(O) signals.$(O) string.$(O) 		readfolds.$(O) writefolds.$(O) $(STRERROR)





origami:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJS):	$(HDRS)

hostbegin.$(O):	../common/rcformat.h

clean:
		rm -f core origami $(OBJS)

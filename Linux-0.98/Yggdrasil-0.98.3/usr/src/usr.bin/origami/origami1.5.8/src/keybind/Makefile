# 1 "Makefile.cpp" 















# 32 "Makefile.cpp" 



CC=		cc
CFLAGS=		-I.. -O -D_POSIX_SOURCE
LDFLAGS=	
LIBS=		-ltermcap
O=		o


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

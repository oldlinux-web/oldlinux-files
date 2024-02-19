OBJS= authenc.o commands.o main.o network.o ring.o sys_bsd.o \
	telnet.o terminal.o tn3270.o utilities.o
CFLAGS=-DTERMCAP -DKLUDGELINEMODE -DUSE_TERMIO -O -I./libtelnet

PROG= telnet
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(OBJS) ./libtelnet/libtelnet.a

clean:
	rm $(PROG) $(OBJS)

install:
	install $(PROG) /usr/bin

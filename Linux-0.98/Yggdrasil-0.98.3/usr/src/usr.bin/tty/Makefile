#
#

PROGRAM = tty
SRC	= tty.c
OBJS	= tty.o

CC=cc
CFLAGS	= -O


all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<


# provided by hlu, modified by obz
NAME = mouse
OBJS = mouse.o
CC = gcc
CFLAGS = -O -fomit-frame-pointer
#LDFLAGS = -static

$(NAME) : $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(NAME) $(OBJS) $(LIBS)

clean:
	-rm -f $(NAME) $(OBJS)

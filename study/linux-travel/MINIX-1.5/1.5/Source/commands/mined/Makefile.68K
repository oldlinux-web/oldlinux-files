
CFLAGS	= -O -DATARI_ST

CMD	= mined
OBJ	= mined1.o mined2.o

all:	$(CMD)

clean:
	rm -f $(CMD) *.o



$(CMD):	$(OBJ)
	$(CC) -o $@ $(OBJ)
	chmem =64000 $@

$(OBJ):	mined.h

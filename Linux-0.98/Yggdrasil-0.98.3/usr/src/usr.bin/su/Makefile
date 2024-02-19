OBJS = getpass.o su.o 
EXE= su

all: $(EXE)

su: $(OBJS)
	$(CC) $(EXE).c -lufc -o $(EXE)

install:
	mv $(EXE) $(INST)
	chown $(OWNER) $(INST)/$(EXE)
	chmod 7555 $(INST)/$(EXE)
	chmod a+s $(INST)/$(EXE)

clean:
	rm -rf $(EXE) *.o


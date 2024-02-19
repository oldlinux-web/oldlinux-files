# Makefile for make!  (Minix PC)

CFLAGS = -Dunix -DMINIXPC 

OBJS =	check.s input.s macro.s main.s make.s reader.s rules.s

make :	$(OBJS)
	$(CC) -i -o make $(OBJS)
 
clean:	
	@rm -f *.s *.bak make core

$(OBJS): h.h

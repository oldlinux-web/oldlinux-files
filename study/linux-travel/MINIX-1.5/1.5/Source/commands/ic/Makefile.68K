CFLAGS	= -O
ICOBJ   = ic.o ic_input.o ic_output.o 

ic:	$(ICOBJ)
	cc  $(ICOBJ) -o ic
	chmem =5000 ic

$(ICOBJ): ic.h

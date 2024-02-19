CFLAGS	= -F
ICOBJ   = ic.s ic_input.s ic_output.s 

ic:	$(ICOBJ)
	cc -T. $(ICOBJ) -o ic
	chmem =5000 ic

$(ICOBJ): ic.h


clean:	
	@rm -f *.bak *.s ic

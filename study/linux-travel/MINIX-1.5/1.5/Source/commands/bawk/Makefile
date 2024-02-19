#
#       Makefile for BAWK		(MINIX)
#
#   			Makefile @(#)1.1        
#

OPSYS   = MINIX

CFLAGS  = -D$(OPSYS)

OBJ     = bawk.s bawkact.s bawksym.s bawkpat.s bawkdo.s

bawk:       $(OBJ)
	    @echo Start linking BAWK
	    @cc -i -o bawk $(OBJ)

bawk.s:     bawk.h bawk.c

bawkact.s:  bawk.h bawkact.c

bawkpat.s:  bawk.h bawkpat.c

bawksym.s:  bawk.h bawksym.c

bawkdo.s:   bawk.h bawkdo.c

clean:	
	@rm -f *.bak *.s bawk

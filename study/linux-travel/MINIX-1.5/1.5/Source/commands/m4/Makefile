#	-DEXTENDED	#if you like to get paste & spaste macros.
#	-DVOID 		#if your C compiler does NOT support void.
#	-DGETOPT	#if you STILL do not have getopt in your library.
#	-DDUFFCP	#if you do not have fast memcpy in your library.
#
CFLAGS = -DEXTENDED
OBJS =  main.s eval.s serv.s look.s misc.s expr.s
INCL =  mdef.h extr.h patchlevel.h
m4: $(OBJS) $(INCL)
	cc -o m4 $(OBJS)


clean:	
	@rm -f *.bak *.s m4

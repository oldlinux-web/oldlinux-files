# makefile for the 'indent' command

CFLAGS = -F -T. -Dlint

OBJS = indent.s io.s lexi.s parse.s comment.s args.s

indent: ${OBJS}
	$(CC) -i -T. -o indent ${OBJS}
	chmem =60000 indent

$(OBJS): globs.h codes.h


clean:	
	@rm -f *.bak *.s indent

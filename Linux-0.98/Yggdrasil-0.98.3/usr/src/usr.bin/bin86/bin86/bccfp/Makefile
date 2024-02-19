# Makefile for bcc 386 software floating point library

.SUFFIXES: .x		# .x files are .s files that need C-preprocessing
.x.o:
	$(CPP) $(CPPFLAGS) $< >tmp
	$(AS) tmp -n $* -o $@

AS		=as -3
CFLAGS		=-O
CPP		=/lib/cpp
CPPFLAGS	=-P
FPDIST		=Makefile $(FPSRC) test.c bccfp.tex
FPSRC		=fadd.x fcomp.x fdiv.x fmul.x fbsr.x \
		fperr.c fperror.x fptoi.x fpushd.x fpulld.x \
		fpushi.x fpushf.x fpullf.x frexp.x ftst.x \
		gcclib.x \
		fabs.x ldexp.x modf.c \
		fperr.h fplib.h
FPOBJ		=fadd.o fcomp.o fdiv.o fmul.o fpbsr.o \
		fperr.o fperror.o fptoi.o fpushd.o fpulld.o \
		fpushi.o fpushf.o fpullf.o frexp.o ftst.o \
		fabs.o ldexp.o modf.o
JUNK		=tmp
LIB		=.

test: test.c $(LIB)/libfp.a
	$(CC) -o $@ test.c $(LIB)/libfp.a -lm

$(FPOBJ): fplib.h
fperr.c fperror.x: fperr.h

$(LIB)/libfp.a: $(FPOBJ)
	ar rc $(LIB)/libfp.a $(FPOBJ)
	rm -f $(JUNK)

dist: $(FPDIST)
	/bin/tar cvf - $(FPDIST) | /bin/compress -b 13 >bccfp.tar.Z
	uue bccfp.tar.Z

clean:
	rm -f $(FPOBJ) $(JUNK) test

realclean: clean
	rm -f $(LIB)/libfp.a bccfp.tar.Z bccfp.uue

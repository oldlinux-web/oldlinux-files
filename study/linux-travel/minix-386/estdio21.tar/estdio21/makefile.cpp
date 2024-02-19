#ifdef	_UNIX
#undef	_UNIX
#nix _UNIX
#define SITE		__SITE__
#define CDIFF		__CDIFF__
/*efine WHOAMI		_UNIX*/
#define DEFINES		__DEFINES__
#define INCLUDES	-I.
#define LIBRARIES	STDIOLIB
#define MATHLIB		__MATHLIB__
#define CCFLAGS
#define LDFLAGS
#define O		o
#define S		s
#define AOUT(x)		x
CPP	=__CPP__ DEFINES
FPCC	=__FPCC__
CC	=__CC__
FPLD	=__FPLD__
LD	=__LD__
RANLIB	=__RANLIB__
LORDER	=__LORDER__
TSORT	=__TSORT__
#define EMPTY		echo -n
#define ASSEMBLE(f)	as -o f.O f.S
#define EXE(x)		x
#define ARC		arc
#define FPLIB		stdiofp.a
#define STDIOLIB	stdio.a
#endif

#ifdef	_MINIX
#undef	_MINIX
#min _MINIX
#define SITE		ack
#define CDIFF		cdiff
#define WHOAMI		_MINIX
#define DEFINES		-D_MINIX -D_POSIX_SOURCE
#define LIBRARIES	STDIOLIB
#define MATHLIB		-f
#define INCLUDES	-I.
#define CCFLAGS		-LIB -c
#define LDFLAGS		-i
#define O		s
#define S		x
#define AOUT(x)		x
CPP	=/usr/lib/cpp -P -D_MINIX
FPCC	=$(CC) -f
CC	=cc
FPLD	=$(LD)
LD	=$(CC) LDFLAGS
RANLIB	=echo
LORDER	=lorder
TSORT	=tsort
#define EMPTY		echo -n
#define ASSEMBLE(f)	libpack < f.S > f.O
#define EXE(x)		-o x
#define ARC		arc
#define FPLIB		stdiofp.a
#define STDIOLIB	stdio.a
#endif

#ifdef	_MSDOS
#undef	_MSDOS
#dos _MSDOS
#define SITE		tcc
#define CDIFF		cdiff
#define WHOAMI		_MSDOS
#define DEFINES		-D_MINIX
#define LIBRARIES	STDIOLIB
#define MATHLIB
#define INCLUDES	-I.
#define CCFLAGS		-Or -Oj -p -c
#define LDFLAGS
#define O		o
#define S		s
#define AOUT(x)		x
CPP	=cpp -P -D_MSDOS
FPCC	=cc
CC	=cc
FPLD	=$(LD)
LD	=$(CC) LDFLAGS
RANLIB	=echo
LORDER	=echo
TSORT	=echo
#define EMPTY		type nul
#define ASSEMBLE(f)	as -c f.S
#define EXE(x)		-o x
#define ARC		pkarc
#define FPLIB		stdiofp.lib
#define STDIOLIB	stdio.lib
#endif

#define NAME estdio

Installdir	=INSTALL/
Testdir		=TEST/
Fptestdir	=FPTEST/
Define		=DEFINES
Include		=INCLUDES
Libraries	=LIBRARIES
MathLibraries	=MATHLIB
FPLib		=FPLIB
StdioLib	=STDIOLIB

All		=README *.x *.c *.g *.h \
		 makefile.cpp makefile.nix makefile.min \
		 $(Testdir)*.c $(Testdir)*.sh \
		 $(Fptestdir)*.c \
		 $(Installdir)*.c $(Installdir)*.sh \
		 $(Installdir)*.txt $(Installdir)*.sit \
		 $(Installdir)*.ans

Everyone	=nix min dos

CFLAGS= $(Define) $(Include) CCFLAGS

FPOBJ= \
_f_cvt.O	    _f_fprintf.O	_f_fscanf.O \
_f_guard.O	    _f_pow10.O		_f_printf.O \
_f_scanf.O	    _f_sprintf.O	_f_sscanf.O \
_f_tvc.O	    _f_vfprintf.O	_f_vprintf.O \
_f_vsprintf.O

LIBOBJ= \
_allocbuf.O	    _bfs.O		_bread.O \
_bwrite.O	    _err.O		_errlist.O \
_fgetlx.O	    _file.O		_fopen.O \
_freebuf.O	    _in.O		_ioread.O \
_iowrite.O	    _ipow10.O		_open3.O \
_os.O		    _out.O		_rename.O \
_rlbf.O		    _stdio.O		_update.O \
_utoa.O		    _vfprintf.O		_vfscanf.O \
_vscanf.O	    _vsscanf.O		_xassert.O \
_z_cvt.O	    _z_tvc.O		_zatexit.O \
_zerr.O		    _zout.O		_zrlbf.O \
_zwrapup.O	    atexit.O		clearerr.O \
ctermid.O	    cuserid.O		exit.O \
fclose.O	    fdopen.O		feof.O \
ferror.O	    fflush.O		fgetc.O \
fgetpos.O	    fgets.O		fileno.O \
fopen.O		    fprintf.O		fputc.O \
fputs.O		    fread.O		freopen.O \
fscanf.O	    fseek.O		fsetpos.O \
ftell.O		    fwrite.O		getc.O \
getchar.O	    gets.O		getw.O \
perror.O	    printf.O		putc.O \
putchar.O	    puts.O		putw.O \
remove.O	    rewind.O		scanf.O \
setbuf.O	    setvbuf.O		sprintf.O \
sscanf.O	    tmpfile.O		tmpnam.O \
ungetc.O	    vfprintf.O		vprintf.O \
vsprintf.O

.SUFFIXES:	.O .c
.c.O:
	$(CC) $(CFLAGS) $<

test:
	-cd $(Testdir) ; \
	for f in *.c ; do \
	  n=`basename $$f .c` ; \
	  $(CC) $(CFLAGS) -I.. $$n.c ; \
	  $(LD) EXE($$n) $$n.O ../stdio.a ; \
	done

fptest:
	-cd $(Fptestdir) ; \
	for f in *.c ; do \
	  n=`basename $$f .c` ; \
	  $(FPCC) $(CFLAGS) -I.. $$n.c ; \
	  $(FPLD) EXE($$n) $$n.O ../stdiofp.a ../stdio.a $(MathLibraries) ; \
	done

update:
	-if test -z "$(LIBC)" ; then \
	  echo Usage: make update LIBC=library ; \
	else \
	  ar t $(LIBC) | sed -e 's/ //g' | sort > libc.files ; \
	  ar rv $(LIBC) `ls *.o | comm -12 - libc.files` ; \
	  rm -f libc.files ; \
	  $(RANLIB) $(LIBC); \
	fi

# Library dependencies

$(FPOBJ) $(LIBOBJ):	stdiolib.h stdio.h

#ifdef	_MSDOS
#else
Hide:	site.h
	rm -f Hidden
	-if grep -s "define HIDDEN" site.h ; then \
	$(CPP) stub.x >Hidden ; \
	for f in `grep '^.define [a-z]' hidden.h | \
		  sed 's/^.define \([a-z]*\).*/\1/'` ; do \
	echo $${f} ; \
	sed -e "s/XYZ/$${f}/g" < Hidden > _u_$${f}.S ; \
	ASSEMBLE(_u_$${f}) ; \
	done ; \
	fi
	EMPTY >Hide
#endif

# Put together a library

#ifdef	_MSDOS
$(StdioLib):	$(LIBOBJ)
	rm -f $(StdioLib)
	arlib ca $(StdioLib) $(LIBOBJ)
#else
FPobjects:	$(FPOBJ)

FPorder:	$(FPOBJ) Hide
	EMPTY >FPorder
	-if test -f Hidden ; then \
	ls _u_*print*.O _u_*scan*.O >> FPorder ; \
	fi
	ls $(FPOBJ) > FPorder.tmp
	$(LORDER) `cat FPorder.tmp` | $(TSORT) >> FPorder; \
	rm -f FPorder.tmp FPsort.tmp

FParchive:	FPorder
	rm -f $(FPLib)
	ar cr $(FPLib) `cat FPorder`
	$(RANLIB) stdiofp.a

Liborder:	$(LIBOBJ) Hide
	EMPTY >Liborder
	-if test -f Hidden ; then \
	ls _u_*.O >> Liborder ; \
	fi
	ls $(LIBOBJ) > Liborder.tmp
	$(LORDER) `cat Liborder.tmp` | $(TSORT) >> Liborder; \
	rm -f Liborder.tmp

stdio $(StdioLib):	Liborder
	rm -f $(StdioLib)
	ar cr $(StdioLib) `cat Liborder`
	$(RANLIB) stdio.a

stdiofp $(FPLib):
	$(MAKE) FPobjects "CC=$(FPCC)" "LD=$(FPLD)"
	$(MAKE) FParchive
#endif

# Pretty list of objects

fpfiles:
	ls _f_*.c | sed -e 's/\.c/.O/g' | \
	pr -3 -l1 -t -w60 > FPfiles
	sed -e 's/O *$$/O \\/' -e '$$s/ \\$$//' < FPfiles > FPfiles.new
	mv FPfiles.new FPfiles

libfiles:
	ls [_a-wz]*.c | sed -e '/^_f_/d' -e 's/\.c/.O/g' | \
 	pr -3 -l1 -t -w60 > Libfiles
	sed -e 's/O *$$/O \\/' -e '$$s/ \\$$//' < Libfiles > Libfiles.new
	mv Libfiles.new Libfiles

# Clean

clean:
	rm -f $(LIBOBJ)
	rm -f $(FPOBJ)
	rm -f _u_*
	rm -f $(StdioLib) $(FPLib)
	rm -f $(Testdir)*.O $(Fptestdir)*.O $(Installdir)*.O
	for d in $(Testdir) $(Fptestdir) $(Installdir) ; do \
	  cd $$d ; \
	  rm -f core *.out ; \
	  for f in *.c ; do \
	    e=`basename $$f .c` ; \
	    rm -f EXE($$e) ; \
	  done ; \
	  cd .. ; \
	done
	rm -f *~ core *.out *order Hid* test.dat
	rm -f *files *.tmp
	rm -f NAME.u* NAME.[01]?

# Site

site:
	-CDIFF stdio.g stdio.h > $(Installdir)SITE.sit
	-if test ! -f site.h ; then \
	  echo site.h not installed ; \
	else \
	  LINES=`wc -l < site.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** site.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "--- site.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "***************" >> $(Installdir)SITE.sit ; \
	  echo "*** 0 ****" >> $(Installdir)SITE.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)SITE.sit ; \
	  sed -e 's/^/+ /' < site.h >> $(Installdir)SITE.sit ; \
	fi
	-if test ! -f Makefile ; then \
	  echo Makefile not installed ; \
	else \
	  LINES=`wc -l < Makefile | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** Makefile	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "--- Makefile	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "***************" >> $(Installdir)SITE.sit ; \
	  echo "*** 0 ****" >> $(Installdir)SITE.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)SITE.sit ; \
	  sed -e 's/^/+ /' < Makefile >> $(Installdir)SITE.sit ; \
	fi
	-if test -s errlist.h ; then \
	  LINES=`wc -l < errlist.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** errlist.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "--- errlist.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "***************" >> $(Installdir)SITE.sit ; \
	  echo "*** 0 ****" >> $(Installdir)SITE.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)SITE.sit ; \
	  sed -e 's/^/+ /' < errlist.h >> $(Installdir)SITE.sit ; \
	fi
	-if test -s powers.h ; then \
	  LINES=`wc -l < powers.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** powers.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "--- powers.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "***************" >> $(Installdir)SITE.sit ; \
	  echo "*** 0 ****" >> $(Installdir)SITE.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)SITE.sit ; \
	  sed -e 's/^/+ /' < powers.h >> $(Installdir)SITE.sit ; \
	fi
	-if test -s lmr.h ; then \
	  LINES=`wc -l < lmr.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** lmr.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "--- lmr.h	$${DATE}" >> $(Installdir)SITE.sit ; \
	  echo "***************" >> $(Installdir)SITE.sit ; \
	  echo "*** 0 ****" >> $(Installdir)SITE.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)SITE.sit ; \
	  sed -e 's/^/+ /' < lmr.h >> $(Installdir)SITE.sit ; \
	fi

# Distribution

distribution:
	ls $(All) | \
	sed -e '/^stdio\.h/d' \
	    -e '/^site\.h/d' \
	    -e '/^errlist\.h/d' \
	    -e '/^powers\.h/d' \
	    -e '/^lmr\.h/d' \
	    -e '/^_u_/d' > files

# Arc

arc:	distribution
	rm -f NAME.arc
	ARC a NAME.arc @files

# Tar

tar NAME.taz:	distribution
	rm -f NAME.taz
	tar cvf - `cat files` | compress -b13 > NAME.taz

# Distribution by tar parts

uu:	distribution
	rm -f NAME.u*
	tar cvf - `cat files` | \
	compress -b13         | \
	uuencode NAME.taz     | \
	split -300 - NAME.u
	for f in NAME.u* ; do \
	  echo '------ Cut Here ------' > uu.tmp ; \
	  cat $$f >> uu.tmp ; \
	  echo '------ Cut Here ------' >> uu.tmp ; \
	  mv uu.tmp $$f ; \
	done

# Shell archive

shar:	distribution
	rm -f NAME.[0-9][0-9]*
	sed -e '/^site\.h/d' \
	    -e '/^stdio\.h/d' \
	    -e 's%.*\.x%&	Assembler stub%' \
	    -e 's%^TEST/.*%&	Exercise file%' \
	    -e 's%^FPTEST/.*%&	Exercise file%' \
	    -e 's%^INSTALL/.*\.c%&	Installation file%' \
	    -e 's%^INSTALL/.*\.sh%&	Installation script%' \
	    -e 's%^INSTALL/.*\.txt%&	Documentation%' \
	    -e 's%^INSTALL/.*\.sit%&	Differences for preconfigured sites%' \
	    -e 's%^INSTALL/.*\.ans%&	Answer file for generating sites%' \
	    -e 's%^makefile.cpp$$%&	Cppmake file%' \
	    -e 's%^makefile.[a-z]*$$%&	Makefile%' \
	    -e 's%^[0-9a-z_]*\.g$$%&	Header file source%' \
	    -e 's%^[0-9a-z_]*\.h$$%&	Header file%' \
	    -e 's%^_z.*\.c$$%&	Stdio fake source code%' \
	    -e 's%^[0-9a-z_]*\.c$$%&	Stdio source code%' < files > MANIFEST
	shar -a -n NAME -C -b13 -c -o NAME -l 40 MANIFEST `cat files`

# Dust and dirt

lint:
	lint -n $(Include) $(Define) `echo $(LIBOBJ) | sed 's/\.o/.c/g'`

# Makefiles

makefiles:
#ifndef	WHOAMI
	-for w in $(Everyone) ; do \
	n=`grep "^.$$w " <CPPMAKEFILE | sed 's/^.[^ ]* //'` ; \
	echo $$n ; \
	cppmake -D$$n -o makefile.$$w ; \
	done
#else
	-for w in $(Everyone) ; do \
	n=`grep "^.$$w " <CPPMAKEFILE | sed 's/^.[^ ]* //'` ; \
	echo $$n ; \
	if test WHOAMI = $$n ; then \
	cppmake -o makefile.$$w ; \
	rm -f Makefile ; \
	cp makefile.$$w Makefile ; \
	else \
	cppmake -U##WHOAMI -D$$n -o makefile.$$w ; \
	fi ; \
	done
#endif

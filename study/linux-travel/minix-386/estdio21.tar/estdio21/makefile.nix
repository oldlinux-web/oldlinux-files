#nix _UNIX
CPP	=__CPP__ __DEFINES__
FPCC	=__FPCC__
CC	=__CC__
FPLD	=__FPLD__
LD	=__LD__
RANLIB	=__RANLIB__
LORDER	=__LORDER__
TSORT	=__TSORT__




Installdir	=INSTALL/
Testdir		=TEST/
Fptestdir	=FPTEST/
Define		=__DEFINES__
Include		=-I.
Libraries	=stdio.a
MathLibraries	=__MATHLIB__
FPLib		=stdiofp.a
StdioLib	=stdio.a

All		=README *.x *.c *.g *.h \
		 makefile.cpp makefile.nix makefile.min \
		 $(Testdir)*.c $(Testdir)*.sh \
		 $(Fptestdir)*.c \
		 $(Installdir)*.c $(Installdir)*.sh \
		 $(Installdir)*.txt $(Installdir)*.sit \
		 $(Installdir)*.ans

Everyone	=nix min dos

CFLAGS= $(Define) $(Include)

FPOBJ= \
_f_cvt.o	    _f_fprintf.o	_f_fscanf.o \
_f_guard.o	    _f_pow10.o		_f_printf.o \
_f_scanf.o	    _f_sprintf.o	_f_sscanf.o \
_f_tvc.o	    _f_vfprintf.o	_f_vprintf.o \
_f_vsprintf.o

LIBOBJ= \
_allocbuf.o	    _bfs.o		_bread.o \
_bwrite.o	    _err.o		_errlist.o \
_fgetlx.o	    _file.o		_fopen.o \
_freebuf.o	    _in.o		_ioread.o \
_iowrite.o	    _ipow10.o		_open3.o \
_os.o		    _out.o		_rename.o \
_rlbf.o		    _stdio.o		_update.o \
_utoa.o		    _vfprintf.o		_vfscanf.o \
_vscanf.o	    _vsscanf.o		_xassert.o \
_z_cvt.o	    _z_tvc.o		_zatexit.o \
_zerr.o		    _zout.o		_zrlbf.o \
_zwrapup.o	    atexit.o		clearerr.o \
ctermid.o	    cuserid.o		exit.o \
fclose.o	    fdopen.o		feof.o \
ferror.o	    fflush.o		fgetc.o \
fgetpos.o	    fgets.o		fileno.o \
fopen.o		    fprintf.o		fputc.o \
fputs.o		    fread.o		freopen.o \
fscanf.o	    fseek.o		fsetpos.o \
ftell.o		    fwrite.o		getc.o \
getchar.o	    gets.o		getw.o \
perror.o	    printf.o		putc.o \
putchar.o	    puts.o		putw.o \
remove.o	    rewind.o		scanf.o \
setbuf.o	    setvbuf.o		sprintf.o \
sscanf.o	    tmpfile.o		tmpnam.o \
ungetc.o	    vfprintf.o		vprintf.o \
vsprintf.o

.SUFFIXES:	.o .c
.c.o:
	$(CC) $(CFLAGS) $<

test:
	-cd $(Testdir) ; \
	for f in *.c ; do \
	  n=`basename $$f .c` ; \
	  $(CC) $(CFLAGS) -I.. $$n.c ; \
	  $(LD) $$n $$n.o ../stdio.a ; \
	done

fptest:
	-cd $(Fptestdir) ; \
	for f in *.c ; do \
	  n=`basename $$f .c` ; \
	  $(FPCC) $(CFLAGS) -I.. $$n.c ; \
	  $(FPLD) $$n $$n.o ../stdiofp.a ../stdio.a $(MathLibraries) ; \
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

Hide:	site.h
	rm -f Hidden
	-if grep -s "define HIDDEN" site.h ; then \
	$(CPP) stub.x >Hidden ; \
	for f in `grep '^.define [a-z]' hidden.h | \
		  sed 's/^.define \([a-z]*\).*/\1/'` ; do \
	echo $${f} ; \
	sed -e "s/XYZ/$${f}/g" < Hidden > _u_$${f}.s ; \
	as -o _u_$${f}.o _u_$${f}.s ; \
	done ; \
	fi
	echo -n >Hide

# Put together a library

FPobjects:	$(FPOBJ)

FPorder:	$(FPOBJ) Hide
	echo -n >FPorder
	-if test -f Hidden ; then \
	ls _u_*print*.o _u_*scan*.o >> FPorder ; \
	fi
	ls $(FPOBJ) > FPorder.tmp
	$(LORDER) `cat FPorder.tmp` | $(TSORT) >> FPorder; \
	rm -f FPorder.tmp FPsort.tmp

FParchive:	FPorder
	rm -f $(FPLib)
	ar cr $(FPLib) `cat FPorder`
	$(RANLIB) stdiofp.a

Liborder:	$(LIBOBJ) Hide
	echo -n >Liborder
	-if test -f Hidden ; then \
	ls _u_*.o >> Liborder ; \
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
	rm -f $(Testdir)*.o $(Fptestdir)*.o $(Installdir)*.o
	for d in $(Testdir) $(Fptestdir) $(Installdir) ; do \
	  cd $$d ; \
	  rm -f core *.out ; \
	  for f in *.c ; do \
	    e=`basename $$f .c` ; \
	    rm -f $$e ; \
	  done ; \
	  cd .. ; \
	done
	rm -f *~ core *.out *order Hid* test.dat
	rm -f *files *.tmp
	rm -f estdio.u* estdio.[01]?

# Site

site:
	-__CDIFF__ stdio.g stdio.h > $(Installdir)__SITE__.sit
	-if test ! -f site.h ; then \
	  echo site.h not installed ; \
	else \
	  LINES=`wc -l < site.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** site.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "--- site.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "***************" >> $(Installdir)__SITE__.sit ; \
	  echo "*** 0 ****" >> $(Installdir)__SITE__.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)__SITE__.sit ; \
	  sed -e 's/^/+ /' < site.h >> $(Installdir)__SITE__.sit ; \
	fi
	-if test ! -f Makefile ; then \
	  echo Makefile not installed ; \
	else \
	  LINES=`wc -l < Makefile | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** Makefile	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "--- Makefile	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "***************" >> $(Installdir)__SITE__.sit ; \
	  echo "*** 0 ****" >> $(Installdir)__SITE__.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)__SITE__.sit ; \
	  sed -e 's/^/+ /' < Makefile >> $(Installdir)__SITE__.sit ; \
	fi
	-if test -s errlist.h ; then \
	  LINES=`wc -l < errlist.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** errlist.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "--- errlist.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "***************" >> $(Installdir)__SITE__.sit ; \
	  echo "*** 0 ****" >> $(Installdir)__SITE__.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)__SITE__.sit ; \
	  sed -e 's/^/+ /' < errlist.h >> $(Installdir)__SITE__.sit ; \
	fi
	-if test -s powers.h ; then \
	  LINES=`wc -l < powers.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** powers.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "--- powers.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "***************" >> $(Installdir)__SITE__.sit ; \
	  echo "*** 0 ****" >> $(Installdir)__SITE__.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)__SITE__.sit ; \
	  sed -e 's/^/+ /' < powers.h >> $(Installdir)__SITE__.sit ; \
	fi
	-if test -s lmr.h ; then \
	  LINES=`wc -l < lmr.h | sed 's/ //g'` ; \
	  DATE=`date` ; \
	  echo "*** lmr.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "--- lmr.h	$${DATE}" >> $(Installdir)__SITE__.sit ; \
	  echo "***************" >> $(Installdir)__SITE__.sit ; \
	  echo "*** 0 ****" >> $(Installdir)__SITE__.sit ; \
	  echo "--- 1,$${LINES} ----" >> $(Installdir)__SITE__.sit ; \
	  sed -e 's/^/+ /' < lmr.h >> $(Installdir)__SITE__.sit ; \
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
	rm -f estdio.arc
	arc a estdio.arc @files

# Tar

tar estdio.taz:	distribution
	rm -f estdio.taz
	tar cvf - `cat files` | compress -b13 > estdio.taz

# Distribution by tar parts

uu:	distribution
	rm -f estdio.u*
	tar cvf - `cat files` | \
	compress -b13         | \
	uuencode estdio.taz     | \
	split -300 - estdio.u
	for f in estdio.u* ; do \
	  echo '------ Cut Here ------' > uu.tmp ; \
	  cat $$f >> uu.tmp ; \
	  echo '------ Cut Here ------' >> uu.tmp ; \
	  mv uu.tmp $$f ; \
	done

# Shell archive

shar:	distribution
	rm -f estdio.[0-9][0-9]*
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
	shar -a -n estdio -C -b13 -c -o estdio -l 40 MANIFEST `cat files`

# Dust and dirt

lint:
	lint -n $(Include) $(Define) `echo $(LIBOBJ) | sed 's/\.o/.c/g'`

# Makefiles

makefiles:
	-for w in $(Everyone) ; do \
	n=`grep "^.$$w " <makefile.cpp | sed 's/^.[^ ]* //'` ; \
	echo $$n ; \
	cppmake -D$$n -o makefile.$$w ; \
	done

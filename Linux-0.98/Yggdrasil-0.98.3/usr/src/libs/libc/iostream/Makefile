# This file was generated automatically by configure.  Do not edit.
VPATH = .
links = 
host_alias = i386-linux
host_cpu = i386
host_vendor = unknown
host_os = linux
target_alias = i386-linux
target_cpu = i386
target_vendor = unknown
target_os = linux
host_makefile_frag = ../config/linux.mh
ALL=all.internal
VERSION=0.81
SUBDIRS = stdio test

srcdir = .

prefix = /usr

exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
libdir = $(exec_prefix)/lib

datadir = $(prefix)/lib
mandir = $(prefix)/man
man1dir = $(mandir)/man1
man2dir = $(mandir)/man2
man3dir = $(mandir)/man3
man4dir = $(mandir)/man4
man5dir = $(mandir)/man5
man6dir = $(mandir)/man6
man7dir = $(mandir)/man7
man8dir = $(mandir)/man8
man9dir = $(mandir)/man9
infodir = $(prefix)/info
includedir = $(prefix)/include
gxx_includedir = $(libdir)/g++-include
docdir = $(datadir)/doc

SHELL = /bin/sh

INSTALL = install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL)

AR = `if [ -f ../../../binutils/ar ] ; \
	then echo ../../../binutils/ar ; \
	else echo ar ; fi`

AR_FLAGS = qc
RANLIB = `if [ -f ../../../binutils/ranlib ] ; \
	then echo ../../../binutils/ranlib ; \
	else echo ranlib ; fi`

MAKEINFO = `if [ -f ../../../texinfo/C/makeinfo ] ; \
	then echo ../../../texinfo/C/makeinfo ; \
	else echo makeinfo ; fi`

CC = gcc

XTRAFLAGS = `if [ -f $${rootprefix}../../gcc/gcc ] ; \
	then echo -I$${rootprefix}../../gcc/include ; \
	else echo ; fi`
WRAP_C_INCLUDES=-I$(srcdir)/../g++-include

#### host and target dependent Makefile fragments come in here.
# Needs some work.

INSTALL = cp
INSTALLDATA = cp
NOSTDINC = -nostdinc++
# Linux puts iostream in libc.a.
STREAM_OBJS =
HAVE_CPLUS_EXTERN = 1
WRAP_C_INCLUDES =
REGEX_OBJ=
# IO_DIR=no-stream might be the right thing for Linux, but you need
# to re-run gendepend in libg++/utils first.  There is also the problem
# that -nostdinc++ won't pick up the iostream include files ...
# IO_DIR=no-stream
G_CONFIG_ARGS = CONFIG_NM="nm -d"
# Don't include iostream files in libg++.a.
IO_OBJECTS_TO_GET = 
##

# List of variables to pass to sub-makes.  This should not be needed
# by GNU make or Sun make (both of which pass command-line variable
# overrides thouh $(MAKE)) but may be needed by older versions.

FLAGS_TO_PASS= \
	"AR=$(AR)" \
	"AR_FLAGS=$(AR_FLAGS)" \
	"CC=$(CC)" \
	"LD=$(LD)" \
	"CFLAGS=$(CFLAGS)" \
	"RANLIB=$(RANLIB)" \
	"LOADLIBES=$(LOADLIBES)" \
	"LDFLAGS=$(LDFLAGS)" \
	"MAKEINFO=$(MAKEINFO)" \
	"LDFLAGS=$(LDFLAGS)" "MV=$(MV)" \
	"XTRAFLAGS=$(XTRAFLAGS)"

.SUFFIXES: .o .C .i .s

.C.o:
	$(CC) $(CFLAGS) $(NOSTDINC) -I. -I.. -I$(srcdir) -I$(srcdir)/../src $(WRAP_C_INCLUDES) -c $(XTRAFLAGS) $<
	$(LD) $(LDFLAGS) $*.o
	$(MV) a.out $*.o

.C.s:
	$(CC) $(CFLAGS) $(NOSTDINC) -I. -I.. -I$(srcdir) -I$(srcdir)/../src $(WRAP_C_INCLUDES) -S $(XTRAFLAGS) $<

.C.i:
	$(CC) $(CFLAGS) $(NOSTDINC) -I. -I.. -I$(srcdir) -I$(srcdir)/../src $(WRAP_C_INCLUDES) -E $(XTRAFLAGS) $< -o $*.i

TARGETLIB=libio.a
TARGETLIB=../libc.a

IOSTREAM_OBS= streambuf.o iostream.o stdstreams.o strstream.o \
  indstream.o PlotFile.o SFile.o fstream.o parsestream.o stream.o makebuf.o \
  editbuf.o filebuf.o sgetline.o igetline.o igetsb.o procbuf.o \
  sbufvform.o sbufvscan.o dtoa.o outfloat.o __flow.o

#  sbufvform.o sbufvscan.o dtoa.o outfloat.o stdstrbufs.o __flow.o

# The following include files are merely to provide partial
# compatibilty with the old stream library from libg++.

COMPAT_INCLUDES = istream.h ostream.h stream.h

# The following include files are publicly visible, and
# should be installed where user programs can find them.

USER_INCLUDES = editbuf.h fstream.h iomanip.h iostream.h indstream.h \
  parsestream.h streambuf.h strstream.h PlotFile.h SFile.h procbuf.h \
  $(COMPAT_INCLUDES)

# The following include files are private to the implementation.

INTERNAL_INCLUDES = ioprivate.h floatio.h

DEPEND_SOURCES = $(srcdir)/*.C

#NOSTDINC = -nostdinc++
CFLAGS = -g

.PHONY: all
all: library
	@rootprefix=`pwd`/; export rootprefix; \
	$(MAKE) subdir_do "DODIRS=$(SUBDIRS)" DO=all $(FLAGS_TO_PASS)

.PHONY: install
install:
.PHONY: check
check:
	@rootprefix=`pwd`/; export rootprefix; \
	$(MAKE) subdir_do "DODIRS=$(SUBDIRS)" DO=check $(FLAGS_TO_PASS)
.PHONY: info
info:
.PHONY: clean-info
clean-info:
.PHONY: install-info
install-info:


.PHONY: boltcc
boltcc:
	$(MAKE) "CC=$(CC)" "XTRAFLAGS=$(XTRAFLAGS)" \
		"AR=$(AR)" "RANLIB=$(RANLIB)" $(TARGETLIB)

# ../libg++.a depends on the stamp file.
library: $(IOSTREAM_OBS)
	$(AR) $(AR_FLAGS) $(TARGETLIB) $(IOSTREAM_OBS)
	$(RANLIB) $(TARGETLIB)
	@touch stamp

DIST_NAME=iostream-${VERSION}
DIST_FILE=$(DIST_NAME).tar.Z
${DIST_FILE}: dist
.PHONY: dist
dist: FORCE
	-mkdir $(DIST_NAME)
	cd $(DIST_NAME); \
	  ln -s $(srcdir)/../Make.defs .; \
	  ln -s $(srcdir)/../g++-include/_G_config.h .; \
	  ln -s $(srcdir)/* \
	    $(srcdir)/../../configure $(srcdir)/../../config.sub .; \
	  rm -rf test stdio *~ CVS configure.in; \
	  sed -e 's|\.\./||' <$(srcdir)/configure.in > configure.in; \
	  mkdir test stdio
	cd $(DIST_NAME)/stdio; \
	  ln -s $(srcdir)/stdio/* .; \
	  rm -rf *~ CVS configure.in; \
	  sed -e 's|\.\./||' <$(srcdir)/stdio/configure.in > configure.in
	cd $(DIST_NAME)/test; \
	  ln -s $(srcdir)/test/* .; \
	  rm -rf *~ CVS configure.in; \
	  sed -e 's|\.\./||' <$(srcdir)/test/configure.in > configure.in
	tar -c -h -f - $(DIST_NAME) | compress -c >$(DIST_NAME).tar.Z
	rm -rf $(DIST_NAME)

# clean rules

.PHONY: mostlyclean clean distclean realclean
.PHONY: do_mostlyclean do_clean do_distclean do_realclean
do_mostlyclean:
	rm -f *.o core *.tar.Z stamp
do_clean: do_mostlyclean
	rm -f *.a
do_distclean: do_clean
	rm -f *~ *.a config.status *.info Makefile
do_realclean: do_distclean
	rm -f depend

mostlyclean: do_mostlyclean
	@$(MAKE) subdir_do "DODIRS=$(SUBDIRS)" DO=mostlyclean
clean: do_clean
	@$(MAKE) subdir_do "DODIRS=$(SUBDIRS)" DO=clean
distclean: do_distclean
	@$(MAKE) subdir_do "DODIRS=$(SUBDIRS)" DO=distclean
realclean: do_realclean
	@$(MAKE) subdir_do "DODIRS=$(SUBDIRS)" DO=realclean

iostream.info: $(srcdir)/iostream.texi
	$(MAKEINFO) -o iostream.info $(srcdir)/iostream.texi

.PHONY: install-include-files
install-include-files:
	[ -d $(gxx_includedir) ] || mkdir $(gxx_includedir)
	cd $(srcdir); \
	for FILE in $(USER_INCLUDES) ; do \
		rm -f $(gxx_includedir)/$$FILE ; \
		$(INSTALL_DATA) $$FILE $(gxx_includedir)/$$FILE ; \
	done

.PHONY: subdir_do
subdir_do: force
	@rootprefix=`pwd`/; export rootprefix; \
	for i in $(DODIRS); do \
		if [ -d ./$$i ] ; then \
			echo "cd $$i; make $(DO) ..." ; \
			(cd $$i ; $(MAKE) $(DO) $(FLAGS_TO_PASS)) || exit 1 ; \
		else true ; fi ; \
	done

.PHONY: force
force:

# with the gnu make, this is done automatically.

Makefile: $(srcdir)/Makefile.in $(host_makefile_frag) $(target_makefile_frag)
	$(SHELL) ./config.status

.NOEXPORT:


# DO NOT DELETE THIS LINE -- g++dep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

PlotFile.o : PlotFile.C \
  PlotFile.h \
  fstream.h \
  iostream.h \
  streambuf.h 
SFile.o : SFile.C \
  SFile.h \
  fstream.h \
  iostream.h \
  streambuf.h 
editbuf.o : editbuf.C \
  ioprivate.h \
  streambuf.h \
  editbuf.h \
  fstream.h \
  iostream.h \
  streambuf.h
filebuf.o : filebuf.C \
  ioprivate.h \
  streambuf.h 
fstream.o : fstream.C \
  ioprivate.h \
  streambuf.h \
  fstream.h \
  iostream.h \
  streambuf.h
igetline.o : igetline.C \
  iostream.h \
  streambuf.h 
igetsb.o : igetsb.C \
  ioprivate.h \
  streambuf.h \
  iostream.h \
  streambuf.h
indstream.o : indstream.C \
  indstream.h \
  iostream.h \
  streambuf.h 
iostream.o : iostream.C \
  ioprivate.h \
  streambuf.h \
  iostream.h \
  streambuf.h \
  floatio.h
makebuf.o : makebuf.C \
  ioprivate.h \
  streambuf.h \
  fstream.h \
  iostream.h \
  streambuf.h 
parsestream.o : parsestream.C \
  ioprivate.h \
  streambuf.h \
  parsestream.h
procbuf.o : procbuf.C \
  ioprivate.h \
  streambuf.h \
  procbuf.h \
  streambuf.h 
sbufvform.o : sbufvform.C \
  ioprivate.h \
  streambuf.h \
  floatio.h
sbufvscan.o : sbufvscan.C \
  ioprivate.h \
  streambuf.h \
  floatio.h
sgetline.o : sgetline.C \
  ioprivate.h \
  streambuf.h 
stdstrbufs.o : stdstrbufs.C \
  ioprivate.h \
  streambuf.h 
stdstreams.o : stdstreams.C \
  ioprivate.h \
  streambuf.h \
  iostream.h \
  streambuf.h
stream.o : stream.C \
  ioprivate.h \
  streambuf.h \
  stream.h \
  iostream.h \
  streambuf.h \
  strstream.h
streambuf.o : streambuf.C \
  ioprivate.h \
  streambuf.h 
strstream.o : strstream.C \
  ioprivate.h \
  streambuf.h \
  strstream.h \
  iostream.h \
  streambuf.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

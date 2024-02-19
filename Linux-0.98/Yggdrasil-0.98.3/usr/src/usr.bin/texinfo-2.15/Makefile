  ####	 Makefile for the version 2 Texinfo package -*- text -*-    ####

# Robert J. Chassell
# Bugs to bob@ai.mit.edu.  I maintain the Emacs Lisp files and the
# manual, and I forward bug reports for info and makeinfo to 
# bfox@gnu.ai.mit.edu, who maintains those utilities.

# A `make dist' command creates a compressed tar file of version 2 of
# the Texinfo package, including both the Texinfo and the `makeinfo'
# files.  The file is called `texinfo-2.15.tar.Z' (or some similar
# name, defined with the 'TEXINFO2_DIRECTORY' variable).
#
# This Makefile presumes that the Texinfo documentation and
# miscellaneous files are in the current directory, the Emacs lisp code
# is in a `elisp' subdirectory and the `makeinfo' files are in a `C'
# subdirectory.

# Use `tar xvzf texinfo-2.15.tar.Z' to uncompress and extract the contents
# of `texinfo-2.15.tar.Z'.

# The contents of `texinfo-2.15.tar.Z' expand to a megabyte or so.

# Before mailing texinfo-2.15.tar.Z, if you should mail it,
# uuencode the file with the following command:
# uuencode texinfo-2.15.tar.Z texinfo-2.15.tar.Z > texinfo-2.15.tar.Z.uu

#### ==> Specify the following as appropriate: texinfo-2.15, or whatever
TEXINFO2_DIRECTORY = texinfo-2.15

TEXINFO_DOCUMENTATION = texinfo2.texi

TEXINFO_SUPPORT = README COPYING ChangeLog Makefile New-features fixfonts

TEXINFO_ELISP_SOURCES = elisp/texinfo.el \
		elisp/texnfo-upd.el \
		elisp/texinfmt.el \
		elisp/texnfo-tex.el \
		elisp/makeinfo.el 

#### include `texi2roff' when it becomes available ####
OTHER_TEXINFO_SOURCES = texi2dvi texinfo.tex

INFO_ELISP_SOURCES = elisp/info.el elisp/informat.el

MAKEINFO_SOURCES = C/makeinfo.c C/info.c C/texindex.c \
		C/getopt.c C/getopt1.c

MAKEINFO_HEADERS = C/getopt.h

MAKEINFO_SUPPORT = C/Makefile.in C/configure C/configure.in C/ChangeLog

# This Makefile creates subdirectories and links the sources into it.
# When the contents of the tar file are extracted, the files extract
# into the a subdirectory called `texinfo2' containing the
# documentation and miscellaneous files, and subsubdirectories, called
# `elisp' and `C', containing the software.

INSTALL = install -c
# These permit overriding just for certain files.
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL)

MAKE = make

# Common prefix for installation directories.
# NOTE: This directory must exist when you start installation.
prefix = /usr/TeX
# Directory in which to put the executable for the commands.
bindir = ${prefix}/bin
# Directory in which to put the directories used by auxiliary programs.
libdir = ${prefix}/lib/tex

all:
	cd C; $(MAKE) all

info:
	cd C; $(MAKE) info

makeinfo:
	cd C; $(MAKE) makeinfo

texindex:
	cd C; $(MAKE) texindex

TAGS:
	cd C; $(MAKE) TAGS

clean:
	cd C; $(MAKE) clean

distclean:
	cd C; $(MAKE) distclean

realclean:
	cd C; $(MAKE) realclean

install:
	$(INSTALL_PROGRAM) texi2dvi $(bindir)/texi2dvi
	$(INSTALL_DATA) texinfo.tex $(libdir)/inputs/texinfo.tex
# Make sure font lcircle10 can be used for formatting and printing.
	./fixfonts
	cd C; $(MAKE) install

dist: $(TEXINFO_DOCUMENTATION) $(TEXINFO_ELISP_SOURCES) \
		$(OTHER_TEXINFO_SOURCES) \
		$(TEXINFO_SUPPORT) \
		$(INFO_ELISP_SOURCES) \
		$(MAKEINFO_SOURCES) $(MAKEINFO_HEADERS) $(MAKEINFO_SUPPORT)
	rm -fr $(TEXINFO2_DIRECTORY)
	mkdir $(TEXINFO2_DIRECTORY) $(TEXINFO2_DIRECTORY)/elisp \
		$(TEXINFO2_DIRECTORY)/C
	ln $(TEXINFO_DOCUMENTATION) $(TEXINFO_SUPPORT) \
		$(OTHER_TEXINFO_SOURCES) \
		$(TEXINFO2_DIRECTORY)
	ln $(TEXINFO_ELISP_SOURCES) $(INFO_ELISP_SOURCES) \
		$(TEXINFO2_DIRECTORY)/elisp
	ln $(MAKEINFO_SOURCES) $(MAKEINFO_HEADERS) \
		$(MAKEINFO_SUPPORT) \
		$(TEXINFO2_DIRECTORY)/C
	tar chvzf $(TEXINFO2_DIRECTORY).tar.Z $(TEXINFO2_DIRECTORY)
	-rm -r $(TEXINFO2_DIRECTORY)

######################################################################

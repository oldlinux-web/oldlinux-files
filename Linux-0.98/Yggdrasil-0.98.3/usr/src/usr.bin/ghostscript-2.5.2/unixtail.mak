#    Copyright (C) 1990, 1992 Aladdin Enterprises.  All rights reserved.
#    Distributed by Free Software Foundation, Inc.
#
# This file is part of Ghostscript.
#
# Ghostscript is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
# to anyone for the consequences of using it or for whether it serves any
# particular purpose or works at all, unless he says so in writing.  Refer
# to the Ghostscript General Public License for full details.
#
# Everyone is granted permission to copy, modify and redistribute
# Ghostscript, but only under the conditions described in the Ghostscript
# General Public License.  A copy of this license is supposed to have been
# given to you along with Ghostscript so you can know your rights and
# responsibilities.  It should be in a file named COPYING.  Among other
# things, the copyright notice and this notice must be preserved on all
# copies.

# Partial makefile for Ghostscript, common to all Unix configurations.

# This is the last part of the makefile for Unix configurations.
# Since Unix make doesn't have an 'include' facility, we concatenate
# the various parts of the makefile together by brute force (in tar_cat).

# The following prevents GNU make from constructing argument lists that
# include all environment variables, which can easily be longer than
# brain-damaged system V allows.

.NOEXPORT:

# -------------------------------- Library -------------------------------- #

## The Unix platforms

# We have to include a test for the existence of sys/time.h,
# because some System V platforms don't have it.

# All reasonable Unix platforms.
unix__=gp_unix.$(OBJ)
unix_.dev: $(unix__)
	$(SHP)gssetmod unix_ $(unix__)

gp_unix.$(OBJ): gp_unix.c $(AK) $(memory__h) $(string__h) $(gx_h) $(gp_h) \
 $(stat__h) $(time__h)
	if ( test -f /usr/include/sys/time.h ) then $(CCC) gp_unix.c;\
	else $(CCC) -DNOSYSTIME gp_unix.c; fi

# Brain-damaged System V platforms.
sysv__=gp_unix.$(OBJ) gp_sysv.$(OBJ)
sysv_.dev: $(sysv__)
	$(SHP)gssetmod sysv_ $(sysv__)

gp_sysv.$(OBJ): gp_sysv.c $(time__h) $(AK)
	if ( test -f /usr/include/sys/time.h ) then $(CCC) gp_sysv.c;\
	else $(CCC) -DNOSYSTIME gp_sysv.c; fi

# -------------------------- Auxiliary programs --------------------------- #

ansi2knr$(XE): ansi2knr.c $(stdio__h) $(string__h) $(malloc__h)
	$(CC) -o ansi2knr$(XE) $(CFLAGS) ansi2knr.c

echogs$(XE): echogs.c
	$(CC) -o echogs$(XE) $(CFLAGS) echogs.c

# On the RS/6000 (at least), compiling genarch.c with gcc with -O
# produces a buggy executable.
genarch$(XE): genarch.c
	$(CC) -o genarch$(XE) genarch.c

# ----------------------------- Main program ------------------------------ #

BEGINFILES=
CCBEGIN=$(CCC) *.c

# Main program

ALLUNIX=gsmain.$(OBJ) $(LIB)

# Interpreter main program

GSUNIX=gs.$(OBJ) $(INT) $(ALLUNIX)

# The second call on echogs writes a \.  This is the only
# way to do it that works with all flavors of shell!
gs: $(GSUNIX) obj.tr lib.tr echogs
	./echogs -n - $(CC) $(LDFLAGS) $(XLIBDIRS) -o gs $(GSUNIX) >_temp_
	./echogs -x 205c >>_temp_
	cat obj.tr >>_temp_
	cat lib.tr >>_temp_
	echo $(EXTRALIBS) -lm >>_temp_
	$(SH) <_temp_

# Installation

TAGS:
	etags -t *.c *.h

docdir=$(gsdatadir)/doc
exdir=$(gsdatadir)/examples
sysdir=$(gsdatadir)/system

install: gs
	-mkdir $(bindir)
	$(INSTALL_PROGRAM) gs $(bindir)
	$(INSTALL_PROGRAM) gsbj $(bindir)
	$(INSTALL_PROGRAM) gsdj $(bindir)
	$(INSTALL_PROGRAM) gslj $(bindir)
	$(INSTALL_PROGRAM) gslp $(bindir)
	$(INSTALL_PROGRAM) gsnd $(bindir)
	$(INSTALL_PROGRAM) bdftops $(bindir)
	$(INSTALL_PROGRAM) font2c $(bindir)
	-mkdir $(gsdatadir)
	$(INSTALL_DATA) README $(gsdatadir)
	$(INSTALL_DATA) gslp.ps $(gsdatadir)
	$(INSTALL_DATA) gs_init.ps $(gsdatadir)
	$(INSTALL_DATA) gs_2asc.ps $(gsdatadir)
	$(INSTALL_DATA) gs_dps1.ps $(gsdatadir)
	$(INSTALL_DATA) gs_fonts.ps $(gsdatadir)
	$(INSTALL_DATA) gs_lev2.ps $(gsdatadir)
	$(INSTALL_DATA) gs_statd.ps $(gsdatadir)
	$(INSTALL_DATA) sym__enc.ps $(gsdatadir)
	$(INSTALL_DATA) quit.ps $(gsdatadir)
	$(INSTALL_DATA) Fontmap $(gsdatadir)
	$(INSTALL_DATA) uglyr.gsf $(gsdatadir)
	$(INSTALL_DATA) bdftops.ps $(gsdatadir)
	$(INSTALL_DATA) decrypt.ps $(gsdatadir)
	$(INSTALL_DATA) font2c.ps $(gsdatadir)
	$(INSTALL_DATA) impath.ps $(gsdatadir)
	$(INSTALL_DATA) landscap.ps $(gsdatadir)
	$(INSTALL_DATA) prfont.ps $(gsdatadir)
	$(INSTALL_DATA) pstoppm.ps $(gsdatadir)
	$(INSTALL_DATA) type1ops.ps $(gsdatadir)
	$(INSTALL_DATA) wrfont.ps $(gsdatadir)
	-mkdir $(docdir)
	$(INSTALL_DATA) NEWS $(docdir)
	$(INSTALL_DATA) history.doc $(docdir)
	$(INSTALL_DATA) drivers.doc $(docdir)
	$(INSTALL_DATA) fonts.doc $(docdir)
	$(INSTALL_DATA) hershey.doc $(docdir)
	$(INSTALL_DATA) humor.doc $(docdir)
	$(INSTALL_DATA) language.doc $(docdir)
	$(INSTALL_DATA) lib.doc $(docdir)
	$(INSTALL_DATA) make.doc $(docdir)
	$(INSTALL_DATA) psfiles.doc $(docdir)
	$(INSTALL_DATA) readme.doc $(docdir)
	$(INSTALL_DATA) use.doc $(docdir)
	-mkdir $(exdir)
	$(INSTALL_DATA) chess.ps $(exdir)
	$(INSTALL_DATA) cheq.ps $(exdir)
	$(INSTALL_DATA) colorcir.ps $(exdir)
	$(INSTALL_DATA) golfer.ps $(exdir)
	$(INSTALL_DATA) escher.ps $(exdir)
	$(INSTALL_DATA) snowflak.ps $(exdir)
	$(INSTALL_DATA) tiger.ps $(exdir)

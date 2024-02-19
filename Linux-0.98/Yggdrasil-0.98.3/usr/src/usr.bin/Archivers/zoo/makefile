# derived from: @(#) makefile 2.2 88/01/27 19:37:59 
# $Id: makefile,v 1.22 91/07/09 04:10:38 dhesi Exp $
# Make Zoo
#
#The contents of this makefile are hereby released to the public domain.
#                                  -- Rahul Dhesi 1991/07/05
#
# This makefile expects two macro names, `CFLAGS' and `EXTRA', to hold
# all the switches to be supplied to the C compiler.  It also expects
# a macro `LDFLAGS' to hold the switch for the loader when invoked.
# The macro "MODEL" holds switches needed for both compile and link, 
# such as "memory model" for Intel and Z8000 processors. OPTIM is the 
# optimize option and may be set on the make command line to -O2 or 
# whatever your compiler thinks is nice.
#
# To run lint, select an appropriate lint_* target (e.g. "make lint_sysv").


MAKE = make	      # needed for some systems e.g. older BSD

CC = cc
CFLAGS =
MODEL =
EXTRA = -DBIG_MEM -DNDEBUG
LINTFLAGS = -DLINT
OPTIM = -O
DESTDIR = /usr/local/bin

#List of all object files created for Zoo
ZOOOBJS = addbfcrc.o addfname.o basename.o comment.o crcdefs.o \
		getfile.o lzc.o lzd.o machine.o makelist.o misc.o misc2.o \
		nextfile.o needed.o options.o parse.o portable.o prterror.o \
		version.o zoo.o zooadd.o zooadd2.o zoodel.o zooext.o zoofilt.o \
		zoolist.o zoopack.o io.o lzh.o maketbl.o maketree.o huf.o \
		encode.o decode.o

FIZOBJS = fiz.o addbfcrc.o portable.o crcdefs.o

.c.o :
	$(CC) $(CFLAGS) $(MODEL) $(EXTRA) $*.c

all : 
	@echo 'There is no default.  Please specify an appropriate target from'
	@echo 'the makefile, or type "make help" for more information'

help :
	@echo "Possible targets are as follows.  Please examine the makefile"
	@echo "for more information."
	@echo ' '
	@echo "generic:      generic **IX environment, minimal functionlity"
	@echo "bsd:          4.3BSD or reasonable equivalent"
	@echo "bsdansi:      4.3BSD with ANSI C"
	@echo "ultrix:       ULTRIX 4.1"
	@echo "convex:       Convex C200 series"
	@echo "linux:        Linux 0.96cPL2, gcc 2.2.2"
	@echo "sysv:         System V Release 2 or 3; or SCO Xenix"
	@echo "scodos:       Cross-compiler under SCO Xenix/UNIX for MS-DOS"
	@echo "xenix286:     Older Xenix/286 (not tested)"
	@echo "xenix68k:     Xenix/68000 (not tested)"
	@echo ' '
	@echo "install:      Move zoo to $(DESTDIR)/tzoo (alpha test)"
	@echo "inst_beta:    Move zoo to $(DESTDIR)/bzoo (beta test)"
	@echo "inst_prod:    Move zoo to $(DESTDIR)/zoo  (production)"
	@echo ' '
	@echo "lint_sysv:    Run lint for System V"
	@echo "lint_bsd:     Run lint for 4.3BSD"
	@echo "lint_generic: Run lint for generic **IX"
	@echo "lint_turboc:  Run lint under **IX for checking Turbo C/MSDOS code"

# install alpha zoo as "tzoo"
install:
	mv zoo $(DESTDIR)/tzoo

# install beta zoo as "bzoo"
inst_beta:
	mv zoo $(DESTDIR)/bzoo

# install production zoo as "zoo"
inst_prod:
	mv zoo $(DESTDIR)/zoo

# executable targets
TARGETS = zoo fiz

#######################################################################
# SYSTEM-SPECIFIC TARGETS
#######################################################################

# A generic system -- may have less than full functionality.
# Compile with -g, since debugging will probably be needed.
generic:
	$(MAKE) CFLAGS="-c -g -DGENERIC" $(TARGETS)

# Reasonably generic BSD 4.3
bsd:
	$(MAKE) CFLAGS="-c $(OPTIM) -DBSD4_3" $(TARGETS)

# ULTRIX 4.1
ultrix:
	$(MAKE) CFLAGS="-c $(OPTIM) -DULTRIX" $(TARGETS)

# BSD with ANSI C - works on MIPS and Ultrix/RISC compilers
bsdansi:
	$(MAKE) CFLAGS="-c $(OPTIM) -DBSD4_3 -DANSI_HDRS" $(TARGETS)

# Convex C200 series
convex:
	$(MAKE) CFLAGS="-c $(OPTIM) -DBSD4_3 -DANSI_HDRS" $(TARGETS)

# Linux 0.96c, gcc 2.2.2
linux:
	$(MAKE) \
	CFLAGS="-c -O2 -DBSD4_3 -DANSI_HDRS -DLINUX" \
	$(TARGETS)

# linux as above, static linked
linux_s:
	$(MAKE) \
	CFLAGS="-c -O2 -DBSD4_3 -DANSI_HDRS -DLINUX" \
	LDFLAGS="-static" $(TARGETS)

# SysV.2, V.3, SCO Xenix
sysv:
	$(MAKE) CFLAGS="-c $(OPTIM) -DSYS_V" $(TARGETS)

# DOS version cross compiled from SCO Xenix/UNIX
scodos:
	$(MAKE) CFLAGS="-c $(OPTIM) -DTURBOC -DANSI_HDRS -DBIG_MEM" \
		EXTRA="-dos -Ml" LDFLAGS="-o zoo.exe" $(TARGETS)

# Tested for zoo 2.01 on: Xenix 3.4 on Greg Laskin's Intel 310/286;
# SCO Xenix 2.2 on Robert Cliff's AT.
# `-Ml' for large memory model, `-M2' to generate code for 80286 cpu,
# `-F xxxx' for xxxx (hex) bytes of stack space.
xenix286:
	@echo "Warning: xenix286 is not fully functional"
	$(MAKE) CFLAGS="-c $(OPTIM) -DSYS_V" \
		MODEL="-Ml -M2 -Md" \
		LDFLAGS="-s -n -Md -Mt500 -F 5000" $(TARGETS)

# Radio Shack Model 16 with Xenix/68000 3.01.01. "-DM_VOID" tells not 
# to try to do a typedef of `void'. "-Dvoid=int" because compiler doesn't 
# know about `void'.  `-s -n' strips and makes it shareable.  Used to work
# with zoo 2.01; not tested with 2.1.
xenix68k:
	$(MAKE) CFLAGS="-c $(OPTIM) -DSYS_V -DM_VOID -Dvoid=int" \
		LDFLAGS="-s -n" $(TARGETS)

#######################################################################
# CLEANUP TARGETS
#######################################################################

# standard clean -- remove all transient files
clean :
	rm -f core a.out $(ZOOOBJS) $(FIZOBJS)

# object clean only -- just remove object files
objclean:
	rm -f *.o

#######################################################################
# BINARY TARGETS
#######################################################################

zoo: $(ZOOOBJS)
	$(CC) -o zoo $(MODEL) $(LDFLAGS) $(ZOOOBJS)

fiz: $(FIZOBJS)
	$(CC) -o fiz $(MODEL) $(LDFLAGS) $(FIZOBJS)

#######################################################################
# SELECTED TARGETS FOR LINT
#######################################################################

# generic system V
lint_sysv:
	echo $(ZOOOBJS) | sed -e 's/\.o/.c/g' | \
	xargs lint -DSYS_V $(EXTRA) $(LINTFLAGS) | \
	grep -v 'possible pointer alignment problem'

# generic BSD
lint_bsd:
	echo $(ZOOOBJS) | sed -e 's/\.o/.c/g' | \
	xargs lint -DBSD4_3 $(EXTRA) $(LINTFLAGS) | \
	grep -v 'possible pointer alignment problem'

# generic **IX
lint_generic:
	echo $(ZOOOBJS) | sed -e 's/\.o/.c/g' | \
	xargs lint -DGENERIC $(EXTRA) $(LINTFLAGS) | \
	grep -v 'possible pointer alignment problem'

# Cross-lint for checking Turbo C code under **IX.  For checking only;
# compilation requires separate makefile called "makefile.tcc"
lint_turboc:
	echo $(ZOOOBJS) turboc.c | sed -e 's/\.o/.c/g' | \
	xargs lint -DTURBOC -DCROSS_LINT $(EXTRA) $(LINTFLAGS)

#######################################################################
# DEPENDENCIES
#######################################################################
# DO NOT DELETE THIS LINE -- it marks the beginning of this dependency list

addbfcrc.o: options.h
addfname.o: zooio.h
addfname.o: zoomem.h
basename.o: various.h
basename.o: zoo.h zoofns.h zooio.h
bsd.o: /usr/include/sys/time.h
bsd.o: nixmode.i nixtime.i
comment.o: /usr/include/stdio.h
comment.o: various.h
comment.o: zoo.h zoofns.h zooio.h
crcdefs.o: options.h
decode.o: zoo.h
encode.o: errors.i lzh.h
encode.o: options.h zoo.h
fiz.o: zoofns.h
fiz.o: zooio.h
generic.o: /usr/include/sys/types.h
generic.o: nixmode.i nixtime.i
getfile.o: zooio.h
getfile.o: zoomem.h
huf.o: zoo.h
io.o: zoo.h
io.o: zooio.h
lzc.o: various.h
lzc.o: zoo.h zoofns.h zooio.h zoomem.h
lzd.o: various.h
lzd.o: zoo.h zoofns.h zooio.h zoomem.h
lzh.o: zoo.h
machine.o: zooio.h
makelist.o: errors.i options.h
makelist.o: portable.h various.h zoo.h zoofns.h zooio.h
maketbl.o: zoo.h
maketree.o: zoo.h
misc.o: /usr/include/sys/signal.h
misc.o: errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
misc2.o: zoo.h
misc2.o: zoofns.h zooio.h zoomem.h
msdos.o: zooio.h
needed.o: zoo.h
needed.o: zoofns.h zooio.h
nextfile.o: zoo.h
options.o: zoofns.h
options.o: zooio.h
parse.o: zoo.h
parse.o: zoofns.h zooio.h
portable.o: options.h
portable.o: portable.h various.h zoo.h zoofns.h zooio.h
prterror.o: options.h various.h
prterror.o: zoofns.h zooio.h
sysv.o: /usr/include/time.h
sysv.o: nixmode.i nixtime.i
turboc.o: /usr/include/sys/signal.h
vms.o: /usr/include/time.h
vmstime.o: /usr/include/stdio.h
zoo.o: zoofns.h
zoo.o: zooio.h zoomem.h
zooadd.o: portable.h
zooadd.o: various.h zoo.h zoofns.h zooio.h zoomem.h
zooadd2.o: parse.h
zooadd2.o: various.h zoo.h zoofns.h zooio.h
zoodel.o: /usr/include/sys/signal.h
zoodel.o: errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
zooext.o: /usr/include/sys/signal.h
zooext.o: errors.i machine.h options.h parse.h portable.h various.h zoo.h
zooext.o: zoofns.h zooio.h
zoofilt.o: options.h
zoolist.o: zoo.h
zoolist.o: zoofns.h zooio.h zoomem.h
zoopack.o: /usr/include/stdio.h
zoopack.o: various.h
zoopack.o: zoo.h zoofns.h zooio.h

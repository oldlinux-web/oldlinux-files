# ===========================================================================
# Makefile for UnZip, ZipInfo & Ship:  Unix, OS/2 and MS-DOS (MSC NMAKE only)
# Version:  decrypt + (conditionally) inflate
# ===========================================================================
#
#
# INSTRUCTIONS (such as they are):
#
# "make vax"	-- makes UnZip on a VAX 11-780 BSD 4.3 in current directory
#		   (or a SysV VAX, or an 8600 running Ultrix, or...)
# "make"	-- uses environment variable SYSTEM to set the type
#		   system to compile for.  This doesn't work for some
#		   particularly brain-damaged versions of make (VAX BSD,
#		   Gould, and SCO Unix are in this group).  If SYSTEM not
#		   set, gives instructions on what to try instead.
# "make list"	-- lists all supported systems (targets), including ship
#		   and zipinfo targets
# "make wombat" -- Chokes and dies if you haven't added the specifics
#		   for your Wombat 68000 (or whatever) to the systems list.
#
# CFLAGS are flags for the C compiler.  LDFLAGS are flags for the loader.
# LDFLAGS2 are more flags for the loader, if they need to be at the end of
# the line instead of at the beginning.
#
# My host (a VAX 11-780 running BSD 4.3) is hereafter referred to as "my host."
#
# My host's /usr/include/sys/param.h defines BSD for me.  You may have to add
# "-DBSD" to the list of CFLAGS for your system.
#
# Some versions of make do not define the macro "$(MAKE)" (my host did not).
# The makefile should now handle such systems correctly, more or less; the
# possible exception to this is if you've used a make command-line option
# (for example, the one which displays the commands which WOULD be executed,
# but doesn't actually execute them).  It probably needs some more tinkering.
# If things still don't work, use "make" instead of "$(MAKE)" in your system's
# makerule.  Or try adding the following line to your .login file:
#   setenv MAKE "make"
# (It didn't help on my host.)
#
# memcpy and memset are provided for those systems that don't have them;
# they're found in misc.c and will be used if -DZMEM is included in the list
# of CFLAGS.  These days ALMOST all systems have them (they're mandated by
# ANSI), but older systems might be lacking.  And at least ONE machine's
# version results in some serious performance degradation...
#
# SCO Unix 3.2.0:  Don't use -Ox with cc (derived from Microsoft 5.1); there
# is a bug in the loop optimization which causes bad CRC's.  [Onno van der
# Linden]
#
# Be sure to test your nice new UnZip; successful compilation does not always
# imply a working program.


#####################
# MACRO DEFINITIONS #
#####################

# Defaults most systems use (use LOCAL_UNZIP in environment to add flags).
# To add inflation:  uncomment INFL_OBJ below or add it to your environment
# as appropriate, and add -DINFLATE to CFLAGS or to LOCAL_UNZIP.  (This
# won't work if you don't have inflate.c, so don't be a goober...)

CC = cc
CR = -DCRYPT
CFLAGS = -O -DUNIX $(CR) $(LOCAL_UNZIP)
ZC = -DZMEM
LD = cc
LDFLAGS = -o unzip
LDFLAGS2 = -s
ZL = -o zipinfo
ZL2 = -s
MV = mv
EXE =
O = .o
#INFL_OBJ = inflate.o  -or-  inflate.obj  (pick one)
OBJS = unzip$O crypt$O extract$O file_io$O $(INFL_OBJ)\
   mapname$O match$O misc$O unimplod$O unreduce$O unshrink$O
OS2_OBJS = unzip.obj crypt.obj dosname.obj extract.obj file_io.obj $(INFL_OBJ)\
   mapname.obj match.obj misc.obj unimplod.obj unreduce.obj unshrink.obj
ZI_OBJS = zipinfo$O misc_$O match$O

SHELL = /bin/sh

# list of supported systems in this version
SYSTEMS1 = 386i 3Bx 7300 amdahl apollo aviion bsd bull coherent convex
SYSTEMS2 = cray cray_cc cyber_sgi dec dnix encore eta gcc_os2 generic
SYSTEMS3 = generic2 gould hk68 hp icc_os2 minix mips msc_dos msc_os2 next
SYSTEMS4 = p_iris pyramid rs6000 rtaix sco sco_dos sco_x286 sequent sgi
SYSTEMS5 = stellar sun sysv tahoe ultrix vax wombat

SYS_UTIL1 = ship ship_dos ship_gcc ship_icc ship_os2 ship_sysv zi_dos
SYS_UTIL2 = zi_gcc zi_icc zi_os2 zipinfo 

####################
# DEFAULT HANDLING #
####################

# The below will try to use your shell variable "SYSTEM" as the type system
# to use (e.g., if you type "make" with no parameters at the command line).
# The test for $(MAKE) is necessary for VAX BSD make (and Gould, apparently),
# as is the "goober" (else stupid makes see an "else ;" statement, which they
# don't like).  "goober" must then be made into a valid target for machines
# which DO define MAKE properly (and have SYSTEM set).  Quel kluge, non?
# And to top it all off, it appears that the VAX, at least, can't pick SYSTEM
# out of the environment either (which, I suppose, should not be surprising).
# [Btw, if the empty "goober" target causes someone else's make to barf, just
# add an "@echo > /dev/null" command (or whatever).  Works OK on the Amdahl
# and Crays, though.]

default:
	@if test -z "$(MAKE)"; then\
		if test -z "$(SYSTEM)";\
		then make ERROR;\
		else make $(SYSTEM) MAKE="make";\
		fi;\
	else\
		if test -z "$(SYSTEM)";\
		then $(MAKE) ERROR;\
		else $(MAKE) $(SYSTEM) goober;\
		fi;\
	fi

goober:

ERROR:
	@echo
	@echo\
 "  If you're not sure about the characteristics of your system, try typing"
	@echo\
 '  "make generic".  If the compiler barfs and says something unpleasant about'
	@echo\
 '  "timezone redefined," try typing "make clean" followed by "make generic2".'
	@echo\
 '  One of these actions should produce a working copy of unzip on most Unix'
	@echo\
 '  systems.  If you know a bit more about the machine on which you work, you'
	@echo\
 '  might try "make list" for a list of the specific systems supported herein.'
	@echo\
 '  And as a last resort, feel free to read the numerous comments within the'
	@echo\
 '  Makefile itself.  Have an excruciatingly pleasant day.'
	@echo

list:
	@echo
	@echo\
 'Type "make <system>", where <system> is one of the following:'
	@echo
	@echo  "	$(SYSTEMS1)"
	@echo  "	$(SYSTEMS2)"
	@echo  "	$(SYSTEMS3)"
	@echo  "	$(SYSTEMS4)"
	@echo  "	$(SYSTEMS5)"
	@echo
	@echo\
 'Otherwise set the shell variable SYSTEM to one of these and just type "make".'
	@echo\
 'Targets for related utilities (ZipInfo and Ship) include:'
	@echo
	@echo  "	$(SYS_UTIL1)"
	@echo  "	$(SYS_UTIL2)"
	@echo
	@echo\
 'For further (very useful) information, please read the comments in Makefile.'
	@echo


###############################################
# BASIC COMPILE INSTRUCTIONS AND DEPENDENCIES #
###############################################

.c$O :
	$(CC) -c $(CFLAGS) $*.c

unzip$(EXE):	$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LDFLAGS2)

crypt$O:        crypt.c unzip.h zip.h	# may or may not be in distribution
dosname.obj:    dosname.c		# for OS/2 only
extract$O:      extract.c unzip.h
file_io$O:      file_io.c unzip.h
inflate$O:      inflate.c unzip.h	# may or may not be in distribution
mapname$O:      mapname.c unzip.h
match$O:        match.c unzip.h
misc$O:         misc.c unzip.h
unimplod$O:     unimplod.c unzip.h
unreduce$O:     unreduce.c unzip.h
unshrink$O:     unshrink.c unzip.h
unzip$O:        unzip.c unzip.h

clean:
	rm -f $(OBJS) unzip$(EXE)


################################
# INDIVIDUAL MACHINE MAKERULES #
################################

# these are the makerules for various systems
# TABS ARE REQUIRED FOR MANY VERSIONS OF "MAKE"!


# ---------------------------------------------------------------------------
#   Generic targets (can't assume make utility groks "$(MAKE)")
# ---------------------------------------------------------------------------

generic:	unzip	# first try if unknown

generic2:		# second try if unknown:  hope make is called "make"...
	make unzip CFLAGS="$(CFLAGS) -DBSD"

# ---------------------------------------------------------------------------
#   "Normal" group (both big- and little-endian, structure-padding or not):
# ---------------------------------------------------------------------------

386i:		unzip	# sun386i, SunOS 4.0.2 ["sun:" works, too, but bigger]
3Bx:		unzip	# AT&T 3B2/1000-80; should work on any WE32XXX machine
7300:		unzip	# AT&T 7300 (M68000/SysV)
apollo:		unzip	# Apollo Domain/OS machines
aviion:         unzip	# Data General AViiONs, DG/UX 4.3x
bull:		unzip	# Bull DPX/2, BOS 2.00.45 (doesn't require -Xk switch)
coherent:	unzip	# Coherent 3.10, Mark Williams C
cray_cc:	unzip	# Cray-2 and Y-MP, using default (possibly old) compiler
dec:		unzip	# DEC 5820 (MIPS RISC), test version of Ultrix v4.0
dnix:		unzip	# 680X0, DIAB dnix 5.2/5.3 (a Swedish System V clone)
encore:		unzip	# Multimax
eta:		unzip	# ETA-10P*, hybrid SysV with BSD 4.3 enhancements
gould:		unzip	# Gould PN9000 running UTX/32 2.1Bu01
hp:		unzip	# HP 9000 series (68020), 4.3BSD or HP-UX A.B3.10 Ver D
hp_ux:		unzip	# (to match zip's makefile entry)
mips:		unzip	# MIPS M120-5(?), SysV R3 [error in sys/param.h file?]
rs6000:		unzip	# IBM RS/6000 under AIX 3
rtaix:		unzip	# IBM RT 6150 under AIX 2.2.1
sco:		unzip	# Xenix/386 (tested on 2.3.1); SCO Unix 3.2.0.
stellar:	unzip	# gs-2000
sun:		unzip	# Sun 4/110, SunOS 4.0.3c; Sun 3 (68020), SunOS 4.0.3
tahoe:		unzip	# tahoe (CCI Power6/32), 4.3BSD
ultrix:		unzip	# VAXen, DEC 58x0 (MIPS guts), DECstation 2100; v4.x
vax:		unzip	# general-purpose VAX target (not counting VMS)

# ---------------------------------------------------------------------------
#   BSD group (for timezone structs [struct timeb]):
# ---------------------------------------------------------------------------

bsd:		_bsd	# generic BSD (BSD 4.2, Ultrix handled in unzip.h)

_bsd:
	$(MAKE) unzip CFLAGS="$(CFLAGS) -DBSD"

# ---------------------------------------------------------------------------
#   SysV group (for extern long timezone and ioctl.h instead of sgtty.h):
# ---------------------------------------------------------------------------

sysv:		_sysv	# generic SysV
amdahl:		_sysv	# Amdahl (IBM) mainframe, UTS (SysV) 1.2.4 and 2.0.1
sgi:		_sysv	# Silicon Graphics Iris 4D, Irix SysV rel. 3.3.2

_sysv:
	$(MAKE) unzip CFLAGS="$(CFLAGS) -DTERMIO"

# ---------------------------------------------------------------------------
#   "Unique" group (require non-standard options):
# ---------------------------------------------------------------------------

# Enclosed you'll find a context diff for the unzip41 makefile
# which enhances compilation on a convex.  The previous version
# probably worked great a couple of years ago, and would still do
# so if one compiles in our "backward compatible" pcc mode.   The
# following allows it to work better in a modern convexian environment
# (define __STDC__ manually because default compilation mode has
# extensions and thus doesn't do so).  [5 Mar 1992:  -D__STDC__ removed
# for now because of problems with stat.h]
#
#	$(MAKE) unzip CFLAGS="$(CFLAGS) -D__STDC__ -DCONVEX -ext" ...
convex:			# previous version was tested on C200/C400
	$(MAKE) unzip CFLAGS="$(CFLAGS) -DCONVEX -ext"\
	 LDFLAGS="$(LDFLAGS) -ext"

# Cray-2 and Y-MP, running Unicos 5.1 or 6.0 (SysV + BSD enhancements)
# and Standard (ANSI) C compiler 1.5, 2.0 or 3.0.
cray:
	$(MAKE) unzip CC="scc" LD="scc"

# The unzip41 build on a Cyber 910/SGI running Irix v3.3.3 was successful
# with the following change to Makefile:
cyber_sgi:
	$(MAKE) unzip CFLAGS="$(CFLAGS) -I/usr/include/bsd"\
	 LDFLAGS="-lbsd $(LDFLAGS)"

# OS/2 2.0 (32-bit) with GNU C compiler (emx)
gcc_os2:
	$(MAKE) unzip.exe CC=gcc LD=gcc EXE=.exe\
	 OBJS="$(OBJS) dosname.o"\
	 CFLAGS="-O -DOS2 -DEMX32 $(CR) $(LOCAL_UNZIP)"\
	 LDFLAGS="-s" LDFLAGS2="-los2 -o unzip.exe"

# Heurikon HK68 (68010), UniPlus+ System V 5.0, Green Hills C-68000
hk68:
	$(MAKE) unzip CC="gcc" LD="gcc" LDFLAGS="-n $(LDFLAGS)" \
	CFLAGS="-ga -X138 -DUNIX $(CR) $(LOCAL_UNZIP) -Dlocaltime=localti -Dtimezone=timezon"

# OS/2 2.0 (32-bit) with IBM C Set/2 compiler
#
file_io2.obj:		# compile this one module without optimization
	$(CC) -c $(CFLAGS) -O- -Fofile_io2.obj file_io.c

icc_os2:
	$(MAKE) -nologo unzip.exe CC=icc LD=icc EXE=.exe O=.obj\
	 OBJS="$(OS2_OBJS:file_io.obj=file_io2.obj)"\
	 CFLAGS="-Q -Sm -O -Gs -DOS2 $(CR) $(LOCAL_UNZIP)"\
	 LDFLAGS="-Q" LDFLAGS2="unzip.def -Fe unzip.exe"

# Minix 1.5 PC for the 386 with gcc or bcc
minix:
	$(MAKE) unzip CC=gcc CFLAGS="$(CFLAGS) -DMINIX"

# PCs (IBM-type), running MS-DOS, Microsoft C 6.00 and NMAKE.  Can't use the
# SYSTEM environment variable; that requires processing the "default:" target,
# which expands to some 200+ characters--well over DOS's 128-character limit.
# "nmake msc_dos" works fine, aside from an annoying message, "temporary file
# e:\ln023193 has been created."  I have no idea how to suppress this, but it
# appears to be benign (comes from the link phase; the file is always deleted).
# The environment variable LOCAL_UNZIP should be set to something appropriate
# if your library uses other than the default floating-point routines; for 
# example, SET LOCAL_UNZIP=-FPi87.  This target assumes the small-model library
# and an 80286 or better.  At present, everything should still fit within the
# 128-character command-line limit (barely); if not, remove the -nologo.  [GRR]
#
msc_dos:
	$(MAKE) unzip.exe\
	 CFLAGS="-Ox $(CR) $(LOCAL_UNZIP) -nologo -G2" CC=cl\
	 LD=link EXE=.exe O=.obj LDFLAGS="/noi /nol" LDFLAGS2=",unzip;"

# The stack size for OS/2 must be increased to 0x1000, i.e. 
# "-F 1000" has to be added to LDFLAGS for msc_os2. Otherwise
# stack overflow occurs, which are only detected if compiled
# with debugging option, i.e. not with -Gs!! Otherwise something
# minor important seems to be overwritten :-)  [K. U. Rommel]
#
# Extra stack causes errors in GRR version ("/st:0x1000"); no problems
# encountered so far without.  EXEHDR /VERBOSE reports 0a00 bytes of
# extra stack already, so maybe the two versions are different... [GRR]
#
# $(LOCAL_UNZIP):  math libraries and/or any other personal or debugging
#                  definitions:  e.g., SET LOCAL_UNZIP=-FPi87 -DDEBUG_STRUC
# $(NOD):  intended to be used as   SET NOD=-link /nod:slibcep   to allow the
#          use of default library names (slibce.lib) instead of protected-mode
#          names (slibcep.lib), but it fails:  MSC adds its own /nod qualifier,
#          and there seems to be no way to override this.  Typical...
#
#msc_os2:		# old Newtware version (may not work)
#	$(MAKE) -nologo unzip.exe CC=cl LD=link EXE=.exe O=.obj\
#	  OBJS="$(OBJS) dosname.obj"\
#	  CFLAGS="-nologo -Ox -G2s -DOS2 $(CR) $(LOCAL_UNZIP) -Lp"\
#	  LDFLAGS="/noi /nol" LDFLAGS2=",unzip,,,unzip.def"
#	bind -nologo unzip.exe -n DOSSETPATHINFO
msc_os2:		# Kai Uwe Rommel version
	$(MAKE) -nologo unzip.exe CC=cl LD=cl EXE=.exe O=.obj\
	 OBJS="$(OS2_OBJS)"\
	 CFLAGS="-nologo -Ox -G2s -DOS2 $(CR) $(LOCAL_UNZIP)"\
	 LDFLAGS="-nologo $(LOCAL_UNZIP) -Lp -F 1000"\
	 LDFLAGS2="unzip.def -o unzip.exe $(NOD)"
	bind -nologo unzip.exe -n DOSSETPATHINFO

# NeXT 2.x: make the executable smaller.
next:			# 68030 BSD 4.3+Mach
	$(MAKE) unzip LDFLAGS2="-object -s"

# I successfully compiled and tested the unzip program (v30) for the
# Silicon Graphics environment (Personal Iris 4D20/G with IRIX v3.2.2)
p_iris:			# Silicon Graphics Personal Iris 4D20
	$(MAKE) unzip CFLAGS="$(CFLAGS) -I/usr/include/bsd -DBSD"\
	 LDFLAGS="-lbsd $(LDFLAGS)"

# I have finished porting unzip 3.0 to the Pyramid 90X under OSX4.1.
# The biggest problem was the default structure alignment yielding two
# extra bytes.  The compiler has the -q option to pack structures, and
# this was all that was needed.  To avoid needing ZMEMS we could compile in
# the att universe, but it runs slower!
#
pyramid:	# Pyramid 90X, probably all, under >= OSx4.1, BSD universe
	make unzip CFLAGS="$(CFLAGS) -q -DBSD -DZMEM"

# SCO cross compile from unix to DOS. Tested with Xenix/386 and
# OpenDeskTop. Should work with xenix/286 as well. (davidsen)
# Note that you *must* remove the unix objects and executable
# before doing this!
#
sco_dos:
	$(MAKE) unzip CFLAGS="-O $(CR) $(LOCAL_UNZIP) -dos -M0" LDFLAGS="-dos"\
	 LDFLAGS2="-o unzip.exe"

# SCO Xenix/286 2.2.1
sco_x286:
	$(MAKE) unzip CFLAGS="$(CFLAGS) -Ml2" LDFLAGS="$(LDFLAGS) -Ml2"

# Sequent Symmetry is a 386 but needs -DZMEM
# This should also work on Balance but I can't test it just yet.
sequent:	# Sequent w/Dynix
	$(MAKE) unzip CFLAGS="$(CFLAGS) -DBSD -DZMEM"

# I didn't do this.  I swear.  No, really.
wombat:		# Wombat 68000 (or whatever)
	@echo
	@echo  '	Ha ha!  Just kidding.'
	@echo


##################
# SHIP MAKERULES #
##################

# Ship section:  ship comes with the Zip distribution and is more properly
# supported there.  But the following targets should at least get you started
# if for some reason you're only interested in UnZip.  The comments near the
# top of ship.c explain how to use it, and a little further poking around
# should clear up any problems related to things which should be defined but
# aren't, or which shouldn't be defined but are.  As with ZipInfo below, we
# assume *some* competence...

_ship:	ship.c $(DEF)
	$(CC) $(CFLAGS) ship.c $(DEF) $(LDFLAGS2)

ship:			# most BSD-type systems, by default
	$(MAKE) _ship LDFLAGS2="-s -o ship"

ship_sysv:		# not tested; DIRENT used only to determine mailer
	$(MAKE) _ship CFLAGS="$(CFLAGS) -DDIRENT" LDFLAGS2="-s -o ship"

ship_dos:		# not tested
	$(MAKE) -nologo _ship CC=cl EXE=.exe\
	 CFLAGS="-nologo -Ox $(LOCAL_UNZIP) -G2s -F 2000"\
	 LDFLAGS2="-o ship.exe"

ship_os2:		# MSC 6.0, 16-bit OS/2
	$(MAKE) -nologo _ship CC=cl EXE=.exe DEF=ship.def\
	 CFLAGS="-nologo -Ox $(LOCAL_UNZIP) -G2s -DOS2 -Lp -F 2000"\
	 LDFLAGS2="-o ship.exe"
	bind -nologo ship.exe

ship_icc:		# IBM C Set/2, 32-bit OS/2
	$(MAKE) -nologo _ship CC=icc EXE=.exe DEF=ship.def\
	 CFLAGS="-Q -Sm -O $(LOCAL_UNZIP) -Gs -DOS2"\
	 LDFLAGS2="-Fe ship.exe"

ship_gcc:		# GNU gcc / emx, 32-bit OS/2
	$(MAKE) _ship CC=gcc LD=gcc EXE=.exe\
	 CFLAGS="-O -DOS2" LDFLAGS2="-s -o ship.exe"


#####################
# ZIPINFO MAKERULES #
#####################

# Zipinfo section:  it is assumed here that anyone competent enough to
# wonder about the internal guts of a zipfile is probably also competent
# enough to compile the program without a lot of hand-holding.  If not...
# oh well. :-)

zipinfo$O:	zipinfo.c unzip.h
	$(CC) -c $(CFLAGS) $(ZC) zipinfo.c

misc_$O:	misc.c unzip.h
	$(MV) misc.c misc_.c
	$(CC) -c $(CFLAGS) $(ZC) -DZIPINFO misc_.c
	$(MV) misc_.c misc.c

zipinfo$(EXE):	$(ZI_OBJS)
	$(LD) $(ZL) $(ZI_OBJS) $(ZL2)

zi_dos:
	$(MAKE) zipinfo.exe CFLAGS="-Ox -nologo $(LOCAL_UNZIP) -G2" CC=cl\
	 LD=link EXE=.exe O=.obj ZL="/noi /nol" ZL2=",zipinfo;" ZC="" MV="ren"

#zi_os2: 		# GRR (Newtware) version (do not delete!)
#	$(MAKE) -nologo zipinfo.exe CC=cl LD=link EXE=.exe O=.obj\
#	 CFLAGS="-nologo -Ox $(LOCAL_UNZIP) -G2s -DOS2 -Lp" ZC="" MV="ren"\
#	 ZL="/nol /noi" ZL2=",zipinfo,,,zipinfo.def"
#	bind -nologo zipinfo.exe
zi_os2: 		# Kai Uwe Rommel version (do not delete!)
	$(MAKE) -nologo zipinfo.exe CC=cl LD=cl EXE=.exe O=.obj\
	 CFLAGS="-nologo -Ox $(LOCAL_UNZIP) -G2s -DOS2" ZC="" MV="ren"\
	 ZL="-nologo $(LOCAL_UNZIP) -Lp -Fb" ZL2="zipinfo.def -o zipinfo.exe"

zi_icc:			# IBM C Set/2, 32-bit OS/2
	$(MAKE) -nologo zipinfo.exe CC=icc LD=icc EXE=.exe O=.obj\
	 CFLAGS="-Q -Sm -O -Gs -DOS2" ZC="" MV="ren"\
	 ZL="-Q" ZL2="zipinfo.def -Fe zipinfo.exe"

zi_gcc:			# GNU gcc / emx, 32-bit OS/2
	$(MAKE) zipinfo.exe CC=gcc LD=gcc EXE=.exe\
	 CFLAGS="-O -DOS2 -DEMX32" ZC="" MV="ren"\
	 ZL="-s" ZL2="-o zipinfo.exe"


################
# ATTRIBUTIONS #
################

# Thanks to the following people for their help in testing and/or porting
# to various machines (and thanks to the many others who aren't listed
# here but should be):
#
#  (original Unix port:  Carl Mascott <cmascott@world.std.com>)
#  386i:	Richard Stephen <stephen@corp.telecom.co.nz>
#  3Bx:		Bob Kemp <hrrca!bobc@cbnewse.att.com>
#  7300:	Richard H. Gumpertz <rhg@cpsolv.CPS.COM>
#		Greg Roelofs <roelofs@amelia.nas.nasa.gov>
#  amdahl:	Kim DeVaughn <ked01@juts.ccc.amdahl.com>, Greg Roelofs
#  apollo:	Tim Geibelhaus
#  aviion:	Bruce Kahn <bkahn@archive.webo.dg.com>
#  bull:	Matt D'Errico <doc@magna.com>
#  coherent:	David Fenyes <dfenyes@thesis1.med.uth.tmc.edu>
#  convex:	Randy Wright <rwright@convex.com>
#  cray:	Greg Roelofs, Paul Borman <prb@cray.com>
#  cyber_sgi:	Clint Pulley <u001@cs910.cciw.ca>
#  dec:		"Moby" Dick O'Connor <djo7613@u.washington.edu>
#  dnix:	Bo Kullmar <bk@kullmar.se>
#  eta:		Greg Flint <afc@klaatu.cc.purdue.edu>
#  gould:	Onno van der Linden <linden@fwi.uva.nl>
#  hk68:	John Limpert <gronk!johnl@uunet.UU.NET>
#  hp:		Randy McCaskile <rmccask@seas.gwu.edu> (HP-UX)
#		Gershon Elber <gershon@cs.utah.edu> (HP BSD 4.3)
#  icc_os2:	Kai Uwe Rommel <rommel@informatik.tu-muenchen.de>
#  minix:	Kai Uwe Rommel (Minix 1.5)
#  mips:	Peter Jones <jones@mips1.uqam.ca>
#  msc_dos:	Greg Roelofs
#  msc_os2:	Wim Bonner <wbonner@yoda.eecs.wsu.edu>
#		Kai Uwe Rommel, Greg Roelofs
#  next:	Mark Adler <madler@piglet.caltech.edu>
#  p_iris:	Valter V. Cavecchia <root@itnsg1.cineca.it>
#  pyramid:	James Dugal <jpd@usl.edu>
#  rs6000:	Filip Gieszczykiewicz <fmg@smi.med.pitt.edu>
#  rtaix:	Erik-Jan Vens
#  sco:		Onno van der Linden (SCO Unix 3.2.0)
#   		Bill Davidsen <davidsen@crdos1.crd.ge.com> (Xenix/386)
#  sco_dos:	Bill Davidsen
#  sco_x286:	Ricky Mobley <ddi1!lrark!rick@uunet.UU.NET>
#  sequent:	Phil Howard <phil@ux1.cso.uiuc.edu>
#  sgi:		Greg Roelofs (Iris 4D/380?)
#  sun:		Onno van der Linden (Sun 4), Greg Roelofs (Sun 3, 4)
#  tahoe:	Mark Edwards <mce%sdcc10@ucsd.edu>
#  ultrix:	Greg Flint (VAX)
#		Michael Graff <explorer@iastate.edu> (DECstation 2100?)
#		Greg Roelofs (DEC 5810)
#		Alex A Sergejew <aas@brain.wph.uq.oz.au>
#  vax:		Forrest Gehrke <feg@dodger.att.com> (SysV)
#		David Kirschbaum <kirsch@usasoc.soc.mil> (BSD 4.3)
#		Jim Steiner <steiner@pica.army.mil> (8600+Ultrix)
#  wombat:	Joe Isuzu <joe@trustme.isuzu.com>
#  zi_dos:	Greg Roelofs
#  zi_icc:	Kai Uwe Rommel
#  zi_os2:	Greg Roelofs, Kai Uwe Rommel
#  zipinfo:	Greg Roelofs

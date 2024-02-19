#    Copyright (C) 1989, 1990, 1991 Aladdin Enterprises.  All rights reserved.
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

# makefile for Ghostscript, MS-DOS/Borland C++ platform.

# ------------------------------- Options ------------------------------- #

###### This section is the only part of the file you should need to edit.

# ------ Generic options ------ #

# Define the default directory/ies for the runtime
# initialization and font files.  Separate multiple directories with \;.
# Use / to indicate directories, not a single \.

GS_LIB_DEFAULT=c:/gs\;c:/gs/fonts

# Define the name of the Ghostscript initialization file.
# (There is no reason to change this.)

GS_INIT=gs_init.ps

# Choose generic configuration options.

# Setting DEBUG=1 includes debugging features (-Z switch) in the code.
# Code runs substantially slower even if no debugging switches are set,
# and also takes about another 25K of memory.

DEBUG=0

# Setting TDEBUG=1 includes symbol table information for the Borland debugger,
# and also enables stack checking.  Code is substantially slower and larger.

TDEBUG=0

# Setting NOPRIVATE=1 makes private (static) procedures and variables public,
# so they are visible to the debugger and profiler.
# No execution time or space penalty, just larger .OBJ and .EXE files.

NOPRIVATE=0

# ------ Platform-specific options ------ #

# If you don't have an assembler, set USE_ASM=0.  Otherwise, set USE_ASM=1,
# and set ASM to the name of the assembler you are using.  This can be
# a full path name if you want.  Normally it will be masm or tasm.

USE_ASM=1
ASM=tasm

# Define the drive, directory, and compiler name for the Turbo C files.
# COMP is the compiler name (tcc for Turbo C++, bcc for Borland C++).
# COMPDIR contains the compiler and linker (normally \bc\bin).
# BGIDIR contains the BGI files (normally /bc/bgi).
#   (These are only needed if you include the BGI driver.)
# INCDIR contains the include files (normally \bc\include).
# LIBDIR contains the library files (normally \bc\lib).
# Note that these prefixes are always followed by a \,
#   so if you want to use the current directory, use an explicit '.'.

COMP=bcc
COMPDIR=c:\bc\bin
# For BGIDIR, use / to indicate directories, not \.
BGIDIR=c:/bc/bgi
INCDIR=c:\bc\include
LIBDIR=c:\bc\lib

# Define whether you want to use the Borland code overlay mechanism
# (VROOMM).  Code overlays make it possible to process larger files,
# but code swapping will slow Ghostscript down even on smaller ones.
# See the file overlay.h to control details of overlaying such as
# the overlay buffer size, whether to use EMS and/or extended memory
# to store evicted overlays, and how much of that memory to use.

OVERLAY=1

# Choose platform-specific options.

# Define the processor (CPU) type.  Options are 86, 186, 286, 386, or 486.
# (The 8086 and 8088 both correspond to processor type 86.)
# 286 and up do not use protected mode.  Higher numbers produce
# code that may be significantly smaller and faster, but the executable
# will bail out with an error message on lower-numbered processor types.

CPU_TYPE=86

# Define the math coprocessor (FPU) type.  Options are 0, 87, 287, or 387.
# If the CPU type is 486, the FPU type is irrelevant, since the 80486
# CPU includes the equivalent of an 80387 on-chip.
# A non-zero option means that the executable will only run if a FPU
# of that type (or higher) is available: this is NOT currently checked
# at runtime.
#   Code is significantly faster.

FPU_TYPE=0

# ---------------------------- End of options ---------------------------- #

# Swapping `make' out of memory makes linking much faster.

.swap

# Define the platform name.

PLATFORM=tbc_

# Define the name of the makefile -- used in dependencies.

MAKEFILE=tbcplus.mak

# Define the ANSI-to-K&R dependency.  Turbo C accepts ANSI syntax,
# but we need to preconstruct ccf.tr to get around the limit on
# the maximum length of a command line.

AK=ccf.tr

# Define the compilation flags for an 80286.
# This is void because we handle -2 and -3 in ccf.tr (see below).

F286=

# Figure out which version of Borland C++ we are running.
# In the MAKE program that comes with Borland C++ 2.0, __MAKE__ is 0x300;
# in the MAKE that comes with Borland C++ 3.0 and 3.1, it is 0x360.
# We care because 3.0 has additional optimization features.
# There are also some places below where we distinguish BC++ 3.1 from 3.0
#   by testing whether $(INCDIR)\win30.h exists (true in 3.1, false in 3.0).

EXIST_BC3_1=exist $(INCDIR)\win30.h

!if $(__MAKE__) >= 0x360
CO=-Obe -Z
!else
CO=-O
!endif

!include "tccommon.mak"

# Define the compilation flags.

!if $(NOPRIVATE)
CP=-DNOPRIVATE
!else
CP=
!endif

!if $(DEBUG) | $(TDEBUG)
CS=-N
!else
CS=
!endif

!if $(DEBUG)
CD=-DDEBUG
!else
CD=
!endif

!if $(TDEBUG)
CT=-v
LCT=/v
!else
CT=-y
LCT=/m /l
!endif

GENOPT=$(CP) $(CS) $(CD) $(CT)

!if $(OVERLAY)
CY=-Y
CYO=-Yo -zAOVLY
LO=/oOVLY
LIBOVL=$(LIBDIR)\overlay
OVLH=overlay.h
!else
CY=
CYO=
LO=
LIBOVL=
OVLH=
!endif

CCFLAGS0=$(GENOPT) $(PLATOPT) $(FPFLAGS)
CCFLAGS=$(CCFLAGS0) -m$(MM)
CC=$(COMPDIR)\$(COMP) -m$(MM) @ccf.tr
CCC=$(CC) $(CYO) $(CO) -c
CCD=$(CC) $(CYO) -O -c
CCCF=$(COMPDIR)\$(COMP) -mh @ccf.tr $(CYO) -O -c
CC0=$(CC) $(CYO) -c
CCINT=$(CC) $(CYO) -c

.c.obj:
	$(CCC) { $<}

# ------ Devices and features ------ #

# Choose the language feature(s) to include.  See gs.mak for details.
# Even though code overlays are available, we don't include any of the
# optional features, because they cost a significant amount of non-code space.

FEATURE_DEVS=

# Choose the device(s) to include.  See devs.mak for details.
# We don't include the BGI driver, because it takes 10K of
# non-overlayable code space.

DEVICE_DEVS=vga.dev ega.dev epson.dev eps9high.dev bj10e.dev
DEVICE_DEVS2=deskjet.dev djet500.dev laserjet.dev ljetplus.dev ljet2p.dev ljet3.dev
DEVICE_DEVS3=cdeskjet.dev cdjcolor.dev cdjmono.dev paintjet.dev pjetxl.dev
DEVICE_DEVS5=gifmono.dev gif8.dev
!include "gs.mak"
!include "devs.mak"

# Build the compiler response file depending on the selected options.

ccf.tr: $(MAKEFILE) makefile
!if $(CPU_TYPE) < 286
	echo -a -d -r -G -X -I$(INCDIR) $(CCFLAGS0) >ccf.tr
!else
	echo -2 -a -d -r -G -X -I$(INCDIR) $(CCFLAGS0) >ccf.tr
!  if $(CPU_TYPE) > 286
	echo -3 -a -d -r -G -X -I$(INCDIR) $(CCFLAGS0) >_temp_
	if $(EXIST_BC3_1) copy _temp_ ccf.tr
!  endif
!endif

# -------------------------------- Library -------------------------------- #

# The Turbo/Borland C(++) platform

tbc__=gp_itbc.$(OBJ) gp_dosfb.$(OBJ) gp_msdos.$(OBJ)
tbc_.dev: $(tbc__)
	$(SHP)gssetmod tbc_ $(tbc__)

# We have to compile gp_itbc without -1, because it includes a run-time
# check to make sure we are running on the right kind of processor.
gp_itbc.$(OBJ): gp_itbc.c $(string__h) $(gx_h) $(gp_h) \
 $(OVLH) $(MAKEFILE) makefile
	$(CC) $(CCFLAGS) -1- $(CY) -DCPU_TYPE=$(CPU_TYPE) -c gp_itbc.c

gp_dosfb.$(OBJ): gp_dosfb.c $(memory__h) $(gx_h) $(gp_h) $(gserrors_h) $(gxdevice_h)

gp_msdos.$(OBJ): gp_msdos.c $(dos__h) $(string__h) $(gx_h) $(gp_h)

# ----------------------------- Main program ------------------------------ #

BEGINFILES=ccf.tr
CCBEGIN=$(CCC) *.c

iutilasm.$(OBJ): iutilasm.asm

# Get around the fact that the DOS shell has a rather small limit on
# the length of a command line.  (sigh)

LIBCTR=libc$(MM).tr

$(LIBCTR): $(MAKEFILE) makefile
	echo $(LIBOVL) $(LIBDIR)\$(FPLIB) $(LIBDIR)\math$(MM) $(LIBDIR)\c$(MM) >$(LIBCTR)

LIBDOS=$(LIB) obj.tr

# Interpreter main program

GS_ALL=gs.$(OBJ) $(INT) $(INTASM) gsmain.$(OBJ)\
  $(LIBDOS) $(LIBCTR) obj.tr lib.tr

gs.exe: $(GS_ALL)
	tlink $(LCT) $(LO) $(LIBDIR)\c0$(MM) @obj.tr $(INTASM) @gs.tr ,gs,gs,@lib.tr @$(LIBCTR)

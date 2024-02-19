#    Copyright (C) 1991, 1992 Aladdin Enterprises.  All rights reserved.
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

# tccommon.mak
# Section of MS-DOS makefile for Ghostscript common to
# Turbo C and Turbo/Borland C++, MS-DOS and MS Windows.

# This file is used by turboc.mak, tbcplus.mak, and bcwin.mak.
# Those files supply the following parameters:
#   Configuration, public:
#	GS_LIB_DEFAULT, GS_INIT, FEATURE_DEVS, DEVICE_DEVS*
#   Configuration, internal, generic:
#	PLATFORM, MAKEFILE, AK, CC*, DEBUG, NOPRIVATE
#   Configuration, internal, specific to DOS/Windows:
#	TDEBUG, USE_ASM, ASM,
#	COMPDIR, (BGIDIR), INCDIR, LIBDIR,
#	CPU_TYPE, FPU_TYPE,
#	F286, GENOPT

# Make sure we get the right default target for make.

dosdefault: default

# Define the extensions for the object and executable files.

OBJ=obj
XE=.exe

# Define the need for uniq.

UNIQ=uniq$(XE)

# Define the current directory prefix, shell quote string, and shell name.

EXP=
QQ="
SH=
SHP=

# Define the memory model for Turbo C.  Don't change it!

MM=l

# Define the generic compilation flags.

!if $(CPU_TYPE) >= 486
ASMFLAGS=/DFOR80386 /DFOR80486
PLATOPT=$(F286) -DFOR80386 -DFOR80486
!elif $(CPU_TYPE) >= 386
ASMFLAGS=/DFOR80386
PLATOPT=$(F286) -DFOR80386
!elif $(CPU_TYPE) >= 286
ASMFLAGS=
PLATOPT=$(F286)
!elif $(CPU_TYPE) >= 186
ASMFLAGS=
PLATOPT=-1
!else
ASMFLAGS=
PLATOPT=
!endif

!if $(CPU_TYPE) == 486 || $(FPU_TYPE) >= 287
FPFLAGS=-f287
FPLIB=fp87
!elif $(FPU_TYPE) != 0
FPFLAGS=-f87
FPLIB=fp87
!else
FPFLAGS=
FPLIB=emu
!endif

!if $(USE_ASM)
INTASM=iutilasm.$(OBJ)
PCFBASM=gdevegaa.$(OBJ)
!else
INTASM=
PCFBASM=
!endif

# Define the generic compilation rules.

.asm.obj:
	$(ASM) $(ASMFLAGS) $<;

# -------------------------- Auxiliary programs --------------------------- #

CCAUX=$(COMPDIR)\$(COMP) -m$(MM) -I$(INCDIR) -L$(LIBDIR) -O

echogs$(XE): echogs.c
	$(CCAUX) echogs.c

genarch$(XE): genarch.c
	$(CCAUX) genarch.c

# We need a substitute for the Unix uniq utility.
# It only has to handle stdin and stdout, no options.
uniq$(XE): uniq.c
	$(CCAUX) uniq.c

# ---------------------- MS-DOS I/O debugging option ---------------------- #

dosio_=zdosio.$(OBJ)
dosio.dev: $(dosio_)
	$(SHP)gssetmod dosio $(dosio_)
	$(SHP)gsaddmod dosio -oper zdosio

zdosio.$(OBJ): zdosio.c $(OP) $(store_h)

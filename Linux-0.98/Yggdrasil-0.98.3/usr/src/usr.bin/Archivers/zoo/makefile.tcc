# $Source: /usr/home/dhesi/zoo/RCS/makefile.tcc,v $
# $Id: makefile.tcc,v 1.6 91/07/07 18:39:28 dhesi Exp $
# Make Zoo -- works with Turbo C++ 1.0 under MS-DOS and
# Don Kneller's NDMAKE version 4.31.
#
# compile is tcc (Turbo C++ 1.0)
CC = tcc
# assembler is tasm
AS = tasm
ASFLAGS =
CFLAGS = -c -DTURBOC -DLINT

# char representing memory model (l = large, c = compact)
MCHAR = c
#
#
model = -m$(MCHAR)					# compiler switch
CRT0 = c:\tc\lib\c0$(MCHAR).obj	# C runtime object
STDLIB = \tc\lib\c$(MCHAR).lib	# C standard library

EXTRA = -DBIG_MEM -DNDEBUG
OPTIM = -O

.SUFFIXES : .exe .obj .asm .c

# Object files for zoo
ZOOOBJS = 	addbftcc.obj addfname.obj basename.obj comment.obj \
		crcdefs.obj getfile.obj lzc.obj lzd.obj machine.obj \
		makelist.obj misc.obj misc2.obj nextfile.obj needed.obj \
		options.obj parse.obj portable.obj prterror.obj \
		version.obj zoo.obj zooadd.obj zooadd2.obj zoodel.obj \
		zooext.obj zoofilt.obj zoolist.obj zoopack.obj \
		io.obj lzh.obj maketbl.obj maketree.obj huf.obj \
		encode.obj decode.obj \
		msdos.obj

# Object files for fiz
FIZOBJS = fiz.obj addbftcc.obj portable.obj crcdefs.obj

#################################################################
# default rule for assembly and compilation
#################################################################

## assembly
## .asm.obj :
## 	$(AS) $(ASFLAGS) $*.asm

# C compilation
.c.obj :
	$(CC) $(CFLAGS) $(model) $(EXTRA) $*.c

#################################################################
# final link
#################################################################

zoo.exe: $(ZOOOBJS)
	link /c /m /s $(CRT0) \
		$(ZOOOBJS),zoo.exe,zoo.map,$(STDLIB)

#################################################################
# miscellaneous targets: install and cleanup
#################################################################

install:  zoo.exe
	copy zoo.exe \bin\tzoo.exe

clean :
	del *.obj

#################################################################
# dependencies
#################################################################

addfname.obj: options.h various.h zoo.h zoofns.h zooio.h zoomem.h
basename.obj: assert.h debug.h options.h parse.h various.h
basename.obj: zoo.h zoofns.h zooio.h
comment.obj: errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
crcdefs.obj: options.h
decode.obj: ar.h lzh.h options.h zoo.h
encode.obj: ar.h errors.i lzh.h options.h zoo.h
fiz.obj: options.h portable.h various.h zoo.h zoofns.h zooio.h
generic.obj: nixmode.i nixtime.i
getfile.obj: options.h various.h zoo.h zoofns.h zooio.h zoomem.h
huf.obj: ar.h errors.i lzh.h options.h zoo.h
io.obj: ar.h errors.i lzh.h options.h portable.h zoo.h zooio.h
lzc.obj: assert.h debug.h lzconst.h options.h various.h
lzc.obj: zoo.h zoofns.h zooio.h zoomem.h
lzd.obj: assert.h debug.h lzconst.h options.h various.h
lzd.obj: zoo.h zoofns.h zooio.h zoomem.h
lzh.obj: ar.h errors.i options.h zoo.h
machine.obj: options.h various.h zoo.h zoofns.h zooio.h
makelist.obj: assert.h debug.h errors.i options.h
makelist.obj: portable.h various.h zoo.h zoofns.h zooio.h
maketbl.obj: ar.h lzh.h options.h zoo.h
maketree.obj: ar.h lzh.h options.h zoo.h
misc.obj: errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
misc2.obj: errors.i options.h portable.h various.h zoo.h
misc2.obj: zoofns.h zooio.h zoomem.h
msdos.obj: errors.i options.h zoo.h zoofns.h zooio.h
needed.obj: debug.h options.h portable.h various.h zoo.h
needed.obj: zoofns.h zooio.h
nextfile.obj: options.h various.h zoo.h
options.obj: errors.i options.h various.h zoo.h zoofns.h zooio.h
parse.obj: assert.h options.h parse.h various.h zoo.h
parse.obj: zoofns.h zooio.h
portable.obj: assert.h debug.h machine.h options.h
portable.obj: portable.h various.h zoo.h zoofns.h zooio.h
prterror.obj: options.h various.h zoofns.h zooio.h
zoo.obj: errors.i options.h various.h zoo.h zoofns.h
zoo.obj: zooio.h zoomem.h
zooadd.obj: debug.h errors.i options.h parse.h portable.h
zooadd.obj: various.h zoo.h zoofns.h zooio.h zoomem.h
zooadd2.obj: assert.h debug.h errors.i options.h parse.h
zooadd2.obj: various.h zoo.h zoofns.h zooio.h
zoodel.obj: errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
zooext.obj: errors.i machine.h options.h parse.h portable.h various.h zoo.h
zooext.obj: zoofns.h zooio.h
zoofilt.obj: options.h
zoolist.obj: errors.i options.h portable.h various.h zoo.h
zoolist.obj: zoofns.h zooio.h zoomem.h
zoopack.obj: errors.i options.h portable.h various.h
zoopack.obj: zoo.h zoofns.h zooio.h

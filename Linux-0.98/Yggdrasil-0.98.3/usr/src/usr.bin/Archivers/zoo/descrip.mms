# derived from: @(#) descrip.mms 2.2 88/01/09 12:10:49
# $Source: /usr/home/dhesi/zoo/RCS/descrip.mms,v $
# $Id: descrip.mms,v 1.9 91/07/07 14:58:21 dhesi Exp $
#Make Zoo for VAX/VMS
#
#The contents of this makefile are hereby released to the public domain.
#                                  -- Rahul Dhesi 1991/07/06

CC = cc
CFLAGS =
EXTRA = /define=(BIG_MEM,NDEBUG,VMS)
ldswitch =

#List of all object files created for Zoo
ZOOOBJS = addbfcrc.obj, addfname.obj, basename.obj, comment.obj,  -
 crcdefs.obj, decode.obj, encode.obj, getfile.obj, huf.obj,  -
 io.obj, lzc.obj, lzd.obj, lzh.obj, machine.obj, makelist.obj,  -
 maketbl.obj, maketree.obj, misc.obj, misc2.obj, needed.obj,  -
 nextfile.obj, options.obj, parse.obj, portable.obj, prterror.obj,  -
 version.obj, vmstime.obj, zoo.obj, zooadd.obj, zooadd2.obj,  -
 zoodel.obj, zooext.obj, zoolist.obj, zoopack.obj

FIZOBJS = fiz.obj, addbfcrc.obj, portable.obj, crcdefs.obj

BILFOBJS = bilf.obj

.c.obj :
	$(CC) $(CFLAGS) $(EXTRA) $*.c

zoo.exe : $(ZOOOBJS)
	link/executable=zoo.exe $(ldswitch) $(ZOOOBJS), options/opt

# bigger but perhaps more (less?) portable across machines -- 
# no shared libraries
zoobig.exe : $(ZOOOBJS)
	link/executable=zoobig.exe $(ldswitch) $(ZOOOBJS)

fiz : $(FIZOBJS)
	link/executable=fiz.exe $(ldswitch) $(FIZOBJS), options/opt

bilf : $(BILFOBJS)
	link/executable=bilf.exe $(ldswitch) $(BILFOBJS), options/opt

#######################################################################
# DEPENDENCIES -- not guaranteed to be up-to-date
#######################################################################

addbfcrc.obj : options.h
addfname.obj : options.h various.h zoo.h zoofns.h zooio.h
addfname.obj : zoomem.h
basename.obj : assert.h debug.h options.h parse.h various.h
basename.obj : zoo.h zoofns.h zooio.h
comment.obj : errors.i options.h portable.h various.h
comment.obj : zoo.h zoofns.h zooio.h
crcdefs.obj : options.h
decode.obj : ar.h lzh.h options.h zoo.h
encode.obj : ar.h errors.i lzh.h
encode.obj : options.h zoo.h
fiz.obj : options.h portable.h various.h zoo.h zoofns.h
fiz.obj : zooio.h
getfile.obj : options.h various.h zoo.h zoofns.h zooio.h
getfile.obj : zoomem.h
huf.obj : ar.h errors.i lzh.h options.h zoo.h
io.obj : ar.h errors.i lzh.h options.h portable.h zoo.h
io.obj : zooio.h
lzc.obj : assert.h debug.h lzconst.h options.h various.h
lzc.obj : zoo.h zoofns.h zooio.h zoomem.h
lzd.obj : assert.h debug.h lzconst.h options.h various.h
lzd.obj : zoo.h zoofns.h zooio.h zoomem.h
lzh.obj : ar.h errors.i options.h zoo.h
machine.obj : options.h various.h zoo.h zoofns.h zooio.h
makelist.obj : assert.h debug.h errors.i options.h
makelist.obj : portable.h various.h zoo.h zoofns.h zooio.h
maketbl.obj : ar.h lzh.h options.h zoo.h
maketree.obj : ar.h lzh.h options.h zoo.h
misc.obj : errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
misc2.obj : errors.i options.h portable.h various.h zoo.h
misc2.obj : zoofns.h zooio.h zoomem.h
msdos.obj : errors.i options.h zoo.h zoofns.h zooio.h
needed.obj : debug.h options.h portable.h various.h zoo.h
needed.obj : zoofns.h zooio.h
nextfile.obj : options.h various.h zoo.h
options.obj : errors.i options.h various.h zoo.h zoofns.h
options.obj : zooio.h
parse.obj : assert.h options.h parse.h various.h zoo.h
parse.obj : zoofns.h zooio.h
portable.obj : assert.h debug.h machine.h options.h
portable.obj : portable.h various.h zoo.h zoofns.h zooio.h
prterror.obj : options.h various.h
prterror.obj : zoofns.h zooio.h
zoo.obj : errors.i options.h various.h zoo.h zoofns.h
zoo.obj : zooio.h zoomem.h
zooadd.obj : debug.h errors.i options.h parse.h portable.h
zooadd.obj : various.h zoo.h zoofns.h zooio.h zoomem.h
zooadd2.obj : assert.h debug.h errors.i options.h parse.h
zooadd2.obj : various.h zoo.h zoofns.h zooio.h
zoodel.obj : errors.i options.h portable.h various.h zoo.h zoofns.h zooio.h
zooext.obj : errors.i machine.h options.h parse.h portable.h various.h zoo.h
zooext.obj : zoofns.h zooio.h
zoofilt.obj : options.h
zoolist.obj : errors.i options.h portable.h various.h zoo.h
zoolist.obj : zoofns.h zooio.h zoomem.h
zoopack.obj : errors.i options.h portable.h various.h
zoopack.obj : zoo.h zoofns.h zooio.h

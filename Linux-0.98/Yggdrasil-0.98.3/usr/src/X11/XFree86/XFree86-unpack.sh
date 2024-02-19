#!/bin/sh

#    This example shows the order of steps taken to unpack the XFree86
#    distribution.  Depending on how you retrieve the sources, your
#    file names or locations may differ.

#    Let's assume all the tar and patch files are in a given
directory.
T=$HOME/t/X386

#    Echo each command executed
set -x

#    Unpack the tar files holding MIT's core X11R5 sources.
for i in mit-1.tar.Z mit-2.tar.Z mit-3.tar.Z mit-4.tar.Z ; do
  zcat $T/$i | tar xf -
done

#    Apply the patches for each MIT's fixes.
for i in fix-01 fix-02 fix-03 fix-04 fix-05 fix-06 fix-07 fix-08 \
fix-09 fix-10 fix-11 fix-12 fix-13 fix-14 fix-15 fix-16 ; do
  patch -p <$T/fixes/$i
done

#    Unpack the sources for the type 1 font renderer.
(cd mit/fonts/lib/font; zcat $T/Type1.tar.Z | tar xf - )
#    Apply the first patch to the renderer
(cd mit; zcat $T/Type1.patch1.Z |patch -p )
#    apply the second patch to the renderer
patch -p <$T/Type1.patch2

#    Remove branches of the directory tree which generally aren't
#    needed for building XFree86
rm -rf \
        mit/doc                         mit/hardcopy \
        mit/demos/auto_box              mit/extensions/lib/PEX \
        mit/demos/beach_ball            mit/extensions/server/PEX \
        mit/demos/gpc                   mit/extensions/test/InsPEX \
        mit/fonts/PEX                   mit/lib/CLX \
        mit/server/ddx/dec              mit/server/ddx/ibm \
        mit/server/ddx/macII            mit/server/ddx/mips \
        mit/server/ddx/omron            mit/server/ddx/snf \
        mit/server/ddx/sun              mit/server/ddx/tek \
        mit/util/PEX \
        mit/fonts/bdf/misc/k14.bdf \
        mit/fonts/bdf/misc/hang*.bdf \
        mit/fonts/bdf/misc/jiskan*.bdf

#    Apply the XFree86 patches
sh $T/XFree86-1.1.sh
zcat $T/XFree86-1.1.diff.Z |patch -p

#    Check for rejects
#    Note: for SVR3.2, rejects end in  '#'
find . -name '*.rej' -print

#    Remove the old copy of each modified file
#    Note: for SVR3.2, originals end in '~'
find . -name '*.orig' | xargs rm -f


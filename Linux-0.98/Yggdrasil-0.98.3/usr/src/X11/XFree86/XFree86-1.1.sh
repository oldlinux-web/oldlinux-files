#!/bin/sh

# Directory reorganisation script.
# Must be run from the directory containing the 'mit' directory.

version_file=mit/server/ddx/x386/x386Version.h

if [ ! -d mit -o ! -d mit/server/ddx/x386 ]; then
  echo `basename $0` must be run from the directory containing your toplevel
  echo \`mit\' directory
  exit 1
fi

if [ ! -d mit/server/ddx/x386/common ]; then
  (set -x; mkdir mit/server/ddx/x386/common)
fi
if [ ! -d mit/server/ddx/x386/vga256 ]; then
  (set -x; mkdir mit/server/ddx/x386/vga256)
fi

common_files="atKeynames.h compiler.h x386.h x386Config.c \
              x386Cursor.c x386Events.c x386Init.c x386Io.c x386Kbd.c \
              x386OSD.h x386Priv.h x386Procs.h"

for i in $common_files; do
  (set -x; mv mit/server/ddx/x386/$i mit/server/ddx/x386/common/$i)
done

(set -x; cp mit/server/ddx/x386/Imakefile mit/server/ddx/x386/common/Imakefile)

vga256_dirs="cfb.banked drivers vga"

for i in $vga256_dirs; do
  echo "Moving mit/server/ddx/x386/$i to mit/server/ddx/x386/vga256/$i"
  (cd mit/server/ddx/x386
   find $i -print | cpio -plmd vga256
   rm -fr $i)
done

#if [ -f mit/server/ddx/x386/vga256/vga/configvga.sh ]; then
#  (set -x; mv mit/server/ddx/x386/vga256/vga/configvga.sh \
#              mit/server/ddx/x386/vga256/confvga256.sh)
#fi

etc_files="CONFIG"

for i in $etc_files; do
  if [ -f mit/server/ddx/x386/$i ]; then
    (set -x; mv mit/server/ddx/x386/$i mit/server/ddx/x386/etc/$i)
  fi
done

(set -x; rm -f mit/server/ddx/x386/README)

echo '/* re-org_done_1.1 */' >> $version_file

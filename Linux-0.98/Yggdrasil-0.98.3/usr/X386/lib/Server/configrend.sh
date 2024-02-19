#!/bin/sh

# $Header: /home/x_cvs/mit/fonts/lib/font/fontfile/configrend.sh,v 1.1 1992/08/19 14:16:03 dawes Exp $
#
# This script generates rendererConf.c
#
# usage: configrend.sh driver1 driver2 ...
#

RENDCONF=./rendererConf.c

cat > $RENDCONF <<EOF
/*
 * This file is generated automatically -- DO NOT EDIT
 */

FontFileRegisterFontFileFunctions ()
{
    BitmapRegisterFontFileFunctions ();
EOF
for i in $*; do
  echo "    ${i}RegisterFontFileFunctions ();" >> $RENDCONF
done
echo '}' >> $RENDCONF

#!/bin/sh
# Compile programs, treating .c files as C++.
: || exec /bin/sh -f $0 $argv:q

newargs=
quote=no
library=-lg++
havefiles=

for arg in $*;
do
  if [ $quote = yes ]
  then
    newargs="$newargs $arg"
    quote=no
  else
    quote=no
    case $arg in
      -nostdlib)
	# Inhibit linking with -lg++.
	newargs="$newargs $arg"
	library=
	;;
      -[bBVDUoeTuIYmLiA] | -Tdata)
	newargs="$newargs $arg"
	# these switches take following word as argument,
	# so don't treat it as a file name.
	quote=yes
	;;
      -[cSEM] | -MM)
	# Don't specify libraries if we won't link,
	# since that would cause a warning.
	newargs="$newargs $arg"
	library=
	;;
      -xnone)
	newargs="$newargs $arg"
	speclang=
	;;
      -x*)
	newargs="$newargs $arg"
	speclang=yes
	;;
      -*)
	# Pass other options through; they don't need -x and aren't inputs.
	newargs="$newargs $arg"
	;;
      *)
	havefiles=yes
	# If file ends in .c or .i, put options around it.
	# But not if a specified -x option is currently active.
	temp=`expr $arg : '.*\.[ci]$'`
	if [ \( x$temp != x0 \) -a \( x$speclang = x \) ]
	then
	  newargs="$newargs -xc++ $arg -xnone"
	else
	  newargs="$newargs $arg"
	fi
	;;
    esac
  fi
done

if [ x$havefiles = x ]
then
  echo "$0: no input files specified"
  exit 1
fi

progname=`echo $0 |sed -e 's|/[^/]*$|/gcc|'`
if [ -f ${progname} ]
then
exec ${progname} $newargs $library
else
exec gcc $newargs $library
fi

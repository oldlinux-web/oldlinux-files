#!/bin/sh
#
# This file outputs a string of text which are CPP defines helping to
# describe the operating system version.  We just take guesses by
# looking at random files.

# Test for shared libraries (this is pretty Sunish).
if test -f /lib/ld.so; then SHLIB=-DHAVE_SHARED_LIBS; fi

# Get name of maintainer.
if test -f /usr/ucb/whoami; then
  WHOAMI=`whoami`@`hostname`;
else
  WHOAMI=`who am i | awk '{ print $1; }'`
fi

if [ "$WHOAMI" = "" ]; then 
  WHOAMI=`id | sed 's/uid=[01-9]*(//' | sed 's/) [) A-Za-z01-9(=,]*//'`
  if test -f /bin/hostname; then
    WHOAMI=$WHOAMI@`/bin/hostname`
  elif test -f /usr/bin/uuname; then
    WHOAMI=`uuname`!$WHOAMI
  fi
fi

#
# Is this a Xenix system?
#
if [ -f /xenix ]; then
  SYSDEF="-DXenix"
 case "`uname -p`" in
  *286) SYSDEF="-DXenix286" ;;
  *386) SYSDEF="-DXenix386" ;;
 esac
fi

echo "-DCPP_MAINTAINER=${WHOAMI} ${SHLIB} ${SYSDEF}"

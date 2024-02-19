#!/bin/sh
# installation of gcc 2.xxx

# do something to kernel souce
kernelstuff ()
{
  workdir=$1
  srcdir=$2

# we may have something in there.

  for d in asm linux; do
    if [ -d /usr/include/${d} ]; then
      for f in /usr/include/${d}/*.h; do
	if [ -f $f ]; then 
	  mv $f ${srcdir}/include/$d
	fi
      done
      /bin/rm -rf /usr/include/${d}
    fi
  done
}

# install jump table
jumptable ()
{
  DIR=$1
  VER=$2
  REL=$3
  M=$4

  mv $DIR/libc.so.${REL} /lib
  mv $DIR/lib${M}.so.${REL} /lib/libm.so.${REL}

  ln -s -f /lib/libc.so.${REL} /lib/libc.so.${VER}
  ln -s -f /lib/libm.so.${REL} /lib/libm.so.${VER}
}

checkreturn ()
{
  if [ $? != 0 ]
  then
    echo Failed to $1.
    exit 1
  fi
}

fixfiles ()
{
  VERBOSE=$1
  VER=$2
  BINS=$3

  if [ $VERBOSE = yes ]; then set -x; fi

  cd /usr/bin
  chown bin:bin $BINS
  chmod 755 $BINS

  chown -R bin:bin /usr/include /usr/g++-include /lib /usr/lib/gcc-lib
  chmod -R 755 /lib /usr/lib/gcc-lib

  # this is for the header files
  chmod 755 /usr/include /usr/g++-include
  # Yes, 'find' would be cleaner and faster, but it is not on rootdisk
  cd /usr/include
  chmod 775 *
  chmod 664 *.h
  chmod 664 */*
  cd /usr/g++-include
  chmod 775 *
  chmod 664 *.h
  chmod 664 */*

  chmod 664 /usr/lib/gcc-lib/i386-linux/${VER}/*.?
  chmod 664 /usr/lib/gcc-lib/i386-linux/${VER}/shared/*.?
}

# check if it is a directory.
checkdir ()
{
  if [ x${1}x = xx ]
  then
    echo No directory to check.
    return 1
  fi

  for f in $*
  do
    if [ ! -d $f ]; then
      echo There is no such a directory, $f.
      echo Please make sure there is one and it has right stuffs in it.
      exit 1
    fi
  done
  return 0
}

checkbinfile ()
{
  for f in $*
  do
    if [ ! -f /bin/$f -a ! -f /usr/bin/$f ]; then
      echo There is no such a file, $f.
      return 1
    fi
  done
  return 0
}

checkfile ()
{
  if [ x${1}x = xx ]
  then
    echo No file to check.
    return 1
  fi

  for f in $*
  do
    if [ ! -f $f ]; then
      echo There is no such a file, $f.
      echo Please make sure there is one and it has right stuffs in it.
      exit 1
    fi
  done
  return 0
}

checkroot ()
{
  uid=`id | sed -e 's/uid=\([0-9]*\)/\1/' -e 's/[^a-z0-9=].*//'`
  if [ $? != 0 ]
  then
    echo Fail to get \"root\" id. You may need a working \"id\" and
    echo \"sed\".
    exit 1
  fi

  if [ x${uid}x != x0x ]
  then
    echo You have to be root to run this script.
    exit 1
  fi
}

usage ()
{
  echo Usage: sh $1 [-v] version {kernel src dir}
  echo Note: \"sh\" must be zsh or bash.
  exit 1
}

checkroot

VERBOSE=no

if [ $# != 2 -a $# != 3 ]
then
  usage $0
fi

case $1 in
-v)
  shift
  set -x
  VERBOSE=yes
  ;;
esac

if [ $# != 2 ]
then
  usage $0
fi

VERSION=$1
echo Installing gcc ${VERSION} ......

# kernel src dir
KERNELSRCDIR=$2

#ln is not on rootdisk, let's use cp
#LN="cp -s"
#HDLN="cp -l"
LN="ln -s"
HDLN="ln"
MV="mv -f"
RM=/bin/rm
PREFIX=/usr
#LN="echo ln"
#MV="echo mv"
#PREFIX=/local
LIBDIR=${PREFIX}/lib/gcc-lib/i386-linux/${VERSION}
SHAREDLIBDIR=${LIBDIR}/shared
JUMPLIBDIR=${LIBDIR}/jump
SHIMAGEDIR=/lib

# change it by hand
JUMP_VERSION=2
JUMP_RELEASE=${JUMP_VERSION}.2

#create directories first

if [ -f $SHIMAGEDIR ]
then
  echo $SHIMAGEDIR should be directory. Remove it first!
  exit 1
fi

if [ ! -d $SHIMAGEDIR ]
then
  mkdir $SHIMAGEDIR
 if [ $? != 0 ]
 then
   echo Unable to create directory: $SHIMAGEDIR. You may have to create
   echo some paths in $SHIMAGEDIR first by hand.
   exit 1
  fi
fi

echo gcc ${VERSION} will only work with a 387 or the kernel above 0.95.
# ask if there is a 387.
echo "Do you have a 387? [y/n] "
read MATH
if [ x${MATH}x = "xyx" ]
then
  MATH=hard
else
  MATH=soft
fi

echo Check if we get the files.
WORKDIR=`echo ${VERSION}*`
if [ "${WORKDIR}" = "${VERSION}""*" ]
then
  echo No files to install. You need at least
  echo 1. ${VERSION}misc.tar.Z
  echo 2. ${VERSION}db.tar.Z
  echo "3. ${VERSION}lib.tar.Z"
  if [ ${VERSION} = 2.2.2d ]; then
    echo "4. binutils.tar.Z"
  else
    echo "4. binutils.tar.Z (optional)"
  fi
  echo "5. xxxinc.tar.Z (header files also used by kernel source)"
  exit 1
fi

KERNELINC=`echo *inc.ta*`
if [ "${KERNELINC}" = "*"inc.ta"*" ]
then
  echo You need xxxinc.tar.Z for the whole set of the header files
  echo to use gcc ${VERSION}.
  exit 1
fi

echo Check ${VERSION}db.tar.Z and ${VERSION}misc.tar.Z
checkfile ${VERSION}db.tar.Z ${VERSION}misc.tar.Z

# check those
BINS="as86 ld86 ar as gprof ld nm objdump ranlib size strip"
echo Check binaries.
checkbinfile "${BINS}"
if [ $? != 0 -o ${VERSION} = 2.2.2d ]
then
  checkfile binutils.tar.Z
fi

LIBFILE=

# optional in gcc 2.2.2d. false.
if [ ${VERSION} != xxxxxxx ]
then
  checkfile ${VERSION}lib.tar.Z
  LIBFILE=${VERSION}lib.tar.Z
else
  # maybe it has gcc 2.2.2 file.
  if [ -f 2.2.2lib.tar.Z ]; then
    BBFILE=2.2.2lib.tar.Z
  else
    OLDLIBDIR=/usr/lib/gcc-lib/i386-linux/2.2.2
    # it neens gcc 2.2.2d file.
    if [ ! -d ${OLDLIBIR} ]; then
      checkfile ${VERSION}lib.tar.Z
      LIBFILE=${VERSION}lib.tar.Z
    else
      # can I use gcc 2.2.2
      if [ ! -f ${OLDLIBDIR}/cc1 -o ! -f ${OLDLIBDIR}/cc1plus ]
      then
	checkfile ${VERSION}lib.tar.Z
	LIBFILE=${VERSION}lib.tar.Z
      fi
    fi
  fi
fi

# double check the kernel sources
checkdir $KERNELSRCDIR
checkdir $KERNELSRCDIR/include/linux $KERNELSRCDIR/include/asm

WORKDIR=`pwd`

cd /usr

# optional
if [ -f ${WORKDIR}/binutils.tar.Z ]
then
  echo installing binutils.tar.Z
#  tar xvvofz ${WORKDIR}/binutils.tar.Z
  zcat ${WORKDIR}/binutils.tar.Z | tar xvvof -
fi

# optional
if [ x${LIBFILE}x != xx ]
then
  echo installing ${LIBFILE}
# tar xvvofz ${WORKDIR}/${LIBFILE}
  zcat ${WORKDIR}/${LIBFILE} | tar xvvof -
fi

echo installing ${VERSION}db.tar.Z.
#tar xvvofz ${WORKDIR}/${VERSION}db.tar.Z
zcat ${WORKDIR}/${VERSION}db.tar.Z | tar xvvof -

echo installing ${VERSION}misc.tar.Z.
#tar xvvofz ${WORKDIR}/${VERSION}misc.tar.Z
zcat ${WORKDIR}/${VERSION}misc.tar.Z | tar xvvof -

$RM -rf /usr/include/const.h /usr/include/asm /usr/include/linux

echo installing ${KERNELINC}
#tar xvvofz ${WORKDIR}/${KERNELINC}
zcat ${WORKDIR}/${KERNELINC} | tar xvvof -

$RM -rf /usr/include/sys/asy.h

if [ ${VERSION} = xxxxx ]
then
  if [ ! -f ${LIBDIR}/cc1 -o ! -f ${LIBDIR}/cc1plus ]
  then
    if [ x${OLDLIBDIR}x = xx ]
    then
      echo While installing gcc 2.2.2d, you don't have cc1/cc1plus
      echo and you don't have gcc 2.2.2. This installation script is
      echo wrong. Please use
      echo "	sh $0 -v verson [kernel src dir]"
      echo "	Note: \"sh\" must be zsh or bash."
      echo to see what is wrong and report it to hlu@eecs.wsu.edu.
      echo Thanks.
      exit 1
    else
      $HDLN ${OLDLIBDIR}/cc1 ${OLDLIBDIR}/cc1plus ${LIBDIR}
    fi
  fi
fi

#checkfile $LIBDIR/lib$MATH.a
checkfile $LIBDIR/lib$MATH.a \
	$LIBDIR/cpp $JUMPLIBDIR/libc.so.${JUMP_RELEASE} \
	$JUMPLIBDIR/lib${MATH}.so.${JUMP_RELEASE}

# not used
#	$SHAREDLIBDIR/libc.${VERSION} 
#	$SHAREDLIBDIR/lib${MATH}.${VERSION}

if [ -f $SHIMAGEDIR/libc.${VERSION} -o -f $SHIMAGEDIR/libm.${VERSION} ]
then
  echo There is a libc.${VERSION} or libm.${VERSION} in $SHIMAGEDIR.
  echo Do you want to deleted them? [y/n]
  read answer
  if [ x${answer}x = "xyx" ]
  then
    $RM -f $SHIMAGEDIR/libc.${VERSION} $SHIMAGEDIR/libm.${VERSION}
  else
    echo Give up.
    exit 1
  fi
fi

if [ -f $SHIMAGEDIR/libc.so.${JUMP_RELEASE} -o -f $SHIMAGEDIR/libm.so.${JUMP_RELEASE} ]
then
  echo There is a libc.so.${JUMP_RELEASE} or libm.so.${JUMP_RELEASE} in $SHIMAGEDIR.
  echo Do you want to deleted them? [y/n]
  read answer
  if [ x${answer}x = "xyx" ]; then
    $RM -f $SHIMAGEDIR/libc.so.${JUMP_RELEASE} $SHIMAGEDIR/libm.so.${JUMP_RELEASE}
  else
    echo Give up.
    exit 1
  fi
fi

#echo moving the shared image.
#$MV $SHAREDLIBDIR/libc.${VERSION} $SHIMAGEDIR
#$MV $SHAREDLIBDIR/lib${MATH}.${VERSION} $SHIMAGEDIR/libm.${VERSION}

$RM -f $LIBDIR/libm.a
$HDLN $LIBDIR/lib$MATH.a $LIBDIR/libm.a
checkreturn "$HDLN $LIBDIR/lib$MATH.a $LIBDIR/libm.a"

#$HDLN $SHAREDLIBDIR/lib$MATH.a $SHAREDLIBDIR/libm.a
#checkreturn "$HDLN $SHAREDLIBDIR/lib$MATH.a $SHAREDLIBDIR/libm.a"

echo install jump table.
jumptable $JUMPLIBDIR $JUMP_VERSION $JUMP_RELEASE $MATH

$RM -rf /usr/lib/cpp /lib/cpp

$LN $LIBDIR/cpp /usr/lib/cpp
checkreturn "$LN $LIBDIR/cpp /usr/lib/cpp"
$LN $LIBDIR/cpp /lib/cpp
checkreturn "$LN $LIBDIR/cpp /lib/cpp"

if [ -f /usr/include/stddef.h -o -f /usr/include/stdarg.h ]
then
  echo The version-dependent stddef.h and stdarg.h should not be in
  echo /usr/include. Please take a look at
  echo "	/usr/lib/gcc-lib/i386-linux/${VERSION}/include"
  exit 1
fi

if [ ! -f /usr/include/time.h ]
then
  echo You have to install xxxxinc.tar.Z first!
  exit 1
fi

echo "Do you have kernel source on line? [y/n] "
read MATH
if [ $MATH = "n" ]
then
   echo Oops. gcc ${VERSION} needs at least the header files from
   echo the kernel source, i.e. linux/include/linux.
   exit 1
fi

echo Ignore the error message.

$RM -rf ${KERNELSRCDIR}/include/*.h  ${KERNELSRCDIR}/include/sys
if [ -f ${KERNELSRCDIR}/include/a.out.h ]
then
  echo 'Fail to clean ${KERNELSRCDIR}/include/*.h and'
  echo '${KERNELSRCDIR}/include/sys.'
  exit 1
fi

checkdir ${KERNELSRCDIR}/include/asm ${KERNELSRCDIR}/include/linux

kernelstuff $WORKDIR $KERNELSRCDIR

$LN  ${KERNELSRCDIR}/include/asm /usr/include
if [ $? != 0 ]
then
   echo Fail to make a link from ${KERNELSRCDIR}/include/asm to
   echo /usr/include/asm.
   exit 1
fi

$LN ${KERNELSRCDIR}/include/linux /usr/include
if [ $? != 0 ]
then
   echo Fail to make a link from ${KERNELSRCDIR}/include/linux to
   echo /usr/include/linux.
   exit 1
fi

echo No more error messages.

echo Fixing the files.
fixfiles ${VERBOSE} ${VERSION} "${BINS}"

# for future use
# gcc -V 2.xxx will read this file.
cd $LIBDIR
chmod 755 /usr/bin/gcc
/usr/bin/gcc -dumpspecs > specs

if [ $? != 0 ]
then
  echo Error during the installation of gcc ${VERSION}
  exit 1
fi

echo Done! gcc ${VERSION} has been installed.
echo Please get rid of all the \"-nostdinc -I\" from Makefiles in
echo kernel source and then do a fresh "make dep".

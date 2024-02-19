#!/bin/sh

#
# Locate the stdio directory
#

swd=`cd ..; pwd`

#
# Make the work directory
#

if test ! -d libc ; then
  mkdir libc
else
  rm -f libc/*
fi
cd libc
cwd=`pwd`

#
# Check for recompilation
#
echo -n 'Do you want to recompile affected library files? [yn] '
read RECOMPILE

#
# Update the floating point library
#
if test -f /usr/lib/libfp.a ; then
  echo Updating floating point library...
  ar x /usr/lib/libfp.a
  cp ${swd}/_f_*.s .
  if test -f ${swd}/Hidden ; then
    cp ${swd}/_u_*print*.s ${swd}/_u_*scan*.s .
  fi
  lorder *.s | tsort > FPorder
  ar cq libfp.a `cat FPorder`
  rm -f *.s
fi

#
# Touch and make all affected files in the old library
#
ANSI="assert system "
OTHER="curses getopt getpass lock nlist popen regexp regsub termcap "
POSIX="getlogin "
cd /usr/src/lib
if test "${RECOMPILE}" = "y" ; then
  cd ansi
  for g in `echo ${ANSI}` ; do
    touch ${g}.c
    make f=${g}
  done
fi
if test "${RECOMPILE}" = "y" ; then
  cd ../other
  for g in `echo ${OTHER}` ; do
    touch ${g}.c
    make f=${g}
  done
fi
if test "${RECOMPILE}" = "y" ; then
  cd ../posix
  for g in `echo ${POSIX}` ; do
    touch ${g}.c
    make f=${g}
  done
fi
cd ${cwd}

#
# Find the compiler support files
#
echo Finding compiler support modules...
ar pv /usr/lib/libc.a | libupack | sed -n -e '
:start
/^p - / {
s/^p - //
h
n
b start
}
/^ *\.define  *[^_]/ {
x
/^..*$/ p
s/.*//
x
n
b start
}' > c.tmp
ar x /usr/lib/libc.a `cat c.tmp`
rm -f c.tmp

#
# Copy all the library files
#
echo -n Copying Minix library...
echo -n ' ansi'
cp /usr/src/lib/ansi/*.s .
echo -n ' other'
cp /usr/src/lib/other/*.s .
echo -n ' posix'
cp /usr/src/lib/posix/*.s .
echo -n ' ibm'
cd /usr/src/lib/ibm
for f in *.s ; do
  libpack < ${f} > ${cwd}/${f}
done
echo ''
cd ${cwd}
rm -f crtso.s prtso.s head.s end.s

#
# Delete the superseded files
#
echo Superseding stdio modules...
rm -f exit.s fclose.s fflush.s fgetc.s fgets.s fopen.s fprintf.s \
      fputc.s fputs.s fread.s freopen.s fseek.s ftell.s fwrite.s \
      gets.s perror.s puts.s scanf.s setbuf.s sprintf.s tmpnam.s \
      ungetc.s vsprintf.s cleanup.s doprintf.s printdat.s ctermid.s \
      cuserid.s fdopen.s

#
# Copy the new stdio files
#
echo Copying new stdio modules...
cp ${swd}/[_a-wz]*.s .
rm -f _f_*.s

#
# Generate the order
#
echo Generating library order...
lorder *.s | tsort > Order

#
# Make library
#
echo Generating library...
rm -f libc.a
ar cq libc.a `cat Order`
rm -f *.s

#!/bin/sh

#
# Name source and target files
#
MAKEFILEG=../GMakefile
MAKEFILE=../Makefile
ERRLISTG=../errlist.g
ERRLISTH=../errlist.h
POWERSG=../powers.g
POWERSH=../powers.h
LMRG=../lmr.g
LMRH=../lmr.h
SITEG=../site.g
SITEH=../site.h
STDIOG=../stdio.g
STDIOH=../stdio.h

#
# IFS manipulation
#
SIFS="${IFS}"

#
# Check for DOS environment
#
if echo ${PATH} | grep ';' >&- ; then
  NULL="nul"
  RM=`which rm`
  SYS="DOS"
else
  NULL="/dev/null"
  RM="/bin/rm"
  SYS="UNIX"
fi

#
# Determine how to echo without trailing new line
#
if echo '\c' | grep . >${NULL} ; then
  ECHO='echo -n'
  NONL=
else
  ECHO=echo
  NONL='\c'
fi

#
# Check for non-interactive mode
#
if tty -s ; then
  RESPONSE=":"
else
  RESPONSE="echo"
fi

#
# Nice heading to show what's about to happen
#
echo ""
echo "Stdio Installation Script"

echo ""
${ECHO} "Do you wish to use a preconfigured site description? [yn] ${NONL}"
read YN
${RESPONSE} "${YN}"
if test "${YN}" = "y" ; then
  echo ""
  for f in `ls *.sit` ; do
    if grep \^`basename $f .sit` site.txt ; then :
    else
      echo "$f	No description"
    fi
  done
  echo ""
  DESC=""
  while test "${DESC}" = "" ; do
    ${ECHO} "Select a site description: ${NONL}"
    read DESC
    ${RESPONSE} "${DESC}"
    if test ! -f "${DESC}.sit" ; then
      echo "${DESC}: invalid site name"
      DESC=""
    fi
  done
  echo ""
  echo "Generating ${STDIOH}..."
  cp ${STDIOG} ${STDIOH}
  echo "Generating ${SITEH}..."
  ${ECHO} "${NONL}" > ${SITEH}
  echo "Generating ${MAKEFILE}..."
  ${ECHO} "${NONL}" > ${MAKEFILE}
  echo "Generating ${ERRLISTH}..."
  ${ECHO} "${NONL}" > ${ERRLISTH}
  echo "Generating ${POWERSH}..."
  ${ECHO} "${NONL}" > ${POWERSH}
  echo "Generating ${LMRH}..."
  ${ECHO} "${NONL}" > ${LMRH}
  patch -d.. -s + -s + -s + -s + -s < ${DESC}.sit
  exit 0
fi

echo ""
${ECHO} "Do you wish to create a new site description? [yn] ${NONL}"
read YN
${RESPONSE} "${YN}"
if test "${YN}" != "y" ; then
  exit 0
fi

echo ""
echo "Respond with ? for help at any prompt"
echo ""

#
# Get the name of this site
#
SITENAME="?"
while test "${SITENAME}" = "?" ; do
  ${ECHO} "Type the name to label this site description: [?] ${NONL}"
  read SITENAME
  ${RESPONSE} "${SITENAME}"
  if test "${SITENAME}" = "?" ; then
    cat <<EOF
This is the name which will be used to generate a .sit file. The makefile
will be edited so that make site will generate a site description of the
correct name.
EOF
  fi
done

#
# Find the makefile
#
echo ""
echo "Makefiles available:"
echo ""
if test -d /usr/include/minix ; then
  BASEMAKE=makefile.min
else
  BASEMAKE=makefile.nix
fi
(cd ..; ls makefile.*) | grep -v makefile.cpp | pr -3 -l1 -t -w60
echo ""
${ECHO} "Enter your choice of makefile (default: ${BASEMAKE}): ${NONL}"
read YN
${RESPONSE} "${YN}"
if test ! -z "${YN}" ; then
  BASEMAKE="${YN}"
fi
rm -f ${MAKEFILEG}
ln ../${BASEMAKE} ${MAKEFILEG}

#
# Determine where to find include files
#
echo ""
echo "Searching for the target include directory..."
INCLUDE=/usr/include
if test -d ${INCLUDE} 2>${NULL} ; then
  YN="?"
  while test "${YN}" = "?" ; do
    ${ECHO} "Is it acceptable to use ${INCLUDE}? [yn?] ${NONL}"
    read YN
    ${RESPONSE} "${YN}"
    if test "${YN}" = "?" ; then
      cat <<EOF
This is the directory in which the include files for the
target environment (not the host environment).
EOF
    fi
  done
else
  YN="n"
fi
if test "${YN}" != "y" ; then
  INCLUDE="?"
  while test "${INCLUDE}" = "?" ; do
    ${ECHO} "Type in your include directory: [?] ${NONL}"
    read INCLUDE
    ${RESPONSE} "${INCLUDE}"
    if test "${INCLUDE}" = "?" ; then
      cat <<EOF
This is the include directory for the target environment. For
example, when cross compiling for Minix under DOS, the DOS
include directory is in c:/tc/include, and the Minix include
directory is in /usr/include. I would respond with '/usr/include'
to the prompt when installing stdio into Minix and 'c:/tc/include'
when installing stdio into DOS.
EOF
    else
      if test ! -d "${INCLUDE}" ; then
	echo "Directory ${INCLUDE} is not accessible"
	INCLUDE="?"
      fi
    fi
  done
fi

#
# Determine where to find C preprocessor
#
echo ""
echo "Searching for host's cpp..."
if test -f /lib/cpp ; then
  CPP="/lib/cpp -P -I${INCLUDE}"
else
  CPP="/usr/lib/cpp -P -I${INCLUDE}"
fi
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to use ${CPP}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
The C preprocessor is used to strip comments from header
files when trying to determine whether some features are
available in the host environment. The preprocessor should
be able to accept input from stdin and produce output on
stdout.
EOF
  fi
done
if test "${YN}" != "y" ; then
  CPP="?"
  while test "${CPP}" = "?" ; do
    ${ECHO} "Type in your cpp command: [?] ${NONL}"
    IFS=""
    read CPP
    ${RESPONSE} "${CPP}"
    IFS="${SIFS}"
    if test "${CPP}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to run
the C preprocessor with input from stdin and output sent
to stdout. Typically a command like:

	/lib/cpp

would be used to do this.
EOF
    fi
  done
fi

#
# Determine where to find host C compiler
#
echo ""
echo "Searching for host's cc..."
CC="cc -o "
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to compile and load using ${CC}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
The host cc compiler is the one used to compile and load C programs
to be run on this machine under this operating system.
EOF
  fi
done
if test "${YN}" != "y" ; then
  CC="?"
  while test "${CC}" = "?" ; do
    ${ECHO} "Type in your host's cc command: [?] ${NONL}"
    IFS=""
    read CC
    ${RESPONSE} "${CC}"
    IFS="${SIFS}"
    if test "${CC}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to compile
foo.c to foo. Typically the command

	cc -o foo foo.c
	
would be used to do this. If this were the case, you would
respond with 'cc -o ' to the prompt (note the space).

Under the DOS environment I use the command:

	tcc -A -Ic:/tc/include -Lc:/tc/lib -efoo foo.c

so I respond to this prompt with 'tcc -A -Ic:/tc/include -Lc:/tc/lib -e'
(note the lack of a space).
EOF
    fi
  done
fi

#
# Determine where to find target C compiler
#
echo ""
echo "Searching for target's cc..."
TCC="${CC}"
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to compile and load using ${TCC}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
The target cc compiler is the one used to compile programs
for the target machine. It should be the same physical
program as the host compiler but with different switches.

If stdio is being installed for this operating system, then
the target and host compiler commands will be the same. If
you are cross compiling, the target command will probably
specify different include directories and the like.
EOF
  fi
done
if test "${YN}" != "y" ; then
  TCC="?"
  while test "${TCC}" = "?" ; do
    ${ECHO} "Type in your target's compile and load cc command: [?] ${NONL}"
    IFS=""
    read TCC
    ${RESPONSE} "${TCC}"
    IFS="${SIFS}"
    if test "${TCC}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to compile
foo.c in this environment to foo for execution in the target
environment. Typically a command like:

	cc -I/cross/include -o foo foo.c
	
would be used to do this. If this were the case, you would
respond with 'cc -I/cross/include -o ' to the prompt (note
the space). The resulting executable will not be executed.
This command will only be used to check that programs can
be compiled successfully.

Under the DOS environment that I use to cross compile for
Minix, I use

	mcc -o foo foo.c

to compile Minix programs, so I would respond to the prompt
with 'mcc -o ' (note the space).
EOF
    fi
  done
fi

#
# Determine how to compile to intermediate code
#
TCCc=`echo "${TCC}" | sed -e 's/ *-[a-z] *$/ -c /'`
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to compile only using ${TCCc}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
This is the command given to the target C compiler to compile
but not load, a program.
EOF
  fi
done
if test "${YN}" != "y" ; then
  TCCc="?"
  while test "${TCCc}" = "?" ; do
    ${ECHO} "Type in your target's compile only cc command: [?] ${NONL}"
    IFS=""
    read TCCc
    ${RESPONSE} "${TCCc}"
    IFS="${SIFS}"
    if test "${TCCc}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to compile
foo.c to foo.o. Typically the command

	cc -c foo.c
	
would be used to do this. If this were the case, you would
respond with 'cc -c ' to the prompt (note the space).

Under the DOS environment I use the command:

	tcc -A -Ic:/tc/include -Lc:/tc/lib -c foo.c

so I respond to this prompt with 'tcc -A -Ic:/tc/include -Lc:/tc/lib -c '
(note the space).
EOF
    fi
  done
fi

#
# Determine if floating point support required
#
echo ""
echo "Floating point support..."
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Do you want floating point support? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
If floating point support is required, extra floating point modules will
be compiled. It may be necessary to place these in a special maths library
which is included at link time.
EOF
  fi
done
FPCODE="${YN}"

#
# Determine how to load floating point library code
#
echo ""
echo "Searching for host's floating point library..."
if test -f /lib/libm.a -o -f /usr/lib/libm.a ; then
  FPLIB="-lm"
else
  FPLIB=""
fi
YN="?"
if test "${FPCODE}" != "y" ; then
  echo "No floating point library."
  YN="y"
  FPLIB=""
fi
while test "${YN}" = "?" ; do
  if test -z "${FPLIB}" ; then
    ${ECHO} "No floating point library. Is this acceptable? [yn?] ${NONL}"
    read YN
    ${RESPONSE} "${YN}"
  else
    ${ECHO} "Is it acceptable to use ${FPLIB}? [yn?] ${NONL}"
    read YN
    ${RESPONSE} "${YN}"
  fi
  if test "${YN}" = "?" ; then
    cat <<EOF
This is the command line argument required to be concatenated to the end
of a compiler invocation command to link in the floating point library code
to a program.
EOF
  fi
done
if test "${YN}" != "y" ; then
  FPLIB="?"
  while test "${FPLIB}" = "?" ; do
    ${ECHO} "Type the floating point library command line switch: [?] ${NONL}"
    IFS=""
    read FPLIB
    ${RESPONSE} "${FPLIB}"
    IFS="${SIFS}"
    if test "${FPLIB}" = "?" ; then
      cat <<EOF
This is the command line switch required to load the floating point library
code. On most systems it would be -lm. Enter an empty line if there is no
floating point library to load.
EOF
    fi
  done
fi

#
# Determine how to compile floating point
#
echo ""
echo "Searching for host's floating point cc..."
if test -d ${INCLUDE}/minix ; then
  FPCC="cc -f -o "
else
  FPCC="${CC}"
fi
YN="?"
if test "${FPCODE}" != "y" ; then
  echo "No floating point support."
  YN="y"
  FPCC=""
fi
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to compile and load using ${FPCC}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
This is the command line require to compile and load floating point
C code to be run on this machine under this operating system.
EOF
  fi
done
if test "${YN}" != "y" ; then
  FPCC="?"
  while test "${FPCC}" = "?" ; do
    ${ECHO} "Type in your host's floating point compile and load cc command: [?] ${NONL}"
    IFS=""
    read FPCC
    ${RESPONSE} "${FPCC}"
    IFS="${SIFS}"
    if test "${FPCC}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to compile
and load a program that contains floating point code.
EOF
    fi
  done
fi

#
# Determine target's fp compile command
#
echo ""
echo "Searching for target's floating point cc..."
if test "${FPCC}" = "${CC}" ; then
  TFPCC="${TCC}"
else
  TFPCC="${FPCC}"
fi
YN="?"
if test "${FPCODE}" != "y" ; then
  echo "No floating point support."
  YN="y"
  TFPCC=""
fi
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to compile and load using ${TFPCC}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
This is the command line require to compile and load floating point
C code to be run on the target machine.
EOF
  fi
done
if test "${YN}" != "y" ; then
  TFPCC="?"
  while test "${TFPCC}" = "?" ; do
    ${ECHO} "Type in your target's floating point compile and load cc command: [?] ${NONL}"
    IFS=""
    read TFPCC
    ${RESPONSE} "${TFPCC}"
    IFS="${SIFS}"
    if test "${TFPCC}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to compile and load
a program that contains floating point code for the target machine.
EOF
    fi
  done
fi
if test "${FPCC}" = "${CC}" ; then
  TFPCCc=`echo "${TCCc}" | sed -e 's/ *-[a-z] *$/ -c /'`
else
  TFPCCc=`echo "${FPCC}" | sed -e 's/ *-[a-z] *$/ -c /'`
fi
YN="?"
if test "${FPCODE}" != "y" ; then
  YN="y"
  TFPCCc=""
fi
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to compile only using ${TFPCCc}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
This is the command line require to compile floating point but not load
C code to be run on the target machine.
EOF
  fi
done
if test "${YN}" != "y" ; then
  TFPCCc="?"
  while test "${TFPCCc}" = "?" ; do
    ${ECHO} "Type in your target's floating point compile only cc command: [?] ${NONL}"
    IFS=""
    read TFPCCc
    ${RESPONSE} "${TFPCCc}"
    IFS="${SIFS}"
    if test "${TFPCCc}" = "?" ; then
      cat <<EOF
This is the command you would type if you wanted to compile, but not load
a program that contains floating point code for the target machine.
EOF
    fi
  done
fi

#
# Ask for any additional definitions
#
echo ""
echo "Additional definitions..."
if test -d /usr/include/minix ; then
  DEFINES="_MINIX"
else
  DEFINES=`sed -n -e '/^#'${SITENAME}' / {
s/^#[^ ]* //
p
n
}' < ${MAKEFILEG}`
fi
if test -z "${DEFINES}" ; then
  DEFINES=-D_`echo ${SITENAME} | tr '[a-z]' '[A-Z]'`
else
  DEFINES="-D${DEFINES}"
fi
DEFINES="${DEFINES} -D_POSIX_SOURCE"
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to use ${DEFINES}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
The additional definitions will be used when the C preprocessor
is invoked by the makefile.
EOF
  fi
done
if test "${YN}" != "y" ; then
  DEFINES="?"
  while test "${DEFINES}" = "?" ; do
    ${ECHO} "Type in the definitions: [?] ${NONL}"
    IFS=""
    read DEFINES
    ${RESPONSE} "${DEFINES}"
    IFS="${SIFS}"
    if test "${DEFINES}" = "?" ; then
      cat <<EOF
Extra definitions will probably include things like a feature
test macro for the current site. For example, _MSDOS may be
required to test for a DOS site, so '-D_MSDOS' would be
typed.
EOF
    fi
  done
fi

#
# Ask how to build a library
#
echo ""
echo "Checking on library construction..."
echo ""
echo "    1  Use ar and ranlib"
echo "    2  Use ar, lorder and tsort"
echo "    3  Use ar only"
echo ""
if test -f /bin/ranlib -o -f /usr/bin/ranlib ; then
  LIBTYPE=1
else
  if test -f /bin/lorder -o -f /usr/bin/lorder ; then
    LIBTYPE=2
  else
    LIBTYPE=3
  fi
fi
echo "Your site appears to use $LIBTYPE."
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is this acceptable? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
This information will be used to modify the makefile to suit this site. The
makefile will then be able to libraries containing stdio. If the guess is
incorrect, respond in the negative and you will be prompted for the correct
combination.
EOF
  fi
done
if test "${YN}" != "y" ; then
  LIBTYPE="?"
  while test "${LIBTYPE}" = "?" ; do
    ${ECHO} "Enter the combination code: ${NONL}"
    read LIBTYPE
    ${RESPONSE} "${LIBTYPE}"
    if test "${LIBTYPE}" != "3" -a "${LIBTYPE}" != "2" -a "${LIBTYPE}" != "1"
    then
      LIBTYPE="?"
      cat <<EOF
The only permissible combinations are the ones listed above. If your site
has different requirements you will have to edit the makefile manually.
EOF
    fi
  done
fi
if test "${LIBTYPE}" = "1" ; then
  RANLIB=ranlib
  LORDER=echo
  TSORT=cat
fi
if test "${LIBTYPE}" = "2" ; then
  RANLIB=":"
  LORDER=lorder
  TSORT=tsort
fi
if test "${LIBTYPE}" = "3" ; then
  RANLIB=":"
  LORDER=echo
  TSORT=cat
fi

#
# Ask for the ack hack
#
if test -d ${INCLUDE}/minix ; then
  ACKHACK="?"
  echo ""
  echo "Checking for ACK NULL hack..."
  while test "${ACKHACK}" = "?" ; do
    ${ECHO} "Do you want stdio.h to #undef NULL? [yn?] ${NONL}"
    read ACKHACK
    ${RESPONSE} "${ACKHACK}"
    if test "${ACKHACK}" = "?" ; then
      cat <<EOF
The Minix ack compiler doesn't understand about ((void *) 0). Users of this
compiler will have to force NULL to be defined as (0). This is done by
including stdio.h as the last file in all programs and allowing stdio.h to
redefine any existing version of NULL.
EOF
    fi
  done
fi

#
# Find a good definition for NULL
#
echo ""
echo "Searching for a definition for NULL..."
STDIONULL="((void *) 0)"
if test -f ${INCLUDE}/stddef.h ; then
  STDIONULL=`${CPP} -DUSESTDDEF <ynull.c | grep '^Null' | sed -e 's/^Null //'`
else
  if test -f ${INCLUDE}/stdio.h ; then
    STDIONULL=`${CPP} -DUSESTDIO <ynull.c | grep '^Null' | sed -e 's/^Null //'`
  fi
fi
YN="?"
while test "${YN}" = "?" ; do
  ${ECHO} "Is it acceptable to use ${STDIONULL}? [yn?] ${NONL}"
  read YN
  ${RESPONSE} "${YN}"
  if test "${YN}" = "?" ; then
    cat <<EOF
NULL is used as an alias for the null pointer. The definition used by
stdio.h should be compatible with those in the other include files.
EOF
  fi
done
if test "${YN}" != "y" ; then
  STDIONULL="?"
  while test "${STDIONULL}" = "?" ; do
    ${ECHO} "Type in your definition for NULL: [?] ${NONL}"
    IFS=""
    read STDIONULL
    ${RESPONSE} "${STDIONULL}"
    IFS="${SIFS}"
    if test "${STDIONULL}" = "?" ; then
      cat <<EOF
This definition for NULL will be inserted into the stdio.h file. It should
be compatible with the definitions in the other include files.
EOF
    fi
  done
fi

#
# Ask for hidden files
#
HIDE="?"
echo ""
echo "Checking for ANSI name hiding..."
while test "${HIDE}" = "?" ; do
  ${ECHO} "Do you want to hide the standard names? [yn?] ${NONL}"
  read HIDE
  ${RESPONSE} "${HIDE}"
  if test "${HIDE}" = "?" ; then
    cat <<EOF
ANSI requires that the standard names be hidden so that the user will not
inadvertently redefine say, vfprintf, and corrupt the operation of stdio.
When the standard names are hidden, stdio will use the hidden versions of
the names. Small assembler stubs are provided to map the visible names
onto the hidden ones. Modification to stub.x may be required if you are
porting stdio to a new host.
EOF
   fi
done

#
# Ask for debugging assertions
#
DEBUG="?"
echo ""
echo "Checking for debugging assertions..."
while test "${DEBUG}" = "?" ; do
  ${ECHO} "Do you want to generate debugging assertions? [yn?] ${NONL}"
  read DEBUG
  ${RESPONSE} "${DEBUG}"
  if test "${DEBUG}" = "?" ; then
    cat <<EOF
The ASSERT() macro is used to generate assertions in the library. These
are useful while debugging the stdio library to ensure assumed conditions
do indeed hold. Stable versions can have this code removed so that
executables will be smaller and faster.
EOF
   fi
done

#
# Edit the Makefile
#
echo ""
echo "Editing ${MAKEFILE}..."
if test -f /bin/cdiff ; then
  CDIFF="cdiff"
else
  CDIFF="diff -c"
fi
if cmp -s ${MAKEFILEG} ../makefile.nix ; then
  sed -e "s~__SITE__~${SITENAME}~g
s~__CPP__~${CPP}~g
s~__CC__~${TCCc}~g
s~__FPCC__~${TFPCCc}~g
s~__LD__~${TCC}~g
s~__FPLD__~${TFPCC}~g
s~__DEFINES__~${DEFINES}~g
s~__MATHLIB__~${FPLIB}~g
s~__RANLIB__~${RANLIB}~g
s~__LORDER__~${LORDER}~g
s~__TSORT__~${TSORT}~g
s~__CDIFF__~${CDIFF}~g" <${MAKEFILEG} >makefile.tmp
  rm -f ${MAKEFILE} ${MAKEFILEG}
  mv makefile.tmp ${MAKEFILE}
else
  rm -f ${MAKEFILE}
  cp ${MAKEFILEG} ${MAKEFILE}
  rm -f ${MAKEFILEG}
fi

#
# Initialise stdio.g modifications
#
echo ""
echo "Creating ${STDIOH}..."
if ${CC}yuchar yuchar.c 1>${NULL} ; then : else exit 1; fi
if ${CC}yumask yumask.c 1>${NULL} ; then : else exit 1; fi
if ${CC}ysizet ysizet.c 1>${NULL} ; then : else exit 1; fi
YARG="${INCLUDE}/stdarg.h"
if test ! -f ${YARG} ; then
  YARG="${INCLUDE}/varargs.h"
fi
VALIST=`grep '^[ 	]*typedef[ 	].*[^A-Za-z0-9_]va_list' <${YARG} | head -1 | sed 's/[ 	]*typedef[ 	]*\(.*[^A-Za-z0-9_]\)va_list[^A-Za-z0-9_]*/\1/'`
if ./yuchar ; then
  UCHAR='0'
else
  UCHAR=`./yumask`
fi
SIZET=`grep 'typedef.*size_t' ${INCLUDE}/sys/types.h | head -1 | sed 's/[ 	]*typedef[ 	]*\([^;]*\)[ 	]size_t[ 	]*;/\1/'`
USIZET=`./ysizet`
cat >ystdio <<'EOF'
/\/\*-\*\// a
/*@*/
EOF
if test "${ACKHACK}" = "y" ; then
  echo '#undef NULL' | tee -a ystdio
fi
cat <<EOF | tee -a ystdio
#ifndef NULL
# define NULL	${STDIONULL}
#endif
#define _STDIO_UCHAR_		${UCHAR}
#define _STDIO_VA_LIST_		${VALIST}
#define _STDIO_SIZE_T_		${SIZET}
#define _STDIO_USIZE_T_		${USIZET}
EOF
sed s/\$/\\\\/ <ystdio >ystdio1
echo '/*=*/' >>ystdio1
sed -f ystdio1 <${STDIOG} | sed /\\/\\\*+\\\*\\//,/\\/\\\*-\\\*\\//d >${STDIOH}
${RM} -f ystdio ystdio1

#
# Initialise configuration specification
#
${ECHO} "${NONL}" >${SITEH}

#
# Generate debugging flag
#
echo ""
echo "Include debugging assertions..."
if test "${DEBUG}" = "y" ; then
  echo "/*efine NDEBUG*/" | tee -a ${SITEH}
else
  echo "#define NDEBUG" | tee -a ${SITEH}
fi

#
# Generate hidden flag
#
echo ""
echo "Hide stdio functions..."
if test "${HIDE}" = "y" ; then
  echo "#define HIDDEN" | tee -a ${SITEH}
else
  echo "/*efine HIDDEN*/" | tee -a ${SITEH}
fi

#
# Generate floating point support flag
#
echo ""
echo "Floating point support..."
if test "${FPCODE}" = "y" ; then
  echo "/*efine NOFLOAT*/" | tee -a ${SITEH}
else
  echo "#define NOFLOAT" | tee -a ${SITEH}
fi

#
# System error message table
#
echo ""
echo "Checking for sys_errlist[] and perror() clash..."
cat <<'EOF' >ygerror.sed
: scan
/^#define[ 	][ 	]*E[^-][^-]*\/\*/ {
s/^#define[ 	][ 	]*\(E[A-Z_0-9]*\)[^0-9]*\([0-9][0-9]*\).*/\1 \2/
p
n
b scan
}
n
b scan
EOF
cat <<'EOF' >yperror.sed
# Scan for first error
: start
/^#define[ 	][ 	]*E[^0-9-][^0-9-]*1[^0-9-]*\/\*/ b scan
s/.*//
N
s/^.//
b start
: scan
/^#define[ 	][ 	]*E[^-][^-]*\/\*/ {
# Extract the comment
s/^#define[ 	]*\(E[A-Z_0-9]*\)[^0-9]*\([0-9][0-9]*\).*\/\*[ 	]*\(.*\)[ 	]*\*\//,{ \2 , "\1" , %\3/
s/[ 	]*$//
# Delete meta-comments
s/\(%.*\)[ 	]*(.*)/\1/
s/%.*:[ 	]*/%/
# Make into string
s/%\(.*\)/"\1"}/
p
n
b scan
}
s/.*//
N
s/^.//
b scan
EOF
YN="y"
${TCC}yperror yperror.c 1>${NULL} 2>${NULL}
if test $? -eq 0 ; then
  ${TCC}yserror yserror.c 1>${NULL} 2>${NULL}
  if test $? -ne 0 ; then
    YN="n"
  fi
fi
if test -d /usr/include/minix ; then
  YN="y"
fi
if test "${YN}" = "n" ; then
  echo "/*efine ERRLIST*/" | tee -a ${SITEH}
else
  if test "${TCC}" = "${CC}" ; then
    if ${CC}ygerror ygerror.c 1>${NULL} ; then : ; else exit 1; fi
    sed -n -f ygerror.sed < ${INCLUDE}/errno.h | ./ygerror >${ERRLISTH}
  else
    sed -n -f yperror.sed < ${INCLUDE}/errno.h > yerrlist.h
    if ${CC}yerrlist yerrlist.c 1>${NULL} ; then : ; else exit 1; fi
    ./yerrlist >${ERRLISTH}
  fi
  echo "#define ERRLIST" | tee -a ${SITEH}
fi
${RM} -f yperror yperror.sed yperror.s yerrlist.h yerrlist
${RM} -f ygerror ygerror.sed ygerror.dat yserror yserror.s

#
# Check out the system
#
echo ""
echo "Checking for ANSI conformant compiler environment..."
STDC=0
${CC}ystdc ystdc.c 1>${NULL} 2>${NULL}
if test $? -eq 0 ; then
  STDC=1
  cat <<EOF | tee -a ${SITEH}
#define STDARG
#define LIMITS
#define FLOAT
EOF
fi
if test ${STDC} -eq 0 ; then
  if test -f ${INCLUDE}/stdarg.h ; then
    echo "#define STDARG" | tee -a ${SITEH}
  else
    echo "/*efine STDARG*/" | tee -a ${SITEH}
  fi
fi
if test ${STDC} -eq 0 ; then
  if ${CC}ylimits ylimits.c 1>${NULL} ; then : ; else exit 1; fi
  if test -f ${INCLUDE}/limits.h ; then
    echo "#define LIMITS" | tee -a ${SITEH}
    if grep -s "[ 	]CHAR_BIT" <${INCLUDE}/limits.h ; then
      echo "/*efine CHAR_BIT*/" | tee -a ${SITEH}
    else
      ./ylimits CHAR_BIT | tee -a ${SITEH}
    fi
    if grep -s "[ 	]UINT_MAX" <${INCLUDE}/limits.h ; then
      echo "/*efine UINT_MAX*/" | tee -a ${SITEH}
    else
      ./ylimits UINT_MAX | tee -a ${SITEH}
    fi
    if grep -s "[ 	]ULONG_MAX" <${INCLUDE}/limits.h ; then
      echo "/*efine ULONG_MAX*/" | tee -a ${SITEH}
    else
      ./ylimits ULONG_MAX | tee -a ${SITEH}
    fi
    if grep -s "[ 	]INT_MAX" <${INCLUDE}/limits.h ; then
      echo "/*efine INT_MAX*/" | tee -a ${SITEH}
    else
      ./ylimits INT_MAX | tee -a ${SITEH}
    fi
    if grep -s "[ 	]LONG_MAX" <${INCLUDE}/limits.h ; then
      echo "/*efine LONG_MAX*/" | tee -a ${SITEH}
    else
      ./ylimits LONG_MAX | tee -a ${SITEH}
    fi
  else
    echo "/*efine LIMITS*/" | tee -a ${SITEH}
    ./ylimits | tee -a ${SITEH}
  fi
fi
if test "${FPCODE}" = "y" ; then
  if ${FPCC}yfloat yfloat.c 1>${NULL} ; then : ; else exit 1; fi
  if test -f ${INCLUDE}/float.h ; then
    if test ${STDC} -eq 0 ; then
      echo "#define FLOAT" | tee -a ${SITEH}
    fi
    if grep -s "[ 	]DBL_DIG" <${INCLUDE}/float.h ; then
      echo "/*efine DBL_DIG*/" | tee -a ${SITEH}
    else
      ./yfloat DBL_DIG | tee -a ${SITEH}
    fi
    if grep -s "[ 	]LDBL_DIG" <${INCLUDE}/float.h ; then
      echo "/*efine LDBL_DIG*/" | tee -a ${SITEH}
    else
      ./yfloat LDBL_DIG | tee -a ${SITEH}
    fi
    if grep -s "[ 	]DBL_MAX_10_EXP" <${INCLUDE}/float.h ; then
      echo "/*efine DBL_MAX_10_EXP*/" | tee -a ${SITEH}
    else
      ./yfloat DBL_MAX_10_EXP | tee -a ${SITEH}
    fi
    if grep -s "[ 	]LDBL_MAX_10_EXP" <${INCLUDE}/float.h ; then
      echo "/*efine LDBL_MAX_10_EXP*/" | tee -a ${SITEH}
    else
      ./yfloat LDBL_MAX_10_EXP | tee -a ${SITEH}
    fi
    if grep -s "[ 	]LDBL_MAX" <${INCLUDE}/float.h ; then
      echo "/*efine LDBL_MAX*/" | tee -a ${SITEH}
    else
      ./yfloat LDBL_MAX | tee -a ${SITEH}
    fi
    if grep -s "[ 	]DBL_MAX" <${INCLUDE}/float.h ; then
      echo "/*efine DBL_MAX*/" | tee -a ${SITEH}
    else
      ./yfloat DBL_MAX | tee -a ${SITEH}
    fi
    if grep -s "[ 	]FLT_RADIX" <${INCLUDE}/float.h ; then
      echo "/*efine FLT_RADIX*/" | tee -a ${SITEH}
    else
      ./yfloat FLT_RADIX | tee -a ${SITEH}
    fi
  else
    echo "/*efine FLOAT*/" | tee -a ${SITEH}
    ./yfloat | tee -a ${SITEH}
  fi
fi
${RM} -f ystdc ystdc.exe ystdc.obj
${RM} -f ystdarg ystdarg.exe ystdarg.obj
${RM} -f ylimits ylimits.exe ylimits.obj
${RM} -f yfloat yfloat.exe yfloat.obj
${RM} -f yumask yumask.exe yumask.obj
${RM} -f yuchar yuchar.exe yuchar.obj
${RM} -f ysizet ysizet.exe ysizet.obj

echo ""
echo "Checking for HUGE_VAL in math.h..."
if grep -s "[ 	]HUGE_VAL" <${INCLUDE}/math.h ; then
  echo "/*efine HUGE_VAL*/" | tee -a ${SITEH}
else
  if grep -s "[ 	]HUGE" <${INCLUDE}/math.h ; then
    echo "#define HUGE_VAL HUGE" | tee -a ${SITEH}
  else
    echo "#define HUGE_VAL LDBL_MAX" | tee -a ${SITEH}
  fi
fi

echo ""
echo "Checking compiler for correct (int) ((unsigned char) (x)) casts..."
if test ${UCHAR} = '0' ; then
  echo "#define UNSIGNEDCHAR" | tee -a ${SITEH}
else
  echo "/*efine UNSIGNEDCHAR*/" | tee -a ${SITEH}
fi

echo ""
echo "Checking for memcpy(), memchr() and memset() functions..."
if test -f ${INCLUDE}/memory.h ; then
  cat <<EOF | tee -a ${SITEH}
#include <memory.h>
#define MEMORY
/*efine MYMEMCPY*/
/*efine MYMEMCHR*/
/*efine MYMEMSET*/
EOF
else
  if grep -s memcpy ${INCLUDE}/string.h 2>${NULL} ; then
  cat <<EOF | tee -a ${SITEH}
#define MEMORY
/*efine MYMEMCPY*/
/*efine MYMEMCHR*/
/*efine MYMEMSET*/
EOF
  else
    echo "/*efine MEMORY*/" | tee -a ${SITEH}
    ${TCC}ybcopy ybcopy.c 1>${NULL} 2>${NULL}
    if test $? -eq 0 ; then
      cat <<EOF | tee -a ${SITEH}
void bcopy();
#define MEMCPY(a,b,c) bcopy((b),(a),(c))
/*efine MYMEMCPY*/
EOF
    else
      echo "#define MYMEMCPY" | tee -a ${SITEH}
    fi
    cat <<EOF | tee -a ${SITEH}
#define MYMEMCHR
#define MYMEMSET
EOF
  fi
fi
${RM} -f ybcopy ybcopy.exe

echo ""
echo "Checking for string.h functions..."
if test -f ${INCLUDE}/string.h ; then
  echo "#define STRING" | tee -a ${SITEH}
else
  echo "/*efine STRING*/" | tee -a ${SITEH}
  if test -f ${INCLUDE}/strings.h ; then
    echo "#include <strings.h>" | tee -a ${SITEH}
  fi
fi

echo ""
echo "Checking for careless tolower..."
grep '#define[ 	][ 	]*tolower[^a-zA-Z0-9_]' <${INCLUDE}/ctype.h >ytolower
if test $? -ne 0 ; then
  if test "${PROTOTYPE}" != "()" ; then
    echo "int tolower(int);" | tee -a ${SITEH}
  else
    echo "int tolower();" | tee -a ${SITEH}
  fi
  echo "#define TOLOWER(c) tolower(c)" | tee -a ${SITEH}
else
  if grep isupper <ytolower >${NULL} ; then
    echo "#define TOLOWER(c) tolower(c)" | tee -a ${SITEH}
  else
    echo "#define TOLOWER(c) (isupper((c))?tolower((c)):(c))" | tee -a ${SITEH}
  fi
fi
rm -r ytolower

echo ""
echo "Checking for 3 argument opens..."
YOPEN="/*efine OPEN3*/"
if grep O_CREAT <${INCLUDE}/fcntl.h >${NULL} ; then
  if grep O_TRUNC <${INCLUDE}/fcntl.h >${NULL} ; then
    if grep O_APPEND <${INCLUDE}/fcntl.h >${NULL} ; then
      YOPEN="#define OPEN3"
    fi
  fi
fi
echo "${YOPEN}" | tee -a ${SITEH}

echo ""
echo "Checking for rename function..."
${TCC}yrename yrename.c 1>${NULL} 2>${NULL}
if test $? -eq 0 ; then
  echo "#define RENAME" | tee -a ${SITEH}
else
  echo "/*efine RENAME*/" | tee -a ${SITEH}
fi
${RM} -f yrename yrename.o yrename.exe

echo ""
echo "Checking for STDIN_FILENO in unistd.h..."
if grep STDIN_FILENO ${INCLUDE}/unistd.h 1>${NULL} 2>${NULL} ; then
  echo "#define UNISTD" | tee -a ${SITEH}
else
  echo "/*efine UNISTD*/" | tee -a ${SITEH}
  echo "#define STDIN_FILENO 0" | tee -a ${SITEH}
  echo "#define STDOUT_FILENO 1" | tee -a ${SITEH}
  echo "#define STDERR_FILENO 2" | tee -a ${SITEH}
fi

echo ""
echo "Checking for mode_t in fcntl.h..."
if grep creat ${INCLUDE}/fcntl.h 2>${NULL} | ${CPP} | grep -s mode_t ; then
  echo "#define MODE_T mode_t" | tee -a ${SITEH}
else
  echo "/*efine MODE_T mode_t*/" | tee -a ${SITEH}
fi

echo ""
echo "Checking for uid_t in pwd.h..."
if grep getpwuid ${INCLUDE}/pwd.h 2>${NULL} | ${CPP} | grep -s uid_t ; then
  echo "#define UID_T uid_t" | tee -a ${SITEH}
else
  echo "/*efine UID_T uid_t*/" | tee -a ${SITEH}
fi

echo ""
echo "Checking for void (*signal())()..."
if grep SIG_DFL ${INCLUDE}/signal.h | grep void 1>${NULL} ; then
  echo "#define VOIDSIGNAL" | tee -a ${SITEH}
else
  echo "/*efine VOIDSIGNAL*/" | tee -a ${SITEH}
fi

echo ""
echo "Checking sys/types.h and time.h for Posix typedefs..."
${ECHO} "${NONL}" > ytypes
if test -f ${INCLUDE}/sys/types.h ; then
  echo "#define SYSTYPES" | tee -a ${SITEH}
else
  echo "/*efine SYSTYPES*/" | tee -a ${SITEH}
fi
grep off_t ${INCLUDE}/sys/types.h >${NULL}
if test $? -ne 0 ; then
  echo "typedef long off_t;			/* best guess */" >>ytypes
fi
grep pid_t ${INCLUDE}/sys/types.h >${NULL}
if test $? -ne 0 ; then
  echo "typedef int pid_t;			/* best guess */" >>ytypes
fi
grep uid_t ${INCLUDE}/sys/types.h >${NULL}
if test $? -ne 0 ; then
  echo "typedef int uid_t;			/* best guess */" >>ytypes
fi
grep time_t ${INCLUDE}/sys/types.h >${NULL}
if test $? -ne 0 ; then
  if grep time_t <${INCLUDE}/time.h >${NULL} ; then
    echo "#define TIME" >>ytypes
  else
    echo "/*efine TIME*/" >>ytypes
    echo "typedef long time_t;		/* best guess */" >>ytypes
  fi
fi
grep mode_t ${INCLUDE}/sys/types.h >${NULL}
if test $? -ne 0 ; then
  echo "typedef int mode_t;		/* best guess */" >>ytypes
fi
if grep size_t ${INCLUDE}/sys/types.h >${NULL} ; then
  grep _SIZE_T ${INCLUDE}/sys/types.h >${NULL}
  if test $? -ne 0 ; then
    echo "#define _SIZE_T" >>ytypes
  fi
fi
tee -a ${SITEH} <ytypes
${RM} -f ytypes

if test "${FPCODE}" != "y" ; then
  echo "/*efine DENORMAL*/" | tee -a ${SITEH}
  echo "/*efine TRUNCATE*/" | tee -a ${SITEH}
  echo "/*efine LMR*/"      | tee -a ${SITEH}
  echo "/*efine POWERS*/"   | tee -a ${SITEH}
fi

if test "${FPCODE}" = "y" ; then
  echo ""
  echo "Checking floating point denormalisation..."
  if ${FPCC}ydenorm ydenorm.c ${FPLIB} 1>${NULL} ; then : ; else exit 1; fi
  if ./ydenorm ; then
    echo "#define DENORMAL" | tee -a ${SITEH}
  else
    echo "/*efine DENORMAL*/" | tee -a ${SITEH}
  fi
  rm -f ydenorm
fi

if test "${FPCODE}" = "y" ; then
  echo ""
  echo "Checking floating point to integer conversion..."
  if ${FPCC}ytrunc ytrunc.c ${FPLIB} 1>${NULL} ; then : ; else exit 1; fi
  if ./ytrunc ; then
    echo "#define TRUNCATE" | tee -a ${SITEH}
  else
    echo "/*efine TRUNCATE*/" | tee -a ${SITEH}
  fi
  rm -f ytrunc
fi

if test "${FPCODE}" = "y" ; then
  echo ""
  echo "Generating LDBL_MAX / FLT_RADIX..."
  if ${FPCC}ylmr ylmr.c ${FPLIB} 1>${NULL} ; then : ; else exit 1; fi
  ./ylmr > ${LMRH}
  echo "#define LMR" | tee -a ${SITEH}
  rm -f ylmr
fi

if test "${FPCODE}" = "y" ; then
  echo ""
  echo "Generating power table..."
  if ${FPCC}ypow10 ypow10.c ${FPLIB} 1>${NULL} ; then : ; else exit 1; fi
  SHIFT=`./ypow10 0 0 1 0 | sed -n -e '/bit/ !d
s/[^0-9]*\([1-9][0-9]*\) bit.*/\1/
p
'`
  SHIFT=`expr ${SHIFT} \* 3 / 2`
  echo "#define NEGEXPONENT	-${SHIFT}" > ${POWERSH}
  echo "#define POSEXPONENT	0"         >> ${POWERSH}
  echo "#ifdef SMALL" >> ${POWERSH}
  echo "#undef SMALL" >> ${POWERSH}
  ./ypow10 g -1 -15 -1 0 >> ${POWERSH}
  ./ypow10 0 15 1 0 >> ${POWERSH}
  echo "#endif" >> ${POWERSH}
  echo "#ifdef LARGE" >> ${POWERSH}
  echo "#undef LARGE" >> ${POWERSH}
fi
if test "${FPCODE}" = "y" ; then
  if test -f ${INCLUDE}/float.h ; then
    ./ypow10 g -16 \
	       `( grep LDBL_MAX_10_EXP ${SITEH}; \
		  grep LDBL_MAX_10_EXP ${INCLUDE}/float.h; \
                  echo "-LDBL_MAX_10_EXP" ) | ${CPP} | \
		  sed -n -e '$s/[ 	]//g' -e '$p'` \
               -16 ${SHIFT} >> ${POWERSH}
    ./ypow10 g 16 \
               `( grep LDBL_MAX_10_EXP ${SITEH}; \
		  grep LDBL_MAX_10_EXP ${INCLUDE}/float.h; \
                  echo "LDBL_MAX_10_EXP" ) | ${CPP} |  \
		  sed -n -e '$s/[ 	]//g' -e '$p'` \
             16 0 >> ${POWERSH}
  fi
fi
if test "${FPCODE}" = "y" ; then
  if test ! -f ${INCLUDE}/float.h ; then
    ./ypow10 g -16 \
               `( grep LDBL_MAX_10_EXP ${SITEH}; \
                  echo "-LDBL_MAX_10_EXP" ) | ${CPP} | \
		  sed -n -e '$s/[ 	]//g' -e '$p'` \
             -16 ${SHIFT} >> ${POWERSH}
    ./ypow10 g 16 \
               `( grep LDBL_MAX_10_EXP ${SITEH}; \
                  echo "LDBL_MAX_10_EXP" ) | ${CPP} | \
		  sed -n -e '$s/[ 	]//g' -e '$p'` \
             16 0 >> ${POWERSH}
  fi
fi
if test "${FPCODE}" = "y" ; then
  echo "#endif" >> ${POWERSH}
  echo "#define POWERS" | tee -a ${SITEH}
fi
rm -f ypow10

exit 0

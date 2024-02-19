:
# $Id: Makefile.SH,v 2.0 90/09/19 20:07:16 paul Rel $
#
# Creates man/Makefile for getty distribution
#
# $Log:	Makefile.SH,v $
# Revision 2.0  90/09/19  20:07:16  paul
# Initial 2.0 release
# 
# 

case $CONFIG in
'')
    if test ! -f config.sh; then
	ln ../config.sh . || \
	ln ../../config.sh . || \
	ln ../../../config.sh . || \
	(echo "Can't find config.sh."; exit 1)
    fi
    . config.sh
    ;;
esac
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

gtab=`basename $gettytab`
all="getty.1m $gtab.4 issue.4"
man="getty.man $gtab.man issue.man"
case "$d_getutent" in
"$undef")
    all="$all getutent.3"
    man="$man getutent.man"
    ;;
esac
case "$d_strdup" in
"$undef")
    all="$all strdup.3"
    man="$man strdup.man"
    ;;
esac
case "$d_putenv" in
"$undef")
    all="$all putenv.3"
    man="$man putenv.man"
    ;;
esac

echo "Extracting man/Makefile (with variable substitutions)"
$spitshell >Makefile <<!GROK!THIS!
#
# Makefile for getty man pages
#

SHELL=	/bin/sh
# no nroff?  get Henry Spencer's AWF!
ROFF=	nroff -man

SRC=	..
TUNE=	\$(SRC)/tune.h
RELEASE=\$(SRC)/release.h

# what to make
all:	$all

man:	$man

install:
	@echo "Manpage installation must be done manually, Sorry."

# how to make it
.SUFFIXES: .m4 .3 .man

.m4.3:
	m4 \$*.m4 > \$*.3

.3.man:
	\$(ROFF) \$*.3 > \$*.man

# dependencies
config.m4:	\$(SRC)/config.sh \$(TUNE) \$(RELEASE)
	chmod +x makeconfig
	./makeconfig

getty.1m:	getty.m4 config.m4
	m4 getty.m4 > getty.1m

getty.man:	getty.1m
	\$(ROFF) getty.1m > getty.man

$gtab.4:	gettytab.m4 config.m4
	m4 gettytab.m4 > $gtab.4

$gtab.man:	$gtab.4
	\$(ROFF) $gtab.4 > $gtab.man

issue.4:	issue.m4 config.m4
	m4 issue.m4 > issue.4

issue.man:	issue.4
	\$(ROFF) issue.4 > issue.man

getutent.3:	getutent.m4 config.m4

getutent.man:	getutent.3

strdup.3:	strdup.m4 config.m4

strdup.man:	strdup.3

putenv.3:	putenv.m4 config.m4

putenv.man:	putenv.3

clean:
	rm -f *.out config.m4 config.sh core

clobber: clean
	rm -f *.1m *.[34] *.man

!GROK!THIS!
chmod 644 Makefile
$eunicefix Makefile

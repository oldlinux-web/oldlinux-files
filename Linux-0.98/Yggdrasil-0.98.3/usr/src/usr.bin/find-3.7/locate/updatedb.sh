#!/bin/sh
# updatedb -- build locate pathname database
# csh original by James Woods; sh conversion by David MacKenzie.
# Public domain.

PATH=/bin:/usr/bin # Just temporarily.

# You can set these in the environment to override their defaults:

# Non-network directories to put in the database.
test -z "$SEARCHPATHS" && SEARCHPATHS="/"

# Network directories to put in the database.
test -z "$NFSPATHS" && NFSPATHS=

# Pathnames that match this regular expression are not searched.
test -z "$PRUNEREGEX" &&
  PRUNEREGEX='\(^/tmp$\)\|\(^/usr/tmp$\)\|\(^/var/tmp$\)'

# User(s) to mail error messages about `sort' overflows to.
test -z "$FINDHONCHO" && FINDHONCHO="root"

# User to search network directories as.
test -z "$NFSUSER" && NFSUSER=daemon

# Directory to hold intermediate files.
test -z "$TMPDIR" && TMPDIR=/usr/tmp

# The database file to build.
test -z "$LOCATE_DB" && LOCATE_DB=@LOCATE_DB@


# These you probably won't need to override:

# The directory containing the subprograms.
LIBDIR=@libdir@

# The directory containing find.
BINDIR=@bindir@

# Any prefix in the GNU find filename, before `find'.
binprefix=@binprefix@

PATH=$LIBDIR:$BINDIR:/usr/ucb:/bin:/usr/bin export PATH

bigrams=$TMPDIR/f.bigrams$$
filelist=$TMPDIR/f.list$$
errs=$TMPDIR/f.errs$$
trap 'rm -f $bigrams $filelist $errs' 0
trap 'rm -f $bigrams $filelist $errs; exit' 1 15

# Make a file list.  Alphabetize `/' before any other char with `tr'.

{
if test -n "$SEARCHPATHS"; then
  ${binprefix}find $SEARCHPATHS \
  \( -fstype nfs -o -type d -regex "$PRUNEREGEX" \) -prune -o -print
fi
if test -n "$NFSPATHS"; then
  su $NFSUSER -c \
  "${binprefix}find $NFSPATHS \\( -type d -regex \"$PRUNEREGEX\" -prune \\) -o -print"
fi
} | tr / '\001' | sort -f 2> $errs | tr '\001' / > $filelist

# Compute common bigrams.

bigram < $filelist | sort 2>> $errs | uniq -c | sort -nr |
  awk '{ if (NR <= 128) print $2 }' | tr -d '\012' > $bigrams

# Code the file list.

if test -s $errs; then
  echo 'updatedb: out of sort space' | mail $FINDHONCHO
else
  code $bigrams < $filelist > $LOCATE_DB
  chmod 644 $LOCATE_DB
fi

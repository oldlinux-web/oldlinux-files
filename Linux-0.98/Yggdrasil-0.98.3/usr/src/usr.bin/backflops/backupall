#!/bin/sh

# What I like to do with this is place it in /etc/passwd like this:
# backup::0:0::/:/usr/local/bin/backupall
# Then all I have to do is type 'backup' at a login prompt
# and it backs up my system. Just a frill.

PATH=$PATH:`dirname $0`; export PATH
set -x
exec backflops /

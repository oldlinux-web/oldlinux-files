Welcome to Bochs, an 80386 emulator!

Bochs is a portable x86 emulator, which runs Minix-386 and
MS-DOS/Windows.  You can compile and run Bochs on almost
any Unix & X Windows environment.

Since software emulation is very performance sensitive, it is
recommended that you run Bochs on at least a 100Mhz machine,
with a minimum of 32MBytes of physical memory.  Additionally,
you'll need a couple Megabytes of disk space to uncompress and
compile the Bochs distribution, plus 10 to 30 Megabytes for
the creation of hard disk image files, depending upon the option
you choose.

Installation begins with the BOCHS.TAZ file.  It is a compressed
tar format file, of the entire Bochs source distribution.  You
need to compile Bochs for your platform before using it.  To
extract the source code, change directory into a parent directory
in which Bochs will be untarred:

  unix:/> cd /usr/local/src
    -or
  unix:/> cd /users/JohnDoe/src

Uncompress and untar Bochs:

  unix:/usr/local/src> zcat BOCHS.TAZ | tar xvf -

You should now have a sub-directory 'bochs-YYMMDD'.  Change directory
into it, and read the files 'INSTALL.MINIX386' & 'INSTALL' for further
detailed installation notes.  For uses of Bochs, outside of running
the version of Minix on this CD, please consult the file 'LICENSE'.
The file 'INSTALL.DOS_WIN31' may also be of interest.


Thanks for your interest in using Bochs!  Current versions of Bochs
as well as other relevant information may be obtained at the following
Web and ftp sites.  Feel free to give me feedback on portability and
other relevant issues.  I'd like to continually enhance Bochs, and
add to its general portability.

  http://world.std.com/~bochs
  ftp://ftp.std.com/pub/bochs

Kevin Lawton
Bochs Software Company
bochs@world.std.com


WELCOME TO MINIX!   
-----------------

The complete MINIX sources and binaries are located in the MINIX directory.
To install MINIX, please print out the README.TXT and INSTALL.TXT files in
the MINIX directory.  (If you have a PostScript printer, print INSTALL.PS
instead of INSTALL.TXT.)  You are VERY VERY strongly urged to read them from
beginning to end before even starting the installation.  Doing so will save
a lot of grief later.  Then go back and read them carefully as you do the
installation.

The source and binary of the fdvol program are also located here.  Fdvol (or
possibly rawrite3 or wrtdsk90) are needed to transfer files from the CD ROM
to floppies and then to the MINIX hard disk.

Before starting to install MINIX, you must do two things.  First, you must
have a supply of (at least 5, preferably 10) formatted floppy disks on hand.
It does not matter what is on them, but they must be formatted.

To run MINIX from a hard disk, you also need a free partition of at least
25 MB, preferably at least 30 MB.  Note that this must be one of the four
primary partitions, not an MS-DOS extended partition.  You can see how your
primary partitions are allocated by running the MS-DOS fdisk program.

If you have enough space, but all of it is in a large MS-DOS partition, it
is possible to split the MS-DOS partition into a smaller MS-DOS partition
and an empty partition for installing MINIX.  To do this, first run scandisk
to repair any errors on the MS-DOS partition. Then remove your Windows swap
file, if any.  Next, run defrag to compact the partition.  Once these steps
have been performed, you can use fips to split the partition. For all the
details, go to the DOSUTILS\FIPS directory on this CD ROM and read the
following files: README.1st, README.TXT, FIPS.DOC, FIPS.FAQ, and SPECIAL.DOC.
These files will explain splitting partitions in great detail.  In theory,
no files should be lost by spliting but just to be on the safe side, please
back up all the files on your MS-DOS partition before splitting.

The MINIX distribution consists of a number of .TAZ files.  These are
compressed MINIX tar files.  To install them, you must first transfer them
to your hard disk, then decompress and de-archive them.  Getting them to the
hard disk is done via the blank floppies mentioned above.  Since MS-DOS
does not provide any way to copy a file to a sequence of floppies, byte for
byte, we have provided a program, fdvol.exe, in this directory and in MINIX
for that purpose.  As an example, if later on you need to put the SYS.TAZ
file on a set of 1.44 MB floppies using the A: drive, you would change to the
MINIX directory and type:

        fdvol 1440 A: SYS.TAZ

Fdvol will prompt you to insert floppies until it is done.  Be sure to label
each floppy with the file name and the floppy number.  For 360K, 720K, or
1.2 MB floppies, use 360, 720, or 1200 as the first parameter, respectively.
You can also use the B: drive if you prefer by replacing the second
parameter.

In summary:
    1. Go get a set of 10 formatted floppies.
    2. Partition your hard disk to have a free primary partition > 25 MB.
    3. Read README.TXT and INSTALL.TXT in the MINIX directory (in that order).


For the current state of the MINIX system, see:

    The MINIX web page:	 http://www.cs.vu.nl/~ast/minix.html
    The MINIX newsgroup: comp.os.minix



OTHER SOFTWARE ON THIS CD ROM
-----------------------------
In addition to the basic MINIX system, this CD ROM also contains the
following directories:

        LISTING   - The full source code listing in several formats
        DOSUTILS  - Some MS-DOS programs to help you install MINIX
        COMPILER  - Some user-contributed compilers
        NETUTILS  - Some user-contributed networking utilities
        DEBUGGER  - A user-contributed MINIX debugger
        BOCHS     - A 386 interpreter for running MINIX on a UNIX system
        SMX       - A version of MINIX that runs as a user program on a SPARC

Each of these directories contains a README.TXT file describing installing
and use of the files.  Except for the files in the LISTING directory, all the
other software and documentation was produced by third parties.  We are
including it here in the hope it may be of interest to some MINIX users,
but we have not tested it and are in no way responsible for it.  Comments
should be directed to the respective authors.

SIMULATING MINIX
----------------
     In some situations, you may prefer to run MINIX on non-Intel computers.
We have provided two ways to do this.  The simulator in BOCHS is a 386
interpreter.  It also simulates the most common I/O devices, so you can run
MINIX on top of it.  Bochs is written in C and runs on any machine that
has a C compiler and supports X Windows.  The disadvantage of any interpreter,
of course, is a performance loss due to the interpretation of every 386
instruction.

     An alternative simulator is located in the SMX directory.  It
uses a different principle: MINIX is run as a user program on top of Solaris.
This program works only on SPARCs, but it allows MINIX to run at full
speed because there is no interpretation. The MINIX binary run this way
is not identical to normal MINIX because it does not run on the bare
hardware. Nevertheless, it is very close, all things considered.  This
simulator must be compiled with gcc (not the Sun compiler), so a copy of
gcc 2.7.2 is provided on this CD ROM.

     Each simulator contains its own documentation.  Please consult these.


Andy Tanenbaum (ast@cs.vu.nl)
Al Woodhull (awoodhull@hamp.hampshire.edu)
Kees Bot (kjb@cs.vu.nl)





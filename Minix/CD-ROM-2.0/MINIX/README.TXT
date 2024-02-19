This directory contains MINIX version 2.0.0 for the PC in the following
files:

	i386\ROOT	- Minix boot & root floppy image (386 and better)
	i386\USR	- Floppy image with programs needed for installation
	i386\USR.TAZ	- All Minix programs, libraries, etc.
	SYS.TAZ		- System sources and manual pages
	CMD.TAZ		- Commands sources
	INSTALL.TXT	- Installation guide (ASCII text)
	INSTALL.PS	- Installation guide (PostScript)
	MANUALS		- Minix manual pages
	EXAMPLE.TXT	- Example screen images of an installation
        NETINSTL.TXT    - Notes about installing networking
        XT640K.TXT      - Notes about running MINIX on older machines
        
The installation guide can be found in INSTALL.TXT (ASCII text), or
INSTALL.PS (PostScript format).  It describes the Minix installation
procedure from floppies.  Additional manuals are present in the
MANUALS directory.  If INSTALL.TXT refers to another manual page,
boot(8) for instance, then MANUALS\CAT8\BOOT.8 contains boot(8) as
ASCII text, MANUALS\PS8\BOOT.8 is in PostScript, and MANUALS\MAN8\BOOT.8
is the original unformatted text (UNIX nroff).  All these manual
pages can be viewed online under Minix with the man(1) command after
installing the SYS.TAZ set.  Try 'man man' or 'man 8 boot'.  The
EXAMPLE.TXT file may be nice to read to see how Minix is installed.

But before attempting to install you will have to put Minix on
floppies.  For a 386, 486, or Pentium using 1.44 MB floppies on drive
A: you must make four sets of floppies.  Type each of these commands in
turn to make them.  You will be prompted to insert floppies.  You do
not have to install SYS.TAZ and CMD.TAZ initially, but life is much
simpler if you have 9 floppies and make each set right now.  Be sure to
label each floppy with the file name and volume number.

        fdvol 1440 A: i386\ROOT i386\USR    - Combined ROOT+USR floppy
        fdvol 1440 A: i386\USR.TAZ          - Base system on 3 floppies
        fdvol 1440 A: SYS.TAZ               - System sources on 2 floppies
        fdvol 1440 A: CMD.TAZ               - Commands sources on 3 floppies

Use 1200 to specify 1.2M floppies.  Likewise for 720k floppies, except
that you have to put ROOT and USR installation images on two separate
floppies:

        fdvol 720 A: i386\ROOT              - Separate ROOT and USR
        fdvol 720 A: i386\USR
        fdvol 720 A: i386\USR.TAZ          
        fdvol 720 A: SYS.TAZ              
        fdvol 720 A: CMD.TAZ            

The i86 directory contains the installation and base system material to
install the 16-bit version of Minix for old XTs and 286 based ATs.  If
the intended machine has little memory or a 360k drive then read
XT\README.TXT and use the installation floppy images in the XT directory
instead of the regular ROOT and USR.

After you have installed MINIX, you may wish to install some of the other
files on the CD ROM.  The SMX and BOCHS programs are simulators that sun
on UNIX workstations, so you must transfer them there using a modem and
unpack and use them there.  The other .TAZ files should be copied to
floppies using fdvol.  To reconstruct a .TAZ file on a sequence of 1.44 MB
floppies via drive A:, boot MINIX, log in, and type:

        setup dir

Where 'dir' is the directory where a package is to be installed.  Use
'install -d dir' to create the directory if it doesn't yet exist.
The README.TXT files for each package hold more detailed information.


In summary:
    1. You should now have a BOOT floppy (or ROOT + USR floppies).
    2. A set of floppies for USR.TAZ, SYS.TAZ, and CMD.TAZ.
    3. A free hard disk partition, or at least enough space not in any
       partition that you can create a new partition in this space.
    4. Now read INSTALL.TXT and follow it carefully.


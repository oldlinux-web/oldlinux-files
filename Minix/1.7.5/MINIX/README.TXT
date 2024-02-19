The installation guide can be found in INSTALL.TXT

More manual pages are in MAN\CAT*\*
If a manual page refers to, say boot(8), then the manual page for boot
can be found in MAN\CAT8\BOOT.8.  Under Minix you can type 'man 8 boot'
if you have installed the SYS.TAZ set.

For a 386, 486, or Pentium using 1.44 MB floppies on drive A: you must make
four sets of floppies.  Go to the MINIX directory and type each of these
commands in turn.  You will be prompted to insert floppies.  You do not have
to install SYS.TAZ and CMD.TAZ initially, but life is much simpler if you
have 9 floppies and make each set right now.  Be sure to label each floppy
correctly.

        fdvol 1440 A: i386\ROOT i386\USR    (Combined boot floppy)
        fdvol 1440 A: i386\USR.TAZ          (Base system on 3 floppies)
        fdvol 1440 A: SYS.TAZ               (System sources on 2 floppies)
        fdvol 1440 A: CMD.TAZ               (Commands sources on 3 floppies)

Use 1200 to specify 1.2M floppies.  Likewise for 720k floppies, except
that you have to put ROOT and USR installation images on two separate
floppies:

        fdvol 720 A: i386\ROOT              (Separate ROOT and USR)
        fdvol 720 A: i386\USR
        fdvol 720 A: i386\USR.TAZ          
        fdvol 720 A: SYS.TAZ              
        fdvol 720 A: CMD.TAZ            

The i86 directory contains the installation and base system material to
install the 16-bits version of Minix for old XTs and 286 based ATs.  If
the intended machine has little memory or a 360k drive then read
XT\README.TXT and use the installation floppy images in the XT directory
instead of the regular ROOT and USR.

After you have installed MINIX, you may wish to install some of the other
files on the CD ROM.  The SMX and BOCHS programs are simulators that sun
on UNIX workstations, so you must transfer them there using a modem and
unpack and use them there.  The other .TAZ files should be copied to
floppies using fdvol.  To reconstruct a .TAZ file on a sequence of 1.44 MB
floppies via drive A:, boot MINIX, log in, and type:

        vol -r 1440 /dev/fd0 | zcat | tar xf -

Please note that the MINIX program is called vol, not fdvol.
For drive B:, use /dev/fd1 instead of /dev/fd0.  The result will be a
fully-expanded and decompressed file tree.


In summary:
    1. You should now have a BOOT floppy (or ROOT + USR floppies).
    2. A set of floppies for USR.TAZ, SYS.TAZ, and CMD.TAZ.
    3. A free hard disk partition, or at least enough space not in any
       partition that you can create a new partition in this space.
    4. Now read INSTALL.TXT and follow it carefully.

























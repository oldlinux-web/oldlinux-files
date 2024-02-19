This describes a driver for QIC-02 tape drive support for Linux 0.99.8.

NOTE1: This will not work for the QIC-40/QIC-80 tape drives, like the
well-known tape drives that attach to the floppy controller.
NOTE2: If you are working on another (e.g. SCSI or QIC-40) tape driver,
please try to use the same sys/mtio.h file! Also, join the TAPE mailing
list!

This code has not been tested in combination with SCSI driver(s).
If you do, make sure you use different interrupts, dma numbers 
and IO addresses!

+-------------------------------------------------------------+
| WARNING: This code is still beta -- There *are* bugs in it! |
+-------------------------------------------------------------+

NOTE: Make sure you get *all* the required files from this directory.
That means at least the latest *.tar.Z file and any additional patches.

Instructions:

	0) Read all of the README and the current `release' files.

	1) Backup your entire system onto floppies. [Seriously ...]

	2) Get a clean kernel source tree. [No other patches.]
	   Look in the `release.note' file to see which version
	   to use.

	3) Apply the required patches:
		patch -p0 <tpqic02.pat
	   Update the linux/include/linux/tpqic02.h include file to
	   reflect your hardware setup. Be careful to avoid DMA/IO/IRQ
	   conflicts! 

	4) Do a `make config' in order to update the kernel configuration,
	   followed by `make dep' and `make clean'.

	5) Make a new kernel, ON FLOPPY, that boots from floppy.
	   This is for testing, and recovery after a hd-crash.
	   KEEP THIS IN A SAFE PLACE!

	6) Compile GNU 'mt' (with minor patches by me). [Or use the pre-
	   compiled binary.] Put them on the root floppy. Don't forget
	   to link them statically if you're not using shared libs on
	   the root floppy!

	7) Make a new root floppy disk. KEEP THIS IN A SAFE PLACE!
	   [I once had my root fs wiped out -- fortunately it was just
	    a test floppy... Turned out to be a bug in an old floppy driver]

	   If you have >=2 floppy drives, you may want to create 
	   a `bare bones' recovery disk and simply use the standard
	   root disk to boot from, then mount the recovery
	   floppy on the other floppy drive. Make sure you put a copy
	   of the root/boot disks with tape support in a safe place.

	8) Usually the `tar' program on the standard root disk is actually
	   `pax', which may not fully understand all GNU tar special stuff,
	   like volume labels. Check this before it's too late.

	   Make sure you have a good tar program. Check that it
	   supports symbolic links. GNU tar is useful because it
	   adds Volume headers (-V) and multivolume support, but it may
	   not fit on your root disk.
	   
	9) Make the device files [Check tpqic02.h for the minor bits]
		mknod /dev/rmt0 c 12 4		# QIC-24
		mknod /dev/rmt8 c 12 6		# QIC-120
		mknod /dev/rmt16 c 12 8		# QIC-150
		mknod /dev/tape c 12 8		# Or whatever appropriate
		mknod /dev/tape-d c 12 `expr 128 + 8`	# ditto
		mknod /dev/tape-reset c 12 255	# For resetting only
		mknod /dev/tape-nodens c 12 0	# Without setting density

	   Make /dev/tape a copy of the device file you prefer as
	   the default device. The names /dev/rmt? are used mainly for
	   historic reasons. (i.e. GNU tar uses them.)
	   NOTE: When using a drive without `set density' command,
	   you should use minor number 0. Beware that lots of programs
	   will default to /dev/tape, especially `mt'. Use of a tape
	   device with any minor number other than 0 will force a `set
	   density' command. Some drives may be unable to recognize other
	   formats after they have received a `set density' command.

	   The actual major number (currently 12) may be different.

	10) insert a blank tape and boot the new kernel.

	11) do a 'mt rewind'. Then the big test:
		tar -cvf /dev/tape /
	    If your fs is slow, the drive will not `stream' continuously.
	    In that case, a buffering program should be used. Unfortunately,
	    that's not available (yet?) for Linux. In the meantime, try:
		tar -b 3000 -cvf /dev/tape /

	    [Newsflash: You may want to try ``edd''. The source for
	     edd can be found in ftp.mei.co.jp:/free/others/edd.tar.Z.]

	12) verify with:
		mt rewind;
		cd /; tar -b 3000 -dvf /dev/tape  [not sure pax can do this]
		[Maybe: ``cd /; tar -cvf - | cmp -'' for pax]

When using tapes, it is important that the tape 'streams', i.e. doesn't
start/stop all the time because that is bad for the tape as well as for
the drive. If you are cursed with a slow hard disk, you should use a large
buffer (-b option for tar) to reduce the starting/stopping.
When doing backups on heavely loaded systems however, you may experience
similar problems. Other *nix variants seem to have special buffering
programs to solve that. Hopefully someone will port those to Linux.
I'm looking for volunteers (yes, YOU) to port/write something like that.

NOTE: A QIC-02 drive will attempt to rewind the tape after
power-on and also after it has been given a reset command. It will
not rewind the tape immediately but instead it will will delay the
rewind until it is given a command that requires actual tape movement.
So, if you don't want any nasty surprises, get used to rewinding the 
tape before/after using it. Otherwise you may insert a tape that has
not been rewound and try to append to the end and discover that your
data has just been overwritten.
The driver sometimes rewinds the tape after reading or writing a file on
the tape. [It should only do this when something goes wrong...] So always
make sure you know the tape position before performing any command that
will write to the tape!

The QIC-02 standard specifies that all drives should support at
least some basic commandset, and specifies some optional
commands as well. I don't know which drives support which optional
commands. Some optional commands are required for certain ioctl
operations. In tpqic02.h you can specify which commands are supported
by your drive using the TP_HAVE_... defines. You could first try it
with all commands enabled and disable the ones that return an 'illegal
command' exception status. The seek-to-end-of-recorded media is particulary
useful as it avoids re-reading the entire tape, but it will only work
correctly if you drive supports it and if there is no data recorded beyond
the end of the most recently written file.

Please send me a note whether it works or not. Try to find out what 
commands are supported by your drive. If I get enough feedback on
this, I will include the TP_HAVEs in the distributed sources.
Bug fixes and suggestions are of course welcome! IF YOU SEND ME A NOTE
ABOUT A PROBLEM OR BUG, MAKE SURE YOU SPECIFY WHAT VERSION OF THE DRIVER,
WHAT VERSION OF LINUX, WHAT KIND OF HARDWARE AND ANY OTHER RELEVANT INFO
YOU HAVE.
The version number of the driver should be printed on-screen when linux
boots.

Keep reading comp.os.linux.announce and the tape mailing list. To join the
mailing list, send mail to linux-activists-request@niksula.cs.hut.fi
with the extra line 'X-Mn-Admin: join TAPE' in your mail message header
or as the first line in your mail message.


Enjoy!

--Hennus Bergman

s0356514@.let.rug.nl		[works usually, but goes through DOS :-(]
csg279@wing.rug.nl		[If the first one doesn't work]
[If you cannot reach me on any of the addresses above, try the TAPE mailing
list. Note that I don't get to read my mail very frequently, so don't expect
me to reply in less than a week.]

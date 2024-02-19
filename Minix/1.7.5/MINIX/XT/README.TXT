Suggestions on installing Minix on a machine with little memory.

The file TINYROOT is a small replacement for the ROOT image that is
needed if your machine has only one floppy drive and less than 2
megabytes of memory.  You can use it to boot your machine, but you
should use ROOT for the installation if you can.

If your floppy drive is only 360 kb then have to use TINYROOT for ROOT,
and TINYUSR1 for USR.  The very last step of the installation, filling
/usr from USR.nn, will fail, because you need some commands that are on
TINYUSR2.  You have to copy those commands into the /usr tree (mount the
floppy and cpdir the lot).  TINYUSR2 also contains the kernel image
used for TINYROOT in the tmp directory that you can put into /minix/.
So when 'setup' suggests that you type 'halt' do this instead:

	mount /dev/fd0 /fd0		# Mount TINYUSR2
	cpdir -v /fd0 /usr		# Add its contents to /usr
	umount /dev/fd0			# Take TINYUSR2 out of the drive
	mount /dev/hd2a /root		# Mount the hard disk root (hd2a??)
	cp -p /usr/tmp/* /root/minix	# Copy the kernel image in place
	halt				# Continue where you left off

A machine with only 640 kb memory will have trouble to run the
installation script.  Use

	exec setup

to overlay the login shell of root with the script.  A few things may
still fail, but nothing critical ("sleep: not found").

If you want to compile a new kernel as bin use

	chsh /bin/sh

to change the login shell of bin from ash to the much smaller sh.  Log
out, log back in, and use

	exec make xxx

If it still fails then you have to make things bit by bit by running
make in subdirectories.
--
						Kees J. Bot  (kjb@cs.vu.nl)

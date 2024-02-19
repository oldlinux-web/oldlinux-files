This is release 0.95c+ of the linux kernel - it contains some
enhancements and bugfixes to the 0.95a kernel, as well as some minor
fixes relative to the last alpha-patch (0.95c).  The release is
available as

- binary		(bootimage-0.95c+.Z)
- full source		(linux-0.95c+.tar.Z)
- patches rel. to 0.95c	(diff-0.95c.c+.Z)
- patches rel. to 0.95a	(diff-0.95a.c+.Z)

NOTE TO PATCHERS!! Before patching, do this:
 - make an empty include-file linux/include/checkpoint.h
 - rename linux/kernel/math/math_emulate.c as just emulate.c
That is, from the linux source directory do:

	$ > include/checkpoint.h
	$ mv kernel/math/math_emulate.c kernel/math/emulate.c

Also note that patching from the 0.95a version is probably not worth it
as it's easier to get the complete new sources.

Although I'm making binaries and the full source available, this isn't
really a major release: there is no new rootdisk, and this is more "my
current kernel" and not really tested (I put in the last changes 5
minutes before packing all this up).

The reason I'm making this available is that with the advent of gcc-2.1
and the VFS-library the old kernel doesn't really do everything the new
libraries want: the readdir system call is needed to get things working. 
The default compiler after this release is considered to be gcc-2.0 or
higher (although 1.40 still works - you don't /have/ to change).  People
who are unable or unwilling to patch a new kernel shouldn't be unable to
run the new binaries. 

This kernel should be totally backwards compatible, so no binaries
should break.  I resisted adding the changed mount() system call into
this release: the next major release will have a third parameter for
mount() - the filesystem type name (ie mount /dev/xxx /mnt minix). 

Fixes relative to 0.95c:

- corrected two minor bugs in readdir() (thanks to R Card)

- lp-patches are in.  I've edited them a bit, and will probably do some
  more editing in the future, but they seem to work fine. 

- 8-bit ISO latin output to the console (ie part of Johan Myreens
  general latin-1 patches: the keyboard patches aren't there)

- other minor bug-fixes (thanks to HH Bergman for noticing the
  timer-table bug)

Things I haven't had time to look into yet:

- select still has some problems
- reports that VC-output sometimes isdiscarded (never seen it myself)
- probably other things I've simply forgot...

			Linus

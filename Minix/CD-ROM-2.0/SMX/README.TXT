The file SMX200.TAZ is a compressed tar file that contains 
release 1 of the version of Solaris Minix (smx) that is based on Minix
2.0.0.  It will only operate under Solaris 2 (Solaris 2.4 and 2.5 have
been used without problems) and must be compiled with the GNU C
compiler, gcc.   This compiler is included on the MINIX CD ROM as
GCC-272.TAZ

To install smx you should:

1. Copy SMX200.TAZ to a Sun running Solaris 2.  On arrival, it should
be 11235655 bytes long.


2. When you run /usr/bin/sum on SMX200.TAZ, it should report:

9577 21945 SMX200.TAZ


3. Change the name from SMX200.TAZ to smx200.tar.Z


4. Uncompress smx200.tar.Z using the command line:

uncompress smx200.tar.Z


5. Extract the file doc/smx_ps from the smx200.tar archive using 
the command line:

tar xvf smx200.tar doc/smx_ps

smx_ps is a postscript file that contains (amongst many other things)
installation instructions for smx.


Paul Ashton (paul@cosc.canterbury.ac.nz)
1996-09-13


The file SMX174B1.TAZ is a compressed tar file that contains beta
release 1 of the version of Solaris Minix (smx) that is based on Minix
1.7.4.  It will only operate under Solaris 2 (Solaris 2.4 and 2.5 have
been used without problems) and assumes that gcc is available. 

To install smx you should:

1. Copy SMX174B1.TAZ to a Sun running Solaris 2.  On arrival, it should
be 11129055 bytes long.


2. When you run sum on SMX174B1.TAZ, it should report:

55758 21737 SMX174B1.TAZ


3. Change the name from SMX174B1.TAZ to smx174b1.tar.Z


4. Uncompress smx174b1.tar.Z using the command line:

uncompress smx174b1.tar.Z


5. Extract the file doc/smx_ps from the smx174b1.tar archive using 
the command line:

tar xvf smx174b1.tar doc/smx_ps

smx_ps is a postscript file that contains (amongst many other things)
installation instructions for smx.

To compile this program, you will need the GNU C compiler, gcc.  This
compiler is included here are GCC-272.TAZ

Paul Ashton (paul@cosc.canterbury.ac.nz)
1996-08-15

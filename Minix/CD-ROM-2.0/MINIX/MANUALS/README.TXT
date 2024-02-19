All the MINIX manual pages are in this directory.  They are also
installed under MINIX as part of the standard installation so they are
available under MINIX. The man pages are divided into nine parts, as in UNIX.

This directory provides the manuals in four different formats, to make it
convenient for both people with and without PostScript printers.  

   CATx  - Flat ASCII text
   ALLPS - PostScript version, one file per part
   PSx   - PostScript version, one file per manual page 
   MANx  - Troff source for producing the man pages (using macros.9)

If you do not have a PostScript printer, just use the CATx/* files. If you
do have a PostScript printer, print all the files in ALLPS and those in PS9.
Both the ASCII and PostScript versions have exactly the same content, only
the PostScript versions use multiple fonts, etc. to improve readability.

The files in PS1 through PS8 have the same content as ALLMAN1.PS through
ALLMAN8.PS, only they are arranged as one manual page per file.  This format
is provided to make it easier to print only selected manual pages, as the
full printout of ALLMANx.PS is quite large (540 pages).  If you print the
ALLMANx.PS files, you will not need the files in PSx.

The (troff) sources for the manual pages are provided to allow people to
modify or reformat them in custom ways.




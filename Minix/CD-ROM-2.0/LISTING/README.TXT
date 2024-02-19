This directory contains the full source code listing of MINIX in two variants:
the full code and the book version.  The difference is that the code in the
book is an abridged version.  To keep the length of the book (somewhat)
manageable, some of the less important parts of MINIX are not listed in the
book and not discussed there.  The BOOK.* files here refer to the listing
as given in the book.  The COMPLETE.* files here refer to the complete listing,
with everything.  The LIB files are the various libraries.  The files are

	XXX.TXT	- Flat ASCII text
	XXX.PS	- Postscript file
        XXX.T   - troff source for making XXX.PS
	XXX.IDX - An index to XXX file names
	XXX.REF - A cross-reference listing of all procedures, macros, etc.

where XXX is one of BOOK, COMPLETE, LIB, LIB386, or LIB86.

The listing in the book is printed in LucidaSans Typewriter, and the XXX.PS
files use this font too.  If you have this font definition, please prepend it 
to LISTING.PS before sending it to a PostScript printer. We cannot supply this
font because it is copyright by Bigelow and Holmes.  

If you do not have the LucidaSans font available locally, change the

.fp 5 CW LucidaT

line in LISTING.T to a locally available monospace font (e.g. letter gothic)
and run troff again.

Alternatively, just print LISTING.PS, but it will print in Courier instead
of Lucida.

If you are unable to print the entire PostScript file in one blow (e.g.
because your paper tray is too small), you should copy it to the hard disk
and edit it.  PostScript files are just ASCII and can be edited with any
editor.  When editing XXX.PS, leave all the material up to and included the
line:

%%EndProlog

After that you will find the code for the various pages, in sequence.  Each
one starts with a line like this:

%%Page: 1 1

and ends with a line like this:

EP

Just remove those pages you do not want to print on the next run, for
example, pages above 100.  Next time edit the orginal file and remove all
pages except 101 to 200, etc.




README for Thinkage YAY

Contents
--------
*About Thinkage YAY
*Installation
*Distribution, copyright
*Registration
*Bug Reports
*Contact information

1. About Thinkage YAY

YAY, which stands for Yet Another YACC, is a package that generates
software to parse text input.  YAY lets you describe the grammar of
the input language you want to parse and uses this grammar to generate
the parser software.  In addition to the usual YACC facilities, YAY
supports LR(2) grammars (with two levels of lookahead), and provides
special "preference" constructs that give you more control over
resolving ambiguities.

This package runs on Linux, SunOS, Windows NT, DOS and OS/2 platforms.

2. Installation

See the end of this section for system-specific instructions.

To unpack YAY 1.0, run one of the following commands depending on
which archive you have:

For yay-1.0.tar.Z (compressed tar file on SunOS):
    zcat yay-1.0.tar.Z | tar xvf -

For yay-1.0.tar.gz (Gnu-zipped tar file on SunOS and Linux):
    zcat yay-1.0.tar.gz | tar xvf - 

For yay-1_0.zip (zipped file on DOS, OS/2 and Windows NT):
    unzip yay-1_0.zip

This operation unpacks everything into a directory called 'yay-1.0' on
SunOS and Linux, and 'yay-1_0' on OS/2, DOS and NT.  To install YAY,
change to this directory and run the 'setup' program.

The setup program will ask you where to put the various YAY-related
files.  It will also create an installation file that tells YAY where
to find the files it needs.  If you don't use the default filename for
the installation file, you must set the YAY_INST environment variable
to be the name of the installation file.  Alternately, you can use
YAY's INSTallation= command-line option.

After setup finishes, you can look at the setup.log file to see what
it did.

Finally, you should copy the 'yay' executable file to a directory that
is in your search path.

2.1 SunOS-Specific Instructions

SunOS users should download either the .Z file or the .gz file.  The
.gz file is smaller, but you will need the gzip package to unpack it.
You can downloand the README from
ftp://prep.ai.mit.edu/pub/gnu/README-about-.gz-files.  Gzip source can
be found at ftp://prep.ai.mit.edu/pub/gnu/gzip-1.2.4.tar.

2.2 OS/2-Specific Instructions

OS/2 users can get unzip from
<ftp://ftp.cdrom.com/pub/os2/archiver/unz512x2.exe>.  Create a new
directory and run unz512x2.exe from within it.  This will create a
number of files including README and unzip.exe.  See the README for
further information.  Using PKZIP for OS/2 is not recommended.

2.3 Windows NT-Specific Instructions

Windows NT users can get unzip from
<ftp://ftp.coast.net/SimTel/nt/archiver/unz512xn.exe>.  Create a new
directory and run unz512xn.exe from within it.  This will create a
number of files including README and unzip.exe.  See the README for
further information.  Using PKZIP for NT is not recommended.

2.4 DOS-Specific Instructions:

DOS users can get unzip from
<ftp://ftp.coast.net/SimTel/msdos/zip/unz512x3.exe>.  Create a new
directory and run unz512x3.exe from within it.  This will create a
number of files including README and unzip.exe.  See the README for
further information.  Using PKZIP for DOS is not recommended.

DOS users require the DOS4GW.EXE 32-bit extender which is included in
the distribution under the DOS4GW directory.  This must be installed
before installing YAY.  See the file DOS4GW.DOC for installation and
configuration instructions.  Normally, you only need to copy
DOS4GW.EXE to a directory that is in your search path, but the
documentation explains settings that you may need if you have unusual
hardware or if you do not like the defaults.

2.5 Removal

Change to a directory that was NOT created during the installation
procedure (such as your home directory or the root directory).
Run the 'uninst' program using the full name of the command.
You will be prompted for a log file name.  This file will be used
to log errors and list all files and directories that are removed.
By default this information will be printed to your display.
After uninst finishes, you can look at the log file to see what
uninst did.

If uninst cannot find the installation file, it will ask you
where it is.  It will then remove all the files that the setup
program installed.  If you have installed yay more than once on
the same system you must be careful to uninstall the correct one.
The safest way to do this is to use the INSTallation= option:

   /usr/local/lib/yay-1.0/uninst INST=/usr/local/lib/yay-1.0/instinfo

or on OS/2, DOS or Windows NT:

   C:\thinkage\yay-1_0\uninst INST=C:\thinkage\yay-1_0\instinfo

Those examples reflect the default values for the INSTallation=
option.  You would not need to use the INSTallation option in those
cases unless the YAY_INST variable was set.  If YAY was installed on
OS/2, DOS or NT in D:\yay-1.0 then you might use

   D:\yay-1.0\uninst INST=D:\yay-1.0\instinfo

Un-installation is slightly different on OS/2.  The OS/2 version of
uninst will usually report that it was unable to delete some of its
files (such as uninst.exe).  This occurs because OS/2 will not allow
files to be deleted if they are in use.  Usually uninst can delete
these files after the uninst program is finished.  You should check
that these files were deleted.  You do not need to reboot after
running uninst.

2.6 Documentation

Documentation is provided in both text and postscript versions.  You
can find the postscript versions of the full manual and the YAY
command-syntax help in the following files:

SunOS and Linux:
	yay-1.0-manual.ps
	yay-1.0-help.ps

DOS, OS/2 and Windows NT:
	yay10man.ps
	yay10hlp.ps


3. Distribution, copyright

For purposes of this section, the term "User" is defined as anyone 
trying out the Program and/or a registered licensee of the Program.

This software is protected by both Canadian Copyright Law and International
Treaty provisions.  Therefore you must treat this software just like a book
with the following single exception.  Thinkage Ltd. authorizes you to make
archival copies of the software for the sole purpose of backing up 
your software and protecting your investment from loss.

By saying, "just like a book", Thinkage means, for example, that this software
may be used by any number of people and may be freely moved from one
location to another so long as there is no possibility of it being used
at one location while it is being used at another.  In the same way 
that two different people in two different places cannot read the same
book at the same time, neither may the software be used by two different
people in two different places at the same time, without violating 
Thinkage's copyright.

Thinkage warrants that it has title to the Programs free of any lien,
charge, encumbrance or claim.  Title to the programs and any materials 
associated therewith shall at all times remain in Thinkage.

Thinkage makes no representation with respect to the adequacy of these
Programs for any particular purpose, or with respect to their adequacy
to produce any particular result.

The User agrees to absolve Thinkage of any responsibility for 
losses, costs, damages, or expenses of every nature or kind due to 
use of the Programs.

If the User finds the programs unsatisfactory in any way, the User's
sole recourse is to request a refund of moneys already paid to Thinkage.
In this case, Thinkage will return this amount to the User in
a timely manner.  When the money has been returned, the User will
destroy all of its copies of the programs and any products derived 
from the programs, and will cease to use the programs in any way.

4. Registration

Registration form for Thinkage Ltd. shareware

Name:

Company Name (if applicable):

Address:

City:

Province/State:

Country:

Postal Code/Zip Code:

Phone:

Fax:

Email:

Software Registering:

   ___	Thinkage YAY, Version 1.0 @ $50US per copy


	Total:	$
	(Payment methods accepted:  cheques or money orders made 
	payable to:  Thinkage Ltd.)

(All applicable taxes included.)

Would you like us to notify you of upgrades to your software?:

Would you like us to notify you of new products from Thinkage?:

The following section is optional.  We have included it to speed up
bug fixes and to get an idea of what systems people are using our software
on.

System Hardware:


Operating System:




Mail your registration to 
Shareware
Thinkage Ltd.
85 McIntyre Drive
Kitchener, Ontario
Canada  N2R 1H6

Fax: 519-895-1864

Email: shareware@thinkage.on.ca

5. Bug Reports

Please contact us with a description of any bugs you find, with as much
detail as possible regarding the system and circumstances of the error.

6. Contact Information
Email: shareware@thinkage.on.ca
Phone:  519-895-1860
Fax: 519-895-1864
Mail:
Thinkage Ltd.
85 McIntyre Drive
Kitchener, Ontario
Canada  N2R 1H6


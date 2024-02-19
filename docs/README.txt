%	~ftp/pub/OS/Linux/README
%	arl@hut.fi ed. (copyright arl edited parts by arl)
%	Created: Tue Jul 16 03:36:31 1991
%	Last modified: Mon Jun  8 10:51:34 1992

 ----------------------------------------------------------------------- 


	Linux -- The free Un*x clone is here.

		Linux is NOT a public domain software,
		Linux is free software copyright held by
		Linus Torvalds, Helsinki University, Finland.

 ----------------------------------------------------------------------- 

	This file is updated constantly.

 ----------------------------------------------------------------------- 


	For hackers only Linux version 0.95c+ code and sources
	available here.

	nic.funet.fi will also be site for Linux binaries,

	Directory structure:

		INSTALL		contains everything you should
				have to get Linux up and running.

		bin		binaries for /bin, /usr/bin and
				/usr/contrib/bin
		doc		documents and man pages
		images		boot and root images etc.
		kernel		kernel and driver sources
		lib		lib sources
		tools		tools for hackers, not for common
				use, like binaries in /etc
		xtra		community contributed diffs and
				utilities.
		incoming	you can FTP 'put' your stuff here,
				see next section for putting.

	Check file 'ls-laR', which contains "ls -laR" listing.

	Eh ... for people having question 'but my site doesn't
	have FTP' ...

		Just mail to 'mailserver@nic.funet.fi' and put
		command "help" to mail _body_ (body is the part
		where you write your messages/text and is not
		related to mail header, which contains lines
		like 'To: foobar').

		Mailserver is not official .. it's under testing,
		to it might work or not, but don't flame me about
		it ... I take only 'thanks' mails (nice to get them
		for work you didn't do ;-)

 ----------------------------------------------------------------------- 


	Other Linux FTP sites might be nearer you (and you get the
	sources and binaries FASTER, and save $$$s and net bandwidth).

	FTP site keeper: inform 'arl@hut.fi' for your site! and join
	to linux-activists mailing list FTP channel!

	The MAIN FTP sites (first ones ;-0):

		Finland:	nic.funet.fi
				[128.214.6.100]
					/pub/OS/Linux

		USA:		tsx-11.mit.edu
				[18.172.1.2]
					/pub/linux

		Germany:	ftp.dfv.rwth-aachen.de
				[137.226.4.105]
					/pub/linux

	nic.funet.fi:/pub/OS/Linux is mirrored at		[how often?]

		Germany:	ftp.thp.uni-koeln.de
					/pub/linux			[?]

		Germany:	ftp.Germany.EU.net			[?]
				[192.76.144.129]
					/pub/comp/i386/Linux

		Germany:	ftp.informatik.rwth-aachen.de
				[137.226.112.172]
					/pub/Linux			[24h]

		UK:		ftp.mcc.ac.uk
				[130.88.200.7]
					/pub/linux/fi.mirror		[24h]

		UK:		doc.ic.ac.uk
				[146.169.3.7]
					/pub/os/linux			[?]

		Australia:	kirk.bu.oz.au
					/pub/OS/Linux			[24h]

	tsx-11.mit.edu:/pub/linux is mirrored at		[how often?]

		Germany:	ftp.fgb.mw.tu-muenchen.de
				[129.187.200.1]
					/pub/linux			[?]

		France:		ftp-masi.ibp.fr
				[132.227.64.26]
					/pub/linux			[24h]
						mirrors banjo GCC,X11

	Subsets + stuff // the most important files available at: 

		Germany:	ftp.regent.e-technik.tu-muenchen.de
					/pub/linux

		UK:		ftp.mcc.ac.uk
				[130.88.200.7]
					/pub/linux

		USA:		headrest.woz.colorado.edu
				[128.138.192.4]
					/pub/linux

		Japan:		yagi.ecei.tohoku.ac.jp
				[130.34.222.67]
					/pub/Linux

		South Africa:	ftp.sun.ac.za
				[146.232.130.3]	9,6kb
					/pub/linux

	Own contributions (utilities programmed locally, NO LINUX
	INSTALLATION or DISTRIBUTION stuff) at:

		DK:		ftp.daimi.aau.dk
				[130.225.16.27]
					/pub/Linux-source

		Australia:	archsci.arch.su.oz.au
				[129.78.66.1]
					/pub/linux

 ----------------------------------------------------------------------- 


	Putting Linux stuff to nic.funet.fi ...

		Please use directory /pub/OS/Linux/incoming

		Please use FTP command "binary" for binary i.e.
		stuff like *.tar, *.tar.Z, or runable binaries etc.

		Our FTP is _hacked_, so you or anybody won't
		see the stuff in there, until I enable it. I try
		to check directories constantly, but you might
		also want to throw me a mail ?-)

		_would be nice_, if you put also a small file
		containing 2-3 line description of the package
		you put to nic. Name it like [file].ind, where
		file is the file you put. This makes the system
		much more confortable for all the users.

 ----------------------------------------------------------------------- 


	If you are willing to support Linux development,
	please "mail arl@hut.fi" .. and tell what goodies
	you are willing to do - I try to organize.

	Or you might use mailing list ...

	Or you might want to send mail directly to
	'torvalds@cs.helsinki.fi'

 ----------------------------------------------------------------------- 


	Newsgroups:

		comp.os.linux

	Archives at nic.funet.fi:/pub/doc/OS/Linux/doc/news

	Old alt.os.linux articles are also archived there.


 ----------------------------------------------------------------------- 


	Linux mailing list (kept with my Mail-Net system) addresses:

		For subscribing, unsubscribing, and other requests:

			linux-activists-request@niksula.hut.fi

			Mail-Net should send you information how to
			subscribe to the mailing list.

		For mailing _real_ articles:

			linux-activists@niksula.hut.fi

	Please, do NOT send subscribe/unsubscribe mails to
	linus-activists@niksula.hut.fi, or I and other people
	will *flame* you, and you'll notice, how your mailbox
	filled up ;-). You even might get dozens of copies
	of netiquette from net users ...

	How to find out, who's on the list ?

		Soon availble ;-)

	Archives at nic.funet.fi:/pub/doc/OS/Linux/doc/mailing_list

 ----------------------------------------------------------------------- 


current status can be asked with "finger torvalds@kruuna.helsinki.fi"
here's sample entry:


		Free UN*X for the 386

The current version of linux is a 0.98.1, released 92.10.04.  There are
various rootdisks that work with the newer versions, although some of
them have problems.  A new SLS release is expected soonish, using either
a 0.97.pl6 or 0.98.1 kernel release. 

0.98.1 supports X11r5 and the new gcc-2.1 (and newer) libraries with
multiple shared libs - as well as any old binaries (except the 0.12
version of gdb which used the older ptrace() interface).  It also
contains support for debugging (core-dumping and attach/detach) as well
as profiling: use gcc-2.2.2d for full utilization of all these features. 

Linux can be gotten by anonymous ftp from 'nic.funet.fi' (128.214.6.100)
in the directory '/pub/OS/Linux'.  This directory structure contains all
the linux OS- and library-sources, and enough binaries to get going.  To
install linux you still need to know something about unices: it's
relatively straightforward to install, but the documentation sucks raw
eggs, and people with no previous unix experience are going to get very
confused. 

There are now a lot of other sites keeping linux archives.  The main
ones (as well as the above-mentioned nic.funet.fi) are:
	tsx-11.mit.edu (18.172.1.2):
		directory /pub/linux
	sunsite.unc.edu	(152.2.22.81):
		directory /pub/Linux

(and many additional sites: there are now sites in the uk, japan etc
that carry linux, but I have lost count)

There is also a mailing list set up 'Linux-activists@niksula.hut.fi'. 
To join, mail a request to 'Linux-activists-request@niksula.hut.fi'. 
It's no use mailing me: I have no actual contact with the mailing-list
(other than being on it, naturally). 

There is also a newsgroup that contain linux-related questions and
information: comp.os.linux. 

Mail me for more info:

		Linus Torvalds (torvalds@kruuna.Helsinki.FI)
		Pietarinkatu 2 A 2
		00140 Helsinki
		Finland

0.98.1 has mainly minor bug-fixes

0.98 has these features:
 - tcp/ip in the standard kernel sources.
 - corrected serial startup checking and setserial ioctl
 - core-dumping corrections
 - various minor fixes

0.97.pl6 has these new features:
 - corrected named pipe problem in pl5
 - dynamic tty queues (no NR_PTY limit etc). Patches by tytso
 - corrected SCSI codes. Patches by Eric

0.97.pl5 has these features:
 - corrected *MAJOR* problem with [f]truncate() system calls
 - swapoff()/wait4() system calls
 - corrected some race-conditions in the minix fs
 - major mm rewrite: 3GB virtual process size, faster swapping
 - filesystem error reporting corrections
 - minor bugfixes

0.97 has these major new things relative to 0.96
 - select() through the VFS routines
 - easily installable IRQ's
 - bus-mouse driver
 - msdos filesystem (alpha)
 - extended filesystem (alpha)
 - serial line changes (faster, changeable irq's etc)
 - dynamic buffer-cache
 - new and improved SCSI drivers

 ----------------------------------------------------------------------- 


	At HUT (Helsinki University of Technology) I try to
	use students for programming work to make Linux better.

	So could you, if you are teacher. Linux might be
	even more suitable for teaching purposes than Minix.

	My experiencies with Minix are not promising, I got about
	1/3 of given projects back i.e. 2/3 were unfinished.
	Projects were like 'create symlinks to Minix' or
	'do a device driver for XX adapter'.

 ----------------------------------------------------------------------- 


	Some of the features I'd like to have (maybe I'll also
	code them):

		o	adding some features from my own OS.
			- STREAMS I/O, now only subset
			- Kernel development environment
				Should be like configuring
				BSD kernel.

		o	Socket stuff from BSD.
		o	BSD net stuff or Xkernel.

		o	Mach. I try to check what's usable in Mach.
				I took keyboard stuff from Mach
				to my own OS, but floppy stuff
				was too complicated (spaghetti-code),
				so I coded it again.
				Maybe some parts of kernel is also usable.

		o	Portability
		o	Modularity
				Like device drivers should have
				multiple levels; high/medium for
				abstraction and low level for
				specific card/chip support.

 ----------------------------------------------------------------------- 


	More info from:

		torvalds@kruuna.helsinki.fi

	And if _you_ have any comments to this README file, please
	send me mail (arl@hut.fi).

	arl



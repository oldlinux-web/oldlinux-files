%-----------------------------------------------------------------------------
%
%  Selected contents from the GNU archives at prep.ai.mit.edu (and UCLA).
%  These files are obtainable by anonymous ftp in the gnu/pub directory there.
%  This Prolog database contains definitions for the following relations:
%
%     pub_gnu( Archivename, Archivesize, Date ).
%
%     archive( Archivename, BriefDescription ).
%
%     archive_dir( Archivename, Directory ).
%
%     dir_subdir( Directory, Subdirectory ).
%
%     file( Filename, Filetype, Directory, Filesize ).
%                     Filetype is one of:
%                         text  -- a text file
%                         exec  -- an executable (binary) file
%                         dir   -- a directory (binary) file
%
%  For HW#2:
%    A file is 'large' if its size is at least 1 megabyte (1,048,576).
%
%    An archive is a binary file.
%    A directory is a binary file.
%    An executable file (a file whose Filetype is 'exec') is a binary file.
%    All other files are text files.
%
%-----------------------------------------------------------------------------
%           PLEASE, PLEASE DO ***NOT*** PRINT THIS FILE!!
%-----------------------------------------------------------------------------

pub_gnu( 'ae.tar.gz',			   149404,  'Oct 24  1992').
pub_gnu( 'autoconf-1.4.tar.gz',		   178833,  'May 22 17:54').
pub_gnu( 'autoconf-1.5.tar.gz',		   191831,  'Jul 22 22:32').
pub_gnu( 'autoconf-1.6-1.7.diff.gz',	     8588,  'Oct 26 15:38').
pub_gnu( 'autoconf-1.6.tar.gz',		   201553,  'Oct  9 12:55').
pub_gnu( 'autoconf-1.7.tar.gz',		   205612,  'Oct 26 15:39').
pub_gnu( 'bash-1.11-1.12.diff.gz',	    34718,  'Jan 28  1992').
pub_gnu( 'bash-1.12.tar.gz',		   638994,  'Jan 28  1992').
pub_gnu( 'bc-1.02.tar.gz',		    81227,  'Mar  4  1992').
pub_gnu( 'binutils-1.9.tar.gz',		   184423,  'Apr 17  1991').
pub_gnu( 'binutils-2.1-2.2.diff.gz',	   452696,  'May 18 22:44').
pub_gnu( 'binutils-2.2-2.2.1.diff.gz',	     1187,  'May 20 20:16').
pub_gnu( 'binutils-2.2.1.tar.gz',	  1321751,  'May 21 12:42').
pub_gnu( 'binutils-testsuite-2.1.tar.gz',   10326,  'May 17 13:47').
pub_gnu( 'bison-1.21.tar.gz',		   252202,  'Apr 18  1993').
pub_gnu( 'bison-1.22.tar.gz',		   258609,  'Sep  6 18:29').
pub_gnu( 'calc-2.02a.tar.gz',		   823220,  'Jun  1 06:48').
pub_gnu( 'calc-2.02b.tar.gz',		   823249,  'Sep  6 17:08').
pub_gnu( 'cperf-2.1.tar.gz',		    60887,  'Jan 15  1992').
pub_gnu( 'cperf-2.1a.tar.gz',		    60893,  'Oct 22 20:17').
pub_gnu( 'cpio-2.2.tar.gz',		    96091,  'Nov 24  1992').
pub_gnu( 'cpio-2.3.tar.gz',		   113823,  'Jul  7 23:05').
pub_gnu( 'cvs-1.0-1.1.diff.gz',		      639,  'Feb  6  1991').
pub_gnu( 'cvs-1.1-1.2.diff.gz',		    24801,  'Feb  6  1991').
pub_gnu( 'cvs-1.3.tar.gz',		   419164,  'Apr 11  1992').
pub_gnu( 'dc-0.2.tar.gz',		    72098,  'May 21 15:04').
pub_gnu( 'dejagnu-1.0.1.tar.gz',	   942855,  'May 27 14:36').
pub_gnu( 'diff-2.1-2.2.diff.gz',	     3090,  'Feb 22  1993').
pub_gnu( 'diffutils-2.3.tar.gz',	   271430,  'May 19 22:55').
pub_gnu( 'diffutils-2.4.tar.gz',	   290292,  'Sep 15 20:19').
pub_gnu( 'diffutils-2.5.tar.gz',	   291683,  'Sep 27 23:15').
pub_gnu( 'dirent.tar.gz',		    14641,  'Sep 13  1990').
pub_gnu( 'dld-3.2.3.tar.gz',		   112444,  'May 30  1991').
pub_gnu( 'doschk-1.1.tar.gz',		    14181,  'May 21 15:22').
pub_gnu( 'ecc-1.2.1.tar.gz',		    15926,  'Jan 19  1993').
pub_gnu( 'elib-0.06.tar.gz',		    45675,  'Jan 25  1993').
pub_gnu( 'elisp-manual-18-1.03.tar.gz',	  1246385,  'Jan 28  1991').
pub_gnu( 'elisp-manual-19-2.01.1-2.02.diff.gz',   172998,  'Aug 16 21:37').
pub_gnu( 'elisp-manual-19-2.01.1.tar.gz',  1717961,  'Jun  1 03:22').
pub_gnu( 'elisp-manual-19-2.02.tar.gz',	  1684440,  'Aug 15 03:14').
pub_gnu( 'elvis-1.7.tar.gz',		   198371,  'Mar 16  1993').
pub_gnu( 'emacs-18.57-18.58.diff.gz',	   271070,  'Feb 18  1992').
pub_gnu( 'emacs-18.58-18.59.diff.gz',	   354222,  'Oct 31  1992').
pub_gnu( 'emacs-18.59.tar.gz',		  2962652,  'Oct 31  1992').
pub_gnu( 'emacs-19.10-19.11.diff.gz',	    88949,  'Jun  1 06:11').
pub_gnu( 'emacs-19.11-19.12.diff.gz',	    59152,  'Jun  2 06:35').
pub_gnu( 'emacs-19.12-19.13.diff.gz',	   145308,  'Jun 11 08:14').
pub_gnu( 'emacs-19.13-19.14.diff.gz',	   668324,  'Jun 17 22:48').
pub_gnu( 'emacs-19.14-19.15.diff.gz',	    31198,  'Jun 19 19:43').
pub_gnu( 'emacs-19.15-19.16.diff.gz',	   174534,  'Jul  6 14:18').
pub_gnu( 'emacs-19.15.tar.gz',		  5486769,  'Jun 19 19:39').
pub_gnu( 'emacs-19.16-19.17.diff.gz',	   168186,  'Jul 19 03:29').
pub_gnu( 'emacs-19.16.tar.gz',		  5521436,  'Jul  6 13:17').
pub_gnu( 'emacs-19.17-19.18.diff.gz',	       26,  'Aug 11 17:45').
pub_gnu( 'emacs-19.17-19.18.diffb.gz',	   453843,  'Aug 11 17:43').
pub_gnu( 'emacs-19.17.tar.gz',		  5669074,  'Jul 19 03:28').
pub_gnu( 'emacs-19.18-19.19.diff.gz',	    90842,  'Aug 15 02:59').
pub_gnu( 'emacs-19.18.tar.gz',		  5985239,  'Aug  9 20:24').
pub_gnu( 'emacs-19.19.tar.gz',		  6045916,  'Aug 15 02:58').
pub_gnu( 'emacs-19.7-19.8.diff.gz',	   115375,  'May 28 03:32').
pub_gnu( 'emacs-19.8-19.9.diff.gz',	    50867,  'May 27 04:07').
pub_gnu( 'emacs-19.9-19.10.diff.gz',	   100798,  'May 30 03:11').
pub_gnu( 'emacs-manual-6.0.dvi.gz',	   300114,  'Mar 22  1991').
pub_gnu( 'f2c-1993.04.28.tar.gz',	   378126,  'Apr 28  1993').
pub_gnu( 'fax-3.2.1.tar.gz',		    74074,  'Aug  9  1992').
pub_gnu( 'fgrep-1.1.tar.gz',		    18927,  'Mar  7  1990').
pub_gnu( 'fileutils-3.6.tar.gz',	   171467,  'May 22 16:32').
pub_gnu( 'fileutils-3.9.tar.gz',	   187278,  'Oct 19 01:05').
pub_gnu( 'find-3.8.tar.gz',		   154737,  'Mar 29  1993').
pub_gnu( 'finger-1.37.tar.gz',		   239953,  'Oct 28  1992').
pub_gnu( 'flex-2.3.8.tar.gz',		   130473,  'Feb 24  1993').
pub_gnu( 'fontutils-0.5-0.6.diff.gz',	   123260,  'Oct 28  1992').
pub_gnu( 'fontutils-0.6.tar.gz',	   628949,  'Oct 28  1992').
pub_gnu( 'g++-1.42.0.tar.gz',		   847157,  'Sep 22  1992').
pub_gnu( 'gas-1.38.1.tar.gz',		   297047,  'Jan 28  1991').
pub_gnu( 'gas-2.0-2.1.diff.gz',		   584104,  'May 18 22:58').
pub_gnu( 'gas-2.1-2.1.1.diff.gz',	     1314,  'May 20 20:17').
pub_gnu( 'gas-2.1.1.tar.gz',		  1706731,  'May 21 12:47').
pub_gnu( 'gawk-2.15-2.15.1.diff.gz',	     4954,  'May 18 20:17').
pub_gnu( 'gawk-2.15.1-2.15.2.diffs.gz',	     2397,  'May 19 20:45').
pub_gnu( 'gawk-2.15.2.tar.gz',		   586991,  'May 19 20:45').
pub_gnu( 'gawk-doc-2.15.2.tar.gz',	   198897,  'May 19 20:45').
pub_gnu( 'gawk-ps-2.15.2.tar.gz',	   288122,  'May 19 20:46').
pub_gnu( 'gcc-1.37-1.38.diff.gz',	   204903,  'Jan  8  1991').
pub_gnu( 'gcc-1.38-1.39.diff.gz',	    45253,  'Jan 17  1991').
pub_gnu( 'gcc-1.39-1.40.diff.gz',	    52499,  'Jun 30  1991').
pub_gnu( 'gcc-1.40-1.41.diff.gz',	    69589,  'Aug 28  1992').
pub_gnu( 'gcc-1.41-1.42.diff.gz',	     7140,  'Sep 20  1992').
pub_gnu( 'gcc-1.42.tar.gz',		  1860607,  'Sep 20  1992').
pub_gnu( 'gcc-2.0-2.1.diff.gz',		   411207,  'Mar 25  1992').
pub_gnu( 'gcc-2.1-2.2.1.diff.gz',	   650987,  'Jun  9  1992').
pub_gnu( 'gcc-2.2-2.2.1.diff.gz',	     1008,  'Jun  9  1992').
pub_gnu( 'gcc-2.2.1-2.2.2.diff.gz',	     7644,  'Jun 16  1992').
pub_gnu( 'gcc-2.2.2-2.3.1.diff.gz',	  1095806,  'Nov  1  1992').
pub_gnu( 'gcc-2.3.1-2.3.2.diff.gz',	   149616,  'Nov 28  1992').
pub_gnu( 'gcc-2.3.2-2.3.3.diff.gz',	    45602,  'Dec 26  1992').
pub_gnu( 'gcc-2.4.0-2.4.1.diff.gz',	    61227,  'May 26 15:40').
pub_gnu( 'gcc-2.4.1-2.4.2.diff.gz',	    14465,  'May 31 03:41').
pub_gnu( 'gcc-2.4.2-2.4.3.1.diff.gz',	    15028,  'Jun  4 21:23').
pub_gnu( 'gcc-2.4.2-2.4.3.diff.gz',	    15028,  'Jun  6 00:48').
pub_gnu( 'gcc-2.4.3-2.4.3.1.diff.gz',	    18590,  'Jun 11 10:39').
pub_gnu( 'gcc-2.4.3.1-2.4.4.diff.gz',	    60085,  'Jun 20 09:32').
pub_gnu( 'gcc-2.4.3.1.tar.gz',		  5512737,  'Jun  4 21:21').
pub_gnu( 'gcc-2.4.4-2.4.5.diff.gz',	     4023,  'Jun 21 18:55').
pub_gnu( 'gcc-2.4.4.tar.gz',		  5524817,  'Jun 20 09:31').
pub_gnu( 'gcc-2.4.5-2.5.0.diff.gz',	       24,  'Oct 22 18:41').
pub_gnu( 'gcc-2.4.5-2.5.0a.diff.gz',	   984825,  'Oct 22 18:34').
pub_gnu( 'gcc-2.4.5.tar.gz',		  5558673,  'Jun 20 21:39').
pub_gnu( 'gcc-2.5.0.tar.gz',		       17,  'Oct 22 18:39').
pub_gnu( 'gcc-2.5.0a.tar.gz',		  5949185,  'Oct 22 18:39').
pub_gnu( 'gcc-testsuite-2.3.3.tar.gz',	   219183,  'May 17 13:47').
pub_gnu( 'gcc-vms-1.42.tar.gz',		   856531,  'Nov 23  1992').
pub_gnu( 'gdb-4.10-4.10.pl1.diff.gz',	     1138,  'Aug 30 14:03').
pub_gnu( 'gdb-4.10-testsuite.tar.gz',	   321223,  'Aug 13 12:28').
pub_gnu( 'gdb-4.10.pl1.tar.gz',		  3217681,  'Aug 30 14:10').
pub_gnu( 'gdb-4.10.tar.gz',		  3217104,  'Aug 13 12:27').
pub_gnu( 'gdb-4.9.tar.gz',		  2951317,  'May 12 15:17').
pub_gnu( 'gdb-testsuite-4.9.tar.gz',	   289178,  'May 12 15:18').
pub_gnu( 'gdbm-1.5.tar.gz',		    38137,  'Mar 15  1991').
pub_gnu( 'gdbm-1.6.tar.gz',		    58972,  'Jul 20 22:45').
pub_gnu( 'ghostscript-2.4-2.4.1.diff.gz',  135020,  'Apr 21  1992').
pub_gnu( 'ghostscript-2.4.1-2.5.diff.gz',  440603,  'Aug 19  1992').
pub_gnu( 'ghostscript-2.5-2.5.1.diff.gz',   85763,  'Sep 16  1992').
pub_gnu( 'ghostscript-2.5.1-2.5.2.diff.gz', 26213,  'Sep 21  1992').
pub_gnu( 'ghostscript-2.5.2-2.6.diff.gz',  718275,  'May 10 02:26').
pub_gnu( 'ghostscript-2.6-2.6.1.diff.gz',  126920,  'May 29 13:12').
pub_gnu( 'ghostscript-2.6.1.fix-01.gz',	    10782,  'Jul 19 12:50').
pub_gnu( 'ghostscript-2.6.1.fix-02.gz',	     1151,  'Jul 19 12:50').
pub_gnu( 'ghostscript-2.6.1.fix-03.gz',	    20186,  'Jul 19 12:50').
pub_gnu( 'ghostscript-2.6.1.fix-04.gz',	     2959,  'Jul 20 22:44').
pub_gnu( 'ghostscript-2.6.1.tar.gz',	   983017,  'May 29 13:12').
pub_gnu( 'ghostscript-2.6.1msdos.tar.gz', 1159449,  'May 29 13:12').
pub_gnu( 'ghostscript-fonts-2.6.1.tar.gz',1706030,  'May 10 02:27').
pub_gnu( 'ghostview-1.3-1.4.diff.gz',	    33757,  'Oct 13  1992').
pub_gnu( 'ghostview-1.4-1.4.1.diff.gz',	     2462,  'Nov  9  1992').
pub_gnu( 'ghostview-1.4.1-1.5.diff.gz',	    37547,  'Jul 25 10:11').
pub_gnu( 'ghostview-1.4.1.tar.gz',	   140526,  'Nov  9  1992').
pub_gnu( 'ghostview-1.5.tar.gz',	   139742,  'Jul 25 10:11').
pub_gnu( 'glibc-1.00-1.01.diff.gz',	    53738,  'Mar 12  1992').
pub_gnu( 'glibc-1.00-1.01.udiff.gz',	    50945,  'Mar 12  1992').
pub_gnu( 'glibc-1.01-1.02.udiff.gz',	    58455,  'Mar 20  1992').
pub_gnu( 'glibc-1.02-1.03.udiff.gz',	    82172,  'Apr 10  1992').
pub_gnu( 'glibc-1.04-1.05.diff.gz',	    39849,  'Sep 24  1992').
pub_gnu( 'glibc-1.06-crypt.tar.gz',	    30171,  'May 23 01:03').
pub_gnu( 'glibc-1.06.tar.gz',		  1331271,  'May 23 01:04').
pub_gnu( 'gmp-1.3.2.tar.gz',		   108358,  'May 21 14:09').
pub_gnu( 'gnats-3.00-3.01.diff.gz',	    74565,  'Apr 14  1993').
pub_gnu( 'gnats-3.01.tar.gz',		   472442,  'Apr 14  1993').
pub_gnu( 'gnu-objc-issues-1.0.tar.gz',	   173366,  'Mar  8  1993').
pub_gnu( 'gnu.ps.gz',			   483990,  'Aug 15 06:13').
pub_gnu( 'gnuchess-3.1.tar.gz',		   121193,  'Aug 16  1992').
pub_gnu( 'gnuchess-3.1.winner-uniform-platform.tar.gz',	    43192,  'Oct  7  1992').
pub_gnu( 'gnuchess-4.0.pl60.tar.gz',	   289169,  'Jan 17  1993').
pub_gnu( 'gnuchess-4.0.pl61.tar.gz',	   702517,  'Feb  6  1993').
pub_gnu( 'gnuchess-4.0.pl62.tar.gz',	   981451,  'Jun 25 20:44').
pub_gnu( 'gnuchess-bmp-windows.tar.gz',	     1162,  'Oct 26  1992').
pub_gnu( 'gnuchess-for-windows.tar.gz',	    81978,  'Oct 26  1992').
pub_gnu( 'gnugo-1.1.tar.gz',		    21890,  'May 17  1989').
pub_gnu( 'gnuplot-3.2.tar.gz',		   316217,  'Jun 18  1992').
pub_gnu( 'gnuplot-3.4-3.4a.diff.gz',	    12187,  'Jul 26 14:35').
pub_gnu( 'gnuplot-3.4a-3.4b.diff.gz',	     1673,  'Jul 27 20:31').
pub_gnu( 'gnuplot3.4.tar.gz',		   600239,  'Aug  2 00:23').
pub_gnu( 'gnuplot3.5.tar.gz',		   626008,  'Sep 30 23:19').
pub_gnu( 'gnushogi-1.1.pl01.tar.gz',	   234184,  'Apr 28  1993').
pub_gnu( 'gnushogi-1.1p02.tar.gz',	   173873,  'Oct 13 18:08').
pub_gnu( 'gp34to35.shar.gz',		    68630,  'Sep 30 23:20').
pub_gnu( 'gptx-0.2.tar.gz',		   149386,  'Oct 11  1991').
pub_gnu( 'graphics-0.17.tar.gz',	   182101,  'Apr  2  1991').
pub_gnu( 'graphics-doc-0.17.ps.gz',	    49660,  'Apr  2  1991').
pub_gnu( 'grep-1.6.tar.gz',		    71198,  'Jun  4  1992').
pub_gnu( 'grep-2.0.tar.gz',		   127553,  'Jul  4 22:18').
pub_gnu( 'groff-1.06-1.07.diff.gz',	   140890,  'Mar  3  1993').
pub_gnu( 'groff-1.07-1.08.diff.gz',	    21048,  'Apr 19  1993').
pub_gnu( 'groff-1.08.tar.gz',		   829584,  'Apr 19  1993').
pub_gnu( 'gzip-1.2.4.tar.gz',		   220623,  'Aug 20 19:31').
pub_gnu( 'hello-1.0-1.1.diff.gz',	    16452,  'Sep 16  1992').
pub_gnu( 'hello-1.1-1.2.diff.gz',	    25676,  'May 21 20:11').
pub_gnu( 'hello-1.2-1.3.diff.gz',	     2919,  'May 22 21:41').
pub_gnu( 'hello-1.3.tar.gz',		    87942,  'May 22 21:41').
pub_gnu( 'hp2xx-3.1.2.tar.gz',		   187065,  'Apr 20  1993').
pub_gnu( 'indent-1.7.tar.gz',		   141813,  'Feb 11  1993').
pub_gnu( 'indent-1.8.tar.gz',		   147373,  'Jun 16 19:34').
pub_gnu( 'ispell-2.0.02.tar.gz',	   170475,  'Nov  9  1990').
pub_gnu( 'ispell-4.0.tar.gz',		   187403,  'Jun  1 05:57').
pub_gnu( 'jarg300.info.gz',		   523845,  'Jul 29 19:23').
pub_gnu( 'jarg300.txt.gz',		   478061,  'Jul 29 19:22').
pub_gnu( 'jargon-upd.gz',		   145946,  'Jul 29 19:52').
pub_gnu( 'jargon.text.gz',		    37055,  'Apr 11  1990').
pub_gnu( 'less-177.tar.gz',		   100698,  'Apr  3  1991').
pub_gnu( 'libg++-1.39.0.tar.gz',	   666168,  'Feb 19  1991').
pub_gnu( 'libg++-2.4.tar.gz',		  1241213,  'Jun 30 15:37').
pub_gnu( 'libg++-2.5.tar.gz',		  1285156,  'Oct 26 03:45').
pub_gnu( 'm4-1.0.3.tar.gz',		   239537,  'Dec 19  1992').
pub_gnu( 'make-3.55-3.56.diff.gz',	    27010,  'Sep 22  1989').
pub_gnu( 'make-3.56-3.57.diff.gz',	    15839,  'Nov  3  1989').
pub_gnu( 'make-3.57-3.58.diff.gz',	    21650,  'Feb  8  1990').
pub_gnu( 'make-3.58-3.59.diff.gz',	    36196,  'Nov 28  1990').
pub_gnu( 'make-3.60-3.62.diff.gz',	    73109,  'Oct 29  1991').
pub_gnu( 'make-3.62-3.63.diff.gz',	   244050,  'Jan 22  1993').
pub_gnu( 'make-3.63-3.64.diff.gz',	    65599,  'Apr 21  1993').
pub_gnu( 'make-3.64-3.65.diff.gz',	    30972,  'May  9 16:09').
pub_gnu( 'make-3.65-3.66.diff.gz',	    10312,  'May 21 16:22').
pub_gnu( 'make-3.65-3.67.diff.gz',	    11632,  'May 22 21:18').
pub_gnu( 'make-3.66-3.67.diff.gz',	     2532,  'May 22 21:14').
pub_gnu( 'make-3.67-3.68.diff.gz',	    51418,  'Jul 28 18:48').
pub_gnu( 'make-3.67.tar.gz',		   410806,  'May 22 21:14').
pub_gnu( 'make-3.68.tar.gz',		   414065,  'Jul 28 18:39').
pub_gnu( 'make-doc-3.67.tar.gz',	   264780,  'May 22 21:14').
pub_gnu( 'make-doc-3.68.tar.gz',	   251118,  'Jul 28 18:39').
pub_gnu( 'malloc.tar.gz',		    34180,  'Jun  3 17:30').
pub_gnu( 'mm-1.07.tar.gz',		    42422,  'Nov 24  1992').
pub_gnu( 'ms-0.06.tar.gz',		    59880,  'Dec  3  1991').
pub_gnu( 'mtools-2.0.7.tar.gz',		    39742,  'Sep 10  1992').
pub_gnu( 'nethack-3.1.tar.gz',		  1811388,  'Mar  9  1993').
pub_gnu( 'nihcl-3.0.tar.gz',		   327743,  'Jan  1  1993').
pub_gnu( 'oleo-1.3.1-1.3.1b.diff.gz',	     2954,  'May 14 16:34').
pub_gnu( 'oleo-1.4.tar.gz',		   377214,  'May 23 17:54').
pub_gnu( 'oleo-1.5.tar.gz',		   380906,  'Aug 12 16:13').
pub_gnu( 'para-0.27.tar.gz',		    59693,  'Apr 18  1991').
pub_gnu( 'patch-2.0.12g11.tar.gz',	    71048,  'Jun  1 04:15').
pub_gnu( 'patch-2.0.12u9.tar.gz',	    57302,  'May 31 01:40').
pub_gnu( 'patch-2.1.tar.gz',		    71541,  'Jun 11 10:40').
pub_gnu( 'perl-4.036.tar.gz',		   586178,  'Feb  8  1993').
pub_gnu( 'rcs-5.6-5.6.0.1.diff.gz',	     1798,  'Mar 27  1993').
pub_gnu( 'rcs-5.6.0.1.tar.gz',		   250585,  'Mar 27  1993').
pub_gnu( 'readline-1.1.tar.gz',		   150750,  'May  3  1991').
pub_gnu( 'recode-3.2.4.tar.gz',		   136792,  'Nov 12  1992').
pub_gnu( 'regex-0.12.tar.gz',		   297790,  'Apr  2  1993').
pub_gnu( 'screen-3.5.0-3.5.1.diff.gz',	    79261,  'Aug 26 16:47').
pub_gnu( 'screen-3.5.1-3.5.2.diff.gz',	    11738,  'Aug 26 16:47').
pub_gnu( 'screen-3.5.2.tar.gz',		   263836,  'Aug 26 16:45').
pub_gnu( 'sed-1.15-1.16.diff.gz',	     1389,  'May 23 12:59').
pub_gnu( 'sed-1.17-1.18.diff.gz',	      350,  'May 30 22:47').
pub_gnu( 'sed-1.18.tar.gz',		    93301,  'Jun  1 19:05').
pub_gnu( 'sed-2.00.tar.gz',		   107849,  'Jul 16 01:29').
pub_gnu( 'sed-2.01.tar.gz',		   108105,  'Jul 21 13:07').
pub_gnu( 'sed-2.02b.tar.gz',		   110608,  'Jul 21 21:18').
pub_gnu( 'sed-2.03.tar.gz',		   114806,  'Aug  8 23:53').
pub_gnu( 'shellutils-1.8.tar.gz',	   156451,  'Oct 28  1992').
pub_gnu( 'smalltalk-1.1.1-dos.tar.gz',	   331313,  'Sep 14 10:11').
pub_gnu( 'smalltalk-1.1.1.tar.gz',	   308895,  'Sep 15  1991').
pub_gnu( 'smalltalk-tutorial.ps.gz',	    62887,  'Feb 27  1993').
pub_gnu( 'smalltalk-tutorial.txt.gz',	    41725,  'Feb 27  1993').
pub_gnu( 'superopt-2.2.tar.gz',		    40746,  'Feb 15  1993').
pub_gnu( 'superopt-2.3.tar.gz',		    55069,  'Oct  7 03:11').
pub_gnu( 'tar-1.11-1.11.1.diff.gz',	    33765,  'Sep 15  1992').
pub_gnu( 'tar-1.11.1-1.11.2.diff.gz',	   160900,  'Mar 25  1993').
pub_gnu( 'tar-1.11.2.shar.gz',		   214300,  'Mar 25  1993').
pub_gnu( 'tar-1.11.2.tar.gz',		   211658,  'Mar 25  1993').
pub_gnu( 'termcap-1.2.tar.gz',		   142720,  'Apr 15  1993').
pub_gnu( 'texinfo-3.1.tar.gz',		   526869,  'May 23 07:04').
pub_gnu( 'textutils-1.6.tar.gz',	   232695,  'May 22 16:33').
pub_gnu( 'tile-forth-2.1.tar.gz',	   148988,  'Nov 13  1991').
pub_gnu( 'time-1.5-1.6.diff.gz',	    10139,  'Oct  9 19:53').
pub_gnu( 'time-1.5.tar.gz',		    79812,  'Sep 10 11:28').
pub_gnu( 'time-1.6.tar.gz',		    80560,  'Oct  9 19:47').
pub_gnu( 'tput-1.0.tar.gz',		    39302,  'Jul 11  1991').
pub_gnu( 'uucp-1.04.tar.gz',		   634624,  'Feb 15  1993').
pub_gnu( 'uucp-doc-1.04.1.tar.gz',	   383413,  'Feb 17  1993').
pub_gnu( 'uuencode-1.0.tar.gz',		    27331,  'Aug 30 15:28').
pub_gnu( 'vh-1.5.tar.gz',		    46461,  'May 12 14:47').
pub_gnu( 'wdiff-0.04.tar.gz',		   107426,  'Dec 24  1992').
pub_gnu( 'xboard-2.1.pl11.tar.gz',	   106686,  'Jun 25 19:51').
pub_gnu( 'xboard-3.0.pl8.tar.gz',	   135968,  'Sep  6 13:47').
pub_gnu( 'xinfo-1.01.01.tar.gz',	    33848,  'Feb 28  1991').
pub_gnu( 'xshogi-1.1.pl02.tar.gz',	    98549,  'Apr 28  1993').
pub_gnu( 'xshogi-1.1p03.tar.gz',	   103816,  'Oct 13 18:08').


% More information about these programs can typically be found in the GNU
% Bulletin.  To receive a copy, write to gnu@prep.ai.mit.edu.
% 
% Because the unix `compress' utility is patented (by two separate patents,
% in fact), we cannot use it; it's not free software.  Therefore, the GNU
% Project has chosen a new compression utility, `gzip', which is free of any
% known software patents and which tends to compress better anyway.  Files
% compressed with this new compression program end in `.gz' (as opposed to
% `compress'-compressed files, which end in `.Z').  Gzip can uncompress
% `compress'-compressed files and SVR4 `pack' files (which also end in `.gz').
% This is possible because the various decompression algorithms are not
% patented---only compression is.
% 
% The gzip program is available from any GNU mirror site in shar, tar, or
% gzipped tar format (for those who already have a prior version of gzip and
% want faster data transmission).  It works on virtually every unix system,
% MSDOS, OS/2, and VMS.


archive( 'COPYING-1.0', 'Version 1 of the GNU General Public License' ).
%   Version 1 of the GNU General Public License.


archive( 'COPYING-2.0', 'Version 2 of the GNU General Public License' ).
%   Version 2 of the GNU General Public License.


archive( 'COPYING.LIB-2.0', 'Version 2 of the GNU General Public Library License' ).
%   Version 2 of the GNU General Public Library License (there is no
%   version 1).


archive( 'GNUinfo', 'general information about the GNU project' ).
%   General files of interest about the GNU Project, most of them included in
%   the GNU Emacs distribution.


archive( 'MailingListArchives', 'GNU mailing list archives' ).
%   Archives of the GNU mailing lists (most of which are also gatewayed to
%   the various gnu.* newsgroups).


archive( 'ae.tar.gz', 'a program performance profiling system' ).
%   This is a complicated profiling system that works with GCC to produce
%   more complete profiling information.  Instructions for compiling `ae'
%   are in the files "README" and "INSTALL".


archive( 'autoconf-1.3.tar.gz', 'automatic system configuration determination' ).
%   Autoconf is an extensible package of m4 macros that creates a
%   non-interactive configuration script for a package from a template
%   file.  The template file lists the operating system features that the
%   package can use, in the form of m4 macro calls, and can also contain
%   arbitrary shell commands.  Autoconf requires GNU m4.
%   
%   Autoconf-generated configure scripts are being used by many GNU
%   packages currently, and will be used by more in the future.


archive( 'bash-1.12.tar.gz', 'the Bourne Again Shell for Un*x' ).
%   BASH (the Bourne Again SHell) is a Posix-compatable shell with full
%   Bourne shell (`sh') syntax and some C-shell commands.  BASH supports
%   emacs-style command-line editing, job control, functions, and on-line
%   help.  Instructions for compiling BASH may be found in the file
%   "README".


archive( 'bc-1.02.tar.gz', 'a desk calculator' ).
%   `bc' is an interactive algebraic language with arbitrary precision.
%   GNU `bc' was implemented from the POSIX 1003.2 draft standard, but it
%   has several extensions including multi-character variable names, an
%   `else' statement, and full Boolean expressions.
%   
%   GNU `bc' does not use the historical method of being a compiler for
%   the `dc' calculator.  This version has a single executable that both
%   compiles the language and runs the resulting "byte code".  The "byte
%   code" is NOT the `dc' language.
%   
%   Instructions for compiling `bc' are in the file "Install".


archive( 'binutils-1.9.tar.gz', 'some binary file utilities' ).
%   The binutils comprise several small programs that are useful when
%   dealing with compiled programs.  They include `ar', `ld' (the linker),
%   `nm', `size', `strip', and `gprof'.  The programs support BSD
%   (Berkeley) format binary files, as well as COFF-encapsulation (BSD
%   format with a wrapper to allow you to run the binaries on COFF
%   systems).  Read the file "README" for instructions on compiling these
%   programs.


archive( 'binutils-2.1.tar.gz', 'some binary file utilities' ).
%   This is a beta release of a completely rewritten binutils
%   distribution.  These programs have been tested on various
%   architectures.  Most recently tested are sun3 and sun4s running
%   sunos4, as well as Sony News running newsos3.  However, since this is
%   a beta release taken directly from an evolving source tree, there
%   might be some problems.  In particular, the programs have not been
%   ported to as many machines as the old binutils.  There are also
%   features of the old versions that are missing on the new programs.  We
%   would appreciate patches to make things run on other machines;
%   especially welcome are fixes for what used to work on the old
%   programs!
%   
%   This release contains the following programs: `ar', `demangle', `ld'
%   (the linker), `nm', `objdump', `ranlib', `size', `strip', and `gprof'.
%   
%   BFD (the Binary File Descripter) library is in the subdirectory `bfd'
%   and is built along with GDB (which uses bfd).
%   
%   See the "README" file for further instructions on where to look for
%   building the various utilities.


archive( 'bison-1.19.tar.gz', 'the bison revision of yacc' ).
%   Bison is an upwardly compatible replacement for the parser generator
%   `yacc', with more features.  The file "README" gives instructions for
%   compiling Bison; the files `bison.1' (a man page) and `bison.texinfo'
%   (a GNU Texinfo file) give instructions for using it.


archive( 'calc-2.02.tar.gz', 'an advanced desk calculator' ).
%   Calc is an extensible, advanced desk calculator and mathematical tool
%   that runs as part of GNU Emacs.  It comes with source for the Calc
%   Manual, which serves as a tutorial and reference.  If you wish, you
%   can use Calc only as a simple four-function calculator, but it
%   provides additional features including choice of algebraic or RPN
%   (stack-based) entry, logarithmic functions, trigonometric and
%   financial functions, arbitrary precision, complex numbers, vectors,
%   matrices, dates, times, infinities, sets, algebraic simplification,
%   differentiation, and integration.  Instructions for install Calc for
%   emacs are in the "README" file.


archive( 'cperf-2.1.tar.gz', 'a perfect hash function generator' ).
%   This is a program to generate minimally perfect hash functions for
%   sets of keywords.  GCC was optimized by using this program.  Other
%   programs that must recognize a set of keywords may also benefit from
%   using this program.  Instructions for compiling cperf may be found in
%   the file "README".  Note that a C++ version of cperf (called `gperf')
%   is included in the libg++ distribution.  This version is for the use
%   of people who do not want to install C++ in order to compile a single
%   program.


archive( 'cpio-2.2.tar.gz', 'a file archiving utility' ).
%   `cpio' is a program for creating, updating, and extracting archives.
%   GNU `cpio' supports the final POSIX 1003.1 "ustar" standard.
%   Instructions for compiling `cpio' are in the file "README".


archive( 'cvs-1.3.tar.gz', 'CVS revision control system, extending RCS' ).
%   CVS is a collection of programs that provide for software release and
%   revision control functions.  CVS is designed to work on top of RCS
%   version 4.  It will parse older RCS formats, but cannot use any of its
%   fancier features without RCS branch support.  The file "README"
%   contains more information about CVS.


archive( 'dclshar', 'VAX/VMS file archiver utility' ).
%   Makes a shar-type .COM file for VAX/VMS systems, so it's possble to
%   transfer whole filesystem heirarchies back and forth.


archive( 'dejagnu-0.9.tar.gz', 'a program testing framework' ).
%   DejaGnu is a framework for testing other programs.  Its purpose is to
%   provide a single front end for all tests.  For more information read
%   the "README" file.


archive( 'diff-2.2.tar.gz', 'a line-by-line file comparator' ).
%   `diff' compares files showing line-by-line changes in several flexible
%   formats.  GNU `diff' is much faster than the traditional Unix
%   versions.  This distribution includes `diff', `diff3', `sdiff', and
%   `cmp'.  Instructions for compiling these are in the "README" file.


archive( 'dirent.tar.gz', 'Unix dirent-like library routines' ).
%   A freely-redistributable implementation of the dirent library
%   routines.  This release predates the GNU C library, which is more
%   complete.


archive( 'dld-3.2.3.tar.gz', 'dynamic link editor' ).
%   Dld is a library package of C functions that performs "dynamic link
%   editing".  Programs that use dld can add compiled object code to or
%   remove such code from a process anytime during its execution.  Loading
%   modules, searching libraries, resolving external references, and
%   allocating storage for global and static data structures are all
%   performed at run time.
%   
%   Dld works on VAX, Sun 3, SPARCstation, Sequent Symmetry, and Atari ST
%   machines.


archive( 'ecc-1.2.1.tar.gz', 'an error correcting code utility' ).
%   ECC is a Reed-Solomon error correction checking program.  It is
%   capable of correcting three byte errors in a block of 255 bytes, and
%   is capable of detecting more severe errors.


archive( 'elib-0.06.tar.gz', 'Some elisp utilities' ).
%   This is a small library of emacs-lisp functions.


archive( 'elisp-manual-1.03.tar.gz', 'The elisp reference manual' ).
%   This is the GNU Emacs Lisp Reference manual with texinfo source and a
%   DVI file that's ready for printing.  A copy of `makeinfo' is also
%   provided so that you can make an online version of the manual.  The
%   manual documents the extension language used by GNU Emacs.


archive( 'elvis-1.7.tar.gz', 'a variant of the vi/ex editor' ).
%   Elvis is a clone of vi/ex, the standard UNIX editor.  It supports
%   nearly all of the vi/ex commands, in both visual mode and colon mode.
%   Elvis runs under BSD UNIX, AT&T SysV UNIX, SCO Xenix, Minix, MS-DOS,
%   Atari TOS, and probably others.  Directions for building elvis are in
%   the "README" file.


archive( 'emacs-18.59.tar.gz', 'the famous emacs editor/environment' ).
%   GNU Emacs is an extensible, customizable fullscreen editor.  Read the
%   files "README" and "INSTALL" for a full description of the parts of
%   GNU Emacs, and the steps needed to install it.  This distribution
%   includes the complete GNU Emacs Manual.


archive( 'f2c-3.2.90.tar.gz', 'a FORTRAN to C converter' ).
%   This is a Fortran-to-C converter program.  Instructions for compiling
%   it are in the file "src/README".


archive( 'fax-3.2.1.tar.gz', 'tools for Group 3 fax transmission' ).
%   This is a set of software which provides Group 3 fax transmission and
%   reception services for a networked unix system.  It requires a
%   faxmodem which conforms to the new EIA-592 Asynchronous Facsimile DCE
%   Control Standard, Service Class 2.


archive( 'fgrep-1.1.tar.gz', 'a fast file string searching utility' ).
%   `fgrep' is a simplified version of `grep' which only searches for
%   literal text rather than regular expressions.  Eventually GNU `fgrep'
%   will disappear into the single grep program mandated by POSIX, but for
%   now it should be a compatible replacement for UNIX fgrep.  The options
%   are the same as those of GNU `egrep', and are a proper superset of the
%   fgrep options on all UNIX systems we are aware of.  Instructions for
%   building fgrep are in the "Makefile" and "README" file.


archive( 'fileutils-3.4.tar.gz', 'file manipulation utilities' ).
%   These are the GNU file-manipulation utilities.  Instructions for
%   compiling these utilities are in the file "README".  The fileutils
%   package contains the following programs: chgrp chmod chown cp dd df
%   dir du ginstall ln ls mkdir mkfifo mknod mv rm rmdir touch vdir.


archive( 'find-3.8.tar.gz', 'a general file searching utility' ).
%   This is a posix-compliant implementation (with many extensions) of
%   `find', a program used for searching filesystems for files that match
%   certain criteria and performing operations (like showing the path)
%   when they are found.  Also included in this distribution are `xargs'
%   and `locate'.


archive( 'finger-1.37.tar.gz', 'Internet information tool' ).
%   GNU Finger is a utility program designed to allow users of Unix hosts
%   on the Internet network to get information about each other.  It is a
%   direct replacement for the Berkeley 4.3 `finger' program, although it
%   produces different looking output.  For more information about why,
%   read "finger-1.37/doc/finger.texinfo".  Instructions for building
%   finger itself are in the "README" file.


archive( 'flex-2.3.8.tar.gz', 'a lexical analyzer compiler' ).
%   This is a faster, but not completely compatible replacement for `lex',
%   a lexical-analyzer-compiler.  Instructions for compiling `flex' are in
%   the file "README".


archive( 'fontutils-0.6.tar.gz', 'GNU font utilities' ).
%   These are the GNU font utilities.  There are various programs for
%   converting between various bitmaps and other graphical data formats,
%   creating fonts using Ghostscript, and other frobnicating utilities.
%   You will need GCC and GNU Make to compile these programs.  For the
%   programs which do online graphics, you will need an X11 server and the
%   X11R4 or R5 libraries.  Instructions for building the fontutils are in
%   the "README" file.


archive( 'g++-1.42.0.tar.gz', 'version 1 of G++, the GNU C++ compiler' ).
%   This is the old source to the GNU C++ compiler, G++.  Note that g++ version
%   1 is separate from GCC version 1, but in version 2 both programs are
%   merged.  Instructions for compilation of this program are in the file
%   "README".
%   
%   The maintainers of g++ recommend use of the 2.x version of g++, as the 1.x
%   version is no longer being maintained.


archive( 'gas-1.38.1.tar.gz', 'version 1 of the GNU assembler' ).
%   GAS is the GNU assembler.  GAS includes several features designed to
%   optimize the output of a C compiler.  In particular, it automatically
%   chooses the correct size for branch instructions (when it knows where
%   it is branching to), and automatically modifies jump-table entries
%   that are out-of-bounds.  GAS works on the VAX, the 68020, the 80386,
%   and the 320xx.  Read the file "README" for instructions on building
%   and using GAS.


archive( 'gas-2.0.tar.gz', 'version 2 of the GNU assembler' ).
%   GAS is the GNU assembler.  Version 2 has many changes over previous
%   GAS releases.  Most notable among the changes are the separation of
%   host system, target CPU, and target file format (i.e. cross-assembling
%   is much easier).  Many CPU types and object file formats are now
%   supported.
%   
%   Read the file "gas-2.0/gas/README" for instructions on building
%   and using GAS.


archive( 'gawk-2.14.tar.gz', 'version 2 of GNU AWK, a general purpose language' ).
%   This version of GNU AWK is upwardly-compatible with the SVR4 version.
%   Read the file "README" for instructions on compiling GAWK.
%   Instructions for using GAWK are in the file "gawk.texinfo".


archive( 'gcc-1.42.tar.gz', 'version 1 of GCC, the GNU C Compiler' ).
%   The GNU C compiler is a fairly portable optimizing compiler.  It
%   supports full ANSI C.  It generates good code for the 32000, 680x0,
%   80386, Alliant, Convex, Tahoe, and Vax CPUs, and for these RISC CPUs:
%   i860, Pyramid, Sparc, and SPUR.  The MIPS RISC CPU is also supported.
%   Machines using these CPUs include 386 running AIX, Alliant FX/8, Altos
%   3068, Apollo 68000/68020 running Aegis, AT&T 3B1, Convex C1 and C2,
%   DECstation 3100, DECstation 5000, DEC VAX, Encore MultiMax (NS32000),
%   Genix NS32000, Harris HCX-7 and HCX-9, HP-UX 68000/68020, HP running
%   BSD, IBM PS/2 running AIX, Intel 386 (System V, Xenix, BSD, but not
%   MS-DOS), Iris MIPS machine, ISI 68000/68020, MIPS, NeXT, Pyramid,
%   Sequent Balance (NS32000), Sequent Symmetry (i386), SONY News, Sun 2,
%   Sun 3 (optionally with FPA), Sun 4, SparcStation, and Sun386i.


archive( 'gcc-2.3.3.tar.gz', 'version 2 of GCC, the GNU C Compiler' ).
%   This is version 2 of GCC, the GNU C Compiler.  In addition to
%   supporting ANSI C, GCC Version 2 includes support for the C++ and
%   Objective C languages.
%   
%   GCC extends the C language to support nested functions, non-local
%   gotos, taking the address of program labels, and unnamed structures as
%   function arguments (among other things).  There are also many new
%   warnings for frequent programming mistakes.
%   
%   GCC 2 can generate output files in a.out, COFF, ECOFF, ELF, XCOFF,
%   VAX-VMS and OSF-Rose formats when used with a suitable assembler.  It
%   can produce debugging information in several formats: BSD stabs, COFF,
%   ECOFF, ECOFF with stabs symbols, VAX-VMS and DWARF.  (We may support
%   XCOFF for the RS/6000 in the future.)
%   
%   GCC can be easily configured as a cross-compiler, running on one
%   platform while generating code for another.
%   
%   GCC Version 2 supports compatible calling conventions for function
%   calling and return values on the Sparc (unlike version 1) as well as
%   the other machine types.
%   
%   Early testing of GCC Version 2 indicates that it produces faster code
%   for SPARC computers than Sun's latest released compilers (both bundled
%   and unbundled).  It is also the fastest known compiler for the
%   Motorola 88k.
%   
%   GCC Version 2 can produce position-independent code for several types
%   of CPU: 68000, 88000, 80386, Sparc, and RS/6000.  Supporting PIC on
%   additional suitable CPU types is not too difficult a task.
%   
%   A list of supported systems and instructions for compiling GCC are in
%   the file "INSTALL".


archive( 'gdb-4.8.tar.gz', 'the GNU source-level debugger' ).
%   This is the GNU source-level debugger.  A list of the machines
%   supported as targets or hosts, as well as a list of new features,
%   appears in "gdb-4.8/gdb/NEWS".
%   
%   Instructions for compiling GDB are in the file "gdb-4.8/gdb/README".
%   
%   BFD (the Binary File Descripter) library is in the subdirectory `bfd' and
%   is built along with GDB (which uses it).


archive( 'gdbm-1.5.tar.gz', 'the GNU dbm file indexing library' ).
%   This is the beta-test version of the gnu DBM library.  See the file
%   "README" for details.


archive( 'ghostscript-2.5.2.tar.gz', 'a tool for viewing PostScript files' ).
%   This program is an interpreter for a language that is intended to be,
%   and very nearly is, compatible with the PostScript language.  It runs
%   under X on Unix and VMS systems, and also runs on MS-DOS machines.  It
%   will drive either displays or low-to-medium-resolution printers.
%   Instructions for compiling Ghostscript are in the file "README".
%   Fonts for Ghostscript are in the directory "ghostscript-2.5.2/fonts".


archive( 'ghostview-1.4.1.tar.gz', 'a tool for viewing PostScript files' ).
%   Ghostview allows you to view PostScript(TM) files on X11 displays.
%   Ghostview handles the user interface details and calls the
%   `ghostscript' interpreter to render the image.  Instructions for
%   compiling ghostview are in the "README" file.


archive( 'glibc-1.05.1.tar.gz', 'the GNU C library' ).
%   This directory contains a test release of the GNU C Library.  Some
%   bugs surely remain.
%   
%   The library is ANSI C-1989 and POSIX 1003.1-1990 compliant and has most
%   of the functions specified in POSIX 1003.2 draft 11.2.  It is upward
%   compatible with the 4.4 BSD C library and includes many System V
%   functions, plus GNU extensions.
%   
%   Version 1.05.1 uses a standard GNU `configure' script and runs on
%   Sun-3 & Sun-4 (SunOS 4.1), HP 9000/300 & Sony NEWS 800 (4.3 BSD), MIPS
%   DECstation (Ultrix 4.2), and i386/i486 (System V & BSD).  The port to
%   the i386 running either SVR3, SVR4, or BSD, is mostly complete but has
%   not been well tested.  Porting the library is not hard.  If you are
%   interested in doing a port, please get on the mailing list by sending
%   electronic mail to bug-glibc-request@prep.ai.mit.edu.
%   
%   GNU stdio lets you define new kinds of streams, just by writing a few
%   C functions.  `fmemopen' function uses this to open a stream on a
%   string, which can grow as necessary.  You can define your own `printf'
%   formats to use a C function you have written.  Also, you can safely
%   use format strings from user input to implement a `printf'-like
%   function for another programming language, for example.  Extended
%   `getopt' functions are already used to parse options, including long
%   options, in many GNU utilities.
%   
%   See the file "INSTALL" for instructions on building the library.


archive( 'glibc-crypt-1.05.tar.gz', 'an encryption/decryption tool' ).
%   The GNU C library now includes Michael Glad's Ultra Fast Crypt, which
%   provides the Unix `crypt' function, plus some other entry points.
%   Because of the United States export restriction on DES
%   implementations, we are distributing this code separately from the
%   rest of the C library.  There is an extra distribution tar file just
%   for crypt; it is called `glibc-crypt-1.05.tar.gz'.  You can just unpack
%   the crypt distribution along with the rest of the C library and build;
%   you can also build the library without getting crypt.  Users outside
%   the USA can get the crypt distribution via anonymous FTP from
%   ftp.uni-c.dk [129.142.6.74], or another archive site outside the USA.
%   Archive maintainers are encouraged to copy this distribution to their
%   archives outside the USA.  Please get it from ftp.uni-c.dk;
%   transferring this distribution from prep.ai.mit.edu (or any other site
%   in the USA) to a site outside the USA is in violation of US export
%   laws.


archive( 'gmp-1.2.tar.gz', 'a library for arbitrary precision arithmetic' ).
%   GNU MP is a library for arbitrary precision arithmetic, operating on
%   signed integers and rational numbers.  It has a rich set of functions,
%   and the functions have a regular interface.  Instructions for using
%   and compiling the MP library are in the file "INSTALL".


archive( 'gnats-3.00.tar.gz', 'a bug tracking system' ).
%   GNATS (GNats: A Tracking System) is a bug-tracking system.  It is
%   based upon the paradigm of a central site or organization which
%   receives problem reports and negotiates their resolution by electronic
%   mail.  Although it's been used primarily as a software bug-tracking
%   system so far, it is sufficiently generalized so that it could be used
%   for handling system administration issues, project management, or any
%   number of other applications.


archive( 'gnuchess-4.0.pl61.tar.gz', 'a chess playing program' ).
%   This is a chess playing program with X, Suntools, curses, ASCII, and
%   IBMPC character set displays available.  Instructions for compiling
%   and using GNUchess may be found in the "README" file.


archive( 'gnugo-1.1.tar.gz', 'the game of Go' ).
%   This program plays the game of "Go".  Instructions for compiling it
%   are in the file "README".


archive( 'gnuplot-3.2.tar.gz', 'the GNU plotting tool' ).
%   Gnuplot is an interactive program for plotting mathematical
%   expressions and data.  Oddly enough, this program was neither done for
%   nor named for the GNU Project---the name is a coincidence.


archive( 'gptx-0.2.tar.gz', 'the GNU permuted index generator' ).
%   This is an alpha release of the GNU version of a permuted index
%   generator.  Among other things, it can produce a readable "KWIC"
%   (KeyWords In their Context) without the need of `nroff'.  There is
%   also an option to produce TeX compatible output.  Instructions for
%   compiling gptx are in the file "README".


archive( 'graphics-0.17.tar.gz', 'the GNU graphics utilities' ).
%   The GNU graphics utilities are a set of programs for producing plot
%   files and displaying them on Tektronix 4010, PostScript, and X window
%   system compatible output devices.  It includes replacements for the
%   `graph' and `plot' commands commonly supplied with some operating
%   systems.  Instructions for compiling the graphics utilities are in the
%   file "Makefile".


archive( 'grep-1.6.tar.gz', 'a fast general regular expression matcher' ).
%   This is the GNU high-speed `grep' program.  It is similar to its Unix
%   counterpart, but it is usually faster.  Instructions for compiling it
%   are in the file "README".


archive( 'groff-1.07.tar.gz', 'a variant of the troff text formatter' ).
%   This is the GNU 'roff document formatting system.  Included in this
%   release are implementations of `troff', `pic', `eqn', `tbl', the
%   `-man' and `-ms' macros, and drivers for PostScript, TeX dvi format,
%   and typewriter-like devices.  Also included is a modified version of
%   the Berkeley `-me' macros, and an enhanced version of the X11R4
%   `xditview'.


archive( 'gzip-1.0.7.shar', 'a general file compression program' ).
archive( 'gzip-1.0.7.tar', 'a general file compression program' ).
archive( 'gzip-1.0.7.tar.gz', 'a general file compression program' ).
%   This is a new compression program free of known patents which the GNU
%   Project is using instead of the traditional `compress' program (which
%   has patent problems).  Gzip can uncompress LZW-compressed files but
%   uses a different algorithm for compression which generally yields
%   smaller compressed files.  This will be the standard compression
%   program in the GNU system.


archive( 'hello-1.1.tar.gz', 'the GNU hello program' ).
%   The GNU `hello' program produces a familiar, friendly greeting.  It
%   allows nonprogrammers to use a classic computer science tool which
%   would otherwise be unavailable to them.  Because it is protected by
%   the GNU General Public License, users are free to share and change it.


archive( 'indent-1.7.tar.gz', 'a variant of the indent program' ).
%   This is the GNU modified version of the freely-distributable `indent'
%   program from BSD.  The file "indent.texinfo" contains instructions on
%   using indent.


archive( 'ispell-2.0.02.tar.gz', 'an interactive spelling corrector' ).
%   Ispell is an interactive spelling corrector based on the ITS SPELL
%   program.


archive( 'less-177.tar.gz', 'a paginator' ).
%   Less is a paginator similiar to `more' and `pg', but with various
%   features (such as the ability to scroll backwards) that some other
%   pagers lack.  Instructions for compiling `less' are in the file
%   "README".


archive( 'libg++-1.39.0.tar.gz', 'the GNU C++ library' ).
%   This is the GNU C++ Library, a collection of C++ classes and support
%   tools for use with the GNU C++ compiler.  This includes a copy of the
%   `gperf' program that is written in C++ (see also the description of
%   `cperf').


archive( 'libg++-2.3.tar.gz', 'the GNU C++ library' ).
%   This is the GNU C++ library, a collection of C++ classes and support
%   tools for use with the GNU C++ compiler.  Other features include:
%   
%   * Uses new iostream classes to replace the old stream classes.  The
%     new design is mostly compatible with AT&T's version 2.x iostream
%     classes, though some features have not been implemented yet, and
%     there are a number of extensions.  The iostream library will track
%     the ANSI C++ standardization effort.
%   
%     The new iostream classes are more extensible that earlier designs:
%     There is a (reasonably) well-defined protocol for writing new
%     streambufs (character sources and sinks) at the application level;
%     some have already been written, others are planned.
%   
%     There is also a fairly complete implementation of C stdio built on
%     *top* of streambuf (where FILE==streambuf).  This provides 100%
%     compatibility between C and C++ I/O.  It is not installed by
%     default.
%   
%   * Include files no longer hard-wire in C header files. (I.e.
%     g++-include/signal.h uses the gcc 2 #include_next feature instead of
%     #include "/usr/include/signal.h".)  This should make it easier to
%     combine g++ headers with non-standard libraries (such as the
%     recently-released GNU C library).
%   
%   Instructions for building the g++ class library and other tools is in
%   the file "libg++-2.3/libg++/README".


archive( 'm4-1.0.3.tar.gz', 'the M4 macro processor' ).
%   `m4' is a macro processor, in the sense that in copies its input to
%   the output, expanding macros as it goes.  Macros are either built-in
%   or user-defined, and can take any number of arguments.  Besides just
%   doing macro expansion, `m4' has built-in functions for including named
%   files, running Unix commands, doing integer arithmetic, manipulating
%   text in various ways, recursion, etc.  Instructions for building m4
%   are in the "README" file.


archive( 'make-3.63.tar.gz', 'GNU Make' ).
%   This is GNU Make.  GNU Make supports many more options and features
%   than the Unix make.  Instructions for using GNU Make are in the file
%   "make.texinfo".  See the file "README" for installation instructions. 


archive( 'mm-1.07.tar.gz', 'the mm macro package for groff' ).
%   This is a macro package for groff.  It is supposed to be compatible
%   with the DWB `mm' macros, and has several extensions.  See the "README"
%   file for more details.


archive( 'ms-0.06.tar.gz', 'MandelSpawn, a fractal generator' ).
%   This is MandelSpawn, a parallel Mandelbrot program for the X window
%   system.  Instructions for compiling it are in the "README" files in
%   each of the three subdirectories.


archive( 'mtools-2.0.7.tar.gz', 'tools for MSDOS format I/O' ).
%   Mtools is a public domain collection of programs to allow Unix systems
%   to read, write, and manipulate files on an MSDOS filesystem (typically
%   a diskette).


archive( 'nethack-3.1.tar.gz', 'the famous nethack game' ).
%   This is the famous game of `nethack'.  Instructions for compiling and
%   using nethack may be found in the "README" file.


archive( 'nihcl-3.0.tar.gz', 'an object-oriented program support library' ).
%   This is an object oriented program support class library with a
%   portable collection of classes similar to those in Smalltalk-80.  This
%   library used to be known as OOPS (Object-Oriented Program Support).


archive( 'oleo-1.2.2.tar.gz', 'a spreadsheet program' ).
%   Oleo is a spreadsheet program (better for you than the more expensive
%   spreadsheet).  It supports X windows and character-based terminals,
%   and can generate embedded PostScript renditions of spreadsheets.
%   Keybindings should be familiar to Emacs users and are configurable by
%   users.
%   
%   There is relatively little documentation for Oleo yet.  The file
%   "USING" contains what there is.


archive( 'patch-2.0.12g8.tar.gz', 'automated file patching utility' ).
archive( 'patch-2.0.12u8.tar.gz', 'automated file patching utility' ).
%   `patch' will take a patch file containing any of the four forms of
%   difference listing produced by the `diff' program and apply those
%   differences to an original file, producing a patched version.
%   Instructions for building patch are in the "README" file.


archive( 'perl-4.036.tar.gz', 'the PERL general purpose language' ).
%   This is version 4.036 of Larry Wall's perl programming language.
%   Perl is intended as a faster replacement for sed, awk, and similar
%   languages.  The file "README" contains instructions for compiling
%   perl.


archive( 'rcs-5.6.0.1.tar.gz', 'the RCS revision control system' ).
%   This is the Revision Control System, a program to manage multiple
%   versions of a software project.  This program keeps the changes from
%   one version to another rather than multiple copies of the entire file;
%   this saves disk space.  Instructions for compiling RCS are in the file
%   "README".


archive( 'readline-1.1.tar.gz', 'a command-line editing utility' ).
%   The GNU Readline Library, which aids in the consistency of user
%   interface across discrete programs than need to provide a command line
%   interface.  Supports both emacs and vi-style editing.
%   
%   This release is pretty old; newer versions with more bug fixes are
%   usually distributed with GDB and BASH.


archive( 'recode-3.2.4.tar.gz', 'a character set translator' ).
%   `recode' converts files between character sets and usages.  When exact
%   transliterations are not possible, it may get rid of the offending
%   characters or fall back on approximations.  It recognizes or produces
%   a little more than a dozen of character sets and can convert each
%   charset to almost any other one.  `recode' pays special attention to
%   superimposition of diacritics, particularily for French.


archive( 'regex-0.11.tar.gz', 'a regular expression processor' ).
%   The GNU regexp library routines.  This release is probably out of
%   date; newer versions of these routines are typically distributed along
%   with programs which use them.


archive( 'screen-3.2b.tar.gz', 'a simple window manager' ).
%   `screen' is a window manager that allows you to handle several
%   independent screens (UNIX ttys) on a single physical terminal; each
%   screen has its own set of processes connected to it (typically
%   interactive shells).  Each virtual terminal created by `screen'
%   emulates a DEC VT100 plus several ANSI X3.64 and ISO 2022 functions
%   (including DEC VT102 features such as line and character deletion and
%   insertion).  See the "README" file for details on compiling screen.


archive( 'sed-1.13.tar.gz', 'a stream-text editor' ).
%   `sed' is a text editor much like `ed', but is stream-oriented.  It is
%   used copiously in shell scripts.  Although GNU sed has fewer static
%   limitations in terms of buffer size, command length, etc., it is a
%   little slower than most implementations.
%   
%   Instructions for building GNU sed are in the file "README".


archive( 'shellutils-1.8.tar.gz', 'some GNU shell utilities' ).
%   These are the GNU shell utilities, comprising small commands that are
%   frequently run on the command line or in shell scripts.  Instructions
%   for compiling these utilities are in the file "README".  The
%   shellutils package contains the following programs: basename date
%   dirname echo env expr false groups id logname nice nohup pathchk
%   printenv printf sleep stty su tee test true tty uname who whoami yes.


archive( 'smalltalk-1.1.1.tar.gz', 'a SmallTalk interpreter' ).
%   This is the GNU implementation of Smalltalk, an object-oriented
%   programming language.  Instructions for compiling it are in the file
%   "README".


archive( 'superopt-2.2.tar.gz', 'a program for generating highly optimized code' ).
%   The superoptimizer is a function sequence generator that uses a
%   exhaustive generate-and-test approach to find the shortest instruction
%   sequence for a given function.
%   
%   The GNU superoptimizer and its application in GCC is described in the
%   ACM SIGPLAN PLDI'92 proceedings.


archive( 'tar-1.11.2.shar.gz', 'a tape archive program' ).
archive( 'tar-1.11.2.tar.gz', 'a tape archive program' ).
%   Tar is a program used for archiving many files in a single file, which
%   makes them easier to transport.
%   
%   GNU tar includes multivolume support, the ability to archive sparse
%   files, automatic archive compression/decompression, remote archives,
%   and special features to allow tar to be used for incremental and full
%   backups.  Unfortunately GNU tar implements an early draft of the POSIX
%   1003.1 `ustar standard which is different from the final standard.
%   Adding support for the new changes in a backward-compatible fashion is
%   not trivial.
%   
%   Instructions for compiling GNU tar may be found in the file `README'.


archive( 'termcap-1.1.tar.gz', 'the portable terminal interface library' ).
%   This is a standalone release of the GNU Termcap library, which has
%   been part of the GNU Emacs distribution for years but is now available
%   separately to make it easier to install as `libtermcap.a'.  The GNU
%   Termcap library does not place an arbitrary limit on the size of
%   termcap entries, unlike most other termcap libraries.  Included is
%   extensive documentation in Texinfo format.  Unfortunately, this
%   release does not have a termcap database included.  Instructions for
%   building the termcap library are in the "README" file.


archive( 'texinfo-2.16.tar.gz', 'TeX on-line information tools' ).
%   This package contains a set of utilities related to Texinfo, which is
%   used to generate printed manuals and online hypertext-style manuals
%   (called `info').  Programs and interfaces for writing, reading, and
%   formatting texinfo files are available both as standalone programs and
%   as GNU Emacs interfaces.  See the file "README" for directions on how
%   to use the various parts of this package.


archive( 'textutils-1.4.tar.gz', 'some text utilities' ).
%   These are the GNU text utilities, commands that are used to operate on
%   textual data.  Instructions for compiling these utilities are in the
%   file "README".  The textutils package contains the following programs:
%   cat cksum comm csplit cut expand fold head join nl od paste pr sort
%   split sum tac tail tr unexpand uniq wc.


archive( 'tile-forth-2.1.tar.gz', 'a Forth compiler' ).
%   TILE Forth is a 32-bit implementation of the Forth-83 Standard written
%   in C. Thus allowing it to be easily moved between different computers
%   compared to traditional Forth implementations in assembly (most Forth
%   implementations are done in assembly to be able to utilize the
%   underlying architecture as optimal as possible).


archive( 'time-1.4.tar.gz', 'statistics interface for system commands' ).
%   `time' is used to time commands and report statistics about, among
%   other things, the amount of user, system, and approximate real time
%   used by a process.  Instructions for making time (no pun intended) are
%   in the file "README".


archive( 'tput-1.0.tar.gz', 'a portable terminal facility for shell scripts' ).
%   `tput' provides a portable way of allowing shell scripts to use
%   special terminal capabilities.  Although its interface is similar to
%   that of `terminfo'-based tput programs, this one uses `termcap'.
%   Instructions for compiling tput are in the "README" file.


archive( 'uucp-1.04.tar.gz', 'a Un*x-to-Un*x communications program' ).
%   This version of UUCP was written by Ian Lance Taylor.  It will be the
%   standard UUCP system for GNU.  It currently supports the `f', `g' (in
%   all window and packet sizes), `G', `t' and `e' protocols, as well a
%   Zmodem protocol and two new bidirectional protocols.  If you have a
%   Berkeley sockets library, it can make TCP connections.  If you have
%   TLI libraries, it can make TLI connections.  Other important notes
%   about this version of UUCP, and instructions for building it, are in
%   the file "README".


archive( 'vh-1.4.tar.gz', 'a browser for jargon files' ).
%   This is a browser for the jargon files which are also in this
%   directory.  It is not part of the GNU distribution proper.


archive( 'wdiff-0.04.tar.gz', 'a file comparator' ).
%   `wdiff' compares two files, finding which words have been deleted or
%   added to the first for getting the second.
%   
%   We hope eventually to integrate `wdiff', as well as some ideas from a
%   similar program called `spiff', into some future release of GNU
%   `diff'.


archive( 'xboard-2.1.pl9.tar.gz', 'an X-based user interface for GNU chess' ).
%   XBoard is an X11/R4-based user interface for GNU Chess.  It uses the
%   R4 Athena widgets and Xt Intrinsics to provide an interactive referee
%   for managing a chess game between a user and a computer opponent or
%   between two computers.  You can also use XBoard without a chess
%   program to play through games in files or to play through games
%   manually (force mode); in this case, moves aren't validated by XBoard.
%   XBoard manages a digital chess clock for each player and resets the
%   clocks if the proper number of moves are played within the time
%   control period.  A game can be started with the initial chess
%   position, with a series of moves from a game file or with a position
%   from a position file.  The "match" shell script runs a series of games
%   between two machines, alternating sides.  The man page xboard.man
%   describes the features of XBoard.


archive( 'xinfo-1.01.01.tar.gz', 'An X-based GNU info widget' ).
%   An X-based GNU info widget. 


archive_dir( 'autoconf-1.3.tar.gz', 'autoconf-1.3' ).
archive_dir( 'bash-1.12.tar.gz', 'bash-1.12' ).
archive_dir( 'bc-1.02.tar.gz', 'bc-1.02' ).
archive_dir( 'binutils-1.9.tar.gz', 'binutils-1.9' ).
archive_dir( 'binutils-2.1.tar.gz', 'binutils-2.1' ).
archive_dir( 'bison-1.19.tar.gz', 'bison-1.19' ).
archive_dir( 'calc-2.02.tar.gz', 'calc-2.02' ).
archive_dir( 'cperf-2.1.tar.gz', 'cperf-2.1' ).
archive_dir( 'cpio-2.2.tar.gz', 'cpio-2.2' ).
archive_dir( 'cvs-1.3.tar.gz', 'cvs-1.3' ).
archive_dir( 'dejagnu-0.9.tar.gz', 'dejagnu-0.9' ).
archive_dir( 'diff-2.2.tar.gz', 'diff-2.2' ).
archive_dir( 'dirent.tar.gz', 'dirent' ).
archive_dir( 'dld-3.2.3.tar.gz', 'dld-3.2.3' ).
archive_dir( 'ecc-1.2.1.tar.gz', 'ecc-1.2.1' ).
archive_dir( 'elib-0.06.tar.gz', 'elib-0.06' ).
archive_dir( 'elisp-manual-1.03.tar.gz', 'elisp-manual-1.03' ).
archive_dir( 'elvis-1.7.tar.gz', 'elvis-1.7' ).
archive_dir( 'emacs-18.59.tar.gz', 'emacs-18.59' ).
archive_dir( 'f2c-3.2.90.tar.gz', 'f2c-3.2.90' ).
archive_dir( 'fax-3.2.1.tar.gz', 'fax-3.2.1' ).
archive_dir( 'fgrep-1.1.tar.gz', 'fgrep-1.1' ).
archive_dir( 'fileutils-3.4.tar.gz', 'fileutils-3.4' ).
archive_dir( 'find-3.8.tar.gz', 'find-3.8' ).
archive_dir( 'finger-1.37.tar.gz', 'finger-1.37' ).
archive_dir( 'flex-2.3.8.tar.gz', 'flex-2.3.8' ).
archive_dir( 'fontutils-0.6.tar.gz', 'fontutils-0.6' ).
archive_dir( 'g++-1.42.0.tar.gz', 'g++-1.42.0' ).
archive_dir( 'gas-1.38.1.tar.gz', 'gas-1.38.1' ).
archive_dir( 'gas-2.0.tar.gz', 'gas-2.0' ).
archive_dir( 'gawk-2.14.tar.gz', 'gawk-2.14' ).
archive_dir( 'gcc-1.42.tar.gz', 'gcc-1.42' ).
archive_dir( 'gcc-2.3.3.tar.gz', 'gcc-2.3.3' ).
archive_dir( 'gdb-4.8.tar.gz', 'gdb-4.8' ).
archive_dir( 'gdbm-1.5.tar.gz', 'gdbm-1.5' ).
archive_dir( 'ghostscript-2.5.2.tar.gz', 'ghostscript-2.5.2' ).
archive_dir( 'ghostview-1.4.1.tar.gz', 'ghostview-1.4.1' ).
archive_dir( 'glibc-1.05.1.tar.gz', 'glibc-1.05.1' ).
archive_dir( 'glibc-crypt-1.05.tar.gz', 'glibc-crypt-1.05' ).
archive_dir( 'gmp-1.2.tar.gz', 'gmp-1.2' ).
archive_dir( 'gnats-3.00.tar.gz', 'gnats-3.00' ).
archive_dir( 'gnuchess-4.0.pl61.tar.gz', 'gnuchess-4.0.pl61' ).
archive_dir( 'gnugo-1.1.tar.gz', 'gnugo-1.1' ).
archive_dir( 'gnuplot-3.2.tar.gz', 'gnuplot-3.2' ).
archive_dir( 'gptx-0.2.tar.gz', 'gptx-0.2' ).
archive_dir( 'graphics-0.17.tar.gz', 'graphics-0.17' ).
archive_dir( 'grep-1.6.tar.gz', 'grep-1.6' ).
archive_dir( 'groff-1.07.tar.gz', 'groff-1.07' ).
archive_dir( 'gzip-1.0.7.tar.gz', 'gzip-1.0.7' ).
archive_dir( 'hello-1.1.tar.gz', 'hello-1.1' ).
archive_dir( 'indent-1.7.tar.gz', 'indent-1.7' ).
archive_dir( 'ispell-2.0.02.tar.gz', 'ispell-2.0.02' ).
archive_dir( 'less-177.tar.gz', 'less-177' ).
archive_dir( 'libg++-1.39.0.tar.gz', 'libg++-1.39.0' ).
archive_dir( 'libg++-2.3.tar.gz', 'libg++-2.3' ).
archive_dir( 'm4-1.0.3.tar.gz', 'm4-1.0.3' ).
archive_dir( 'make-3.63.tar.gz', 'make-3.63' ).
archive_dir( 'mm-1.07.tar.gz', 'mm-1.07' ).
archive_dir( 'ms-0.06.tar.gz', 'ms-0.06' ).
archive_dir( 'mtools-2.0.7.tar.gz', 'mtools-2.0.7' ).
archive_dir( 'nethack-3.1.tar.gz', 'nethack-3.1' ).
archive_dir( 'nihcl-3.0.tar.gz', 'nihcl-3.0' ).
archive_dir( 'oleo-1.2.2.tar.gz', 'oleo-1.2.2' ).
archive_dir( 'patch-2.0.12g8.tar.gz', 'patch-2.0.12g8' ).
archive_dir( 'patch-2.0.12u8.tar.gz', 'patch-2.0.12u8' ).
archive_dir( 'perl-4.036.tar.gz', 'perl-4.036' ).
archive_dir( 'rcs-5.6.0.1.tar.gz', 'rcs-5.6.0.1' ).
archive_dir( 'readline-1.1.tar.gz', 'readline-1.1' ).
archive_dir( 'recode-3.2.4.tar.gz', 'recode-3.2.4' ).
archive_dir( 'regex-0.11.tar.gz', 'regex-0.11' ).
archive_dir( 'screen-3.2b.tar.gz', 'screen-3.2b' ).
archive_dir( 'sed-1.13.tar.gz', 'sed-1.13' ).
archive_dir( 'shellutils-1.8.tar.gz', 'shellutils-1.8' ).
archive_dir( 'smalltalk-1.1.1.tar.gz', 'smalltalk-1.1.1' ).
archive_dir( 'superopt-2.2.tar.gz', 'superopt-2.2' ).
archive_dir( 'tar-1.11.2.shar.gz', 'tar-1.11.2.shar' ).
archive_dir( 'tar-1.11.2.tar.gz', 'tar-1.11.2' ).
archive_dir( 'termcap-1.1.tar.gz', 'termcap-1.1' ).
archive_dir( 'texinfo-2.16.tar.gz', 'texinfo-2.16' ).
archive_dir( 'textutils-1.4.tar.gz', 'textutils-1.4' ).
archive_dir( 'tile-forth-2.1.tar.gz', 'tile-forth-2.1' ).
archive_dir( 'time-1.4.tar.gz', 'time-1.4' ).
archive_dir( 'tput-1.0.tar.gz', 'tput-1.0' ).
archive_dir( 'uucp-1.04.tar.gz', 'uucp-1.04' ).
archive_dir( 'vh-1.4.tar.gz', 'vh-1.4' ).
archive_dir( 'wdiff-0.04.tar.gz', 'wdiff-0.04' ).
archive_dir( 'xboard-2.1.pl9.tar.gz', 'xboard-2.1.pl9' ).
archive_dir( 'xinfo-1.01.01.tar.gz', 'xinfo-1.01.01' ).


dir_subdir('.', 'TeX-draw').
dir_subdir('.', 'bash-1.12').
dir_subdir('bash-1.12', 'bash-1.12/builtins').
dir_subdir('bash-1.12', 'bash-1.12/documentation').
dir_subdir('bash-1.12', 'bash-1.12/examples').
dir_subdir('bash-1.12/examples', 'bash-1.12/examples/functions').
dir_subdir('bash-1.12/examples', 'bash-1.12/examples/scripts').
dir_subdir('bash-1.12/examples', 'bash-1.12/examples/startup-files').
dir_subdir('bash-1.12', 'bash-1.12/lib').
dir_subdir('bash-1.12/lib', 'bash-1.12/lib/glob').
dir_subdir('bash-1.12/lib', 'bash-1.12/lib/malloc').
dir_subdir('bash-1.12/lib', 'bash-1.12/lib/readline').
dir_subdir('bash-1.12/lib/readline', 'bash-1.12/lib/readline/doc').
dir_subdir('bash-1.12/lib/readline', 'bash-1.12/lib/readline/examples').
dir_subdir('bash-1.12', 'bash-1.12/support').
dir_subdir('bash-1.12', 'bash-1.12/test-suite').
dir_subdir('.', 'binutils-2.2.1').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/bfd').
dir_subdir('binutils-2.2.1/bfd', 'binutils-2.2.1/bfd/config').
dir_subdir('binutils-2.2.1/bfd', 'binutils-2.2.1/bfd/doc').
dir_subdir('binutils-2.2.1/bfd', 'binutils-2.2.1/bfd/hosts').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/binutils').
dir_subdir('binutils-2.2.1/binutils', 'binutils-2.2.1/binutils/config').
dir_subdir('binutils-2.2.1/binutils', 'binutils-2.2.1/binutils/testsuite').
dir_subdir('binutils-2.2.1/binutils/testsuite', 'binutils-2.2.1/binutils/testsuite/binutils.all').
dir_subdir('binutils-2.2.1/binutils/testsuite', 'binutils-2.2.1/binutils/testsuite/config').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/config').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/etc').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/gprof').
dir_subdir('binutils-2.2.1/gprof', 'binutils-2.2.1/gprof/config').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/include').
dir_subdir('binutils-2.2.1/include', 'binutils-2.2.1/include/aout').
dir_subdir('binutils-2.2.1/include', 'binutils-2.2.1/include/coff').
dir_subdir('binutils-2.2.1/include', 'binutils-2.2.1/include/elf').
dir_subdir('binutils-2.2.1/include', 'binutils-2.2.1/include/opcode').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/ld').
dir_subdir('binutils-2.2.1/ld', 'binutils-2.2.1/ld/config').
dir_subdir('binutils-2.2.1/ld', 'binutils-2.2.1/ld/scripts').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/libiberty').
dir_subdir('binutils-2.2.1/libiberty', 'binutils-2.2.1/libiberty/config').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/opcodes').
dir_subdir('binutils-2.2.1', 'binutils-2.2.1/texinfo').
dir_subdir('.', 'bison-1.21').
dir_subdir('.', 'diff').
dir_subdir('.', 'dld-3.2.3').
dir_subdir('.', 't').
dir_subdir('t', 't/add1').
dir_subdir('t', 't/general').
dir_subdir('t', 't/overlay').
dir_subdir('t', 't/reload').
dir_subdir('t', 't/simple').
dir_subdir('.', 'ecc-1.2').
dir_subdir('.', 'expect3.24').
dir_subdir('expect3.24', 'expect3.24/config').
dir_subdir('expect3.24', 'expect3.24/etc').
dir_subdir('expect3.24', 'expect3.24/expect').
dir_subdir('expect3.24/expect', 'expect3.24/expect/config').
dir_subdir('expect3.24/expect', 'expect3.24/expect/scripts').
dir_subdir('expect3.24/expect', 'expect3.24/expect/test').
dir_subdir('expect3.24', 'expect3.24/tcl').
dir_subdir('expect3.24/tcl', 'expect3.24/tcl/compat').
dir_subdir('expect3.24/tcl', 'expect3.24/tcl/doc').
dir_subdir('expect3.24/tcl', 'expect3.24/tcl/library').
dir_subdir('expect3.24/tcl', 'expect3.24/tcl/tests').
dir_subdir('expect3.24/tcl/tests', 'expect3.24/tcl/tests/globTest').
dir_subdir('expect3.24/tcl/tests/globTest', 'expect3.24/tcl/tests/globTest/a1').
dir_subdir('expect3.24/tcl/tests/globTest/a1', 'expect3.24/tcl/tests/globTest/a1/b1').
dir_subdir('expect3.24/tcl/tests/globTest/a1', 'expect3.24/tcl/tests/globTest/a1/b2').
dir_subdir('expect3.24/tcl/tests/globTest', 'expect3.24/tcl/tests/globTest/a2').
dir_subdir('expect3.24/tcl/tests/globTest/a2', 'expect3.24/tcl/tests/globTest/a2/b3').
dir_subdir('expect3.24/tcl/tests/globTest', 'expect3.24/tcl/tests/globTest/a3').
dir_subdir('.', 'fileutils-3.6').
dir_subdir('fileutils-3.6', 'fileutils-3.6/lib').
dir_subdir('fileutils-3.6', 'fileutils-3.6/man').
dir_subdir('fileutils-3.6', 'fileutils-3.6/src').
dir_subdir('.', 'flex-2.3.8').
dir_subdir('flex-2.3.8', 'flex-2.3.8/MISC').
dir_subdir('.', 'fontutils-0.6').
dir_subdir('fontutils-0.6', 'fontutils-0.6/bin').
dir_subdir('fontutils-0.6', 'fontutils-0.6/bpltobzr').
dir_subdir('fontutils-0.6', 'fontutils-0.6/bzr').
dir_subdir('fontutils-0.6', 'fontutils-0.6/bzrto').
dir_subdir('fontutils-0.6', 'fontutils-0.6/charspace').
dir_subdir('fontutils-0.6', 'fontutils-0.6/data').
dir_subdir('fontutils-0.6', 'fontutils-0.6/doc').
dir_subdir('fontutils-0.6', 'fontutils-0.6/fontconvert').
dir_subdir('fontutils-0.6', 'fontutils-0.6/gf').
dir_subdir('fontutils-0.6', 'fontutils-0.6/gsrenderfont').
dir_subdir('fontutils-0.6', 'fontutils-0.6/imageto').
dir_subdir('fontutils-0.6', 'fontutils-0.6/imgrotate').
dir_subdir('fontutils-0.6', 'fontutils-0.6/include').
dir_subdir('fontutils-0.6', 'fontutils-0.6/lib').
dir_subdir('fontutils-0.6', 'fontutils-0.6/limn').
dir_subdir('fontutils-0.6', 'fontutils-0.6/pbm').
dir_subdir('fontutils-0.6', 'fontutils-0.6/pk').
dir_subdir('fontutils-0.6', 'fontutils-0.6/tfm').
dir_subdir('fontutils-0.6', 'fontutils-0.6/widgets').
dir_subdir('fontutils-0.6', 'fontutils-0.6/xbfe').
dir_subdir('.', 'gas-2.1.1').
dir_subdir('gas-2.1.1', 'gas-2.1.1/bfd').
dir_subdir('gas-2.1.1/bfd', 'gas-2.1.1/bfd/config').
dir_subdir('gas-2.1.1/bfd', 'gas-2.1.1/bfd/doc').
dir_subdir('gas-2.1.1/bfd', 'gas-2.1.1/bfd/hosts').
dir_subdir('gas-2.1.1', 'gas-2.1.1/config').
dir_subdir('gas-2.1.1', 'gas-2.1.1/etc').
dir_subdir('gas-2.1.1', 'gas-2.1.1/gas').
dir_subdir('gas-2.1.1/gas', 'gas-2.1.1/gas/config').
dir_subdir('gas-2.1.1/gas', 'gas-2.1.1/gas/doc').
dir_subdir('gas-2.1.1/gas', 'gas-2.1.1/gas/testsuite').
dir_subdir('gas-2.1.1/gas/testsuite', 'gas-2.1.1/gas/testsuite/config').
dir_subdir('gas-2.1.1/gas/testsuite', 'gas-2.1.1/gas/testsuite/gas').
dir_subdir('gas-2.1.1/gas/testsuite/gas', 'gas-2.1.1/gas/testsuite/gas/all').
dir_subdir('gas-2.1.1/gas/testsuite/gas', 'gas-2.1.1/gas/testsuite/gas/i386').
dir_subdir('gas-2.1.1/gas/testsuite/gas', 'gas-2.1.1/gas/testsuite/gas/ieee.fp').
dir_subdir('gas-2.1.1/gas/testsuite/gas', 'gas-2.1.1/gas/testsuite/gas/m68k').
dir_subdir('gas-2.1.1/gas/testsuite/gas', 'gas-2.1.1/gas/testsuite/gas/m68k-coff').
dir_subdir('gas-2.1.1/gas/testsuite/gas', 'gas-2.1.1/gas/testsuite/gas/sparc-solaris').
dir_subdir('gas-2.1.1/gas/testsuite', 'gas-2.1.1/gas/testsuite/lib').
dir_subdir('gas-2.1.1', 'gas-2.1.1/include').
dir_subdir('gas-2.1.1/include', 'gas-2.1.1/include/aout').
dir_subdir('gas-2.1.1/include', 'gas-2.1.1/include/coff').
dir_subdir('gas-2.1.1/include', 'gas-2.1.1/include/elf').
dir_subdir('gas-2.1.1/include', 'gas-2.1.1/include/opcode').
dir_subdir('gas-2.1.1', 'gas-2.1.1/libiberty').
dir_subdir('gas-2.1.1/libiberty', 'gas-2.1.1/libiberty/config').
dir_subdir('gas-2.1.1', 'gas-2.1.1/opcodes').
dir_subdir('gas-2.1.1', 'gas-2.1.1/texinfo').
dir_subdir('.', 'gawk-2.15.2').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/atari').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/config').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/missing').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/pc').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/support').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/test').
dir_subdir('gawk-2.15.2/test', 'gawk-2.15.2/test/lib').
dir_subdir('gawk-2.15.2/test', 'gawk-2.15.2/test/reg').
dir_subdir('gawk-2.15.2', 'gawk-2.15.2/vms').
dir_subdir('.', 'gcc-2.4.5').
dir_subdir('gcc-2.4.5', 'gcc-2.4.5/config').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/a29k').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/alpha').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/arm').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/clipper').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/convex').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/elxsi').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/fx80').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/gmicro').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/h8300').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/i386').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/i860').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/i960').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/m6811').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/m68k').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/m88k').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/mips').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/ns32k').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/pa').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/pyr').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/romp').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/rs6000').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/sh').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/sparc').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/spur').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/tahoe').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/vax').
dir_subdir('gcc-2.4.5/config', 'gcc-2.4.5/config/we32k').
dir_subdir('gcc-2.4.5', 'gcc-2.4.5/objc').
dir_subdir('.', 'gdb-4.10.pl1').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/bfd').
dir_subdir('gdb-4.10.pl1/bfd', 'gdb-4.10.pl1/bfd/config').
dir_subdir('gdb-4.10.pl1/bfd', 'gdb-4.10.pl1/bfd/doc').
dir_subdir('gdb-4.10.pl1/bfd', 'gdb-4.10.pl1/bfd/hosts').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/config').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/etc').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/gdb').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/29k-share').
dir_subdir('gdb-4.10.pl1/gdb/29k-share', 'gdb-4.10.pl1/gdb/29k-share/udi').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/config').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/a29k').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/alpha').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/arm').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/convex').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/gould').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/h8300').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/h8500').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/i386').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/i960').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/m68k').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/m88k').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/mips').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/none').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/ns32k').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/pa').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/pyr').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/romp').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/rs6000').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/sh').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/sparc').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/tahoe').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/vax').
dir_subdir('gdb-4.10.pl1/gdb/config', 'gdb-4.10.pl1/gdb/config/z8k').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/doc').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/gdbserver').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/nindy-share').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/sparclite').
dir_subdir('gdb-4.10.pl1/gdb', 'gdb-4.10.pl1/gdb/vx-share').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/glob').
dir_subdir('gdb-4.10.pl1/glob', 'gdb-4.10.pl1/glob/config').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/include').
dir_subdir('gdb-4.10.pl1/include', 'gdb-4.10.pl1/include/aout').
dir_subdir('gdb-4.10.pl1/include', 'gdb-4.10.pl1/include/coff').
dir_subdir('gdb-4.10.pl1/include', 'gdb-4.10.pl1/include/elf').
dir_subdir('gdb-4.10.pl1/include', 'gdb-4.10.pl1/include/nlm').
dir_subdir('gdb-4.10.pl1/include', 'gdb-4.10.pl1/include/opcode').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/libiberty').
dir_subdir('gdb-4.10.pl1/libiberty', 'gdb-4.10.pl1/libiberty/config').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/mmalloc').
dir_subdir('gdb-4.10.pl1/mmalloc', 'gdb-4.10.pl1/mmalloc/config').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/opcodes').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/readline').
dir_subdir('gdb-4.10.pl1/readline', 'gdb-4.10.pl1/readline/config').
dir_subdir('gdb-4.10.pl1/readline', 'gdb-4.10.pl1/readline/doc').
dir_subdir('gdb-4.10.pl1/readline', 'gdb-4.10.pl1/readline/examples').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/sim').
dir_subdir('gdb-4.10.pl1/sim', 'gdb-4.10.pl1/sim/h8300').
dir_subdir('gdb-4.10.pl1/sim', 'gdb-4.10.pl1/sim/h8500').
dir_subdir('gdb-4.10.pl1/sim', 'gdb-4.10.pl1/sim/sh').
dir_subdir('gdb-4.10.pl1/sim', 'gdb-4.10.pl1/sim/z8k').
dir_subdir('gdb-4.10.pl1', 'gdb-4.10.pl1/texinfo').
dir_subdir('.', 'glibc-1.06').
dir_subdir('glibc-1.06', 'glibc-1.06/arpa').
dir_subdir('glibc-1.06', 'glibc-1.06/assert').
dir_subdir('glibc-1.06', 'glibc-1.06/conf').
dir_subdir('glibc-1.06', 'glibc-1.06/ctype').
dir_subdir('glibc-1.06', 'glibc-1.06/dirent').
dir_subdir('glibc-1.06', 'glibc-1.06/gnu').
dir_subdir('glibc-1.06', 'glibc-1.06/gnulib').
dir_subdir('glibc-1.06', 'glibc-1.06/grp').
dir_subdir('glibc-1.06', 'glibc-1.06/hurd').
dir_subdir('glibc-1.06', 'glibc-1.06/inet').
dir_subdir('glibc-1.06/inet', 'glibc-1.06/inet/arpa').
dir_subdir('glibc-1.06/inet', 'glibc-1.06/inet/netinet').
dir_subdir('glibc-1.06/inet', 'glibc-1.06/inet/protocols').
dir_subdir('glibc-1.06/inet', 'glibc-1.06/inet/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/io').
dir_subdir('glibc-1.06/io', 'glibc-1.06/io/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/locale').
dir_subdir('glibc-1.06', 'glibc-1.06/mach').
dir_subdir('glibc-1.06', 'glibc-1.06/malloc').
dir_subdir('glibc-1.06/malloc', 'glibc-1.06/malloc/malloc').
dir_subdir('glibc-1.06', 'glibc-1.06/manual').
dir_subdir('glibc-1.06/manual', 'glibc-1.06/manual/examples').
dir_subdir('glibc-1.06', 'glibc-1.06/math').
dir_subdir('glibc-1.06', 'glibc-1.06/misc').
dir_subdir('glibc-1.06/misc', 'glibc-1.06/misc/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/netinet').
dir_subdir('glibc-1.06', 'glibc-1.06/posix').
dir_subdir('glibc-1.06/posix', 'glibc-1.06/posix/gnu').
dir_subdir('glibc-1.06/posix', 'glibc-1.06/posix/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/protocols').
dir_subdir('glibc-1.06', 'glibc-1.06/pwd').
dir_subdir('glibc-1.06', 'glibc-1.06/resource').
dir_subdir('glibc-1.06/resource', 'glibc-1.06/resource/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/setjmp').
dir_subdir('glibc-1.06', 'glibc-1.06/signal').
dir_subdir('glibc-1.06/signal', 'glibc-1.06/signal/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/socket').
dir_subdir('glibc-1.06/socket', 'glibc-1.06/socket/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/stdio').
dir_subdir('glibc-1.06', 'glibc-1.06/stdlib').
dir_subdir('glibc-1.06', 'glibc-1.06/string').
dir_subdir('glibc-1.06', 'glibc-1.06/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/sysdeps').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/alpha').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/am29k').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/generic').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/i386').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/i860').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/ieee754').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/m68k').
dir_subdir('glibc-1.06/sysdeps/m68k', 'glibc-1.06/sysdeps/m68k/fpu').
dir_subdir('glibc-1.06/sysdeps/m68k/fpu', 'glibc-1.06/sysdeps/m68k/fpu/switch').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/m88k').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/mach').
dir_subdir('glibc-1.06/sysdeps/mach', 'glibc-1.06/sysdeps/mach/hurd').
dir_subdir('glibc-1.06/sysdeps/mach/hurd', 'glibc-1.06/sysdeps/mach/hurd/i386').
dir_subdir('glibc-1.06/sysdeps/mach', 'glibc-1.06/sysdeps/mach/i386').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/mips').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/posix').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/rs6000').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/sparc').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/stub').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/tahoe').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/unix').
dir_subdir('glibc-1.06/sysdeps/unix', 'glibc-1.06/sysdeps/unix/bsd').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/bsd4.4').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/hp').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/hp', 'glibc-1.06/sysdeps/unix/bsd/hp/m68k').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/i386').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/m68k').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/sequent').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sequent', 'glibc-1.06/sysdeps/unix/bsd/sequent/i386').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/sony').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sony', 'glibc-1.06/sysdeps/unix/bsd/sony/m68k').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sony', 'glibc-1.06/sysdeps/unix/bsd/sony/newsos4').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sony/newsos4', 'glibc-1.06/sysdeps/unix/bsd/sony/newsos4/m68k').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/sun').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sun', 'glibc-1.06/sysdeps/unix/bsd/sun/m68k').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sun', 'glibc-1.06/sysdeps/unix/bsd/sun/sparc').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sun', 'glibc-1.06/sysdeps/unix/bsd/sun/sunos3').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sun/sunos3', 'glibc-1.06/sysdeps/unix/bsd/sun/sunos3/m68k').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/sun', 'glibc-1.06/sysdeps/unix/bsd/sun/sunos4').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/tahoe').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/ultrix4').
dir_subdir('glibc-1.06/sysdeps/unix/bsd/ultrix4', 'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips').
dir_subdir('glibc-1.06/sysdeps/unix/bsd', 'glibc-1.06/sysdeps/unix/bsd/vax').
dir_subdir('glibc-1.06/sysdeps/unix', 'glibc-1.06/sysdeps/unix/common').
dir_subdir('glibc-1.06/sysdeps/unix', 'glibc-1.06/sysdeps/unix/i386').
dir_subdir('glibc-1.06/sysdeps/unix', 'glibc-1.06/sysdeps/unix/inet').
dir_subdir('glibc-1.06/sysdeps/unix', 'glibc-1.06/sysdeps/unix/sparc').
dir_subdir('glibc-1.06/sysdeps/unix', 'glibc-1.06/sysdeps/unix/sysv').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/i386').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/isc2.2').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/linux').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/minix').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/sco3.2').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/sco3.2.4').
dir_subdir('glibc-1.06/sysdeps/unix/sysv', 'glibc-1.06/sysdeps/unix/sysv/sysv4').
dir_subdir('glibc-1.06/sysdeps/unix/sysv/sysv4', 'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2').
dir_subdir('glibc-1.06/sysdeps', 'glibc-1.06/sysdeps/vax').
dir_subdir('glibc-1.06', 'glibc-1.06/termios').
dir_subdir('glibc-1.06/termios', 'glibc-1.06/termios/sys').
dir_subdir('glibc-1.06', 'glibc-1.06/time').
dir_subdir('glibc-1.06/time', 'glibc-1.06/time/sys').
dir_subdir('.', 'gnuplot-3.4').
dir_subdir('gnuplot-3.4', 'gnuplot-3.4/demo').
dir_subdir('gnuplot-3.4', 'gnuplot-3.4/docs').
dir_subdir('gnuplot-3.4/docs', 'gnuplot-3.4/docs/latextut').
dir_subdir('gnuplot-3.4', 'gnuplot-3.4/os2').
dir_subdir('gnuplot-3.4', 'gnuplot-3.4/term').
dir_subdir('gnuplot-3.4', 'gnuplot-3.4/win').
dir_subdir('.', 'gperf-2.1').
dir_subdir('gperf-2.1', 'gperf-2.1/src').
dir_subdir('gperf-2.1', 'gperf-2.1/tests').
dir_subdir('.', 'grep-1.5').
dir_subdir('grep-1.5', 'grep-1.5/tests').
dir_subdir('.', 'groff-1.06').
dir_subdir('groff-1.06', 'groff-1.06/addftinfo').
dir_subdir('groff-1.06', 'groff-1.06/afmtodit').
dir_subdir('groff-1.06', 'groff-1.06/devascii').
dir_subdir('groff-1.06', 'groff-1.06/devdvi').
dir_subdir('groff-1.06/devdvi', 'groff-1.06/devdvi/generate').
dir_subdir('groff-1.06', 'groff-1.06/devlatin1').
dir_subdir('groff-1.06', 'groff-1.06/devps').
dir_subdir('groff-1.06/devps', 'groff-1.06/devps/generate').
dir_subdir('groff-1.06', 'groff-1.06/doc').
dir_subdir('groff-1.06', 'groff-1.06/eqn').
dir_subdir('groff-1.06', 'groff-1.06/grodvi').
dir_subdir('groff-1.06', 'groff-1.06/groff').
dir_subdir('groff-1.06', 'groff-1.06/grog').
dir_subdir('groff-1.06', 'groff-1.06/grops').
dir_subdir('groff-1.06', 'groff-1.06/grotty').
dir_subdir('groff-1.06', 'groff-1.06/include').
dir_subdir('groff-1.06', 'groff-1.06/indxbib').
dir_subdir('groff-1.06', 'groff-1.06/libbib').
dir_subdir('groff-1.06', 'groff-1.06/libdriver').
dir_subdir('groff-1.06', 'groff-1.06/libgroff').
dir_subdir('groff-1.06', 'groff-1.06/lkbib').
dir_subdir('groff-1.06', 'groff-1.06/lookbib').
dir_subdir('groff-1.06', 'groff-1.06/man').
dir_subdir('groff-1.06', 'groff-1.06/mm').
dir_subdir('groff-1.06/mm', 'groff-1.06/mm/mm').
dir_subdir('groff-1.06', 'groff-1.06/nroff').
dir_subdir('groff-1.06', 'groff-1.06/pfbtops').
dir_subdir('groff-1.06', 'groff-1.06/pic').
dir_subdir('groff-1.06', 'groff-1.06/psbb').
dir_subdir('groff-1.06', 'groff-1.06/refer').
dir_subdir('groff-1.06', 'groff-1.06/soelim').
dir_subdir('groff-1.06', 'groff-1.06/tbl').
dir_subdir('groff-1.06', 'groff-1.06/tfmtodit').
dir_subdir('groff-1.06', 'groff-1.06/tmac').
dir_subdir('groff-1.06', 'groff-1.06/troff').
dir_subdir('groff-1.06', 'groff-1.06/xditview').
dir_subdir('.', 'gs26').
dir_subdir('gs26', 'gs26/fonts').
dir_subdir('.', 'gzip-1.2.3').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/amiga').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/atari').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/msdos').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/os2').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/primos').
dir_subdir('gzip-1.2.3/primos', 'gzip-1.2.3/primos/include').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/sample').
dir_subdir('gzip-1.2.3', 'gzip-1.2.3/vms').
dir_subdir('.', 'libg++-2.4').
dir_subdir('libg++-2.4', 'libg++-2.4/config').
dir_subdir('libg++-2.4', 'libg++-2.4/etc').
dir_subdir('libg++-2.4', 'libg++-2.4/include').
dir_subdir('libg++-2.4', 'libg++-2.4/libg++').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/config').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/etc').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/ADT-examples').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/PlotFile3D').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/benchmarks').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/fib').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/graph').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/lf').
dir_subdir('libg++-2.4/libg++/etc', 'libg++-2.4/libg++/etc/trie-gen').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/g++-include').
dir_subdir('libg++-2.4/libg++/g++-include', 'libg++-2.4/libg++/g++-include/sys').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/genclass').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/gperf').
dir_subdir('libg++-2.4/libg++/gperf', 'libg++-2.4/libg++/gperf/src').
dir_subdir('libg++-2.4/libg++/gperf', 'libg++-2.4/libg++/gperf/tests').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/iostream').
dir_subdir('libg++-2.4/libg++/iostream', 'libg++-2.4/libg++/iostream/stdio').
dir_subdir('libg++-2.4/libg++/iostream', 'libg++-2.4/libg++/iostream/test').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/no-stream').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/old-stream').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/proto-kit').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/src').
dir_subdir('libg++-2.4/libg++/src', 'libg++-2.4/libg++/src/gen').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/test-install').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/tests').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/utils').
dir_subdir('libg++-2.4/libg++', 'libg++-2.4/libg++/vms').
dir_subdir('libg++-2.4', 'libg++-2.4/libiberty').
dir_subdir('libg++-2.4/libiberty', 'libg++-2.4/libiberty/config').
dir_subdir('libg++-2.4', 'libg++-2.4/texinfo').
dir_subdir('libg++-2.4', 'libg++-2.4/xiberty').
dir_subdir('.', 'make-3.68').
dir_subdir('make-3.68', 'make-3.68/glob').
dir_subdir('.', 'patch-2.1').
dir_subdir('.', 'portability').
dir_subdir('.', 'rcs').
dir_subdir('rcs', 'rcs/man').
dir_subdir('rcs', 'rcs/src').
dir_subdir('.', 'rdist-6.0.0').
dir_subdir('.', 'regex-0.12').
dir_subdir('regex-0.12', 'regex-0.12/doc').
dir_subdir('regex-0.12', 'regex-0.12/test').
dir_subdir('.', 'regex-doc').
dir_subdir('.', 'sed-1.17').
dir_subdir('.', 'shellutils-1.8').
dir_subdir('shellutils-1.8', 'shellutils-1.8/lib').
dir_subdir('shellutils-1.8', 'shellutils-1.8/man').
dir_subdir('shellutils-1.8', 'shellutils-1.8/src').
dir_subdir('.', 'tar-1.11.2').
dir_subdir('.', 'termcap-1.1').
dir_subdir('.', 'texi2roff').
dir_subdir('.', 'texinfo-3.1').
dir_subdir('texinfo-3.1', 'texinfo-3.1/emacs').
dir_subdir('texinfo-3.1', 'texinfo-3.1/info').
dir_subdir('texinfo-3.1', 'texinfo-3.1/libtxi').
dir_subdir('texinfo-3.1', 'texinfo-3.1/makeinfo').
dir_subdir('texinfo-3.1', 'texinfo-3.1/util').
dir_subdir('.', 'time-1.4').


file('TeX-draw',dir,'.', 2560 ).
file('att',dir,'.', 512 ).
file('bash-1.12',dir,'.', 2560 ).
file('bash-1.12.diffs',text,'.', 7943 ).
file('binutils-2.2.1',dir,'.', 512 ).
file('bison-1.21',dir,'.', 2048 ).
file('dat.dvi',text,'.', 19440 ).
file('diff',dir,'.', 512 ).
file('dir',text,'.', 18825 ).
file('dld-3.2.3',dir,'.', 1024 ).
file('ecc-1.2',dir,'.', 512 ).
file('expect3.24',dir,'.', 512 ).
file('fileutils-3.6',dir,'.', 512 ).
file('flex-2.3.8',dir,'.', 1024 ).
file('fontutils-0.6',dir,'.', 1024 ).
file('gas-2.1.1',dir,'.', 512 ).
file('gawk-2.15.2',dir,'.', 2048 ).
file('gcc-2.4.5',dir,'.', 9728 ).
file('gdb-4.10.pl1',dir,'.', 512 ).
file('glibc-1.06',dir,'.', 3072 ).
file('gnuplot-3.4',dir,'.', 2560 ).
file('gperf-2.1',dir,'.', 512 ).
file('grep-1.5',dir,'.', 512 ).
file('groff-1.06',dir,'.', 1536 ).
file('gs26',dir,'.', 9216 ).
file('gzip-1.2.3',dir,'.', 1536 ).
file('libg++-2.4',dir,'.', 512 ).
file('make-3.68',dir,'.', 2048 ).
file('patch-2.1',dir,'.', 1024 ).
file('portability',dir,'.', 1024 ).
file('rcs',dir,'.', 512 ).
file('rdist-6.0.0',dir,'.', 1024 ).
file('regex-0.12',dir,'.', 512 ).
file('regex-doc',dir,'.', 512 ).
file('sed-1.17',dir,'.', 512 ).
file('shellutils-1.8',dir,'.', 512 ).
file('standards.info',text,'.', 48854 ).
file('standards.texi',text,'.', 48250 ).
file('standards.text',text,'.', 30925 ).
file('tar-1.11.2',dir,'.', 1024 ).
file('termcap-1.1',dir,'.', 1024 ).
file('texi2roff',dir,'.', 512 ).
file('texinfo-3.1',dir,'.', 1024 ).
file('time-1.4',dir,'.', 1024 ).
file('AlDraTex.sty',text,'TeX-draw', 78006 ).
file('AlProTex.sty',text,'TeX-draw', 14097 ).
file('DraTex.sty',text,'TeX-draw', 54849 ).
file('Examples.tex',text,'TeX-draw', 33447 ).
file('Examples.trc',text,'TeX-draw', 12848 ).
file('ProTex.sty',text,'TeX-draw', 9621 ).
file('execute.me',exec,'TeX-draw', 4140 ).
file('COPYING',text,'bash-1.12', 12934 ).
file('ChangeLog',text,'bash-1.12', 40107 ).
file('INSTALL',text,'bash-1.12', 6869 ).
file('Makefile',text,'bash-1.12', 3321 ).
file('Makefile.orig',text,'bash-1.12', 3315 ).
file('README',text,'bash-1.12', 1571 ).
file('RELEASE',text,'bash-1.12', 3120 ).
file('alias.c',text,'bash-1.12', 11919 ).
file('alias.h',text,'bash-1.12', 1905 ).
file('bash-Makefile',text,'bash-1.12', 17669 ).
file('bashline.c',text,'bash-1.12', 35379 ).
file('braces.c',text,'bash-1.12', 8425 ).
file('builtins',dir,'bash-1.12', 1536 ).
file('builtins.h',text,'bash-1.12', 1297 ).
file('command.h',text,'bash-1.12', 6634 ).
file('compile',exec,'bash-1.12', 88 ).
file('config.h',text,'bash-1.12', 3985 ).
file('config.h.orig',text,'bash-1.12', 3966 ).
file('configure',exec,'bash-1.12', 251 ).
file('copy_cmd.c',text,'bash-1.12', 7351 ).
file('cpp-Makefile',text,'bash-1.12', 27936 ).
file('cpp-Makefile.orig',text,'bash-1.12', 27945 ).
file('dispose_cmd.c',text,'bash-1.12', 3873 ).
file('documentation',dir,'bash-1.12', 512 ).
file('endian.aux',exec,'bash-1.12', 11108 ).
file('endian.c',text,'bash-1.12', 2219 ).
file('endian.h',text,'bash-1.12', 305 ).
file('error.c',text,'bash-1.12', 3195 ).
file('error.h',text,'bash-1.12', 466 ).
file('examples',dir,'bash-1.12', 512 ).
file('execute_cmd.c',text,'bash-1.12', 78981 ).
file('expr.c',text,'bash-1.12', 13702 ).
file('filecntl.h',text,'bash-1.12', 468 ).
file('flags.c',text,'bash-1.12', 6536 ).
file('flags.h',text,'bash-1.12', 1006 ).
file('general.c',text,'bash-1.12', 20208 ).
file('general.h',text,'bash-1.12', 1991 ).
file('getcwd.c',text,'bash-1.12', 4240 ).
file('hash.c',text,'bash-1.12', 5779 ).
file('hash.c.orig',text,'bash-1.12', 5945 ).
file('hash.h',text,'bash-1.12', 938 ).
file('input.h',text,'bash-1.12', 611 ).
file('jobs.c',text,'bash-1.12', 54966 ).
file('jobs.h',text,'bash-1.12', 7109 ).
file('lib',dir,'bash-1.12', 512 ).
file('longest_sig.c',text,'bash-1.12', 1365 ).
file('machines.h',text,'bash-1.12', 33674 ).
file('machines.h.orig',text,'bash-1.12', 33214 ).
file('mailcheck.c',text,'bash-1.12', 10717 ).
file('make_cmd.c',text,'bash-1.12', 13154 ).
file('make_cmd.c.orig',text,'bash-1.12', 13134 ).
file('maxpath.h',text,'bash-1.12', 520 ).
file('newversion.aux',exec,'bash-1.12', 12892 ).
file('newversion.c',text,'bash-1.12', 4089 ).
file('nojobs.c',text,'bash-1.12', 9192 ).
file('parse.y',text,'bash-1.12', 70260 ).
file('parser.h',text,'bash-1.12', 206 ).
file('posixstat.h',text,'bash-1.12', 4566 ).
file('print_cmd.c',text,'bash-1.12', 13272 ).
file('quit.h',text,'bash-1.12', 425 ).
file('shell.c',text,'bash-1.12', 39893 ).
file('shell.h',text,'bash-1.12', 2011 ).
file('siglist.c',text,'bash-1.12', 4493 ).
file('subst.c',text,'bash-1.12', 84104 ).
file('support',dir,'bash-1.12', 512 ).
file('sysdefs.h',text,'bash-1.12', 773 ).
file('test-suite',dir,'bash-1.12', 512 ).
file('test.c',text,'bash-1.12', 23183 ).
file('trap.c',text,'bash-1.12', 13333 ).
file('trap.h',text,'bash-1.12', 361 ).
file('unwind_prot.c',text,'bash-1.12', 7059 ).
file('unwind_prot.h',text,'bash-1.12', 959 ).
file('variables.c',text,'bash-1.12', 39016 ).
file('variables.h',text,'bash-1.12', 2355 ).
file('version.c',text,'bash-1.12', 932 ).
file('version.h',text,'bash-1.12', 295 ).
file('y.tab.c',text,'bash-1.12', 108172 ).
file('y.tab.h',text,'bash-1.12', 823 ).
file('Makefile',text,'bash-1.12/builtins', 2258 ).
file('Makefile.orig',text,'bash-1.12/builtins', 2259 ).
file('alias.def',text,'bash-1.12/builtins', 4114 ).
file('bind.def',text,'bash-1.12/builtins', 4182 ).
file('break.def',text,'bash-1.12/builtins', 2763 ).
file('builtin.def',text,'bash-1.12/builtins', 1987 ).
file('cd.def',text,'bash-1.12/builtins', 14870 ).
file('colon.def',text,'bash-1.12/builtins', 1125 ).
file('command.def',text,'bash-1.12/builtins', 3980 ).
file('common.c',text,'bash-1.12/builtins', 14889 ).
file('declare.def',text,'bash-1.12/builtins', 6597 ).
file('echo.def',text,'bash-1.12/builtins', 4328 ).
file('enable.def',text,'bash-1.12/builtins', 2748 ).
file('eval.def',text,'bash-1.12/builtins', 1399 ).
file('exec.def',text,'bash-1.12/builtins', 3664 ).
file('exit.def',text,'bash-1.12/builtins', 3472 ).
file('fc.def',text,'bash-1.12/builtins', 16172 ).
file('fg_bg.def',text,'bash-1.12/builtins', 2975 ).
file('getopt.c',text,'bash-1.12/builtins', 17828 ).
file('getopt.h',text,'bash-1.12/builtins', 3767 ).
file('getopts.def',text,'bash-1.12/builtins', 7096 ).
file('hash.def',text,'bash-1.12/builtins', 5034 ).
file('hashcom.h',text,'bash-1.12/builtins', 349 ).
file('hashcom.h.orig',text,'bash-1.12/builtins', 342 ).
file('help.def',text,'bash-1.12/builtins', 3346 ).
file('history.def',text,'bash-1.12/builtins', 4272 ).
file('jobs.def',text,'bash-1.12/builtins', 4665 ).
file('kill.def',text,'bash-1.12/builtins', 6373 ).
file('let.def',text,'bash-1.12/builtins', 2318 ).
file('mkbuiltins',exec,'bash-1.12/builtins', 44764 ).
file('mkbuiltins.c',text,'bash-1.12/builtins', 30860 ).
file('psize.c',text,'bash-1.12/builtins', 1630 ).
file('psize.sh',exec,'bash-1.12/builtins', 389 ).
file('read.def',text,'bash-1.12/builtins', 5356 ).
file('reserved.def',text,'bash-1.12/builtins', 5034 ).
file('return.def',text,'bash-1.12/builtins', 1726 ).
file('set.def',text,'bash-1.12/builtins', 12708 ).
file('setattr.def',text,'bash-1.12/builtins', 6008 ).
file('shift.def',text,'bash-1.12/builtins', 2305 ).
file('source.def',text,'bash-1.12/builtins', 4141 ).
file('suspend.def',text,'bash-1.12/builtins', 2225 ).
file('test.def',text,'bash-1.12/builtins', 4604 ).
file('times.def',text,'bash-1.12/builtins', 2639 ).
file('trap.def',text,'bash-1.12/builtins', 4777 ).
file('type.def',text,'bash-1.12/builtins', 6053 ).
file('ulimit.def',text,'bash-1.12/builtins', 13504 ).
file('umask.def',text,'bash-1.12/builtins', 6393 ).
file('wait.def',text,'bash-1.12/builtins', 3801 ).
file('Makefile',text,'bash-1.12/documentation', 768 ).
file('bash.1',text,'bash-1.12/documentation', 105596 ).
file('features.dvi',text,'bash-1.12/documentation', 86156 ).
file('features.texi',text,'bash-1.12/documentation', 25062 ).
file('texindex.c',text,'bash-1.12/documentation', 38493 ).
file('texinfo.tex',text,'bash-1.12/documentation', 92648 ).
file('alias-conv.sh',exec,'bash-1.12/examples', 742 ).
file('functions',dir,'bash-1.12/examples', 512 ).
file('scripts',dir,'bash-1.12/examples', 512 ).
file('startup-files',dir,'bash-1.12/examples', 512 ).
file('suncmd.termcap',text,'bash-1.12/examples', 1458 ).
file('whatis',text,'bash-1.12/examples', 1158 ).
file('autoload',text,'bash-1.12/examples/functions', 2635 ).
file('basename',text,'bash-1.12/examples/functions', 645 ).
file('csh-compat',text,'bash-1.12/examples/functions', 597 ).
file('dirname',text,'bash-1.12/examples/functions', 573 ).
file('exitstat',text,'bash-1.12/examples/functions', 614 ).
file('external',text,'bash-1.12/examples/functions', 1338 ).
file('external.~1~',text,'bash-1.12/examples/functions', 720 ).
file('fact',text,'bash-1.12/examples/functions', 303 ).
file('fstty',text,'bash-1.12/examples/functions', 1069 ).
file('ksh-compat',text,'bash-1.12/examples/functions', 2643 ).
file('ksh-compat.~1~',text,'bash-1.12/examples/functions', 2554 ).
file('kshenv',text,'bash-1.12/examples/functions', 1552 ).
file('shcat',text,'bash-1.12/examples/functions', 54 ).
file('substr',text,'bash-1.12/examples/functions', 1148 ).
file('substr2',text,'bash-1.12/examples/functions', 1390 ).
file('term',text,'bash-1.12/examples/functions', 480 ).
file('whatis',text,'bash-1.12/examples/functions', 993 ).
file('whence',text,'bash-1.12/examples/functions', 985 ).
file('whence2',text,'bash-1.12/examples/functions', 957 ).
file('shprompt',text,'bash-1.12/examples/scripts', 2224 ).
file('Bash_aliases',text,'bash-1.12/examples/startup-files', 1184 ).
file('Bash_profile',text,'bash-1.12/examples/startup-files', 358 ).
file('Bashrc',text,'bash-1.12/examples/startup-files', 1816 ).
file('glob',dir,'bash-1.12/lib', 512 ).
file('malloc',dir,'bash-1.12/lib', 512 ).
file('readline',dir,'bash-1.12/lib', 512 ).
file('ChangeLog',text,'bash-1.12/lib/glob', 24 ).
file('Makefile',text,'bash-1.12/lib/glob', 515 ).
file('Makefile.orig',text,'bash-1.12/lib/glob', 516 ).
file('fnmatch.c',text,'bash-1.12/lib/glob', 3852 ).
file('fnmatch.h',text,'bash-1.12/lib/glob', 1445 ).
file('glob.c',text,'bash-1.12/lib/glob', 12406 ).
file('glob.texinfo',text,'bash-1.12/lib/glob', 22 ).
file('tilde.c',text,'bash-1.12/lib/glob', 8958 ).
file('alloca.c',text,'bash-1.12/lib/malloc', 5266 ).
file('getpagesize.h',text,'bash-1.12/lib/malloc', 416 ).
file('i386-alloca.s',text,'bash-1.12/lib/malloc', 269 ).
file('malloc.c',text,'bash-1.12/lib/malloc', 24083 ).
file('x386-alloca.s',text,'bash-1.12/lib/malloc', 1750 ).
file('xmalloc.c',text,'bash-1.12/lib/malloc', 1934 ).
file('COPYING',text,'bash-1.12/lib/readline', 12929 ).
file('ChangeLog',text,'bash-1.12/lib/readline', 7395 ).
file('Makefile',text,'bash-1.12/lib/readline', 4129 ).
file('Makefile.orig',text,'bash-1.12/lib/readline', 4130 ).
file('chardefs.h',text,'bash-1.12/lib/readline', 1684 ).
file('doc',dir,'bash-1.12/lib/readline', 512 ).
file('emacs_keymap.c',text,'bash-1.12/lib/readline', 18513 ).
file('examples',dir,'bash-1.12/lib/readline', 512 ).
file('funmap.c',text,'bash-1.12/lib/readline', 8028 ).
file('history.c',text,'bash-1.12/lib/readline', 40020 ).
file('history.h',text,'bash-1.12/lib/readline', 4844 ).
file('keymaps.c',text,'bash-1.12/lib/readline', 4051 ).
file('keymaps.h',text,'bash-1.12/lib/readline', 1546 ).
file('readline.c',text,'bash-1.12/lib/readline', 154088 ).
file('readline.c.orig',text,'bash-1.12/lib/readline', 153849 ).
file('readline.h',text,'bash-1.12/lib/readline', 8183 ).
file('vi_keymap.c',text,'bash-1.12/lib/readline', 17702 ).
file('vi_mode.c',text,'bash-1.12/lib/readline', 24501 ).
file('Makefile',text,'bash-1.12/lib/readline/doc', 1010 ).
file('hist.texinfo',text,'bash-1.12/lib/readline/doc', 3440 ).
file('history.dvi',text,'bash-1.12/lib/readline/doc', 23392 ).
file('history.info',text,'bash-1.12/lib/readline/doc', 17943 ).
file('hstech.texinfo',text,'bash-1.12/lib/readline/doc', 10512 ).
file('hsuser.texinfo',text,'bash-1.12/lib/readline/doc', 5068 ).
file('readline.dvi',text,'bash-1.12/lib/readline/doc', 95672 ).
file('readline.info',text,'bash-1.12/lib/readline/doc', 62791 ).
file('rlman.texinfo',text,'bash-1.12/lib/readline/doc', 3417 ).
file('rltech.texinfo',text,'bash-1.12/lib/readline/doc', 32512 ).
file('rluser.texinfo',text,'bash-1.12/lib/readline/doc', 19556 ).
file('texindex.c',text,'bash-1.12/lib/readline/doc', 38493 ).
file('texinfo.tex',text,'bash-1.12/lib/readline/doc', 92648 ).
file('Inputrc',text,'bash-1.12/lib/readline/examples', 1503 ).
file('Makefile',text,'bash-1.12/lib/readline/examples', 248 ).
file('fileman.c',text,'bash-1.12/lib/readline/examples', 9051 ).
file('manexamp.c',text,'bash-1.12/lib/readline/examples', 2465 ).
file('bash.xbm',text,'bash-1.12/support', 3325 ).
file('cat-s',text,'bash-1.12/support', 247 ).
file('cppmagic',exec,'bash-1.12/support', 450 ).
file('getcppsyms',exec,'bash-1.12/support', 7616 ).
file('getcppsyms.c',text,'bash-1.12/support', 6961 ).
file('getcppsyms.c.orig',text,'bash-1.12/support', 6909 ).
file('inform',text,'bash-1.12/support', 1540 ).
file('mail-shell',exec,'bash-1.12/support', 1323 ).
file('mksysdefs',exec,'bash-1.12/support', 4677 ).
file('mksysdefs.orig',exec,'bash-1.12/support', 5089 ).
file('mktarfile',exec,'bash-1.12/support', 1453 ).
file('precedence',exec,'bash-1.12/test-suite', 1241 ).
file('Makefile',text,'binutils-2.2.1', 30794 ).
file('Makefile.in',text,'binutils-2.2.1', 29632 ).
file('README',text,'binutils-2.2.1', 1190 ).
file('bfd',dir,'binutils-2.2.1', 2560 ).
file('binutils',dir,'binutils-2.2.1', 1536 ).
file('config',dir,'binutils-2.2.1', 512 ).
file('config.guess',exec,'binutils-2.2.1', 4427 ).
file('config.status',exec,'binutils-2.2.1', 202 ).
file('config.sub',exec,'binutils-2.2.1', 16490 ).
file('configure',exec,'binutils-2.2.1', 38075 ).
file('configure.in',text,'binutils-2.2.1', 7178 ).
file('etc',dir,'binutils-2.2.1', 512 ).
file('gprof',dir,'binutils-2.2.1', 1024 ).
file('include',dir,'binutils-2.2.1', 512 ).
file('ld',dir,'binutils-2.2.1', 2560 ).
file('libiberty',dir,'binutils-2.2.1', 1536 ).
file('move-if-change',exec,'binutils-2.2.1', 129 ).
file('opcodes',dir,'binutils-2.2.1', 1024 ).
file('texinfo',dir,'binutils-2.2.1', 512 ).
file('COPYING',text,'binutils-2.2.1/bfd', 17982 ).
file('ChangeLog',text,'binutils-2.2.1/bfd', 161851 ).
file('Makefile',text,'binutils-2.2.1/bfd', 16354 ).
file('Makefile.in',text,'binutils-2.2.1/bfd', 15749 ).
file('PORTING',text,'binutils-2.2.1/bfd', 2154 ).
file('README.hppaelf',text,'binutils-2.2.1/bfd', 774 ).
file('TODO',text,'binutils-2.2.1/bfd', 1363 ).
file('VERSION',text,'binutils-2.2.1/bfd', 4 ).
file('aix386-core.c',text,'binutils-2.2.1/bfd', 11107 ).
file('aout-adobe.c',text,'binutils-2.2.1/bfd', 16201 ).
file('aout-encap.c',text,'binutils-2.2.1/bfd', 6873 ).
file('aout-target.h',text,'binutils-2.2.1/bfd', 12699 ).
file('aout32.c',text,'binutils-2.2.1/bfd', 898 ).
file('aout64.c',text,'binutils-2.2.1/bfd', 1039 ).
file('aoutf1.h',text,'binutils-2.2.1/bfd', 18958 ).
file('aoutx.h',text,'binutils-2.2.1/bfd', 76547 ).
file('archive.c',text,'binutils-2.2.1/bfd', 47914 ).
file('archures.c',text,'binutils-2.2.1/bfd', 16226 ).
file('bfd-in.h',text,'binutils-2.2.1/bfd', 12680 ).
file('bfd.c',text,'binutils-2.2.1/bfd', 16881 ).
file('bout.c',text,'binutils-2.2.1/bfd', 39428 ).
file('cache.c',text,'binutils-2.2.1/bfd', 7028 ).
file('coff-a29k.c',text,'binutils-2.2.1/bfd', 10440 ).
file('coff-alpha.c',text,'binutils-2.2.1/bfd', 122074 ).
file('coff-h8300.c',text,'binutils-2.2.1/bfd', 5842 ).
file('coff-h8500.c',text,'binutils-2.2.1/bfd', 8551 ).
file('coff-i386.c',text,'binutils-2.2.1/bfd', 13021 ).
file('coff-i960.c',text,'binutils-2.2.1/bfd', 7243 ).
file('coff-m68k.c',text,'binutils-2.2.1/bfd', 4370 ).
file('coff-m88k.c',text,'binutils-2.2.1/bfd', 4549 ).
file('coff-mips.c',text,'binutils-2.2.1/bfd', 129340 ).
file('coff-msym.c',text,'binutils-2.2.1/bfd', 29715 ).
file('coff-rs6000.c',text,'binutils-2.2.1/bfd', 9115 ).
file('coff-sh.c',text,'binutils-2.2.1/bfd', 5049 ).
file('coff-we32k.c',text,'binutils-2.2.1/bfd', 3427 ).
file('coff-z8k.c',text,'binutils-2.2.1/bfd', 6850 ).
file('coffcode.h',text,'binutils-2.2.1/bfd', 63941 ).
file('coffgen.c',text,'binutils-2.2.1/bfd', 41667 ).
file('coffswap.h',text,'binutils-2.2.1/bfd', 21957 ).
file('config',dir,'binutils-2.2.1/bfd', 2048 ).
file('config.status',exec,'binutils-2.2.1/bfd', 260 ).
file('configure.bat',text,'binutils-2.2.1/bfd', 338 ).
file('configure.host',text,'binutils-2.2.1/bfd', 2480 ).
file('configure.in',text,'binutils-2.2.1/bfd', 4171 ).
file('core.c',text,'binutils-2.2.1/bfd', 2518 ).
file('cpu-a29k.c',text,'binutils-2.2.1/bfd', 1330 ).
file('cpu-alpha.c',text,'binutils-2.2.1/bfd', 1280 ).
file('cpu-h8300.c',text,'binutils-2.2.1/bfd', 10353 ).
file('cpu-h8500.c',text,'binutils-2.2.1/bfd', 5279 ).
file('cpu-hppa.c',text,'binutils-2.2.1/bfd', 1303 ).
file('cpu-i386.c',text,'binutils-2.2.1/bfd', 1280 ).
file('cpu-i960.c',text,'binutils-2.2.1/bfd', 4512 ).
file('cpu-m68k.c',text,'binutils-2.2.1/bfd', 1587 ).
file('cpu-m88k.c',text,'binutils-2.2.1/bfd', 1357 ).
file('cpu-mips.c',text,'binutils-2.2.1/bfd', 1680 ).
file('cpu-rs6000.c',text,'binutils-2.2.1/bfd', 1370 ).
file('cpu-sh.c',text,'binutils-2.2.1/bfd', 1940 ).
file('cpu-sparc.c',text,'binutils-2.2.1/bfd', 1280 ).
file('cpu-vax.c',text,'binutils-2.2.1/bfd', 1321 ).
file('cpu-we32k.c',text,'binutils-2.2.1/bfd', 1361 ).
file('cpu-z8k.c',text,'binutils-2.2.1/bfd', 5214 ).
file('ctor.c',text,'binutils-2.2.1/bfd', 5275 ).
file('demo64.c',text,'binutils-2.2.1/bfd', 1171 ).
file('doc',dir,'binutils-2.2.1/bfd', 1024 ).
file('elf32-hppa.c',text,'binutils-2.2.1/bfd', 46914 ).
file('elf32-hppa.h',text,'binutils-2.2.1/bfd', 15374 ).
file('elf32-i386.c',text,'binutils-2.2.1/bfd', 1847 ).
file('elf32-i860.c',text,'binutils-2.2.1/bfd', 1147 ).
file('elf32-m68k.c',text,'binutils-2.2.1/bfd', 1148 ).
file('elf32-sparc.c',text,'binutils-2.2.1/bfd', 6233 ).
file('elf32-target.h',text,'binutils-2.2.1/bfd', 7592 ).
file('elf32.c',text,'binutils-2.2.1/bfd', 82154 ).
file('filemode.c',text,'binutils-2.2.1/bfd', 4489 ).
file('format.c',text,'binutils-2.2.1/bfd', 7016 ).
file('gen-aout.c',text,'binutils-2.2.1/bfd', 2797 ).
file('host-aout.c',text,'binutils-2.2.1/bfd', 2292 ).
file('hosts',dir,'binutils-2.2.1/bfd', 1024 ).
file('hp300bsd.c',text,'binutils-2.2.1/bfd', 1315 ).
file('hp300hpux.c',text,'binutils-2.2.1/bfd', 26468 ).
file('hppa.c',text,'binutils-2.2.1/bfd', 26939 ).
file('i386aout.c',text,'binutils-2.2.1/bfd', 2075 ).
file('i386bsd.c',text,'binutils-2.2.1/bfd', 1475 ).
file('i386linux.c',text,'binutils-2.2.1/bfd', 1538 ).
file('ieee.c',text,'binutils-2.2.1/bfd', 73353 ).
file('init.c',text,'binutils-2.2.1/bfd', 1780 ).
file('libaout.h',text,'binutils-2.2.1/bfd', 12699 ).
file('libbfd-in.h',text,'binutils-2.2.1/bfd', 8045 ).
file('libbfd.c',text,'binutils-2.2.1/bfd', 19801 ).
file('libbfd.h',text,'binutils-2.2.1/bfd', 9754 ).
file('libcoff-in.h',text,'binutils-2.2.1/bfd', 4546 ).
file('libcoff.h',text,'binutils-2.2.1/bfd', 11906 ).
file('libecoff.h',text,'binutils-2.2.1/bfd', 3391 ).
file('libelf.h',text,'binutils-2.2.1/bfd', 3252 ).
file('libhppa.h',text,'binutils-2.2.1/bfd', 5443 ).
file('libieee.h',text,'binutils-2.2.1/bfd', 3422 ).
file('liboasys.h',text,'binutils-2.2.1/bfd', 2225 ).
file('mipsbsd.c',text,'binutils-2.2.1/bfd', 12404 ).
file('newsos3.c',text,'binutils-2.2.1/bfd', 1448 ).
file('oasys.c',text,'binutils-2.2.1/bfd', 36044 ).
file('opncls.c',text,'binutils-2.2.1/bfd', 12356 ).
file('reloc.c',text,'binutils-2.2.1/bfd', 29351 ).
file('reloc16.c',text,'binutils-2.2.1/bfd', 9846 ).
file('rs6000-core.c',text,'binutils-2.2.1/bfd', 12103 ).
file('sco-core.c',text,'binutils-2.2.1/bfd', 10516 ).
file('seclet.c',text,'binutils-2.2.1/bfd', 4935 ).
file('seclet.h',text,'binutils-2.2.1/bfd', 1376 ).
file('section.c',text,'binutils-2.2.1/bfd', 23596 ).
file('srec.c',text,'binutils-2.2.1/bfd', 23127 ).
file('stab-syms.c',text,'binutils-2.2.1/bfd', 2168 ).
file('sunos.c',text,'binutils-2.2.1/bfd', 959 ).
file('syms.c',text,'binutils-2.2.1/bfd', 13266 ).
file('targets.c',text,'binutils-2.2.1/bfd', 17477 ).
file('tekhex.c',text,'binutils-2.2.1/bfd', 25167 ).
file('trad-core.c',text,'binutils-2.2.1/bfd', 12139 ).
file('a29k-aout.mt',text,'binutils-2.2.1/bfd/config', 122 ).
file('a29k-coff.mt',text,'binutils-2.2.1/bfd/config', 124 ).
file('adobe.mt',text,'binutils-2.2.1/bfd/config', 90 ).
file('alphaosf.mh',text,'binutils-2.2.1/bfd/config', 31 ).
file('alphaosf.mt',text,'binutils-2.2.1/bfd/config', 148 ).
file('amix.mh',text,'binutils-2.2.1/bfd/config', 114 ).
file('apollov68.mh',text,'binutils-2.2.1/bfd/config', 307 ).
file('bigmips.mt',text,'binutils-2.2.1/bfd/config', 78 ).
file('decstation.mh',text,'binutils-2.2.1/bfd/config', 48 ).
file('decstation.mt',text,'binutils-2.2.1/bfd/config', 82 ).
file('delta88.mh',text,'binutils-2.2.1/bfd/config', 43 ).
file('dgux.mh',text,'binutils-2.2.1/bfd/config', 46 ).
file('go32.mh',text,'binutils-2.2.1/bfd/config', 42 ).
file('h8300-coff.mt',text,'binutils-2.2.1/bfd/config', 248 ).
file('h8500-coff.mt',text,'binutils-2.2.1/bfd/config', 206 ).
file('harris.mh',text,'binutils-2.2.1/bfd/config', 53 ).
file('hp300bsd.mh',text,'binutils-2.2.1/bfd/config', 43 ).
file('hp300bsd.mt',text,'binutils-2.2.1/bfd/config', 99 ).
file('hp300hpux.mt',text,'binutils-2.2.1/bfd/config', 43 ).
file('hppa-elf.mt',text,'binutils-2.2.1/bfd/config', 44 ).
file('hppabsd.mh',text,'binutils-2.2.1/bfd/config', 25 ).
file('hppabsd.mt',text,'binutils-2.2.1/bfd/config', 38 ).
file('hppahpux.mh',text,'binutils-2.2.1/bfd/config', 26 ).
file('hppahpux.mt',text,'binutils-2.2.1/bfd/config', 38 ).
file('hppaosf.mh',text,'binutils-2.2.1/bfd/config', 150 ).
file('i386-aout.mt',text,'binutils-2.2.1/bfd/config', 77 ).
file('i386-bsd.mt',text,'binutils-2.2.1/bfd/config', 87 ).
file('i386-coff.mt',text,'binutils-2.2.1/bfd/config', 77 ).
file('i386-elf.mt',text,'binutils-2.2.1/bfd/config', 77 ).
file('i386-linux.mt',text,'binutils-2.2.1/bfd/config', 90 ).
file('i386-sco.mt',text,'binutils-2.2.1/bfd/config', 187 ).
file('i386aix.mh',text,'binutils-2.2.1/bfd/config', 150 ).
file('i386bsd.mh',text,'binutils-2.2.1/bfd/config', 43 ).
file('i386isc.mh',text,'binutils-2.2.1/bfd/config', 37 ).
file('i386linux.mh',text,'binutils-2.2.1/bfd/config', 61 ).
file('i386v.mh',text,'binutils-2.2.1/bfd/config', 37 ).
file('i386v4.mh',text,'binutils-2.2.1/bfd/config', 100 ).
file('i860-elf.mt',text,'binutils-2.2.1/bfd/config', 76 ).
file('i960-bout.mt',text,'binutils-2.2.1/bfd/config', 180 ).
file('i960-coff.mt',text,'binutils-2.2.1/bfd/config', 195 ).
file('irix3.mh',text,'binutils-2.2.1/bfd/config', 47 ).
file('irix4.mh',text,'binutils-2.2.1/bfd/config', 62 ).
file('m68k-aout.mt',text,'binutils-2.2.1/bfd/config', 80 ).
file('m68k-coff.mt',text,'binutils-2.2.1/bfd/config', 79 ).
file('m68k-elf.mt',text,'binutils-2.2.1/bfd/config', 81 ).
file('m88k-aout.mt',text,'binutils-2.2.1/bfd/config', 63 ).
file('m88k-coff.mt',text,'binutils-2.2.1/bfd/config', 142 ).
file('mipsbsd.mh',text,'binutils-2.2.1/bfd/config', 43 ).
file('mipsdecbsd.mt',text,'binutils-2.2.1/bfd/config', 104 ).
file('ncr3000.mh',text,'binutils-2.2.1/bfd/config', 642 ).
file('news.mt',text,'binutils-2.2.1/bfd/config', 61 ).
file('rs6000.mh',text,'binutils-2.2.1/bfd/config', 328 ).
file('rs6000.mt',text,'binutils-2.2.1/bfd/config', 79 ).
file('rtbsd.mh',text,'binutils-2.2.1/bfd/config', 251 ).
file('sh-coff.mt',text,'binutils-2.2.1/bfd/config', 225 ).
file('solaris2.mh',text,'binutils-2.2.1/bfd/config', 72 ).
file('sparc-aout.mt',text,'binutils-2.2.1/bfd/config', 73 ).
file('sparc-elf.mt',text,'binutils-2.2.1/bfd/config', 84 ).
file('sparc-ll.mh',text,'binutils-2.2.1/bfd/config', 70 ).
file('st2000.mt',text,'binutils-2.2.1/bfd/config', 250 ).
file('stratus.mh',text,'binutils-2.2.1/bfd/config', 89 ).
file('sysv4.mh',text,'binutils-2.2.1/bfd/config', 12 ).
file('tahoe.mh',text,'binutils-2.2.1/bfd/config', 63 ).
file('tahoe.mt',text,'binutils-2.2.1/bfd/config', 125 ).
file('ultra3.mh',text,'binutils-2.2.1/bfd/config', 53 ).
file('vax.mt',text,'binutils-2.2.1/bfd/config', 118 ).
file('vaxbsd.mh',text,'binutils-2.2.1/bfd/config', 63 ).
file('vaxult.mh',text,'binutils-2.2.1/bfd/config', 63 ).
file('vaxult2.mh',text,'binutils-2.2.1/bfd/config', 62 ).
file('we32k.mt',text,'binutils-2.2.1/bfd/config', 77 ).
file('z8k-coff.mt',text,'binutils-2.2.1/bfd/config', 228 ).
file('ChangeLog',text,'binutils-2.2.1/bfd/doc', 4187 ).
file('Makefile',text,'binutils-2.2.1/bfd/doc', 9977 ).
file('Makefile.in',text,'binutils-2.2.1/bfd/doc', 9619 ).
file('aoutx.texi',text,'binutils-2.2.1/bfd/doc', 6368 ).
file('archive.texi',text,'binutils-2.2.1/bfd/doc', 4119 ).
file('archures.texi',text,'binutils-2.2.1/bfd/doc', 9417 ).
file('bfd.info',text,'binutils-2.2.1/bfd/doc', 1994 ).
file('bfd.info-1',text,'binutils-2.2.1/bfd/doc', 46670 ).
file('bfd.info-2',text,'binutils-2.2.1/bfd/doc', 47939 ).
file('bfd.info-3',text,'binutils-2.2.1/bfd/doc', 50552 ).
file('bfd.info-4',text,'binutils-2.2.1/bfd/doc', 6382 ).
file('bfd.texi',text,'binutils-2.2.1/bfd/doc', 10484 ).
file('bfd.texinfo',text,'binutils-2.2.1/bfd/doc', 18071 ).
file('cache.texi',text,'binutils-2.2.1/bfd/doc', 3066 ).
file('chew',exec,'binutils-2.2.1/bfd/doc', 47728 ).
file('chew.c',text,'binutils-2.2.1/bfd/doc', 22655 ).
file('coffcode.texi',text,'binutils-2.2.1/bfd/doc', 19437 ).
file('config.status',exec,'binutils-2.2.1/bfd/doc', 211 ).
file('configure.in',text,'binutils-2.2.1/bfd/doc', 302 ).
file('core.texi',text,'binutils-2.2.1/bfd/doc', 1095 ).
file('ctor.texi',text,'binutils-2.2.1/bfd/doc', 3217 ).
file('doc.str',text,'binutils-2.2.1/bfd/doc', 1503 ).
file('format.texi',text,'binutils-2.2.1/bfd/doc', 2558 ).
file('init.texi',text,'binutils-2.2.1/bfd/doc', 689 ).
file('libbfd.texi',text,'binutils-2.2.1/bfd/doc', 4975 ).
file('opncls.texi',text,'binutils-2.2.1/bfd/doc', 3822 ).
file('proto.str',text,'binutils-2.2.1/bfd/doc', 545 ).
file('reloc.texi',text,'binutils-2.2.1/bfd/doc', 19658 ).
file('section.texi',text,'binutils-2.2.1/bfd/doc', 16720 ).
file('syms.texi',text,'binutils-2.2.1/bfd/doc', 10681 ).
file('targets.texi',text,'binutils-2.2.1/bfd/doc', 12036 ).
file('alphaosf.h',text,'binutils-2.2.1/bfd/hosts', 1087 ).
file('amix.h',text,'binutils-2.2.1/bfd/hosts', 1585 ).
file('apollo68.h',text,'binutils-2.2.1/bfd/hosts', 933 ).
file('apollov68.h',text,'binutils-2.2.1/bfd/hosts', 1233 ).
file('decstation.h',text,'binutils-2.2.1/bfd/hosts', 1219 ).
file('delta88.h',text,'binutils-2.2.1/bfd/hosts', 2059 ).
file('dgux.h',text,'binutils-2.2.1/bfd/hosts', 577 ).
file('dose.h',text,'binutils-2.2.1/bfd/hosts', 711 ).
file('go32.h',text,'binutils-2.2.1/bfd/hosts', 763 ).
file('harris.h',text,'binutils-2.2.1/bfd/hosts', 935 ).
file('hp300.h',text,'binutils-2.2.1/bfd/hosts', 1095 ).
file('hp300bsd.h',text,'binutils-2.2.1/bfd/hosts', 1569 ).
file('hppabsd.h',text,'binutils-2.2.1/bfd/hosts', 1197 ).
file('hppahpux.h',text,'binutils-2.2.1/bfd/hosts', 1020 ).
file('i386aix.h',text,'binutils-2.2.1/bfd/hosts', 1493 ).
file('i386bsd.h',text,'binutils-2.2.1/bfd/hosts', 1261 ).
file('i386isc.h',text,'binutils-2.2.1/bfd/hosts', 1354 ).
file('i386linux.h',text,'binutils-2.2.1/bfd/hosts', 92 ).
file('i386mach.h',text,'binutils-2.2.1/bfd/hosts', 1137 ).
file('i386v.h',text,'binutils-2.2.1/bfd/hosts', 1192 ).
file('i386v4.h',text,'binutils-2.2.1/bfd/hosts', 1572 ).
file('irix3.h',text,'binutils-2.2.1/bfd/hosts', 825 ).
file('irix4.h',text,'binutils-2.2.1/bfd/hosts', 845 ).
file('miniframe.h',text,'binutils-2.2.1/bfd/hosts', 279 ).
file('mipsbsd.h',text,'binutils-2.2.1/bfd/hosts', 1139 ).
file('ncr3000.h',text,'binutils-2.2.1/bfd/hosts', 1572 ).
file('rs6000.h',text,'binutils-2.2.1/bfd/hosts', 753 ).
file('rtbsd.h',text,'binutils-2.2.1/bfd/hosts', 780 ).
file('solaris2.h',text,'binutils-2.2.1/bfd/hosts', 119 ).
file('sparc-ll.h',text,'binutils-2.2.1/bfd/hosts', 3019 ).
file('sparc.h',text,'binutils-2.2.1/bfd/hosts', 246 ).
file('std-host.h',text,'binutils-2.2.1/bfd/hosts', 3758 ).
file('stratus.h',text,'binutils-2.2.1/bfd/hosts', 1576 ).
file('sun3.h',text,'binutils-2.2.1/bfd/hosts', 2020 ).
file('sysv4.h',text,'binutils-2.2.1/bfd/hosts', 1598 ).
file('tahoe.h',text,'binutils-2.2.1/bfd/hosts', 998 ).
file('ultra3.h',text,'binutils-2.2.1/bfd/hosts', 775 ).
file('vaxbsd.h',text,'binutils-2.2.1/bfd/hosts', 1211 ).
file('vaxult.h',text,'binutils-2.2.1/bfd/hosts', 966 ).
file('vaxult2.h',text,'binutils-2.2.1/bfd/hosts', 1265 ).
file('we32k.h',text,'binutils-2.2.1/bfd/hosts', 1054 ).
file('ChangeLog',text,'binutils-2.2.1/binutils', 28532 ).
file('Makefile',text,'binutils-2.2.1/binutils', 15943 ).
file('Makefile.in',text,'binutils-2.2.1/binutils', 15313 ).
file('NEWS',text,'binutils-2.2.1/binutils', 1585 ).
file('README',text,'binutils-2.2.1/binutils', 3596 ).
file('TODO',text,'binutils-2.2.1/binutils', 476 ).
file('alloca.c',text,'binutils-2.2.1/binutils', 5226 ).
file('ar.1',text,'binutils-2.2.1/binutils', 9865 ).
file('ar.c',text,'binutils-2.2.1/binutils', 24333 ).
file('arlex.c',text,'binutils-2.2.1/binutils', 33224 ).
file('arlex.l',text,'binutils-2.2.1/binutils', 2274 ).
file('arparse.c',text,'binutils-2.2.1/binutils', 26314 ).
file('arparse.h',text,'binutils-2.2.1/binutils', 425 ).
file('arparse.y',text,'binutils-2.2.1/binutils', 3053 ).
file('arsup.c',text,'binutils-2.2.1/binutils', 8395 ).
file('arsup.h',text,'binutils-2.2.1/binutils', 1532 ).
file('binutils.info',text,'binutils-2.2.1/binutils', 33701 ).
file('binutils.texi',text,'binutils-2.2.1/binutils', 36169 ).
file('bucomm.c',text,'binutils-2.2.1/binutils', 3616 ).
file('bucomm.h',text,'binutils-2.2.1/binutils', 1019 ).
file('config',dir,'binutils-2.2.1/binutils', 512 ).
file('config.status',exec,'binutils-2.2.1/binutils', 241 ).
file('configure.bat',exec,'binutils-2.2.1/binutils', 362 ).
file('configure.in',text,'binutils-2.2.1/binutils', 760 ).
file('filemode.c',text,'binutils-2.2.1/binutils', 4507 ).
file('gmalloc.c',text,'binutils-2.2.1/binutils', 32892 ).
file('is-ranlib.c',text,'binutils-2.2.1/binutils', 93 ).
file('is-strip.c',text,'binutils-2.2.1/binutils', 104 ).
file('maybe-ranlib.c',text,'binutils-2.2.1/binutils', 137 ).
file('maybe-strip.c',text,'binutils-2.2.1/binutils', 145 ).
file('nm.1',text,'binutils-2.2.1/binutils', 3259 ).
file('nm.c',text,'binutils-2.2.1/binutils', 10239 ).
file('not-ranlib.c',text,'binutils-2.2.1/binutils', 93 ).
file('not-strip.c',text,'binutils-2.2.1/binutils', 104 ).
file('objcopy.c',text,'binutils-2.2.1/binutils', 16984 ).
file('objdump.1',text,'binutils-2.2.1/binutils', 4884 ).
file('objdump.c',text,'binutils-2.2.1/binutils', 26941 ).
file('objdump.h',text,'binutils-2.2.1/binutils', 829 ).
file('ranlib.1',text,'binutils-2.2.1/binutils', 1897 ).
file('ranlib.sh',exec,'binutils-2.2.1/binutils', 90 ).
file('sanity.sh',exec,'binutils-2.2.1/binutils', 868 ).
file('size.1',text,'binutils-2.2.1/binutils', 3382 ).
file('size.c',text,'binutils-2.2.1/binutils', 8624 ).
file('strip.1',text,'binutils-2.2.1/binutils', 3004 ).
file('testsuite',dir,'binutils-2.2.1/binutils', 512 ).
file('version.c',text,'binutils-2.2.1/binutils', 141 ).
file('mh-apollo68v',text,'binutils-2.2.1/binutils/config', 334 ).
file('mh-delta88',text,'binutils-2.2.1/binutils/config', 15 ).
file('ChangeLog',text,'binutils-2.2.1/binutils/testsuite', 1205 ).
file('Makefile',text,'binutils-2.2.1/binutils/testsuite', 3982 ).
file('Makefile.in',text,'binutils-2.2.1/binutils/testsuite', 3602 ).
file('binutils.all',dir,'binutils-2.2.1/binutils/testsuite', 512 ).
file('config',dir,'binutils-2.2.1/binutils/testsuite', 512 ).
file('config.status',exec,'binutils-2.2.1/binutils/testsuite', 211 ).
file('configure.in',text,'binutils-2.2.1/binutils/testsuite', 1531 ).
file('Makefile',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 910 ).
file('Makefile.in',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 552 ).
file('bintest.c',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 252 ).
file('config.status',exec,'binutils-2.2.1/binutils/testsuite/binutils.all', 214 ).
file('configure.in',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 625 ).
file('nm.exp',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 2131 ).
file('objdump.exp',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 1902 ).
file('size.exp',text,'binutils-2.2.1/binutils/testsuite/binutils.all', 2665 ).
file('abug.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('aout.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('coff.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('nind.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('udi.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('unix.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('vx.exp',text,'binutils-2.2.1/binutils/testsuite/config', 1424 ).
file('ChangeLog',text,'binutils-2.2.1/config', 1369 ).
file('mh-a68bsd',text,'binutils-2.2.1/config', 90 ).
file('mh-aix',text,'binutils-2.2.1/config', 185 ).
file('mh-aix386',text,'binutils-2.2.1/config', 25 ).
file('mh-apollo68',text,'binutils-2.2.1/config', 91 ).
file('mh-decstation',text,'binutils-2.2.1/config', 198 ).
file('mh-delta88',text,'binutils-2.2.1/config', 30 ).
file('mh-dgux',text,'binutils-2.2.1/config', 76 ).
file('mh-hpux',text,'binutils-2.2.1/config', 193 ).
file('mh-irix4',text,'binutils-2.2.1/config', 286 ).
file('mh-linux',text,'binutils-2.2.1/config', 118 ).
file('mh-ncr3000',text,'binutils-2.2.1/config', 832 ).
file('mh-sco',text,'binutils-2.2.1/config', 215 ).
file('mh-solaris',text,'binutils-2.2.1/config', 485 ).
file('mh-sun',text,'binutils-2.2.1/config', 116 ).
file('mh-sun3',text,'binutils-2.2.1/config', 268 ).
file('mh-sysv',text,'binutils-2.2.1/config', 111 ).
file('mh-sysv4',text,'binutils-2.2.1/config', 218 ).
file('mh-vaxult2',text,'binutils-2.2.1/config', 73 ).
file('Makefile',text,'binutils-2.2.1/etc', 2567 ).
file('Makefile.in',text,'binutils-2.2.1/etc', 2189 ).
file('cfg-paper.texi',text,'binutils-2.2.1/etc', 29217 ).
file('config.status',exec,'binutils-2.2.1/etc', 208 ).
file('configure.in',text,'binutils-2.2.1/etc', 378 ).
file('configure.man',text,'binutils-2.2.1/etc', 3188 ).
file('configure.texi',text,'binutils-2.2.1/etc', 73535 ).
file('ChangeLog',text,'binutils-2.2.1/gprof', 4860 ).
file('Makefile',text,'binutils-2.2.1/gprof', 3662 ).
file('Makefile.in',text,'binutils-2.2.1/gprof', 3221 ).
file('arcs.c',text,'binutils-2.2.1/gprof', 16073 ).
file('bsd_callg_bl.c',text,'binutils-2.2.1/gprof', 5732 ).
file('bsd_callg_bl.m',text,'binutils-2.2.1/gprof', 3267 ).
file('config',dir,'binutils-2.2.1/gprof', 512 ).
file('config.status',exec,'binutils-2.2.1/gprof', 232 ).
file('configure.in',text,'binutils-2.2.1/gprof', 745 ).
file('dfn.c',text,'binutils-2.2.1/gprof', 7380 ).
file('dummy.c',text,'binutils-2.2.1/gprof', 291 ).
file('dummy.h',text,'binutils-2.2.1/gprof', 2234 ).
file('flat_bl.c',text,'binutils-2.2.1/gprof', 1822 ).
file('flat_bl.m',text,'binutils-2.2.1/gprof', 1047 ).
file('fsf_callg_bl.c',text,'binutils-2.2.1/gprof', 5366 ).
file('fsf_callg_bl.m',text,'binutils-2.2.1/gprof', 3426 ).
file('gen-c-prog.awk',text,'binutils-2.2.1/gprof', 569 ).
file('gmon.h',text,'binutils-2.2.1/gprof', 3582 ).
file('gprof.1',text,'binutils-2.2.1/gprof', 7522 ).
file('gprof.c',text,'binutils-2.2.1/gprof', 17358 ).
file('gprof.h',text,'binutils-2.2.1/gprof', 7548 ).
file('gprof.info',text,'binutils-2.2.1/gprof', 41098 ).
file('gprof.texi',text,'binutils-2.2.1/gprof', 44135 ).
file('hertz.c',text,'binutils-2.2.1/gprof', 1589 ).
file('i386.c',text,'binutils-2.2.1/gprof', 4118 ).
file('i386.h',text,'binutils-2.2.1/gprof', 1363 ).
file('lookup.c',text,'binutils-2.2.1/gprof', 3857 ).
file('printgprof.c',text,'binutils-2.2.1/gprof', 20750 ).
file('printlist.c',text,'binutils-2.2.1/gprof', 2519 ).
file('sparc.c',text,'binutils-2.2.1/gprof', 4009 ).
file('sparc.h',text,'binutils-2.2.1/gprof', 1345 ).
file('tahoe.c',text,'binutils-2.2.1/gprof', 8440 ).
file('tahoe.h',text,'binutils-2.2.1/gprof', 1699 ).
file('vax.c',text,'binutils-2.2.1/gprof', 8492 ).
file('vax.h',text,'binutils-2.2.1/gprof', 1779 ).
file('mt-dummy',text,'binutils-2.2.1/gprof/config', 14 ).
file('mt-i386',text,'binutils-2.2.1/gprof/config', 13 ).
file('mt-sparc',text,'binutils-2.2.1/gprof/config', 14 ).
file('mt-tahoe',text,'binutils-2.2.1/gprof/config', 14 ).
file('mt-vax',text,'binutils-2.2.1/gprof/config', 12 ).
file('COPYING',text,'binutils-2.2.1/include', 17982 ).
file('ChangeLog',text,'binutils-2.2.1/include', 18052 ).
file('ansidecl.h',text,'binutils-2.2.1/include', 4066 ).
file('aout',dir,'binutils-2.2.1/include', 512 ).
file('bfd.h',text,'binutils-2.2.1/include', 55101 ).
file('bout.h',text,'binutils-2.2.1/include', 6929 ).
file('coff',dir,'binutils-2.2.1/include', 512 ).
file('demangle.h',text,'binutils-2.2.1/include', 2775 ).
file('dis-asm.h',text,'binutils-2.2.1/include', 4187 ).
file('elf',dir,'binutils-2.2.1/include', 512 ).
file('fopen-bin.h',text,'binutils-2.2.1/include', 918 ).
file('fopen-same.h',text,'binutils-2.2.1/include', 898 ).
file('gdbm.h',text,'binutils-2.2.1/include', 2464 ).
file('getopt.h',text,'binutils-2.2.1/include', 4333 ).
file('ieee-float.h',text,'binutils-2.2.1/include', 2347 ).
file('ieee.h',text,'binutils-2.2.1/include', 4220 ).
file('oasys.h',text,'binutils-2.2.1/include', 3789 ).
file('obstack.h',text,'binutils-2.2.1/include', 18564 ).
file('opcode',dir,'binutils-2.2.1/include', 512 ).
file('wait.h',text,'binutils-2.2.1/include', 1450 ).
file('ChangeLog',text,'binutils-2.2.1/include/aout', 1966 ).
file('adobe.h',text,'binutils-2.2.1/include/aout', 10416 ).
file('aout64.h',text,'binutils-2.2.1/include/aout', 14023 ).
file('ar.h',text,'binutils-2.2.1/include/aout', 761 ).
file('encap.h',text,'binutils-2.2.1/include/aout', 4722 ).
file('host.h',text,'binutils-2.2.1/include/aout', 577 ).
file('hp.h',text,'binutils-2.2.1/include/aout', 2930 ).
file('hp300hpux.h',text,'binutils-2.2.1/include/aout', 4386 ).
file('hppa.h',text,'binutils-2.2.1/include/aout', 104 ).
file('ranlib.h',text,'binutils-2.2.1/include/aout', 2463 ).
file('reloc.h',text,'binutils-2.2.1/include/aout', 2227 ).
file('stab.def',text,'binutils-2.2.1/include/aout', 11050 ).
file('stab_gnu.h',text,'binutils-2.2.1/include/aout', 815 ).
file('sun4.h',text,'binutils-2.2.1/include/aout', 1173 ).
file('ChangeLog',text,'binutils-2.2.1/include/coff', 5686 ).
file('a29k.h',text,'binutils-2.2.1/include/coff', 8709 ).
file('alpha.h',text,'binutils-2.2.1/include/coff', 5884 ).
file('ecoff-ext.h',text,'binutils-2.2.1/include/coff', 11050 ).
file('h8300.h',text,'binutils-2.2.1/include/coff', 5140 ).
file('h8500.h',text,'binutils-2.2.1/include/coff', 5140 ).
file('i386.h',text,'binutils-2.2.1/include/coff', 5414 ).
file('i960.h',text,'binutils-2.2.1/include/coff', 6705 ).
file('internal.h',text,'binutils-2.2.1/include/coff', 18200 ).
file('m68k.h',text,'binutils-2.2.1/include/coff', 5243 ).
file('m88k.h',text,'binutils-2.2.1/include/coff', 6506 ).
file('mips.h',text,'binutils-2.2.1/include/coff', 6146 ).
file('rs6000.h',text,'binutils-2.2.1/include/coff', 6143 ).
file('sh.h',text,'binutils-2.2.1/include/coff', 5127 ).
file('sym.h',text,'binutils-2.2.1/include/coff', 16119 ).
file('symconst.h',text,'binutils-2.2.1/include/coff', 6503 ).
file('we32k.h',text,'binutils-2.2.1/include/coff', 5500 ).
file('z8k.h',text,'binutils-2.2.1/include/coff', 5209 ).
file('ChangeLog',text,'binutils-2.2.1/include/elf', 1487 ).
file('common.h',text,'binutils-2.2.1/include/elf', 8054 ).
file('dwarf.h',text,'binutils-2.2.1/include/elf', 10157 ).
file('external.h',text,'binutils-2.2.1/include/elf', 4918 ).
file('internal.h',text,'binutils-2.2.1/include/elf', 5326 ).
file('ChangeLog',text,'binutils-2.2.1/include/opcode', 9110 ).
file('a29k.h',text,'binutils-2.2.1/include/opcode', 10112 ).
file('arm.h',text,'binutils-2.2.1/include/opcode', 11402 ).
file('convex.h',text,'binutils-2.2.1/include/opcode', 42852 ).
file('h8300.h',text,'binutils-2.2.1/include/opcode', 11658 ).
file('hppa.h',text,'binutils-2.2.1/include/opcode', 18593 ).
file('i386.h',text,'binutils-2.2.1/include/opcode', 32188 ).
file('i860.h',text,'binutils-2.2.1/include/opcode', 27424 ).
file('i960.h',text,'binutils-2.2.1/include/opcode', 18627 ).
file('m68k.h',text,'binutils-2.2.1/include/opcode', 136524 ).
file('m68kmri.h',text,'binutils-2.2.1/include/opcode', 251 ).
file('m88k.h',text,'binutils-2.2.1/include/opcode', 32625 ).
file('mips.h',text,'binutils-2.2.1/include/opcode', 15600 ).
file('np1.h',text,'binutils-2.2.1/include/opcode', 18168 ).
file('ns32k.h',text,'binutils-2.2.1/include/opcode', 25900 ).
file('pn.h',text,'binutils-2.2.1/include/opcode', 12397 ).
file('pyr.h',text,'binutils-2.2.1/include/opcode', 11238 ).
file('rs6k.h',text,'binutils-2.2.1/include/opcode', 9232 ).
file('sparc.h',text,'binutils-2.2.1/include/opcode', 4987 ).
file('tahoe.h',text,'binutils-2.2.1/include/opcode', 7973 ).
file('vax.h',text,'binutils-2.2.1/include/opcode', 13509 ).
file('ChangeLog',text,'binutils-2.2.1/ld', 42547 ).
file('Makefile.in',text,'binutils-2.2.1/ld', 19069 ).
file('NEWS',text,'binutils-2.2.1/ld', 497 ).
file('README',text,'binutils-2.2.1/ld', 1223 ).
file('TODO',text,'binutils-2.2.1/ld', 397 ).
file('a29k.sc-sh',exec,'binutils-2.2.1/ld', 910 ).
file('a29k.sh',exec,'binutils-2.2.1/ld', 123 ).
file('aout.sc-sh',exec,'binutils-2.2.1/ld', 627 ).
file('cdtest-foo.cc',text,'binutils-2.2.1/ld', 1964 ).
file('cdtest-foo.h',text,'binutils-2.2.1/ld', 399 ).
file('cdtest-func.cc',text,'binutils-2.2.1/ld', 250 ).
file('cdtest-main.cc',text,'binutils-2.2.1/ld', 1088 ).
file('cdtest.exp',text,'binutils-2.2.1/ld', 655 ).
file('config',dir,'binutils-2.2.1/ld', 1024 ).
file('config.h',text,'binutils-2.2.1/ld', 1026 ).
file('configure.bat',text,'binutils-2.2.1/ld', 350 ).
file('configure.in',text,'binutils-2.2.1/ld', 2955 ).
file('ebmon29k.sc-sh',exec,'binutils-2.2.1/ld', 382 ).
file('ebmon29k.sh',exec,'binutils-2.2.1/ld', 125 ).
file('gen-doc.texi',text,'binutils-2.2.1/ld', 290 ).
file('generic.em',text,'binutils-2.2.1/ld', 2992 ).
file('genscripts.sh',exec,'binutils-2.2.1/ld', 3114 ).
file('gld960.em',text,'binutils-2.2.1/ld', 2848 ).
file('gld960.sh',exec,'binutils-2.2.1/ld', 131 ).
file('go32.sh',exec,'binutils-2.2.1/ld', 166 ).
file('h8-doc.texi',text,'binutils-2.2.1/ld', 294 ).
file('h8300hms.em',text,'binutils-2.2.1/ld', 2163 ).
file('h8300hms.sc-sh',exec,'binutils-2.2.1/ld', 644 ).
file('h8300hms.sh',exec,'binutils-2.2.1/ld', 143 ).
file('h8500hms.em',text,'binutils-2.2.1/ld', 2143 ).
file('h8500hms.sc-sh',exec,'binutils-2.2.1/ld', 644 ).
file('h8500hms.sh',exec,'binutils-2.2.1/ld', 143 ).
file('hp300bsd.sh',exec,'binutils-2.2.1/ld', 115 ).
file('hp3hpux.sh',text,'binutils-2.2.1/ld', 151 ).
file('hppaosf.em',text,'binutils-2.2.1/ld', 2643 ).
file('hppaosf.sc-sh',text,'binutils-2.2.1/ld', 563 ).
file('hppaosf.sh',text,'binutils-2.2.1/ld', 153 ).
file('i386aout.sh',exec,'binutils-2.2.1/ld', 145 ).
file('i386bsd.sh',text,'binutils-2.2.1/ld', 148 ).
file('i386coff.sc-sh',text,'binutils-2.2.1/ld', 599 ).
file('i386coff.sh',text,'binutils-2.2.1/ld', 127 ).
file('i960.sc-sh',exec,'binutils-2.2.1/ld', 360 ).
file('ld.1',text,'binutils-2.2.1/ld', 21690 ).
file('ld.dvi',text,'binutils-2.2.1/ld', 166148 ).
file('ld.h',text,'binutils-2.2.1/ld', 3903 ).
file('ld.info',text,'binutils-2.2.1/ld', 1512 ).
file('ld.info-1',text,'binutils-2.2.1/ld', 49050 ).
file('ld.info-2',text,'binutils-2.2.1/ld', 29197 ).
file('ld.texinfo',text,'binutils-2.2.1/ld', 80180 ).
file('ldctor.c',text,'binutils-2.2.1/ld', 3962 ).
file('ldctor.h',text,'binutils-2.2.1/ld', 225 ).
file('ldemul.c',text,'binutils-2.2.1/ld', 2866 ).
file('ldemul.h',text,'binutils-2.2.1/ld', 1979 ).
file('lderror.c',text,'binutils-2.2.1/ld', 1962 ).
file('lderror.h',text,'binutils-2.2.1/ld', 44 ).
file('ldexp.c',text,'binutils-2.2.1/ld', 18856 ).
file('ldexp.h',text,'binutils-2.2.1/ld', 2504 ).
file('ldfile.c',text,'binutils-2.2.1/ld', 7349 ).
file('ldfile.h',text,'binutils-2.2.1/ld', 1061 ).
file('ldgram.c',text,'binutils-2.2.1/ld', 74766 ).
file('ldgram.h',text,'binutils-2.2.1/ld', 2768 ).
file('ldgram.y',text,'binutils-2.2.1/ld', 17398 ).
file('ldindr.c',text,'binutils-2.2.1/ld', 3341 ).
file('ldindr.h',text,'binutils-2.2.1/ld', 83 ).
file('ldint.texinfo',text,'binutils-2.2.1/ld', 10206 ).
file('ldlang.c',text,'binutils-2.2.1/ld', 69811 ).
file('ldlang.h',text,'binutils-2.2.1/ld', 10739 ).
file('ldlex.c',text,'binutils-2.2.1/ld', 83926 ).
file('ldlex.h',text,'binutils-2.2.1/ld', 958 ).
file('ldlex.l',text,'binutils-2.2.1/ld', 15802 ).
file('ldmain.c',text,'binutils-2.2.1/ld', 26428 ).
file('ldmain.h',text,'binutils-2.2.1/ld', 936 ).
file('ldmisc.c',text,'binutils-2.2.1/ld', 8116 ).
file('ldmisc.h',text,'binutils-2.2.1/ld', 1409 ).
file('ldsym.c',text,'binutils-2.2.1/ld', 16497 ).
file('ldsym.h',text,'binutils-2.2.1/ld', 2418 ).
file('ldver.c',text,'binutils-2.2.1/ld', 1242 ).
file('ldver.h',text,'binutils-2.2.1/ld', 811 ).
file('ldwarn.c',text,'binutils-2.2.1/ld', 2461 ).
file('ldwarn.h',text,'binutils-2.2.1/ld', 999 ).
file('ldwrite.c',text,'binutils-2.2.1/ld', 3363 ).
file('ldwrite.h',text,'binutils-2.2.1/ld', 820 ).
file('lexsup.c',text,'binutils-2.2.1/ld', 5562 ).
file('lnk960.em',text,'binutils-2.2.1/ld', 6085 ).
file('lnk960.sh',exec,'binutils-2.2.1/ld', 119 ).
file('m68kcoff.sc-sh',text,'binutils-2.2.1/ld', 995 ).
file('m68kcoff.sh',text,'binutils-2.2.1/ld', 127 ).
file('m88kbcs.sc-sh',exec,'binutils-2.2.1/ld', 1002 ).
file('m88kbcs.sh',exec,'binutils-2.2.1/ld', 109 ).
file('mips.sc-sh',text,'binutils-2.2.1/ld', 1238 ).
file('mipsbig.sh',text,'binutils-2.2.1/ld', 100 ).
file('mipsbsd.sc-sh',text,'binutils-2.2.1/ld', 519 ).
file('mipsbsd.sh',text,'binutils-2.2.1/ld', 124 ).
file('mipsidt.sh',text,'binutils-2.2.1/ld', 172 ).
file('mipslit.sh',text,'binutils-2.2.1/ld', 103 ).
file('mkscript.c',text,'binutils-2.2.1/ld', 1197 ).
file('mri.c',text,'binutils-2.2.1/ld', 7802 ).
file('mri.h',text,'binutils-2.2.1/ld', 2 ).
file('news.sh',exec,'binutils-2.2.1/ld', 112 ).
file('relax.c',text,'binutils-2.2.1/ld', 5305 ).
file('relax.h',text,'binutils-2.2.1/ld', 916 ).
file('sa29200.sc-sh',exec,'binutils-2.2.1/ld', 440 ).
file('sa29200.sh',exec,'binutils-2.2.1/ld', 127 ).
file('scripts',dir,'binutils-2.2.1/ld', 512 ).
file('sh.em',text,'binutils-2.2.1/ld', 2046 ).
file('sh.sc-sh',exec,'binutils-2.2.1/ld', 518 ).
file('sh.sh',exec,'binutils-2.2.1/ld', 122 ).
file('st2000.em',text,'binutils-2.2.1/ld', 2022 ).
file('st2000.sc-sh',exec,'binutils-2.2.1/ld', 225 ).
file('st2000.sh',exec,'binutils-2.2.1/ld', 133 ).
file('sun3.sh',exec,'binutils-2.2.1/ld', 172 ).
file('sun4.sh',exec,'binutils-2.2.1/ld', 152 ).
file('vanilla.em',text,'binutils-2.2.1/ld', 1830 ).
file('vanilla.sc-sh',exec,'binutils-2.2.1/ld', 17 ).
file('vanilla.sh',exec,'binutils-2.2.1/ld', 110 ).
file('vax.sh',exec,'binutils-2.2.1/ld', 104 ).
file('z8ksim.em',text,'binutils-2.2.1/ld', 2021 ).
file('z8ksim.sc-sh',exec,'binutils-2.2.1/ld', 225 ).
file('z8ksim.sh',exec,'binutils-2.2.1/ld', 131 ).
file('coff-a29k.mt',text,'binutils-2.2.1/ld/config', 10 ).
file('coff-h8300.mt',text,'binutils-2.2.1/ld/config', 14 ).
file('coff-h8500.mt',text,'binutils-2.2.1/ld/config', 14 ).
file('coff-sh.mt',text,'binutils-2.2.1/ld/config', 8 ).
file('delta88.mh',text,'binutils-2.2.1/ld/config', 27 ).
file('dgux.mh',text,'binutils-2.2.1/ld/config', 149 ).
file('ebmon29k.mt',text,'binutils-2.2.1/ld/config', 14 ).
file('go32.mt',text,'binutils-2.2.1/ld/config', 10 ).
file('hp300.mh',text,'binutils-2.2.1/ld/config', 191 ).
file('hp300bsd.mt',text,'binutils-2.2.1/ld/config', 15 ).
file('hp300hpux.mt',text,'binutils-2.2.1/ld/config', 15 ).
file('hppaosf.mh',text,'binutils-2.2.1/ld/config', 94 ).
file('hppaosf.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('i386-aout.mt',text,'binutils-2.2.1/ld/config', 14 ).
file('i386-bsd.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('i386-coff.mt',text,'binutils-2.2.1/ld/config', 14 ).
file('i386v.mt',text,'binutils-2.2.1/ld/config', 41 ).
file('i960.mt',text,'binutils-2.2.1/ld/config', 12 ).
file('ieee-h8300.mt',text,'binutils-2.2.1/ld/config', 15 ).
file('m68k-coff.mt',text,'binutils-2.2.1/ld/config', 14 ).
file('m68k.mt',text,'binutils-2.2.1/ld/config', 39 ).
file('m68kv.mt',text,'binutils-2.2.1/ld/config', 48 ).
file('m88k-bcs.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('mips-big.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('mips-idt.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('mips-lit.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('mipsbsd.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('news.mt',text,'binutils-2.2.1/ld/config', 10 ).
file('ose68.mt',text,'binutils-2.2.1/ld/config', 10 ).
file('rtbsd.mh',text,'binutils-2.2.1/ld/config', 284 ).
file('sa29200.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('sparc-ll.mh',text,'binutils-2.2.1/ld/config', 35 ).
file('st2000.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('sun3.mh',text,'binutils-2.2.1/ld/config', 131 ).
file('sun3.mt',text,'binutils-2.2.1/ld/config', 11 ).
file('sun4.mt',text,'binutils-2.2.1/ld/config', 10 ).
file('vax.mt',text,'binutils-2.2.1/ld/config', 9 ).
file('vxworks68.mt',text,'binutils-2.2.1/ld/config', 10 ).
file('z8ksim.mt',text,'binutils-2.2.1/ld/config', 13 ).
file('isc-sysv3.2.sc',text,'binutils-2.2.1/ld/scripts', 540 ).
file('COPYING.LIB',text,'binutils-2.2.1/libiberty', 25265 ).
file('ChangeLog',text,'binutils-2.2.1/libiberty', 25274 ).
file('Makefile',text,'binutils-2.2.1/libiberty', 8225 ).
file('Makefile.in',text,'binutils-2.2.1/libiberty', 7766 ).
file('README',text,'binutils-2.2.1/libiberty', 5400 ).
file('alloca-botch.h',text,'binutils-2.2.1/libiberty', 188 ).
file('alloca-norm.h',text,'binutils-2.2.1/libiberty', 217 ).
file('alloca.c',text,'binutils-2.2.1/libiberty', 5483 ).
file('argv.c',text,'binutils-2.2.1/libiberty', 7387 ).
file('basename.c',text,'binutils-2.2.1/libiberty', 1367 ).
file('bcmp.c',text,'binutils-2.2.1/libiberty', 1567 ).
file('bcopy.c',text,'binutils-2.2.1/libiberty', 1522 ).
file('bzero.c',text,'binutils-2.2.1/libiberty', 1286 ).
file('concat.c',text,'binutils-2.2.1/libiberty', 3087 ).
file('config',dir,'binutils-2.2.1/libiberty', 512 ).
file('config.status',exec,'binutils-2.2.1/libiberty', 231 ).
file('configure.bat',text,'binutils-2.2.1/libiberty', 292 ).
file('configure.in',text,'binutils-2.2.1/libiberty', 982 ).
file('copysign.c',text,'binutils-2.2.1/libiberty', 2490 ).
file('cplus-dem.c',text,'binutils-2.2.1/libiberty', 55204 ).
file('dummy.c',text,'binutils-2.2.1/libiberty', 1368 ).
file('fdmatch.c',text,'binutils-2.2.1/libiberty', 2041 ).
file('functions.def',text,'binutils-2.2.1/libiberty', 2656 ).
file('getcwd.c',text,'binutils-2.2.1/libiberty', 1705 ).
file('getopt.c',text,'binutils-2.2.1/libiberty', 19056 ).
file('getopt1.c',text,'binutils-2.2.1/libiberty', 3598 ).
file('getpagesize.c',text,'binutils-2.2.1/libiberty', 2161 ).
file('ieee-float.c',exec,'binutils-2.2.1/libiberty', 4775 ).
file('index.c',text,'binutils-2.2.1/libiberty', 143 ).
file('insque.c',text,'binutils-2.2.1/libiberty', 1703 ).
file('memchr.c',text,'binutils-2.2.1/libiberty', 1185 ).
file('memcmp.c',text,'binutils-2.2.1/libiberty', 1548 ).
file('memcpy.c',text,'binutils-2.2.1/libiberty', 1280 ).
file('memmove.c',text,'binutils-2.2.1/libiberty', 1355 ).
file('memset.c',text,'binutils-2.2.1/libiberty', 1124 ).
file('msdos.c',text,'binutils-2.2.1/libiberty', 206 ).
file('obstack.c',text,'binutils-2.2.1/libiberty', 13365 ).
file('random.c',text,'binutils-2.2.1/libiberty', 12800 ).
file('rename.c',text,'binutils-2.2.1/libiberty', 1110 ).
file('rindex.c',text,'binutils-2.2.1/libiberty', 147 ).
file('sigsetmask.c',text,'binutils-2.2.1/libiberty', 1415 ).
file('spaces.c',text,'binutils-2.2.1/libiberty', 1545 ).
file('strchr.c',text,'binutils-2.2.1/libiberty', 1343 ).
file('strdup.c',text,'binutils-2.2.1/libiberty', 172 ).
file('strerror.c',text,'binutils-2.2.1/libiberty', 20370 ).
file('strrchr.c',text,'binutils-2.2.1/libiberty', 1360 ).
file('strsignal.c',text,'binutils-2.2.1/libiberty', 14678 ).
file('strstr.c',text,'binutils-2.2.1/libiberty', 1717 ).
file('strtod.c',text,'binutils-2.2.1/libiberty', 2605 ).
file('strtol.c',text,'binutils-2.2.1/libiberty', 1196 ).
file('strtoul.c',text,'binutils-2.2.1/libiberty', 1778 ).
file('tmpnam.c',text,'binutils-2.2.1/libiberty', 619 ).
file('vfork.c',text,'binutils-2.2.1/libiberty', 916 ).
file('vfprintf.c',text,'binutils-2.2.1/libiberty', 184 ).
file('vprintf.c',text,'binutils-2.2.1/libiberty', 160 ).
file('vsprintf.c',text,'binutils-2.2.1/libiberty', 1501 ).
file('mh-a68bsd',text,'binutils-2.2.1/libiberty/config', 70 ).
file('mh-aix',text,'binutils-2.2.1/libiberty/config', 442 ).
file('mh-apollo68',text,'binutils-2.2.1/libiberty/config', 68 ).
file('mh-hpux',text,'binutils-2.2.1/libiberty/config', 24 ).
file('mh-irix4',text,'binutils-2.2.1/libiberty/config', 65 ).
file('mh-ncr3000',text,'binutils-2.2.1/libiberty/config', 741 ).
file('mh-sysv',text,'binutils-2.2.1/libiberty/config', 12 ).
file('mh-sysv4',text,'binutils-2.2.1/libiberty/config', 51 ).
file('ChangeLog',text,'binutils-2.2.1/opcodes', 7824 ).
file('Makefile',text,'binutils-2.2.1/opcodes', 5247 ).
file('Makefile.in',text,'binutils-2.2.1/opcodes', 4762 ).
file('a29k-dis.c',text,'binutils-2.2.1/opcodes', 9267 ).
file('alpha-dis.c',text,'binutils-2.2.1/opcodes', 3688 ).
file('alpha-opc.h',text,'binutils-2.2.1/opcodes', 16809 ).
file('config.status',exec,'binutils-2.2.1/opcodes', 241 ).
file('configure.in',text,'binutils-2.2.1/opcodes', 740 ).
file('dis-buf.c',text,'binutils-2.2.1/opcodes', 2290 ).
file('h8500-dis.c',text,'binutils-2.2.1/opcodes', 7557 ).
file('h8500-opc.h',text,'binutils-2.2.1/opcodes', 282120 ).
file('hppa-dis.c',text,'binutils-2.2.1/opcodes', 17109 ).
file('i386-dis.c',text,'binutils-2.2.1/opcodes', 37793 ).
file('i960-dis.c',text,'binutils-2.2.1/opcodes', 20254 ).
file('m68881-ext.c',text,'binutils-2.2.1/opcodes', 157 ).
file('m68k-dis.c',text,'binutils-2.2.1/opcodes', 23928 ).
file('mips-dis.c',text,'binutils-2.2.1/opcodes', 6184 ).
file('sh-dis.c',text,'binutils-2.2.1/opcodes', 5204 ).
file('sh-opc.h',text,'binutils-2.2.1/opcodes', 13518 ).
file('sparc-dis.c',text,'binutils-2.2.1/opcodes', 13403 ).
file('sparc-opc.c',text,'binutils-2.2.1/opcodes', 43447 ).
file('z8k-dis.c',text,'binutils-2.2.1/opcodes', 13658 ).
file('z8k-opc.h',text,'binutils-2.2.1/opcodes', 120797 ).
file('z8kgen.c',text,'binutils-2.2.1/opcodes', 50115 ).
file('gpl.texinfo',text,'binutils-2.2.1/texinfo', 18545 ).
file('tex3patch',exec,'binutils-2.2.1/texinfo', 1762 ).
file('texinfo.tex',text,'binutils-2.2.1/texinfo', 129710 ).
file('COPYING',text,'bison-1.21', 17982 ).
file('ChangeLog',text,'bison-1.21', 35262 ).
file('INSTALL',text,'bison-1.21', 5733 ).
file('LR0.c',text,'bison-1.21', 15548 ).
file('Makefile',text,'bison-1.21', 5691 ).
file('Makefile.in',text,'bison-1.21', 5595 ).
file('README',text,'bison-1.21', 615 ).
file('REFERENCES',text,'bison-1.21', 1197 ).
file('alloca.c',text,'bison-1.21', 5418 ).
file('allocate.c',text,'bison-1.21', 1311 ).
file('bison.1',text,'bison-1.21', 4979 ).
file('bison.cld',text,'bison-1.21', 641 ).
file('bison.hairy',text,'bison-1.21', 6477 ).
file('bison.info',text,'bison-1.21', 3640 ).
file('bison.info-1',text,'bison-1.21', 50249 ).
file('bison.info-2',text,'bison-1.21', 47578 ).
file('bison.info-3',text,'bison-1.21', 51075 ).
file('bison.info-4',text,'bison-1.21', 51012 ).
file('bison.rnh',text,'bison-1.21', 3776 ).
file('bison.simple',text,'bison-1.21', 15923 ).
file('bison.texinfo',text,'bison-1.21', 194483 ).
file('build.com',text,'bison-1.21', 2613 ).
file('closure.c',text,'bison-1.21', 6750 ).
file('config.status',exec,'bison-1.21', 1431 ).
file('configure',exec,'bison-1.21', 14387 ).
file('configure.bat',text,'bison-1.21', 904 ).
file('configure.in',text,'bison-1.21', 269 ).
file('conflicts.c',text,'bison-1.21', 15480 ).
file('derives.c',text,'bison-1.21', 2281 ).
file('files.c',text,'bison-1.21', 9655 ).
file('files.h',text,'bison-1.21', 2177 ).
file('getargs.c',text,'bison-1.21', 3038 ).
file('getopt.c',text,'bison-1.21', 20882 ).
file('getopt.h',text,'bison-1.21', 4418 ).
file('getopt1.c',text,'bison-1.21', 4104 ).
file('gram.c',text,'bison-1.21', 1369 ).
file('gram.h',text,'bison-1.21', 4214 ).
file('lalr.c',text,'bison-1.21', 13874 ).
file('lex.c',text,'bison-1.21', 9954 ).
file('lex.h',text,'bison-1.21', 1401 ).
file('machine.h',text,'bison-1.21', 1391 ).
file('main.c',text,'bison-1.21', 3562 ).
file('new.h',text,'bison-1.21', 1088 ).
file('nullable.c',text,'bison-1.21', 2771 ).
file('output.c',text,'bison-1.21', 26664 ).
file('print.c',text,'bison-1.21', 7676 ).
file('reader.c',text,'bison-1.21', 37986 ).
file('reduce.c',text,'bison-1.21', 14036 ).
file('state.h',text,'bison-1.21', 4663 ).
file('symtab.c',text,'bison-1.21', 2539 ).
file('symtab.h',text,'bison-1.21', 1248 ).
file('system.h',text,'bison-1.21', 698 ).
file('texinfo.tex',text,'bison-1.21', 128926 ).
file('types.h',text,'bison-1.21', 931 ).
file('version.c',text,'bison-1.21', 861 ).
file('vmsgetargs.c',text,'bison-1.21', 3890 ).
file('vmshlp.mar',text,'bison-1.21', 1524 ).
file('warshall.c',text,'bison-1.21', 2428 ).
file('COPYING',text,'diff', 12488 ).
file('ChangeLog',text,'diff', 19300 ).
file('Makefile',text,'diff', 3536 ).
file('README',text,'diff', 5997 ).
file('alloca.c',text,'diff', 5214 ).
file('analyze.c',text,'diff', 26295 ).
file('context.c',text,'diff', 12479 ).
file('diagmeet.note',text,'diff', 1069 ).
file('diff.c',text,'diff', 17901 ).
file('diff.h',text,'diff', 10400 ).
file('diff3.c',text,'diff', 46642 ).
file('dir.c',text,'diff', 5765 ).
file('ed.c',text,'diff', 5254 ).
file('getopt.c',text,'diff', 16740 ).
file('getopt.h',text,'diff', 3554 ).
file('getopt1.c',text,'diff', 3388 ).
file('ifdef.c',text,'diff', 2564 ).
file('io.c',text,'diff', 18072 ).
file('limits.h',text,'diff', 1170 ).
file('normal.c',text,'diff', 2210 ).
file('regex.c',text,'diff', 83353 ).
file('regex.h',text,'diff', 9584 ).
file('util.c',text,'diff', 14888 ).
file('version.c',text,'diff', 67 ).
file('COPYING',text,'dld-3.2.3', 12488 ).
file('ChangeLog',text,'dld-3.2.3', 9471 ).
file('Makefile',text,'dld-3.2.3', 387 ).
file('README',text,'dld-3.2.3', 646 ).
file('TAGS',text,'dld-3.2.3', 2773 ).
file('TODO',text,'dld-3.2.3', 1640 ).
file('define.c',text,'dld-3.2.3', 1724 ).
file('defs.h',text,'dld-3.2.3', 6497 ).
file('dld.c',text,'dld-3.2.3', 59414 ).
file('dld.h',text,'dld-3.2.3', 2542 ).
file('doc',dir,'dld-3.2.3', 512 ).
file('error.c',text,'dld-3.2.3', 1920 ).
file('find_exec.c',text,'dld-3.2.3', 2099 ).
file('get_func.c',text,'dld-3.2.3', 1199 ).
file('get_symbol.c',text,'dld-3.2.3', 1134 ).
file('libdld.a',text,'dld-3.2.3', 113166 ).
file('list_undef.c',text,'dld-3.2.3', 1407 ).
file('mk_dummy.c',text,'dld-3.2.3', 1178 ).
file('ref.c',text,'dld-3.2.3', 1586 ).
file('remove.c',text,'dld-3.2.3', 1755 ).
file('test',dir,'dld-3.2.3', 512 ).
file('ul_file.c',text,'dld-3.2.3', 1462 ).
file('ul_symbol.c',text,'dld-3.2.3', 1346 ).
file('Makefile',text,'t', 220 ).
file('add1',dir,'t', 512 ).
file('general',dir,'t', 512 ).
file('overlay',dir,'t', 512 ).
file('reload',dir,'t', 512 ).
file('simple',dir,'t', 512 ).
file('Makefile',text,'t/add1', 323 ).
file('add1.c',text,'t/add1', 35 ).
file('call_add1.c',text,'t/add1', 587 ).
file('EXPECTED-OUTPUT',text,'t/general', 3428 ).
file('Makefile',text,'t/general', 440 ).
file('SAMPLE_INPUT',text,'t/general', 641 ).
file('get-sym.c',text,'t/general', 309 ).
file('hello.c',text,'t/general', 64 ).
file('list-undefined.c',text,'t/general', 392 ).
file('main.c',text,'t/general', 2343 ).
file('need.c',text,'t/general', 518 ).
file('print_arg.c',text,'t/general', 119 ).
file('print_global.c',text,'t/general', 104 ).
file('read-a.out.c',text,'t/general', 2813 ).
file('remove.c',text,'t/general', 144 ).
file('test-define.c',text,'t/general', 295 ).
file('Makefile',text,'t/overlay', 339 ).
file('chain1.c',text,'t/overlay', 222 ).
file('chain2.c',text,'t/overlay', 138 ).
file('chain3.c',text,'t/overlay', 133 ).
file('overlay.c',text,'t/overlay', 1319 ).
file('Makefile',text,'t/reload', 386 ).
file('reload-test.c',text,'t/reload', 160 ).
file('reload-test.s',text,'t/reload', 481 ).
file('reload.c',text,'t/reload', 1026 ).
file('Makefile',text,'t/simple', 285 ).
file('simple.c',text,'t/simple', 271 ).
file('LICENSE',text,'ecc-1.2', 12488 ).
file('Makefile',text,'ecc-1.2', 152 ).
file('ecc.1',text,'ecc-1.2', 1513 ).
file('ecc.c',text,'ecc-1.2', 2428 ).
file('ecc.h',text,'ecc-1.2', 1104 ).
file('gf.h',text,'ecc-1.2', 3044 ).
file('gflib.c',text,'ecc-1.2', 1961 ).
file('rslib.c',text,'ecc-1.2', 7431 ).
file('COPYING',text,'expect3.24', 17982 ).
file('COPYING.LIB',text,'expect3.24', 25265 ).
file('CYGNUS',text,'expect3.24', 1750 ).
file('Makefile',text,'expect3.24', 39428 ).
file('Makefile.in',text,'expect3.24', 38775 ).
file('README',text,'expect3.24', 894 ).
file('config',dir,'expect3.24', 512 ).
file('config.status',exec,'expect3.24', 195 ).
file('config.sub',exec,'expect3.24', 14981 ).
file('configure',exec,'expect3.24', 26841 ).
file('configure.in',text,'expect3.24', 5825 ).
file('etc',dir,'expect3.24', 512 ).
file('expect',dir,'expect3.24', 1536 ).
file('move-if-change',exec,'expect3.24', 129 ).
file('tcl',dir,'expect3.24', 1536 ).
file('test-build.mk',text,'expect3.24', 14892 ).
file('ChangeLog',text,'expect3.24/config', 1032 ).
file('mh-a68bsd',text,'expect3.24/config', 90 ).
file('mh-aix',text,'expect3.24/config', 13 ).
file('mh-apollo68',text,'expect3.24/config', 91 ).
file('mh-decstation',text,'expect3.24/config', 198 ).
file('mh-delta88',text,'expect3.24/config', 94 ).
file('mh-dgux',text,'expect3.24/config', 76 ).
file('mh-hpux',text,'expect3.24/config', 193 ).
file('mh-irix4',text,'expect3.24/config', 286 ).
file('mh-linux',text,'expect3.24/config', 118 ).
file('mh-ncr3000',text,'expect3.24/config', 832 ).
file('mh-sco',text,'expect3.24/config', 120 ).
file('mh-solaris',text,'expect3.24/config', 282 ).
file('mh-sun',text,'expect3.24/config', 116 ).
file('mh-sysv',text,'expect3.24/config', 111 ).
file('mh-sysv4',text,'expect3.24/config', 218 ).
file('mt-a29k',text,'expect3.24/config', 258 ).
file('mt-ebmon29k',text,'expect3.24/config', 201 ).
file('mt-os68k',text,'expect3.24/config', 45 ).
file('mt-ose68000',text,'expect3.24/config', 45 ).
file('mt-ose68k',text,'expect3.24/config', 45 ).
file('mt-vxworks68',text,'expect3.24/config', 45 ).
file('mt-vxworks960',text,'expect3.24/config', 45 ).
file('ChangeLog',text,'expect3.24/etc', 435 ).
file('Makefile',text,'expect3.24/etc', 2241 ).
file('Makefile.in',text,'expect3.24/etc', 1904 ).
file('cfg-paper.texi',text,'expect3.24/etc', 29217 ).
file('config.status',exec,'expect3.24/etc', 190 ).
file('configure.in',text,'expect3.24/etc', 378 ).
file('configure.man',text,'expect3.24/etc', 2684 ).
file('configure.texi',text,'expect3.24/etc', 51270 ).
file('install-texi.in',text,'expect3.24/etc', 65231 ).
file('intro.texi',text,'expect3.24/etc', 16186 ).
file('relnotes.texi',text,'expect3.24/etc', 34297 ).
file('standards.texi',text,'expect3.24/etc', 54949 ).
file('substitute-strings',exec,'expect3.24/etc', 2021 ).
file('CONVERTING',text,'expect3.24/expect', 2319 ).
file('ChangeLog',text,'expect3.24/expect', 1352 ).
file('FAQ',text,'expect3.24/expect', 25586 ).
file('HISTORY',text,'expect3.24/expect', 56986 ).
file('INSTALL',text,'expect3.24/expect', 3034 ).
file('Makefile',text,'expect3.24/expect', 10965 ).
file('Makefile.in',text,'expect3.24/expect', 10580 ).
file('README',text,'expect3.24/expect', 5170 ).
file('command.c',text,'expect3.24/expect', 43223 ).
file('command.h',text,'expect3.24/expect', 2166 ).
file('config',dir,'expect3.24/expect', 512 ).
file('config.status',exec,'expect3.24/expect', 211 ).
file('configure.in',text,'expect3.24/expect', 922 ).
file('expect.c',text,'expect3.24/expect', 46667 ).
file('expect.h',text,'expect3.24/expect', 2550 ).
file('expect.man',text,'expect3.24/expect', 56836 ).
file('fixline1',text,'expect3.24/expect', 366 ).
file('global.h',text,'expect3.24/expect', 2060 ).
file('install.log',text,'expect3.24/expect', 1473 ).
file('inter_poll.c',text,'expect3.24/expect', 20683 ).
file('inter_select.c',text,'expect3.24/expect', 21996 ).
file('inter_simple.c',text,'expect3.24/expect', 19885 ).
file('lib_debug.c',text,'expect3.24/expect', 951 ).
file('lib_exp.c',text,'expect3.24/expect', 13887 ).
file('lib_string.c',text,'expect3.24/expect', 2926 ).
file('libexpect.man',text,'expect3.24/expect', 14480 ).
file('main.c',text,'expect3.24/expect', 20163 ).
file('pty_aix3.c',text,'expect3.24/expect', 2799 ).
file('pty_bsd.c',text,'expect3.24/expect', 8965 ).
file('pty_sgi.c',text,'expect3.24/expect', 2604 ).
file('pty_sgi3.c',text,'expect3.24/expect', 2784 ).
file('pty_svr4.c',text,'expect3.24/expect', 3387 ).
file('pty_unicos.c',text,'expect3.24/expect', 5755 ).
file('pty_usg.c',text,'expect3.24/expect', 4446 ).
file('regress.ps',text,'expect3.24/expect', 161342 ).
file('scripts',dir,'expect3.24/expect', 512 ).
file('scripts.ps',text,'expect3.24/expect', 115945 ).
file('seminal.ps',text,'expect3.24/expect', 113608 ).
file('sysadm.ps',text,'expect3.24/expect', 90609 ).
file('term.h',text,'expect3.24/expect', 628 ).
file('test',dir,'expect3.24/expect', 512 ).
file('translate.h',text,'expect3.24/expect', 895 ).
file('unit_random.c',text,'expect3.24/expect', 610 ).
file('vgrindefs',text,'expect3.24/expect', 911 ).
file('mh-aix',text,'expect3.24/expect/config', 48 ).
file('mh-bsd',text,'expect3.24/expect/config', 13 ).
file('mh-irix',text,'expect3.24/expect/config', 14 ).
file('mh-irix4',text,'expect3.24/expect/config', 65 ).
file('mh-sysv',text,'expect3.24/expect/config', 49 ).
file('mh-sysv4',text,'expect3.24/expect/config', 53 ).
file('mh-unicos',text,'expect3.24/expect/config', 16 ).
file('README',text,'expect3.24/expect/scripts', 856 ).
file('bonfield.exp',text,'expect3.24/expect/scripts', 2699 ).
file('faxstat',text,'expect3.24/expect/scripts', 1210 ).
file('noidle',text,'expect3.24/expect/scripts', 498 ).
file('script.exp',text,'expect3.24/expect/scripts', 270 ).
file('su2',text,'expect3.24/expect/scripts', 535 ).
file('Makefile',text,'expect3.24/expect/test', 1044 ).
file('archie',text,'expect3.24/expect/test', 776 ).
file('chess.exp',exec,'expect3.24/expect/test', 1317 ).
file('chesslib++.c',text,'expect3.24/expect/test', 1336 ).
file('chesslib.c',text,'expect3.24/expect/test', 1276 ).
file('chesslib2.c',text,'expect3.24/expect/test', 1304 ).
file('dvorak',exec,'expect3.24/expect/test', 940 ).
file('ftp-rfc',exec,'expect3.24/expect/test', 571 ).
file('ftp.exp',exec,'expect3.24/expect/test', 1334 ).
file('kibitz',exec,'expect3.24/expect/test', 9985 ).
file('kibitz.man',text,'expect3.24/expect/test', 5428 ).
file('lpunlock',exec,'expect3.24/expect/test', 2544 ).
file('passmass',exec,'expect3.24/expect/test', 4464 ).
file('passwd.exp',exec,'expect3.24/expect/test', 258 ).
file('rftp',exec,'expect3.24/expect/test', 9079 ).
file('rlogin.exp',text,'expect3.24/expect/test', 448 ).
file('robohunt',exec,'expect3.24/expect/test', 2053 ).
file('rogue.exp',exec,'expect3.24/expect/test', 291 ).
file('time.exp',exec,'expect3.24/expect/test', 163 ).
file('timed_read',exec,'expect3.24/expect/test', 236 ).
file('weather',exec,'expect3.24/expect/test', 3094 ).
file('ChangeLog',text,'expect3.24/tcl', 484 ).
file('Makefile',text,'expect3.24/tcl', 5510 ).
file('Makefile.in',text,'expect3.24/tcl', 5389 ).
file('README',text,'expect3.24/tcl', 8761 ).
file('changes',text,'expect3.24/tcl', 17924 ).
file('compat',dir,'expect3.24/tcl', 512 ).
file('config.status',exec,'expect3.24/tcl', 1521 ).
file('configure',exec,'expect3.24/tcl', 10411 ).
file('configure.in',text,'expect3.24/tcl', 1190 ).
file('doc',dir,'expect3.24/tcl', 1024 ).
file('install.log',text,'expect3.24/tcl', 851 ).
file('library',dir,'expect3.24/tcl', 512 ).
file('panic.c',text,'expect3.24/tcl', 1607 ).
file('porting.notes',text,'expect3.24/tcl', 5608 ).
file('regexp.c',text,'expect3.24/tcl', 28040 ).
file('regexp.h',text,'expect3.24/tcl', 806 ).
file('strerror.c',text,'expect3.24/tcl', 11483 ).
file('strtoul.c',text,'expect3.24/tcl', 4320 ).
file('tcl.h',text,'expect3.24/tcl', 11759 ).
file('tclAssem.c',text,'expect3.24/tcl', 5819 ).
file('tclBasic.c',text,'expect3.24/tcl', 28644 ).
file('tclCkalloc.c',text,'expect3.24/tcl', 15324 ).
file('tclCmdAH.c',text,'expect3.24/tcl', 22171 ).
file('tclCmdIL.c',text,'expect3.24/tcl', 29323 ).
file('tclCmdMZ.c',text,'expect3.24/tcl', 35679 ).
file('tclEnv.c',text,'expect3.24/tcl', 11193 ).
file('tclExpr.c',text,'expect3.24/tcl', 34329 ).
file('tclGet.c',text,'expect3.24/tcl', 5186 ).
file('tclGlob.c',text,'expect3.24/tcl', 15072 ).
file('tclHash.c',text,'expect3.24/tcl', 25021 ).
file('tclHash.h',text,'expect3.24/tcl', 4970 ).
file('tclHistory.c',text,'expect3.24/tcl', 30517 ).
file('tclInt.h',text,'expect3.24/tcl', 32374 ).
file('tclParse.c',text,'expect3.24/tcl', 32725 ).
file('tclProc.c',text,'expect3.24/tcl', 14710 ).
file('tclTest.c',text,'expect3.24/tcl', 3308 ).
file('tclUnix.h',text,'expect3.24/tcl', 8345 ).
file('tclUnixAZ.c',text,'expect3.24/tcl', 42972 ).
file('tclUnixStr.c',text,'expect3.24/tcl', 15238 ).
file('tclUnixUtil.c',text,'expect3.24/tcl', 28026 ).
file('tclUtil.c',text,'expect3.24/tcl', 37219 ).
file('tclVar.c',text,'expect3.24/tcl', 63857 ).
file('tests',dir,'expect3.24/tcl', 1024 ).
file('README',text,'expect3.24/tcl/compat', 363 ).
file('dirent.h',text,'expect3.24/tcl/compat', 811 ).
file('dirent2.h',text,'expect3.24/tcl/compat', 1660 ).
file('limits.h',text,'expect3.24/tcl/compat', 895 ).
file('opendir.c',text,'expect3.24/tcl/compat', 1983 ).
file('stdlib.h',text,'expect3.24/tcl/compat', 1941 ).
file('strerror.c',text,'expect3.24/tcl/compat', 11483 ).
file('string.h',text,'expect3.24/tcl/compat', 2143 ).
file('strstr.c',text,'expect3.24/tcl/compat', 1924 ).
file('strtod.c',text,'expect3.24/tcl/compat', 6091 ).
file('strtol.c',text,'expect3.24/tcl/compat', 2269 ).
file('strtoul.c',text,'expect3.24/tcl/compat', 4320 ).
file('testpid.c',text,'expect3.24/tcl/compat', 911 ).
file('teststrtoul.c',text,'expect3.24/tcl/compat', 1281 ).
file('testuid.c',text,'expect3.24/tcl/compat', 911 ).
file('testwait.c',text,'expect3.24/tcl/compat', 1248 ).
file('AddErrInfo.man',text,'expect3.24/tcl/doc', 9018 ).
file('AssembCmd.man',text,'expect3.24/tcl/doc', 6536 ).
file('Backslash.man',text,'expect3.24/tcl/doc', 5466 ).
file('Concat.man',text,'expect3.24/tcl/doc', 5497 ).
file('CrtCommand.man',text,'expect3.24/tcl/doc', 8855 ).
file('CrtInterp.man',text,'expect3.24/tcl/doc', 5395 ).
file('CrtPipelin.man',text,'expect3.24/tcl/doc', 8162 ).
file('CrtTrace.man',text,'expect3.24/tcl/doc', 8334 ).
file('Eval.man',text,'expect3.24/tcl/doc', 8648 ).
file('ExprLong.man',text,'expect3.24/tcl/doc', 7453 ).
file('Fork.man',text,'expect3.24/tcl/doc', 9152 ).
file('GetInt.man',text,'expect3.24/tcl/doc', 6943 ).
file('Hash.man',text,'expect3.24/tcl/doc', 12118 ).
file('History.man',text,'expect3.24/tcl/doc', 6275 ).
file('Interp.man',text,'expect3.24/tcl/doc', 9244 ).
file('SetResult.man',text,'expect3.24/tcl/doc', 9940 ).
file('SetVar.man',text,'expect3.24/tcl/doc', 9695 ).
file('SplitList.man',text,'expect3.24/tcl/doc', 10068 ).
file('StrMatch.man',text,'expect3.24/tcl/doc', 4948 ).
file('Tcl.man',text,'expect3.24/tcl/doc', 119443 ).
file('TildeSubst.man',text,'expect3.24/tcl/doc', 5900 ).
file('TraceVar.man',text,'expect3.24/tcl/doc', 17097 ).
file('library.man',text,'expect3.24/tcl/doc', 11250 ).
file('usenix.ps',text,'expect3.24/tcl/doc', 118931 ).
file('usenix.text',text,'expect3.24/tcl/doc', 41391 ).
file('init.tcl',text,'expect3.24/tcl/library', 4245 ).
file('mkindex.tcl',text,'expect3.24/tcl/library', 1607 ).
file('parray.tcl',text,'expect3.24/tcl/library', 1003 ).
file('tclIndex',text,'expect3.24/tcl/library', 277 ).
file('README',text,'expect3.24/tcl/tests', 3434 ).
file('all',text,'expect3.24/tcl/tests', 317 ).
file('append.test',text,'expect3.24/tcl/tests', 3296 ).
file('case.test',text,'expect3.24/tcl/tests', 3095 ).
file('cd.test',text,'expect3.24/tcl/tests', 3042 ).
file('concat.test',text,'expect3.24/tcl/tests', 1606 ).
file('defs',text,'expect3.24/tcl/tests', 2256 ).
file('env.test',text,'expect3.24/tcl/tests', 2834 ).
file('error.test',text,'expect3.24/tcl/tests', 5001 ).
file('eval.test',text,'expect3.24/tcl/tests', 1767 ).
file('exec.test',text,'expect3.24/tcl/tests', 6322 ).
file('expr.test',text,'expect3.24/tcl/tests', 19886 ).
file('file.test',text,'expect3.24/tcl/tests', 12074 ).
file('for.test',text,'expect3.24/tcl/tests', 4311 ).
file('format.test',text,'expect3.24/tcl/tests', 12169 ).
file('glob.test',text,'expect3.24/tcl/tests', 4378 ).
file('globTest',dir,'expect3.24/tcl/tests', 512 ).
file('history.test',text,'expect3.24/tcl/tests', 12712 ).
file('if.test',text,'expect3.24/tcl/tests', 4549 ).
file('incr.test',text,'expect3.24/tcl/tests', 2298 ).
file('info.test',text,'expect3.24/tcl/tests', 11209 ).
file('join.test',text,'expect3.24/tcl/tests', 1521 ).
file('lindex.test',text,'expect3.24/tcl/tests', 2292 ).
file('linsert.test',text,'expect3.24/tcl/tests', 2401 ).
file('list.test',text,'expect3.24/tcl/tests', 3011 ).
file('llength.test',text,'expect3.24/tcl/tests', 1418 ).
file('lrange.test',text,'expect3.24/tcl/tests', 2525 ).
file('lreplace.test',text,'expect3.24/tcl/tests', 2953 ).
file('lsearch.test',text,'expect3.24/tcl/tests', 1632 ).
file('lsort.test',text,'expect3.24/tcl/tests', 1507 ).
file('misc.test',text,'expect3.24/tcl/tests', 2409 ).
file('open.test',text,'expect3.24/tcl/tests', 16453 ).
file('parse.test',text,'expect3.24/tcl/tests', 11361 ).
file('proc.test',text,'expect3.24/tcl/tests', 9161 ).
file('regexp.test',text,'expect3.24/tcl/tests', 9486 ).
file('rename.test',text,'expect3.24/tcl/tests', 2524 ).
file('scan.test',text,'expect3.24/tcl/tests', 20342 ).
file('set.test',text,'expect3.24/tcl/tests', 16974 ).
file('source.test',text,'expect3.24/tcl/tests', 2611 ).
file('split.test',text,'expect3.24/tcl/tests', 1707 ).
file('string.test',text,'expect3.24/tcl/tests', 9966 ).
file('trace.test',text,'expect3.24/tcl/tests', 20396 ).
file('unknown.test',text,'expect3.24/tcl/tests', 1952 ).
file('uplevel.test',text,'expect3.24/tcl/tests', 2667 ).
file('upvar.test',text,'expect3.24/tcl/tests', 5412 ).
file('while.test',text,'expect3.24/tcl/tests', 2872 ).
file('a1',dir,'expect3.24/tcl/tests/globTest', 512 ).
file('a2',dir,'expect3.24/tcl/tests/globTest', 512 ).
file('a3',dir,'expect3.24/tcl/tests/globTest', 512 ).
file('x1.c',text,'expect3.24/tcl/tests/globTest', 3 ).
file('y1.c',text,'expect3.24/tcl/tests/globTest', 3 ).
file('z1.c',text,'expect3.24/tcl/tests/globTest', 3 ).
file('b1',dir,'expect3.24/tcl/tests/globTest/a1', 512 ).
file('b2',dir,'expect3.24/tcl/tests/globTest/a1', 512 ).
file('b3',dir,'expect3.24/tcl/tests/globTest/a2', 512 ).
file('COPYING',text,'fileutils-3.6', 17982 ).
file('COPYING.LIB',text,'fileutils-3.6', 25265 ).
file('ChangeLog',text,'fileutils-3.6', 81117 ).
file('INSTALL',text,'fileutils-3.6', 5776 ).
file('Makefile',text,'fileutils-3.6', 6774 ).
file('Makefile.in',text,'fileutils-3.6', 6331 ).
file('NEWS',text,'fileutils-3.6', 1313 ).
file('README',text,'fileutils-3.6', 4413 ).
file('config.status',exec,'fileutils-3.6', 2109 ).
file('configure',exec,'fileutils-3.6', 34507 ).
file('configure.in',text,'fileutils-3.6', 4138 ).
file('lib',dir,'fileutils-3.6', 2048 ).
file('man',dir,'fileutils-3.6', 512 ).
file('mkinstalldirs',exec,'fileutils-3.6', 585 ).
file('src',dir,'fileutils-3.6', 1536 ).
file('Makefile',text,'fileutils-3.6/lib', 3194 ).
file('Makefile.in',text,'fileutils-3.6/lib', 3127 ).
file('alloca.c',text,'fileutils-3.6/lib', 13198 ).
file('argmatch.c',text,'fileutils-3.6/lib', 2485 ).
file('backupfile.c',text,'fileutils-3.6/lib', 5766 ).
file('backupfile.h',text,'fileutils-3.6/lib', 1325 ).
file('basename.c',text,'fileutils-3.6/lib', 1117 ).
file('dirname.c',text,'fileutils-3.6/lib', 1775 ).
file('eaccess.c',text,'fileutils-3.6/lib', 3949 ).
file('error.c',text,'fileutils-3.6/lib', 2926 ).
file('fileblocks.c',text,'fileutils-3.6/lib', 1779 ).
file('filemode.c',text,'fileutils-3.6/lib', 5594 ).
file('fnmatch.c',text,'fileutils-3.6/lib', 4878 ).
file('fnmatch.h',text,'fileutils-3.6/lib', 2158 ).
file('fsusage.c',text,'fileutils-3.6/lib', 5899 ).
file('fsusage.h',text,'fileutils-3.6/lib', 1259 ).
file('ftruncate.c',text,'fileutils-3.6/lib', 1351 ).
file('getdate.c',text,'fileutils-3.6/lib', 48862 ).
file('getdate.y',text,'fileutils-3.6/lib', 24360 ).
file('getopt.c',text,'fileutils-3.6/lib', 20876 ).
file('getopt.h',text,'fileutils-3.6/lib', 4412 ).
file('getopt1.c',text,'fileutils-3.6/lib', 4142 ).
file('getversion.c',text,'fileutils-3.6/lib', 1573 ).
file('idcache.c',text,'fileutils-3.6/lib', 5230 ).
file('isdir.c',text,'fileutils-3.6/lib', 1154 ).
file('makepath.c',text,'fileutils-3.6/lib', 6654 ).
file('mkdir.c',text,'fileutils-3.6/lib', 3371 ).
file('mktime.c',text,'fileutils-3.6/lib', 7273 ).
file('modechange.c',text,'fileutils-3.6/lib', 9227 ).
file('modechange.h',text,'fileutils-3.6/lib', 2029 ).
file('mountlist.c',text,'fileutils-3.6/lib', 11536 ).
file('mountlist.h',text,'fileutils-3.6/lib', 1258 ).
file('pathmax.h',text,'fileutils-3.6/lib', 1691 ).
file('posixtm.c',text,'fileutils-3.6/lib', 23042 ).
file('posixtm.y',text,'fileutils-3.6/lib', 3802 ).
file('rename.c',text,'fileutils-3.6/lib', 2496 ).
file('savedir.c',text,'fileutils-3.6/lib', 3189 ).
file('stpcpy.c',text,'fileutils-3.6/lib', 1145 ).
file('strdup.c',text,'fileutils-3.6/lib', 1140 ).
file('stripslash.c',text,'fileutils-3.6/lib', 1390 ).
file('strstr.c',text,'fileutils-3.6/lib', 1388 ).
file('system.h',text,'fileutils-3.6/lib', 6191 ).
file('userspec.c',text,'fileutils-3.6/lib', 4175 ).
file('xgetcwd.c',text,'fileutils-3.6/lib', 1872 ).
file('xmalloc.c',text,'fileutils-3.6/lib', 1615 ).
file('xstrdup.c',text,'fileutils-3.6/lib', 1073 ).
file('yesno.c',text,'fileutils-3.6/lib', 1097 ).
file('Makefile',text,'fileutils-3.6/man', 1571 ).
file('Makefile.in',text,'fileutils-3.6/man', 1528 ).
file('chgrp.1',text,'fileutils-3.6/man', 974 ).
file('chmod.1',text,'fileutils-3.6/man', 3004 ).
file('chown.1',text,'fileutils-3.6/man', 1660 ).
file('cp.1',text,'fileutils-3.6/man', 4082 ).
file('dd.1',text,'fileutils-3.6/man', 2374 ).
file('df.1',text,'fileutils-3.6/man', 2886 ).
file('du.1',text,'fileutils-3.6/man', 2421 ).
file('install.1',text,'fileutils-3.6/man', 2181 ).
file('ln.1',text,'fileutils-3.6/man', 2695 ).
file('ls.1',text,'fileutils-3.6/man', 6780 ).
file('mkdir.1',text,'fileutils-3.6/man', 974 ).
file('mkfifo.1',text,'fileutils-3.6/man', 703 ).
file('mknod.1',text,'fileutils-3.6/man', 1111 ).
file('mv.1',text,'fileutils-3.6/man', 2812 ).
file('rm.1',text,'fileutils-3.6/man', 1889 ).
file('rmdir.1',text,'fileutils-3.6/man', 698 ).
file('touch.1',text,'fileutils-3.6/man', 1940 ).
file('Makefile',text,'fileutils-3.6/src', 5896 ).
file('Makefile.in',text,'fileutils-3.6/src', 5857 ).
file('chgrp.c',text,'fileutils-3.6/src', 6826 ).
file('chmod.c',text,'fileutils-3.6/src', 7308 ).
file('chown.c',text,'fileutils-3.6/src', 7393 ).
file('cp-aux.c',text,'fileutils-3.6/src', 1843 ).
file('cp-hash.c',text,'fileutils-3.6/src', 5629 ).
file('cp.c',text,'fileutils-3.6/src', 30685 ).
file('cp.h',text,'fileutils-3.6/src', 2124 ).
file('dd.c',text,'fileutils-3.6/src', 28008 ).
file('df.c',text,'fileutils-3.6/src', 11874 ).
file('dir.c',text,'fileutils-3.6/src', 41447 ).
file('du.c',text,'fileutils-3.6/src', 17011 ).
file('install.c',text,'fileutils-3.6/src', 12351 ).
file('ln.c',text,'fileutils-3.6/src', 7659 ).
file('ls.c',text,'fileutils-3.6/src', 41447 ).
file('mkdir.c',text,'fileutils-3.6/src', 3422 ).
file('mkfifo.c',text,'fileutils-3.6/src', 2902 ).
file('mknod.c',text,'fileutils-3.6/src', 4028 ).
file('mv.c',text,'fileutils-3.6/src', 10300 ).
file('mvdir.c',text,'fileutils-3.6/src', 6113 ).
file('rm.c',text,'fileutils-3.6/src', 13189 ).
file('rmdir.c',text,'fileutils-3.6/src', 3166 ).
file('touch.c',text,'fileutils-3.6/src', 8331 ).
file('vdir.c',text,'fileutils-3.6/src', 41447 ).
file('version.c',text,'fileutils-3.6/src', 71 ).
file('version.h',text,'fileutils-3.6/src', 35 ).
file('COPYING',text,'flex-2.3.8', 1802 ).
file('Changes',text,'flex-2.3.8', 13731 ).
file('MISC',dir,'flex-2.3.8', 512 ).
file('Makefile',text,'flex-2.3.8', 4926 ).
file('README',text,'flex-2.3.8', 1628 ).
file('ccl.c',text,'flex-2.3.8', 4115 ).
file('dfa.c',text,'flex-2.3.8', 26921 ).
file('ecs.c',text,'flex-2.3.8', 8758 ).
file('flex.1',text,'flex-2.3.8', 20799 ).
file('flex.skel',text,'flex-2.3.8', 19822 ).
file('flexdef.h',text,'flex-2.3.8', 29063 ).
file('flexdoc.1',text,'flex-2.3.8', 65395 ).
file('gen.c',text,'flex-2.3.8', 31541 ).
file('initscan.c',text,'flex-2.3.8', 67296 ).
file('libmain.c',text,'flex-2.3.8', 256 ).
file('main.c',text,'flex-2.3.8', 19472 ).
file('makefile',text,'flex-2.3.8', 4930 ).
file('misc.c',text,'flex-2.3.8', 14932 ).
file('nfa.c',text,'flex-2.3.8', 17603 ).
file('parse.y',text,'flex-2.3.8', 14931 ).
file('scan.c',text,'flex-2.3.8', 67296 ).
file('scan.l',text,'flex-2.3.8', 12440 ).
file('sym.c',text,'flex-2.3.8', 7543 ).
file('tblcmp.c',text,'flex-2.3.8', 25192 ).
file('yylex.c',text,'flex-2.3.8', 4244 ).
file('Atari.patches',text,'flex-2.3.8/MISC', 29474 ).
file('MSDOS.notes',text,'flex-2.3.8/MISC', 5328 ).
file('Makefile.VMS',text,'flex-2.3.8/MISC', 3537 ).
file('README',text,'flex-2.3.8/MISC', 520 ).
file('Turbo-C.notes',text,'flex-2.3.8/MISC', 6089 ).
file('AUTHORS',text,'fontutils-0.6', 377 ).
file('COPYING',text,'fontutils-0.6', 17982 ).
file('COPYING.LIB',text,'fontutils-0.6', 25265 ).
file('ChangeLog',text,'fontutils-0.6', 15989 ).
file('GNUmakefile',text,'fontutils-0.6', 6808 ).
file('GNUmakefile.in',text,'fontutils-0.6', 6769 ).
file('INSTALL',text,'fontutils-0.6', 12568 ).
file('NEWS',text,'fontutils-0.6', 2201 ).
file('README',text,'fontutils-0.6', 1845 ).
file('aclocal.m4',text,'fontutils-0.6', 1960 ).
file('bin',dir,'fontutils-0.6', 512 ).
file('bpltobzr',dir,'fontutils-0.6', 512 ).
file('bzr',dir,'fontutils-0.6', 512 ).
file('bzrto',dir,'fontutils-0.6', 1024 ).
file('charspace',dir,'fontutils-0.6', 1024 ).
file('config.status',exec,'fontutils-0.6', 2623 ).
file('configure',exec,'fontutils-0.6', 16950 ).
file('configure.in',text,'fontutils-0.6', 805 ).
file('data',dir,'fontutils-0.6', 512 ).
file('doc',dir,'fontutils-0.6', 1024 ).
file('fontconvert',dir,'fontutils-0.6', 1024 ).
file('gf',dir,'fontutils-0.6', 512 ).
file('gsrenderfont',dir,'fontutils-0.6', 512 ).
file('imageto',dir,'fontutils-0.6', 1024 ).
file('imgrotate',dir,'fontutils-0.6', 512 ).
file('include',dir,'fontutils-0.6', 1536 ).
file('lib',dir,'fontutils-0.6', 3072 ).
file('limn',dir,'fontutils-0.6', 1024 ).
file('pbm',dir,'fontutils-0.6', 512 ).
file('pk',dir,'fontutils-0.6', 512 ).
file('tfm',dir,'fontutils-0.6', 512 ).
file('widgets',dir,'fontutils-0.6', 512 ).
file('xbfe',dir,'fontutils-0.6', 512 ).
file('imagestrip',exec,'fontutils-0.6/bin', 440 ).
file('makepsfonts',exec,'fontutils-0.6/bin', 869 ).
file('otest',exec,'fontutils-0.6/bin', 60 ).
file('pcharstr',exec,'fontutils-0.6/bin', 1009 ).
file('proof',exec,'fontutils-0.6/bin', 999 ).
file('rfont',exec,'fontutils-0.6/bin', 251 ).
file('show-gf',exec,'fontutils-0.6/bin', 679 ).
file('ChangeLog',text,'fontutils-0.6/bpltobzr', 1381 ).
file('GNUmakefile',text,'fontutils-0.6/bpltobzr', 915 ).
file('M.depend',text,'fontutils-0.6/bpltobzr', 1101 ).
file('README',text,'fontutils-0.6/bpltobzr', 99 ).
file('bpl.h',text,'fontutils-0.6/bpltobzr', 647 ).
file('bpl.y',text,'fontutils-0.6/bpltobzr', 10631 ).
file('main.c',text,'fontutils-0.6/bpltobzr', 4283 ).
file('main.h',text,'fontutils-0.6/bpltobzr', 861 ).
file('version.c',text,'fontutils-0.6/bpltobzr', 47 ).
file('ChangeLog',text,'fontutils-0.6/bzr', 7191 ).
file('GNUmakefile',text,'fontutils-0.6/bzr', 910 ).
file('M.depend',text,'fontutils-0.6/bzr', 1141 ).
file('README',text,'fontutils-0.6/bzr', 420 ).
file('bzr_input.c',text,'fontutils-0.6/bzr', 11143 ).
file('bzr_opcodes.h',text,'fontutils-0.6/bzr', 1253 ).
file('bzr_output.c',text,'fontutils-0.6/bzr', 11171 ).
file('bzr_types.h',text,'fontutils-0.6/bzr', 1095 ).
file('ChangeLog',text,'fontutils-0.6/bzrto', 44968 ).
file('GNUmakefile',text,'fontutils-0.6/bzrto', 1167 ).
file('M.depend',text,'fontutils-0.6/bzrto', 5062 ).
file('README',text,'fontutils-0.6/bzrto', 587 ).
file('bzrbuildch.PS',text,'fontutils-0.6/bzrto', 1449 ).
file('bzredit.el',text,'fontutils-0.6/bzrto', 5687 ).
file('bzrsetup.mf',text,'fontutils-0.6/bzrto', 7414 ).
file('ccc.h',text,'fontutils-0.6/bzrto', 616 ).
file('ccc.y',text,'fontutils-0.6/bzrto', 10842 ).
file('char.c',text,'fontutils-0.6/bzrto', 7281 ).
file('char.h',text,'fontutils-0.6/bzrto', 1802 ).
file('input-ccc.c',text,'fontutils-0.6/bzrto', 5446 ).
file('input-ccc.h',text,'fontutils-0.6/bzrto', 3004 ).
file('main.c',text,'fontutils-0.6/bzrto', 19332 ).
file('main.h',text,'fontutils-0.6/bzrto', 1180 ).
file('metafont.c',text,'fontutils-0.6/bzrto', 18997 ).
file('metafont.h',text,'fontutils-0.6/bzrto', 1542 ).
file('pstype1.c',text,'fontutils-0.6/bzrto', 25228 ).
file('pstype1.h',text,'fontutils-0.6/bzrto', 1759 ).
file('pstype3.c',text,'fontutils-0.6/bzrto', 7821 ).
file('pstype3.h',text,'fontutils-0.6/bzrto', 1206 ).
file('psutil.c',text,'fontutils-0.6/bzrto', 11552 ).
file('psutil.h',text,'fontutils-0.6/bzrto', 2211 ).
file('text.c',text,'fontutils-0.6/bzrto', 4088 ).
file('text.h',text,'fontutils-0.6/bzrto', 1094 ).
file('version.c',text,'fontutils-0.6/bzrto', 44 ).
file('ChangeLog',text,'fontutils-0.6/charspace', 13020 ).
file('GNUmakefile',text,'fontutils-0.6/charspace', 971 ).
file('M.depend',text,'fontutils-0.6/charspace', 3632 ).
file('README',text,'fontutils-0.6/charspace', 1016 ).
file('char.c',text,'fontutils-0.6/charspace', 12377 ).
file('char.h',text,'fontutils-0.6/charspace', 2505 ).
file('cmi.h',text,'fontutils-0.6/charspace', 313 ).
file('cmi.y',text,'fontutils-0.6/charspace', 7998 ).
file('input-cmi.c',text,'fontutils-0.6/charspace', 1627 ).
file('input-cmi.h',text,'fontutils-0.6/charspace', 1195 ).
file('kern.c',text,'fontutils-0.6/charspace', 1653 ).
file('kern.h',text,'fontutils-0.6/charspace', 1378 ).
file('main.c',text,'fontutils-0.6/charspace', 10590 ).
file('main.h',text,'fontutils-0.6/charspace', 1175 ).
file('output.c',text,'fontutils-0.6/charspace', 10859 ).
file('output.h',text,'fontutils-0.6/charspace', 1228 ).
file('realstrval.h',text,'fontutils-0.6/charspace', 1403 ).
file('symtab.c',text,'fontutils-0.6/charspace', 7686 ).
file('symtab.h',text,'fontutils-0.6/charspace', 2471 ).
file('version.c',text,'fontutils-0.6/charspace', 48 ).
file('ChangeLog',text,'fontutils-0.6/data', 14271 ).
file('adobestd.enc',text,'fontutils-0.6/data', 1810 ).
file('ascii.enc',text,'fontutils-0.6/data', 697 ).
file('common.cmi',text,'fontutils-0.6/data', 8020 ).
file('defs.make',text,'fontutils-0.6/data', 5171 ).
file('defslib.make',text,'fontutils-0.6/data', 1241 ).
file('defsprog.make',text,'fontutils-0.6/data', 1600 ).
file('dvips.enc',text,'fontutils-0.6/data', 2372 ).
file('encoding.map',text,'fontutils-0.6/data', 440 ).
file('ggmr.1200cmi',text,'fontutils-0.6/data', 310 ).
file('ggmr.ifi',text,'fontutils-0.6/data', 1533 ).
file('gnulatin.enc',text,'fontutils-0.6/data', 3497 ).
file('gnulcomp.enc',text,'fontutils-0.6/data', 7691 ).
file('mflogo.enc',text,'fontutils-0.6/data', 691 ).
file('printeps.tex',text,'fontutils-0.6/data', 575 ).
file('psymbol.enc',text,'fontutils-0.6/data', 2251 ).
file('testfont.tex',text,'fontutils-0.6/data', 9408 ).
file('texlatin.enc',text,'fontutils-0.6/data', 3053 ).
file('textext.enc',text,'fontutils-0.6/data', 943 ).
file('zdingbat.enc',text,'fontutils-0.6/data', 1491 ).
file('Makefile',text,'fontutils-0.6/doc', 2596 ).
file('Makefile.in',text,'fontutils-0.6/doc', 2520 ).
file('bpltobzr.texi',text,'fontutils-0.6/doc', 8642 ).
file('bugs.texi',text,'fontutils-0.6/doc', 9232 ).
file('bzredit.texi',text,'fontutils-0.6/doc', 7970 ).
file('bzrto.texi',text,'fontutils-0.6/doc', 36273 ).
file('charspace.texi',text,'fontutils-0.6/doc', 29901 ).
file('copying.texi',text,'fontutils-0.6/doc', 18346 ).
file('enhance.texi',text,'fontutils-0.6/doc', 17054 ).
file('filefmts.texi',text,'fontutils-0.6/doc', 16417 ).
file('fontcvt.texi',text,'fontutils-0.6/doc', 14699 ).
file('fontu.texi',text,'fontutils-0.6/doc', 13877 ).
file('freedom.texi',text,'fontutils-0.6/doc', 10549 ).
file('gsrenderf.texi',text,'fontutils-0.6/doc', 8739 ).
file('imageto.texi',text,'fontutils-0.6/doc', 19039 ).
file('imgrotate.texi',text,'fontutils-0.6/doc', 3994 ).
file('index.texi',text,'fontutils-0.6/doc', 216 ).
file('install.texi',text,'fontutils-0.6/doc', 16306 ).
file('intro.texi',text,'fontutils-0.6/doc', 1416 ).
file('limn.texi',text,'fontutils-0.6/doc', 39363 ).
file('overview.texi',text,'fontutils-0.6/doc', 30502 ).
file('texinfo.tex',text,'fontutils-0.6/doc', 119539 ).
file('update.el',text,'fontutils-0.6/doc', 241 ).
file('xbfe.texi',text,'fontutils-0.6/doc', 13068 ).
file('ChangeLog',text,'fontutils-0.6/fontconvert', 19771 ).
file('GNUmakefile',text,'fontutils-0.6/fontconvert', 976 ).
file('M.depend',text,'fontutils-0.6/fontconvert', 2938 ).
file('README',text,'fontutils-0.6/fontconvert', 854 ).
file('filter.c',text,'fontutils-0.6/fontconvert', 3641 ).
file('filter.h',text,'fontutils-0.6/fontconvert', 1003 ).
file('main.c',text,'fontutils-0.6/fontconvert', 23067 ).
file('main.h',text,'fontutils-0.6/fontconvert', 1082 ).
file('output-epsf.c',text,'fontutils-0.6/fontconvert', 4056 ).
file('output-epsf.h',text,'fontutils-0.6/fontconvert', 1008 ).
file('output-gf.c',text,'fontutils-0.6/fontconvert', 2538 ).
file('output-gf.h',text,'fontutils-0.6/fontconvert', 1155 ).
file('output-tfm.c',text,'fontutils-0.6/fontconvert', 8715 ).
file('output-tfm.h',text,'fontutils-0.6/fontconvert', 1478 ).
file('random.c',text,'fontutils-0.6/fontconvert', 5342 ).
file('random.h',text,'fontutils-0.6/fontconvert', 1373 ).
file('version.c',text,'fontutils-0.6/fontconvert', 50 ).
file('ChangeLog',text,'fontutils-0.6/gf', 9948 ).
file('GNUmakefile',text,'fontutils-0.6/gf', 895 ).
file('M.depend',text,'fontutils-0.6/gf', 1057 ).
file('README',text,'fontutils-0.6/gf', 600 ).
file('gf_input.c',text,'fontutils-0.6/gf', 27771 ).
file('gf_opcodes.h',text,'fontutils-0.6/gf', 1331 ).
file('gf_output.c',text,'fontutils-0.6/gf', 11777 ).
file('ChangeLog',text,'fontutils-0.6/gsrenderfont', 12700 ).
file('GNUmakefile',text,'fontutils-0.6/gsrenderfont', 1541 ).
file('M.depend',text,'fontutils-0.6/gsrenderfont', 957 ).
file('README',text,'fontutils-0.6/gsrenderfont', 853 ).
file('fixifibb.awk',text,'fontutils-0.6/gsrenderfont', 311 ).
file('gsrf.in',text,'fontutils-0.6/gsrenderfont', 8088 ).
file('input-pbm.c',text,'fontutils-0.6/gsrenderfont', 4789 ).
file('input-pbm.h',text,'fontutils-0.6/gsrenderfont', 1156 ).
file('main.c',text,'fontutils-0.6/gsrenderfont', 5566 ).
file('version.c',text,'fontutils-0.6/gsrenderfont', 46 ).
file('writefont.PS',text,'fontutils-0.6/gsrenderfont', 21627 ).
file('ChangeLog',text,'fontutils-0.6/imageto', 37623 ).
file('GNUmakefile',text,'fontutils-0.6/imageto', 1117 ).
file('M.depend',text,'fontutils-0.6/imageto', 4289 ).
file('README',text,'fontutils-0.6/imageto', 2254 ).
file('bitmap2.c',text,'fontutils-0.6/imageto', 2622 ).
file('bitmap2.h',text,'fontutils-0.6/imageto', 1162 ).
file('extract.c',text,'fontutils-0.6/imageto', 7705 ).
file('extract.h',text,'fontutils-0.6/imageto', 2174 ).
file('ifi.c',text,'fontutils-0.6/imageto', 5652 ).
file('ifi.h',text,'fontutils-0.6/imageto', 2628 ).
file('image-char.c',text,'fontutils-0.6/imageto', 2030 ).
file('image-char.h',text,'fontutils-0.6/imageto', 2632 ).
file('image-header.h',text,'fontutils-0.6/imageto', 1263 ).
file('input-img.c',text,'fontutils-0.6/imageto', 4494 ).
file('input-img.h',text,'fontutils-0.6/imageto', 1364 ).
file('input-pbm.c',text,'fontutils-0.6/imageto', 1884 ).
file('input-pbm.h',text,'fontutils-0.6/imageto', 1365 ).
file('main.c',text,'fontutils-0.6/imageto', 13074 ).
file('main.h',text,'fontutils-0.6/imageto', 1379 ).
file('out-chars.c',text,'fontutils-0.6/imageto', 21354 ).
file('out-chars.h',text,'fontutils-0.6/imageto', 1099 ).
file('out-epsf.c',text,'fontutils-0.6/imageto', 5751 ).
file('out-epsf.h',text,'fontutils-0.6/imageto', 988 ).
file('out-strips.c',text,'fontutils-0.6/imageto', 3261 ).
file('out-strips.h',text,'fontutils-0.6/imageto', 921 ).
file('strips.tex',text,'fontutils-0.6/imageto', 2039 ).
file('version.c',text,'fontutils-0.6/imageto', 46 ).
file('ChangeLog',text,'fontutils-0.6/imgrotate', 3897 ).
file('GNUmakefile',text,'fontutils-0.6/imgrotate', 942 ).
file('M.depend',text,'fontutils-0.6/imgrotate', 1174 ).
file('README',text,'fontutils-0.6/imgrotate', 119 ).
file('img-input.c',text,'fontutils-0.6/imgrotate', 2721 ).
file('img-input.h',text,'fontutils-0.6/imgrotate', 1194 ).
file('img-output.c',text,'fontutils-0.6/imgrotate', 2277 ).
file('img-output.h',text,'fontutils-0.6/imgrotate', 1173 ).
file('img.h',text,'fontutils-0.6/imgrotate', 1243 ).
file('main.c',text,'fontutils-0.6/imgrotate', 10046 ).
file('version.c',text,'fontutils-0.6/imgrotate', 48 ).
file('Bitmap.h',text,'fontutils-0.6/include', 4848 ).
file('ChangeLog',text,'fontutils-0.6/include', 42943 ).
file('Item.h',text,'fontutils-0.6/include', 2486 ).
file('bb-list.h',text,'fontutils-0.6/include', 1521 ).
file('bb-outline.h',text,'fontutils-0.6/include', 1372 ).
file('bitmap.h',text,'fontutils-0.6/include', 4441 ).
file('bounding-box.h',text,'fontutils-0.6/include', 2162 ).
file('bzr.h',text,'fontutils-0.6/include', 4557 ).
file('c-auto.h',text,'fontutils-0.6/include', 942 ).
file('c-auto.h.in',text,'fontutils-0.6/include', 866 ).
file('c-errno.h',text,'fontutils-0.6/include', 873 ).
file('c-fopen.h',text,'fontutils-0.6/include', 1528 ).
file('c-limits.h',text,'fontutils-0.6/include', 1000 ).
file('c-memstr.h',text,'fontutils-0.6/include', 1680 ).
file('c-minmax.h',text,'fontutils-0.6/include', 1423 ).
file('c-namemx.h',text,'fontutils-0.6/include', 1249 ).
file('c-pathch.h',text,'fontutils-0.6/include', 1770 ).
file('c-pathmx.h',text,'fontutils-0.6/include', 1063 ).
file('c-proto.h',text,'fontutils-0.6/include', 2173 ).
file('c-seek.h',text,'fontutils-0.6/include', 915 ).
file('c-std.h',text,'fontutils-0.6/include', 1914 ).
file('charspec.h',text,'fontutils-0.6/include', 1308 ).
file('cmdline.h',text,'fontutils-0.6/include', 4761 ).
file('config.h',text,'fontutils-0.6/include', 1679 ).
file('dirio.h',text,'fontutils-0.6/include', 1592 ).
file('edge.h',text,'fontutils-0.6/include', 2265 ).
file('encoding.h',text,'fontutils-0.6/include', 3084 ).
file('file-input.h',text,'fontutils-0.6/include', 1921 ).
file('file-output.h',text,'fontutils-0.6/include', 1535 ).
file('filename.h',text,'fontutils-0.6/include', 1752 ).
file('fix-num.h',text,'fontutils-0.6/include', 1209 ).
file('font.h',text,'fontutils-0.6/include', 8732 ).
file('getopt.h',text,'fontutils-0.6/include', 4333 ).
file('getpagesize.h',text,'fontutils-0.6/include', 416 ).
file('gf.h',text,'fontutils-0.6/include', 5379 ).
file('global.h',text,'fontutils-0.6/include', 7968 ).
file('hexify.h',text,'fontutils-0.6/include', 1118 ).
file('lib.h',text,'fontutils-0.6/include', 2860 ).
file('libfile.h',text,'fontutils-0.6/include', 2192 ).
file('line.h',text,'fontutils-0.6/include', 1043 ).
file('list.h',text,'fontutils-0.6/include', 1621 ).
file('logreport.h',text,'fontutils-0.6/include', 1748 ).
file('paths.h',text,'fontutils-0.6/include', 848 ).
file('paths.h.in',text,'fontutils-0.6/include', 723 ).
file('pathsrch.h',text,'fontutils-0.6/include', 2183 ).
file('pbm.h',text,'fontutils-0.6/include', 1430 ).
file('pbmplus.h',text,'fontutils-0.6/include', 6474 ).
file('pk.h',text,'fontutils-0.6/include', 3100 ).
file('rand.h',text,'fontutils-0.6/include', 1174 ).
file('report.h',text,'fontutils-0.6/include', 1677 ).
file('scaled-num.h',text,'fontutils-0.6/include', 1314 ).
file('spline.h',text,'fontutils-0.6/include', 4208 ).
file('statistics.h',text,'fontutils-0.6/include', 1426 ).
file('str-lcase.h',text,'fontutils-0.6/include', 1111 ).
file('tfm.h',text,'fontutils-0.6/include', 11295 ).
file('types.h',text,'fontutils-0.6/include', 2458 ).
file('varstring.h',text,'fontutils-0.6/include', 1969 ).
file('vector.h',text,'fontutils-0.6/include', 3829 ).
file('xmessage.h',text,'fontutils-0.6/include', 1221 ).
file('xstat.h',text,'fontutils-0.6/include', 2325 ).
file('xt-common.h',text,'fontutils-0.6/include', 4983 ).
file('ChangeLog',text,'fontutils-0.6/lib', 41210 ).
file('GNUmakefile',text,'fontutils-0.6/lib', 1622 ).
file('M.depend',text,'fontutils-0.6/lib', 20342 ).
file('README',text,'fontutils-0.6/lib', 2408 ).
file('atou.c',text,'fontutils-0.6/lib', 925 ).
file('basename.c',text,'fontutils-0.6/lib', 1036 ).
file('bb-list.c',text,'fontutils-0.6/lib', 1974 ).
file('bb-outline.c',text,'fontutils-0.6/lib', 4078 ).
file('bitmap.c',text,'fontutils-0.6/lib', 8168 ).
file('bounding-box.c',text,'fontutils-0.6/lib', 2185 ).
file('charcode.c',text,'fontutils-0.6/lib', 2667 ).
file('charspec.c',text,'fontutils-0.6/lib', 1737 ).
file('cmdline.c',text,'fontutils-0.6/lib', 1800 ).
file('concat.c',text,'fontutils-0.6/lib', 1039 ).
file('concat3.c',text,'fontutils-0.6/lib', 996 ).
file('concat4.c',text,'fontutils-0.6/lib', 1077 ).
file('concat5.c',text,'fontutils-0.6/lib', 1125 ).
file('dir-p.c',text,'fontutils-0.6/lib', 998 ).
file('dlsym.c',text,'fontutils-0.6/lib', 677 ).
file('edge.c',text,'fontutils-0.6/lib', 8339 ).
file('encoding.c',text,'fontutils-0.6/lib', 4745 ).
file('extend-fname.c',text,'fontutils-0.6/lib', 999 ).
file('file-input.c',text,'fontutils-0.6/lib', 3166 ).
file('file-output.c',text,'fontutils-0.6/lib', 2224 ).
file('file-p.c',text,'fontutils-0.6/lib', 1305 ).
file('filename.c',text,'fontutils-0.6/lib', 2546 ).
file('find-suffix.c',text,'fontutils-0.6/lib', 1033 ).
file('fix-num.c',text,'fontutils-0.6/lib', 1119 ).
file('float-ok.c',text,'fontutils-0.6/lib', 1437 ).
file('fmod.c',text,'fontutils-0.6/lib', 3596 ).
file('font.c',text,'fontutils-0.6/lib', 14129 ).
file('getopt.c',text,'fontutils-0.6/lib', 19292 ).
file('getopt1.c',text,'fontutils-0.6/lib', 3427 ).
file('hexify.c',text,'fontutils-0.6/lib', 1360 ).
file('integer-ok.c',text,'fontutils-0.6/lib', 1142 ).
file('libfile.c',text,'fontutils-0.6/lib', 3090 ).
file('line.c',text,'fontutils-0.6/lib', 1461 ).
file('list.c',text,'fontutils-0.6/lib', 1670 ).
file('logreport.c',text,'fontutils-0.6/lib', 1029 ).
file('make-prefix.c',text,'fontutils-0.6/lib', 1060 ).
file('make-suffix.c',text,'fontutils-0.6/lib', 1231 ).
file('malloc.c',text,'fontutils-0.6/lib', 34564 ).
file('math.c',text,'fontutils-0.6/lib', 4212 ).
file('now.c',text,'fontutils-0.6/lib', 1119 ).
file('numtoa.c',text,'fontutils-0.6/lib', 1226 ).
file('pathsrch.c',text,'fontutils-0.6/lib', 15451 ).
file('rand.c',text,'fontutils-0.6/lib', 2398 ).
file('remove-suffx.c',text,'fontutils-0.6/lib', 899 ).
file('report.c',text,'fontutils-0.6/lib', 1041 ).
file('safe-free.c',text,'fontutils-0.6/lib', 1069 ).
file('scaled-num.c',text,'fontutils-0.6/lib', 1672 ).
file('spline.c',text,'fontutils-0.6/lib', 5592 ).
file('statistics.c',text,'fontutils-0.6/lib', 2193 ).
file('str-lcase.c',text,'fontutils-0.6/lib', 1386 ).
file('str-to-bit.c',text,'fontutils-0.6/lib', 3729 ).
file('strstr.c',text,'fontutils-0.6/lib', 1727 ).
file('substring.c',text,'fontutils-0.6/lib', 1491 ).
file('varstring.c',text,'fontutils-0.6/lib', 2516 ).
file('vector.c',text,'fontutils-0.6/lib', 4597 ).
file('xcalloc.c',text,'fontutils-0.6/lib', 1050 ).
file('xchdir.c',text,'fontutils-0.6/lib', 873 ).
file('xfopen.c',text,'fontutils-0.6/lib', 1139 ).
file('xfseek.c',text,'fontutils-0.6/lib', 904 ).
file('xftell.c',text,'fontutils-0.6/lib', 903 ).
file('xgetcwd.c',text,'fontutils-0.6/lib', 3106 ).
file('xmalloc.c',text,'fontutils-0.6/lib', 995 ).
file('xmessage.c',text,'fontutils-0.6/lib', 3106 ).
file('xopendir.c',text,'fontutils-0.6/lib', 1091 ).
file('xrealloc.c',text,'fontutils-0.6/lib', 1181 ).
file('xrename.c',text,'fontutils-0.6/lib', 1335 ).
file('xstat.c',text,'fontutils-0.6/lib', 1148 ).
file('xstrdup.c',text,'fontutils-0.6/lib', 933 ).
file('ChangeLog',text,'fontutils-0.6/limn', 59278 ).
file('GNUmakefile',text,'fontutils-0.6/limn', 1231 ).
file('Limn.ad',text,'fontutils-0.6/limn', 168 ).
file('M.depend',text,'fontutils-0.6/limn', 3679 ).
file('README',text,'fontutils-0.6/limn', 1709 ).
file('curve.c',text,'fontutils-0.6/limn', 7705 ).
file('curve.h',text,'fontutils-0.6/limn', 5313 ).
file('display.c',text,'fontutils-0.6/limn', 22376 ).
file('display.h',text,'fontutils-0.6/limn', 1740 ).
file('fit.c',text,'fontutils-0.6/limn', 67781 ).
file('fit.h',text,'fontutils-0.6/limn', 1706 ).
file('main.c',text,'fontutils-0.6/limn', 12411 ).
file('main.h',text,'fontutils-0.6/limn', 1003 ).
file('output-bzr.c',text,'fontutils-0.6/limn', 3826 ).
file('output-bzr.h',text,'fontutils-0.6/limn', 1093 ).
file('pxl-outline.c',text,'fontutils-0.6/limn', 6597 ).
file('pxl-outline.h',text,'fontutils-0.6/limn', 2222 ).
file('version.c',text,'fontutils-0.6/limn', 43 ).
file('xserver.c',text,'fontutils-0.6/limn', 5761 ).
file('xserver.h',text,'fontutils-0.6/limn', 1647 ).
file('ChangeLog',text,'fontutils-0.6/pbm', 786 ).
file('GNUmakefile',text,'fontutils-0.6/pbm', 920 ).
file('M.depend',text,'fontutils-0.6/pbm', 1059 ).
file('README',text,'fontutils-0.6/pbm', 96 ).
file('libpbm.h',text,'fontutils-0.6/pbm', 439 ).
file('libpbm1.c',text,'fontutils-0.6/pbm', 8105 ).
file('libpbm2.c',text,'fontutils-0.6/pbm', 2800 ).
file('libpbm4.c',text,'fontutils-0.6/pbm', 1555 ).
file('pbmascii.c',text,'fontutils-0.6/pbm', 665 ).
file('ChangeLog',text,'fontutils-0.6/pk', 4098 ).
file('GNUmakefile',text,'fontutils-0.6/pk', 865 ).
file('M.depend',text,'fontutils-0.6/pk', 501 ).
file('README',text,'fontutils-0.6/pk', 624 ).
file('pk_input.c',text,'fontutils-0.6/pk', 22317 ).
file('pk_opcodes.h',text,'fontutils-0.6/pk', 1322 ).
file('ChangeLog',text,'fontutils-0.6/tfm', 12078 ).
file('GNUmakefile',text,'fontutils-0.6/tfm', 936 ).
file('M.depend',text,'fontutils-0.6/tfm', 2673 ).
file('README',text,'fontutils-0.6/tfm', 833 ).
file('fontdimen.c',text,'fontutils-0.6/tfm', 6439 ).
file('tfm_char.c',text,'fontutils-0.6/tfm', 1431 ).
file('tfm_fontdim.c',text,'fontutils-0.6/tfm', 6328 ).
file('tfm_header.c',text,'fontutils-0.6/tfm', 2862 ).
file('tfm_input.c',text,'fontutils-0.6/tfm', 14466 ).
file('tfm_kern.c',text,'fontutils-0.6/tfm', 1945 ).
file('tfm_ligature.c',text,'fontutils-0.6/tfm', 1488 ).
file('tfm_output.c',text,'fontutils-0.6/tfm', 9473 ).
file('Bitmap.c',text,'fontutils-0.6/widgets', 36161 ).
file('BitmapP.h',text,'fontutils-0.6/widgets', 2478 ).
file('Box.c',text,'fontutils-0.6/widgets', 18676 ).
file('ChangeLog',text,'fontutils-0.6/widgets', 11537 ).
file('Fontview.c',text,'fontutils-0.6/widgets', 15986 ).
file('Fontview.h',text,'fontutils-0.6/widgets', 2567 ).
file('FontviewP.h',text,'fontutils-0.6/widgets', 2321 ).
file('Form.c',text,'fontutils-0.6/widgets', 20832 ).
file('GNUmakefile',text,'fontutils-0.6/widgets', 955 ).
file('Item.c',text,'fontutils-0.6/widgets', 12676 ).
file('ItemP.h',text,'fontutils-0.6/widgets', 1793 ).
file('Label.c',text,'fontutils-0.6/widgets', 15103 ).
file('M.depend',text,'fontutils-0.6/widgets', 1115 ).
file('README',text,'fontutils-0.6/widgets', 362 ).
file('Viewport.c',text,'fontutils-0.6/widgets', 31218 ).
file('ChangeLog',text,'fontutils-0.6/xbfe', 2795 ).
file('GNUmakefile',text,'fontutils-0.6/xbfe', 1056 ).
file('M.depend',text,'fontutils-0.6/xbfe', 2263 ).
file('README',text,'fontutils-0.6/xbfe', 2870 ).
file('XBfe.ad',text,'fontutils-0.6/xbfe', 4911 ).
file('char.c',text,'fontutils-0.6/xbfe', 5283 ).
file('char.h',text,'fontutils-0.6/xbfe', 2133 ).
file('commands.c',text,'fontutils-0.6/xbfe', 15128 ).
file('commands.h',text,'fontutils-0.6/xbfe', 1838 ).
file('kern.c',text,'fontutils-0.6/xbfe', 9262 ).
file('kern.h',text,'fontutils-0.6/xbfe', 1669 ).
file('main.c',text,'fontutils-0.6/xbfe', 18320 ).
file('main.h',text,'fontutils-0.6/xbfe', 1926 ).
file('version.c',text,'fontutils-0.6/xbfe', 43 ).
file('Makefile',text,'gas-2.1.1', 30349 ).
file('Makefile.in',text,'gas-2.1.1', 29187 ).
file('README',text,'gas-2.1.1', 1190 ).
file('bfd',dir,'gas-2.1.1', 4096 ).
file('config',dir,'gas-2.1.1', 512 ).
file('config.guess',exec,'gas-2.1.1', 4427 ).
file('config.status',exec,'gas-2.1.1', 200 ).
file('config.sub',exec,'gas-2.1.1', 16490 ).
file('configure',exec,'gas-2.1.1', 38075 ).
file('configure.in',text,'gas-2.1.1', 7178 ).
file('etc',dir,'gas-2.1.1', 512 ).
file('gas',dir,'gas-2.1.1', 1536 ).
file('include',dir,'gas-2.1.1', 512 ).
file('libiberty',dir,'gas-2.1.1', 2048 ).
file('move-if-change',exec,'gas-2.1.1', 129 ).
file('opcodes',dir,'gas-2.1.1', 1024 ).
file('texinfo',dir,'gas-2.1.1', 512 ).
file('COPYING',text,'gas-2.1.1/bfd', 17982 ).
file('ChangeLog',text,'gas-2.1.1/bfd', 161851 ).
file('Makefile',text,'gas-2.1.1/bfd', 16354 ).
file('Makefile.in',text,'gas-2.1.1/bfd', 15749 ).
file('PORTING',text,'gas-2.1.1/bfd', 2154 ).
file('README.hppaelf',text,'gas-2.1.1/bfd', 774 ).
file('TODO',text,'gas-2.1.1/bfd', 1363 ).
file('VERSION',text,'gas-2.1.1/bfd', 4 ).
file('aix386-core.c',text,'gas-2.1.1/bfd', 11107 ).
file('aout-adobe.c',text,'gas-2.1.1/bfd', 16201 ).
file('aout-encap.c',text,'gas-2.1.1/bfd', 6873 ).
file('aout-target.h',text,'gas-2.1.1/bfd', 12699 ).
file('aout32.c',text,'gas-2.1.1/bfd', 898 ).
file('aout64.c',text,'gas-2.1.1/bfd', 1039 ).
file('aoutf1.h',text,'gas-2.1.1/bfd', 18958 ).
file('aoutx.h',text,'gas-2.1.1/bfd', 76547 ).
file('archive.c',text,'gas-2.1.1/bfd', 47914 ).
file('archures.c',text,'gas-2.1.1/bfd', 16226 ).
file('bfd-in.h',text,'gas-2.1.1/bfd', 12680 ).
file('bfd.c',text,'gas-2.1.1/bfd', 16881 ).
file('bout.c',text,'gas-2.1.1/bfd', 39428 ).
file('cache.c',text,'gas-2.1.1/bfd', 7028 ).
file('coff-a29k.c',text,'gas-2.1.1/bfd', 10440 ).
file('coff-alpha.c',text,'gas-2.1.1/bfd', 122074 ).
file('coff-h8300.c',text,'gas-2.1.1/bfd', 5842 ).
file('coff-h8500.c',text,'gas-2.1.1/bfd', 8551 ).
file('coff-i386.c',text,'gas-2.1.1/bfd', 13021 ).
file('coff-i960.c',text,'gas-2.1.1/bfd', 7243 ).
file('coff-m68k.c',text,'gas-2.1.1/bfd', 4370 ).
file('coff-m88k.c',text,'gas-2.1.1/bfd', 4549 ).
file('coff-mips.c',text,'gas-2.1.1/bfd', 129340 ).
file('coff-msym.c',text,'gas-2.1.1/bfd', 29715 ).
file('coff-rs6000.c',text,'gas-2.1.1/bfd', 9115 ).
file('coff-sh.c',text,'gas-2.1.1/bfd', 5049 ).
file('coff-we32k.c',text,'gas-2.1.1/bfd', 3427 ).
file('coff-z8k.c',text,'gas-2.1.1/bfd', 6850 ).
file('coffcode.h',text,'gas-2.1.1/bfd', 63941 ).
file('coffgen.c',text,'gas-2.1.1/bfd', 41667 ).
file('coffswap.h',text,'gas-2.1.1/bfd', 21957 ).
file('config',dir,'gas-2.1.1/bfd', 1536 ).
file('config.status',exec,'gas-2.1.1/bfd', 258 ).
file('configure.bat',text,'gas-2.1.1/bfd', 338 ).
file('configure.host',text,'gas-2.1.1/bfd', 2480 ).
file('configure.in',text,'gas-2.1.1/bfd', 4171 ).
file('core.c',text,'gas-2.1.1/bfd', 2518 ).
file('cpu-a29k.c',text,'gas-2.1.1/bfd', 1330 ).
file('cpu-alpha.c',text,'gas-2.1.1/bfd', 1280 ).
file('cpu-h8300.c',text,'gas-2.1.1/bfd', 10353 ).
file('cpu-h8500.c',text,'gas-2.1.1/bfd', 5279 ).
file('cpu-hppa.c',text,'gas-2.1.1/bfd', 1303 ).
file('cpu-i386.c',text,'gas-2.1.1/bfd', 1280 ).
file('cpu-i960.c',text,'gas-2.1.1/bfd', 4512 ).
file('cpu-m68k.c',text,'gas-2.1.1/bfd', 1587 ).
file('cpu-m88k.c',text,'gas-2.1.1/bfd', 1357 ).
file('cpu-mips.c',text,'gas-2.1.1/bfd', 1680 ).
file('cpu-rs6000.c',text,'gas-2.1.1/bfd', 1370 ).
file('cpu-sh.c',text,'gas-2.1.1/bfd', 1940 ).
file('cpu-sparc.c',text,'gas-2.1.1/bfd', 1280 ).
file('cpu-vax.c',text,'gas-2.1.1/bfd', 1321 ).
file('cpu-we32k.c',text,'gas-2.1.1/bfd', 1361 ).
file('cpu-z8k.c',text,'gas-2.1.1/bfd', 5214 ).
file('ctor.c',text,'gas-2.1.1/bfd', 5275 ).
file('demo64.c',text,'gas-2.1.1/bfd', 1171 ).
file('doc',dir,'gas-2.1.1/bfd', 512 ).
file('elf32-hppa.c',text,'gas-2.1.1/bfd', 46914 ).
file('elf32-hppa.h',text,'gas-2.1.1/bfd', 15374 ).
file('elf32-i386.c',text,'gas-2.1.1/bfd', 1847 ).
file('elf32-i860.c',text,'gas-2.1.1/bfd', 1147 ).
file('elf32-m68k.c',text,'gas-2.1.1/bfd', 1148 ).
file('elf32-sparc.c',text,'gas-2.1.1/bfd', 6233 ).
file('elf32-target.h',text,'gas-2.1.1/bfd', 7592 ).
file('elf32.c',text,'gas-2.1.1/bfd', 82154 ).
file('filemode.c',text,'gas-2.1.1/bfd', 4489 ).
file('format.c',text,'gas-2.1.1/bfd', 7016 ).
file('gen-aout.c',text,'gas-2.1.1/bfd', 2797 ).
file('host-aout.c',text,'gas-2.1.1/bfd', 2292 ).
file('hosts',dir,'gas-2.1.1/bfd', 1024 ).
file('hp300bsd.c',text,'gas-2.1.1/bfd', 1315 ).
file('hp300hpux.c',text,'gas-2.1.1/bfd', 26468 ).
file('hppa.c',text,'gas-2.1.1/bfd', 26939 ).
file('i386aout.c',text,'gas-2.1.1/bfd', 2075 ).
file('i386bsd.c',text,'gas-2.1.1/bfd', 1475 ).
file('i386linux.c',text,'gas-2.1.1/bfd', 1538 ).
file('ieee.c',text,'gas-2.1.1/bfd', 73353 ).
file('init.c',text,'gas-2.1.1/bfd', 1780 ).
file('libaout.h',text,'gas-2.1.1/bfd', 12699 ).
file('libbfd-in.h',text,'gas-2.1.1/bfd', 8045 ).
file('libbfd.c',text,'gas-2.1.1/bfd', 19801 ).
file('libbfd.h',text,'gas-2.1.1/bfd', 9754 ).
file('libcoff-in.h',text,'gas-2.1.1/bfd', 4546 ).
file('libcoff.h',text,'gas-2.1.1/bfd', 11906 ).
file('libecoff.h',text,'gas-2.1.1/bfd', 3391 ).
file('libelf.h',text,'gas-2.1.1/bfd', 3252 ).
file('libhppa.h',text,'gas-2.1.1/bfd', 5443 ).
file('libieee.h',text,'gas-2.1.1/bfd', 3422 ).
file('liboasys.h',text,'gas-2.1.1/bfd', 2225 ).
file('mipsbsd.c',text,'gas-2.1.1/bfd', 12404 ).
file('newsos3.c',text,'gas-2.1.1/bfd', 1448 ).
file('oasys.c',text,'gas-2.1.1/bfd', 36044 ).
file('opncls.c',text,'gas-2.1.1/bfd', 12356 ).
file('reloc.c',text,'gas-2.1.1/bfd', 29351 ).
file('reloc16.c',text,'gas-2.1.1/bfd', 9846 ).
file('rs6000-core.c',text,'gas-2.1.1/bfd', 12103 ).
file('sco-core.c',text,'gas-2.1.1/bfd', 10516 ).
file('seclet.c',text,'gas-2.1.1/bfd', 4935 ).
file('seclet.h',text,'gas-2.1.1/bfd', 1376 ).
file('section.c',text,'gas-2.1.1/bfd', 23596 ).
file('srec.c',text,'gas-2.1.1/bfd', 23127 ).
file('stab-syms.c',text,'gas-2.1.1/bfd', 2168 ).
file('sunos.c',text,'gas-2.1.1/bfd', 959 ).
file('syms.c',text,'gas-2.1.1/bfd', 13266 ).
file('targets.c',text,'gas-2.1.1/bfd', 17477 ).
file('tekhex.c',text,'gas-2.1.1/bfd', 25167 ).
file('trad-core.c',text,'gas-2.1.1/bfd', 12139 ).
file('a29k-aout.mt',text,'gas-2.1.1/bfd/config', 122 ).
file('a29k-coff.mt',text,'gas-2.1.1/bfd/config', 124 ).
file('adobe.mt',text,'gas-2.1.1/bfd/config', 90 ).
file('alphaosf.mh',text,'gas-2.1.1/bfd/config', 31 ).
file('alphaosf.mt',text,'gas-2.1.1/bfd/config', 148 ).
file('amix.mh',text,'gas-2.1.1/bfd/config', 114 ).
file('apollov68.mh',text,'gas-2.1.1/bfd/config', 307 ).
file('bigmips.mt',text,'gas-2.1.1/bfd/config', 78 ).
file('decstation.mh',text,'gas-2.1.1/bfd/config', 48 ).
file('decstation.mt',text,'gas-2.1.1/bfd/config', 82 ).
file('delta88.mh',text,'gas-2.1.1/bfd/config', 43 ).
file('dgux.mh',text,'gas-2.1.1/bfd/config', 46 ).
file('go32.mh',text,'gas-2.1.1/bfd/config', 42 ).
file('h8300-coff.mt',text,'gas-2.1.1/bfd/config', 248 ).
file('h8500-coff.mt',text,'gas-2.1.1/bfd/config', 206 ).
file('harris.mh',text,'gas-2.1.1/bfd/config', 53 ).
file('hp300bsd.mh',text,'gas-2.1.1/bfd/config', 43 ).
file('hp300bsd.mt',text,'gas-2.1.1/bfd/config', 99 ).
file('hp300hpux.mt',text,'gas-2.1.1/bfd/config', 43 ).
file('hppa-elf.mt',text,'gas-2.1.1/bfd/config', 44 ).
file('hppabsd.mh',text,'gas-2.1.1/bfd/config', 25 ).
file('hppabsd.mt',text,'gas-2.1.1/bfd/config', 38 ).
file('hppahpux.mh',text,'gas-2.1.1/bfd/config', 26 ).
file('hppahpux.mt',text,'gas-2.1.1/bfd/config', 38 ).
file('hppaosf.mh',text,'gas-2.1.1/bfd/config', 150 ).
file('i386-aout.mt',text,'gas-2.1.1/bfd/config', 77 ).
file('i386-bsd.mt',text,'gas-2.1.1/bfd/config', 87 ).
file('i386-coff.mt',text,'gas-2.1.1/bfd/config', 77 ).
file('i386-elf.mt',text,'gas-2.1.1/bfd/config', 77 ).
file('i386-linux.mt',text,'gas-2.1.1/bfd/config', 90 ).
file('i386-sco.mt',text,'gas-2.1.1/bfd/config', 187 ).
file('i386aix.mh',text,'gas-2.1.1/bfd/config', 150 ).
file('i386bsd.mh',text,'gas-2.1.1/bfd/config', 43 ).
file('i386isc.mh',text,'gas-2.1.1/bfd/config', 37 ).
file('i386linux.mh',text,'gas-2.1.1/bfd/config', 61 ).
file('i386v.mh',text,'gas-2.1.1/bfd/config', 37 ).
file('i386v4.mh',text,'gas-2.1.1/bfd/config', 100 ).
file('i860-elf.mt',text,'gas-2.1.1/bfd/config', 76 ).
file('i960-bout.mt',text,'gas-2.1.1/bfd/config', 180 ).
file('i960-coff.mt',text,'gas-2.1.1/bfd/config', 195 ).
file('irix3.mh',text,'gas-2.1.1/bfd/config', 47 ).
file('irix4.mh',text,'gas-2.1.1/bfd/config', 62 ).
file('m68k-aout.mt',text,'gas-2.1.1/bfd/config', 80 ).
file('m68k-coff.mt',text,'gas-2.1.1/bfd/config', 79 ).
file('m68k-elf.mt',text,'gas-2.1.1/bfd/config', 81 ).
file('m88k-aout.mt',text,'gas-2.1.1/bfd/config', 63 ).
file('m88k-coff.mt',text,'gas-2.1.1/bfd/config', 142 ).
file('mipsbsd.mh',text,'gas-2.1.1/bfd/config', 43 ).
file('mipsdecbsd.mt',text,'gas-2.1.1/bfd/config', 104 ).
file('ncr3000.mh',text,'gas-2.1.1/bfd/config', 642 ).
file('news.mt',text,'gas-2.1.1/bfd/config', 61 ).
file('rs6000.mh',text,'gas-2.1.1/bfd/config', 328 ).
file('rs6000.mt',text,'gas-2.1.1/bfd/config', 79 ).
file('rtbsd.mh',text,'gas-2.1.1/bfd/config', 251 ).
file('sh-coff.mt',text,'gas-2.1.1/bfd/config', 225 ).
file('solaris2.mh',text,'gas-2.1.1/bfd/config', 72 ).
file('sparc-aout.mt',text,'gas-2.1.1/bfd/config', 73 ).
file('sparc-elf.mt',text,'gas-2.1.1/bfd/config', 84 ).
file('sparc-ll.mh',text,'gas-2.1.1/bfd/config', 70 ).
file('st2000.mt',text,'gas-2.1.1/bfd/config', 250 ).
file('stratus.mh',text,'gas-2.1.1/bfd/config', 89 ).
file('sysv4.mh',text,'gas-2.1.1/bfd/config', 12 ).
file('tahoe.mh',text,'gas-2.1.1/bfd/config', 63 ).
file('tahoe.mt',text,'gas-2.1.1/bfd/config', 125 ).
file('ultra3.mh',text,'gas-2.1.1/bfd/config', 53 ).
file('vax.mt',text,'gas-2.1.1/bfd/config', 118 ).
file('vaxbsd.mh',text,'gas-2.1.1/bfd/config', 63 ).
file('vaxult.mh',text,'gas-2.1.1/bfd/config', 63 ).
file('vaxult2.mh',text,'gas-2.1.1/bfd/config', 62 ).
file('we32k.mt',text,'gas-2.1.1/bfd/config', 77 ).
file('z8k-coff.mt',text,'gas-2.1.1/bfd/config', 228 ).
file('ChangeLog',text,'gas-2.1.1/bfd/doc', 4187 ).
file('Makefile',text,'gas-2.1.1/bfd/doc', 9977 ).
file('Makefile.in',text,'gas-2.1.1/bfd/doc', 9619 ).
file('bfd.texinfo',text,'gas-2.1.1/bfd/doc', 18071 ).
file('chew.c',text,'gas-2.1.1/bfd/doc', 22655 ).
file('config.status',exec,'gas-2.1.1/bfd/doc', 209 ).
file('configure.in',text,'gas-2.1.1/bfd/doc', 302 ).
file('doc.str',text,'gas-2.1.1/bfd/doc', 1503 ).
file('proto.str',text,'gas-2.1.1/bfd/doc', 545 ).
file('alphaosf.h',text,'gas-2.1.1/bfd/hosts', 1087 ).
file('amix.h',text,'gas-2.1.1/bfd/hosts', 1585 ).
file('apollo68.h',text,'gas-2.1.1/bfd/hosts', 933 ).
file('apollov68.h',text,'gas-2.1.1/bfd/hosts', 1233 ).
file('decstation.h',text,'gas-2.1.1/bfd/hosts', 1219 ).
file('delta88.h',text,'gas-2.1.1/bfd/hosts', 2059 ).
file('dgux.h',text,'gas-2.1.1/bfd/hosts', 577 ).
file('dose.h',text,'gas-2.1.1/bfd/hosts', 711 ).
file('go32.h',text,'gas-2.1.1/bfd/hosts', 763 ).
file('harris.h',text,'gas-2.1.1/bfd/hosts', 935 ).
file('hp300.h',text,'gas-2.1.1/bfd/hosts', 1095 ).
file('hp300bsd.h',text,'gas-2.1.1/bfd/hosts', 1569 ).
file('hppabsd.h',text,'gas-2.1.1/bfd/hosts', 1197 ).
file('hppahpux.h',text,'gas-2.1.1/bfd/hosts', 1020 ).
file('i386aix.h',text,'gas-2.1.1/bfd/hosts', 1493 ).
file('i386bsd.h',text,'gas-2.1.1/bfd/hosts', 1261 ).
file('i386isc.h',text,'gas-2.1.1/bfd/hosts', 1354 ).
file('i386linux.h',text,'gas-2.1.1/bfd/hosts', 92 ).
file('i386mach.h',text,'gas-2.1.1/bfd/hosts', 1137 ).
file('i386v.h',text,'gas-2.1.1/bfd/hosts', 1192 ).
file('i386v4.h',text,'gas-2.1.1/bfd/hosts', 1572 ).
file('irix3.h',text,'gas-2.1.1/bfd/hosts', 825 ).
file('irix4.h',text,'gas-2.1.1/bfd/hosts', 845 ).
file('miniframe.h',text,'gas-2.1.1/bfd/hosts', 279 ).
file('mipsbsd.h',text,'gas-2.1.1/bfd/hosts', 1139 ).
file('ncr3000.h',text,'gas-2.1.1/bfd/hosts', 1572 ).
file('rs6000.h',text,'gas-2.1.1/bfd/hosts', 753 ).
file('rtbsd.h',text,'gas-2.1.1/bfd/hosts', 780 ).
file('solaris2.h',text,'gas-2.1.1/bfd/hosts', 119 ).
file('sparc-ll.h',text,'gas-2.1.1/bfd/hosts', 3019 ).
file('sparc.h',text,'gas-2.1.1/bfd/hosts', 246 ).
file('std-host.h',text,'gas-2.1.1/bfd/hosts', 3758 ).
file('stratus.h',text,'gas-2.1.1/bfd/hosts', 1576 ).
file('sun3.h',text,'gas-2.1.1/bfd/hosts', 2020 ).
file('sysv4.h',text,'gas-2.1.1/bfd/hosts', 1598 ).
file('tahoe.h',text,'gas-2.1.1/bfd/hosts', 998 ).
file('ultra3.h',text,'gas-2.1.1/bfd/hosts', 775 ).
file('vaxbsd.h',text,'gas-2.1.1/bfd/hosts', 1211 ).
file('vaxult.h',text,'gas-2.1.1/bfd/hosts', 966 ).
file('vaxult2.h',text,'gas-2.1.1/bfd/hosts', 1265 ).
file('we32k.h',text,'gas-2.1.1/bfd/hosts', 1054 ).
file('ChangeLog',text,'gas-2.1.1/config', 1369 ).
file('mh-a68bsd',text,'gas-2.1.1/config', 90 ).
file('mh-aix',text,'gas-2.1.1/config', 185 ).
file('mh-aix386',text,'gas-2.1.1/config', 25 ).
file('mh-apollo68',text,'gas-2.1.1/config', 91 ).
file('mh-decstation',text,'gas-2.1.1/config', 198 ).
file('mh-delta88',text,'gas-2.1.1/config', 30 ).
file('mh-dgux',text,'gas-2.1.1/config', 76 ).
file('mh-hpux',text,'gas-2.1.1/config', 193 ).
file('mh-irix4',text,'gas-2.1.1/config', 286 ).
file('mh-linux',text,'gas-2.1.1/config', 118 ).
file('mh-ncr3000',text,'gas-2.1.1/config', 832 ).
file('mh-sco',text,'gas-2.1.1/config', 215 ).
file('mh-solaris',text,'gas-2.1.1/config', 485 ).
file('mh-sun',text,'gas-2.1.1/config', 116 ).
file('mh-sun3',text,'gas-2.1.1/config', 268 ).
file('mh-sysv',text,'gas-2.1.1/config', 111 ).
file('mh-sysv4',text,'gas-2.1.1/config', 218 ).
file('mh-vaxult2',text,'gas-2.1.1/config', 73 ).
file('Makefile',text,'gas-2.1.1/etc', 2567 ).
file('Makefile.in',text,'gas-2.1.1/etc', 2189 ).
file('cfg-paper.texi',text,'gas-2.1.1/etc', 29217 ).
file('config.status',exec,'gas-2.1.1/etc', 206 ).
file('configure.in',text,'gas-2.1.1/etc', 378 ).
file('configure.man',text,'gas-2.1.1/etc', 3188 ).
file('configure.texi',text,'gas-2.1.1/etc', 73535 ).
file('CONTRIBUTORS',text,'gas-2.1.1/gas', 3973 ).
file('COPYING',text,'gas-2.1.1/gas', 17982 ).
file('ChangeLog',text,'gas-2.1.1/gas', 80804 ).
file('Makefile.in',text,'gas-2.1.1/gas', 14659 ).
file('NEWS',text,'gas-2.1.1/gas', 2495 ).
file('NOTES',text,'gas-2.1.1/gas', 3146 ).
file('NOTES.config',text,'gas-2.1.1/gas', 1989 ).
file('README',text,'gas-2.1.1/gas', 16239 ).
file('README-quirks',text,'gas-2.1.1/gas', 314 ).
file('README-vms',text,'gas-2.1.1/gas', 12824 ).
file('README.coff',text,'gas-2.1.1/gas', 3124 ).
file('README.rich',text,'gas-2.1.1/gas', 5626 ).
file('app.c',text,'gas-2.1.1/gas', 15654 ).
file('as.c',text,'gas-2.1.1/gas', 12107 ).
file('as.h',text,'gas-2.1.1/gas', 13856 ).
file('atof-generic.c',text,'gas-2.1.1/gas', 16983 ).
file('bignum-copy.c',text,'gas-2.1.1/gas', 2330 ).
file('bignum.h',text,'gas-2.1.1/gas', 2001 ).
file('bit_fix.h',text,'gas-2.1.1/gas', 1970 ).
file('cond.c',text,'gas-2.1.1/gas', 6746 ).
file('config',dir,'gas-2.1.1/gas', 2560 ).
file('config-gas.com',text,'gas-2.1.1/gas', 1868 ).
file('configure.bat',text,'gas-2.1.1/gas', 678 ).
file('configure.in',text,'gas-2.1.1/gas', 6810 ).
file('debug.c',text,'gas-2.1.1/gas', 2637 ).
file('doc',dir,'gas-2.1.1/gas', 512 ).
file('expr.c',text,'gas-2.1.1/gas', 31991 ).
file('expr.h',text,'gas-2.1.1/gas', 2628 ).
file('flonum-copy.c',text,'gas-2.1.1/gas', 2272 ).
file('flonum-konst.c',text,'gas-2.1.1/gas', 6976 ).
file('flonum-mult.c',text,'gas-2.1.1/gas', 5086 ).
file('flonum.h',text,'gas-2.1.1/gas', 4210 ).
file('frags.c',text,'gas-2.1.1/gas', 7844 ).
file('frags.h',text,'gas-2.1.1/gas', 1914 ).
file('hash.c',text,'gas-2.1.1/gas', 28993 ).
file('hash.h',text,'gas-2.1.1/gas', 2050 ).
file('hex-value.c',text,'gas-2.1.1/gas', 2306 ).
file('input-file.c',text,'gas-2.1.1/gas', 5519 ).
file('input-file.h',text,'gas-2.1.1/gas', 2504 ).
file('input-scrub.c',text,'gas-2.1.1/gas', 11915 ).
file('link.cmd',text,'gas-2.1.1/gas', 325 ).
file('listing.c',text,'gas-2.1.1/gas', 20916 ).
file('listing.h',text,'gas-2.1.1/gas', 1859 ).
file('make-gas.com',text,'gas-2.1.1/gas', 3760 ).
file('messages.c',text,'gas-2.1.1/gas', 9208 ).
file('obj.h',text,'gas-2.1.1/gas', 1690 ).
file('output-file.c',text,'gas-2.1.1/gas', 3323 ).
file('output-file.h',text,'gas-2.1.1/gas', 1014 ).
file('read.c',text,'gas-2.1.1/gas', 65559 ).
file('read.h',text,'gas-2.1.1/gas', 3280 ).
file('struc-symbol.h',text,'gas-2.1.1/gas', 4003 ).
file('subsegs.c',text,'gas-2.1.1/gas', 10685 ).
file('subsegs.h',text,'gas-2.1.1/gas', 3287 ).
file('symbols.c',text,'gas-2.1.1/gas', 25453 ).
file('symbols.h',text,'gas-2.1.1/gas', 3013 ).
file('tc.h',text,'gas-2.1.1/gas', 3342 ).
file('testsuite',dir,'gas-2.1.1/gas', 512 ).
file('write.c',text,'gas-2.1.1/gas', 47387 ).
file('write.h',text,'gas-2.1.1/gas', 3629 ).
file('xmalloc.c',text,'gas-2.1.1/gas', 1706 ).
file('aout_gnu.h',text,'gas-2.1.1/gas/config', 13146 ).
file('atof-ieee.c',text,'gas-2.1.1/gas/config', 15289 ).
file('atof-ns32k.c',text,'gas-2.1.1/gas/config', 12823 ).
file('atof-tahoe.c',text,'gas-2.1.1/gas/config', 11425 ).
file('atof-vax.c',text,'gas-2.1.1/gas/config', 13533 ).
file('coff_gnu.h',text,'gas-2.1.1/gas/config', 26629 ).
file('cplus-dem.c',text,'gas-2.1.1/gas/config', 17659 ).
file('ebmon29k.mt',text,'gas-2.1.1/gas/config', 144 ).
file('h8300.mt',text,'gas-2.1.1/gas/config', 133 ).
file('h8300hds.mt',text,'gas-2.1.1/gas/config', 140 ).
file('h8500.mt',text,'gas-2.1.1/gas/config', 130 ).
file('ho-ansi.h',text,'gas-2.1.1/gas/config', 1022 ).
file('ho-decstatn.h',text,'gas-2.1.1/gas/config', 1017 ).
file('ho-generic.h',text,'gas-2.1.1/gas/config', 1202 ).
file('ho-go32.h',text,'gas-2.1.1/gas/config', 875 ).
file('ho-hpux.h',text,'gas-2.1.1/gas/config', 1129 ).
file('ho-i386.h',text,'gas-2.1.1/gas/config', 912 ).
file('ho-i386aix.h',text,'gas-2.1.1/gas/config', 907 ).
file('ho-irix.h',text,'gas-2.1.1/gas/config', 991 ).
file('ho-mipsbsd.h',text,'gas-2.1.1/gas/config', 951 ).
file('ho-rs6000.h',text,'gas-2.1.1/gas/config', 1471 ).
file('ho-sun3.h',text,'gas-2.1.1/gas/config', 46 ).
file('ho-sun386.h',text,'gas-2.1.1/gas/config', 72 ).
file('ho-sun4.h',text,'gas-2.1.1/gas/config', 46 ).
file('ho-sunos.h',text,'gas-2.1.1/gas/config', 2005 ).
file('ho-sysv.h',text,'gas-2.1.1/gas/config', 901 ).
file('ho-vax.h',text,'gas-2.1.1/gas/config', 951 ).
file('ho-vms.h',text,'gas-2.1.1/gas/config', 1150 ).
file('i386coff.mt',text,'gas-2.1.1/gas/config', 143 ).
file('m68k.mt',text,'gas-2.1.1/gas/config', 55 ).
file('m68kcoff.mt',text,'gas-2.1.1/gas/config', 111 ).
file('m88k-opcode.h',text,'gas-2.1.1/gas/config', 11302 ).
file('m88k.patches',text,'gas-2.1.1/gas/config', 4379 ).
file('mips-big.mt',text,'gas-2.1.1/gas/config', 35 ).
file('mips-lit.mt',text,'gas-2.1.1/gas/config', 38 ).
file('mips-opcode.h',text,'gas-2.1.1/gas/config', 25419 ).
file('obj-aout.c',text,'gas-2.1.1/gas/config', 16195 ).
file('obj-aout.h',text,'gas-2.1.1/gas/config', 7975 ).
file('obj-bout.c',text,'gas-2.1.1/gas/config', 15657 ).
file('obj-bout.h',text,'gas-2.1.1/gas/config', 11430 ).
file('obj-coff.c',text,'gas-2.1.1/gas/config', 72605 ).
file('obj-coff.h',text,'gas-2.1.1/gas/config', 23392 ).
file('obj-coffbfd.c',text,'gas-2.1.1/gas/config', 66098 ).
file('obj-coffbfd.h',text,'gas-2.1.1/gas/config', 20140 ).
file('obj-ecoff.c',text,'gas-2.1.1/gas/config', 142427 ).
file('obj-ecoff.h',text,'gas-2.1.1/gas/config', 1654 ).
file('obj-elf.c',text,'gas-2.1.1/gas/config', 31164 ).
file('obj-elf.h',text,'gas-2.1.1/gas/config', 1899 ).
file('obj-generic.c',text,'gas-2.1.1/gas/config', 1370 ).
file('obj-generic.h',text,'gas-2.1.1/gas/config', 2357 ).
file('obj-hp300.c',text,'gas-2.1.1/gas/config', 1447 ).
file('obj-hp300.h',text,'gas-2.1.1/gas/config', 2531 ).
file('obj-ieee.c',text,'gas-2.1.1/gas/config', 12515 ).
file('obj-ieee.h',text,'gas-2.1.1/gas/config', 1120 ).
file('obj-vms.c',text,'gas-2.1.1/gas/config', 137484 ).
file('obj-vms.h',text,'gas-2.1.1/gas/config', 15735 ).
file('sh.mt',text,'gas-2.1.1/gas/config', 130 ).
file('sparc.mt',text,'gas-2.1.1/gas/config', 32 ).
file('tc-a29k.c',text,'gas-2.1.1/gas/config', 28659 ).
file('tc-a29k.h',text,'gas-2.1.1/gas/config', 1478 ).
file('tc-generic.h',text,'gas-2.1.1/gas/config', 1236 ).
file('tc-h8300.c',text,'gas-2.1.1/gas/config', 26489 ).
file('tc-h8300.h',text,'gas-2.1.1/gas/config', 1304 ).
file('tc-h8500.c',text,'gas-2.1.1/gas/config', 32302 ).
file('tc-h8500.h',text,'gas-2.1.1/gas/config', 1421 ).
file('tc-hppa.c',text,'gas-2.1.1/gas/config', 145430 ).
file('tc-hppa.h',text,'gas-2.1.1/gas/config', 14244 ).
file('tc-i386.c',text,'gas-2.1.1/gas/config', 70085 ).
file('tc-i386.h',text,'gas-2.1.1/gas/config', 9537 ).
file('tc-i860.c',text,'gas-2.1.1/gas/config', 31833 ).
file('tc-i860.h',text,'gas-2.1.1/gas/config', 307 ).
file('tc-i960.c',text,'gas-2.1.1/gas/config', 85956 ).
file('tc-i960.h',text,'gas-2.1.1/gas/config', 10290 ).
file('tc-m68851.h',text,'gas-2.1.1/gas/config', 12827 ).
file('tc-m68k.c',text,'gas-2.1.1/gas/config', 116900 ).
file('tc-m68k.h',text,'gas-2.1.1/gas/config', 2433 ).
file('tc-m88k.c',text,'gas-2.1.1/gas/config', 23425 ).
file('tc-m88k.h',text,'gas-2.1.1/gas/config', 1583 ).
file('tc-mips.c',text,'gas-2.1.1/gas/config', 83452 ).
file('tc-mips.h',text,'gas-2.1.1/gas/config', 2600 ).
file('tc-ns32k.c',text,'gas-2.1.1/gas/config', 56803 ).
file('tc-ns32k.h',text,'gas-2.1.1/gas/config', 1630 ).
file('tc-sh.c',text,'gas-2.1.1/gas/config', 27923 ).
file('tc-sh.h',text,'gas-2.1.1/gas/config', 1474 ).
file('tc-sparc.c',text,'gas-2.1.1/gas/config', 39034 ).
file('tc-sparc.h',text,'gas-2.1.1/gas/config', 1919 ).
file('tc-tahoe.c',text,'gas-2.1.1/gas/config', 61244 ).
file('tc-tahoe.h',text,'gas-2.1.1/gas/config', 1100 ).
file('tc-vax.c',text,'gas-2.1.1/gas/config', 95508 ).
file('tc-vax.h',text,'gas-2.1.1/gas/config', 304 ).
file('tc-z8k.c',text,'gas-2.1.1/gas/config', 25098 ).
file('tc-z8k.h',text,'gas-2.1.1/gas/config', 1317 ).
file('te-386bsd.h',text,'gas-2.1.1/gas/config', 1010 ).
file('te-dpx2.h',text,'gas-2.1.1/gas/config', 203 ).
file('te-generic.h',text,'gas-2.1.1/gas/config', 653 ).
file('te-hp300.h',text,'gas-2.1.1/gas/config', 1196 ).
file('te-hppa.h',text,'gas-2.1.1/gas/config', 1087 ).
file('te-i386aix.h',text,'gas-2.1.1/gas/config', 561 ).
file('te-ic960.h',text,'gas-2.1.1/gas/config', 1287 ).
file('te-irix.h',text,'gas-2.1.1/gas/config', 1117 ).
file('te-linux.h',text,'gas-2.1.1/gas/config', 41 ).
file('te-sco386.h',text,'gas-2.1.1/gas/config', 167 ).
file('te-sun3.h',text,'gas-2.1.1/gas/config', 1459 ).
file('te-sysv32.h',text,'gas-2.1.1/gas/config', 119 ).
file('vax-inst.h',text,'gas-2.1.1/gas/config', 3442 ).
file('z8k.mt',text,'gas-2.1.1/gas/config', 151 ).
file('Makefile.in',text,'gas-2.1.1/gas/doc', 4166 ).
file('all.texi',text,'gas-2.1.1/gas/doc', 1506 ).
file('as.1',text,'gas-2.1.1/gas/doc', 6232 ).
file('as.dvi',text,'gas-2.1.1/gas/doc', 484664 ).
file('as.texinfo',text,'gas-2.1.1/gas/doc', 263836 ).
file('configure.in',text,'gas-2.1.1/gas/doc', 1278 ).
file('h8.texi',text,'gas-2.1.1/gas/doc', 518 ).
file('ChangeLog',text,'gas-2.1.1/gas/testsuite', 2577 ).
file('Makefile.in',text,'gas-2.1.1/gas/testsuite', 2869 ).
file('config',dir,'gas-2.1.1/gas/testsuite', 512 ).
file('configure.in',text,'gas-2.1.1/gas/testsuite', 500 ).
file('gas',dir,'gas-2.1.1/gas/testsuite', 512 ).
file('lib',dir,'gas-2.1.1/gas/testsuite', 512 ).
file('unix-gas.exp',text,'gas-2.1.1/gas/testsuite/config', 32 ).
file('all',dir,'gas-2.1.1/gas/testsuite/gas', 512 ).
file('i386',dir,'gas-2.1.1/gas/testsuite/gas', 512 ).
file('ieee.fp',dir,'gas-2.1.1/gas/testsuite/gas', 512 ).
file('m68k',dir,'gas-2.1.1/gas/testsuite/gas', 512 ).
file('m68k-coff',dir,'gas-2.1.1/gas/testsuite/gas', 512 ).
file('sparc-solaris',dir,'gas-2.1.1/gas/testsuite/gas', 512 ).
file('template',text,'gas-2.1.1/gas/testsuite/gas', 3803 ).
file('comment.s',text,'gas-2.1.1/gas/testsuite/gas/all', 137 ).
file('float.s',text,'gas-2.1.1/gas/testsuite/gas/all', 71 ).
file('gas.exp',text,'gas-2.1.1/gas/testsuite/gas/all', 1781 ).
file('p1480.s',text,'gas-2.1.1/gas/testsuite/gas/all', 77 ).
file('p2425.s',text,'gas-2.1.1/gas/testsuite/gas/all', 110 ).
file('x930509.s',text,'gas-2.1.1/gas/testsuite/gas/all', 47 ).
file('all.exp',text,'gas-2.1.1/gas/testsuite/gas/i386', 94 ).
file('inout.s',text,'gas-2.1.1/gas/testsuite/gas/i386', 280 ).
file('x930509a.exp',text,'gas-2.1.1/gas/testsuite/gas/ieee.fp', 554 ).
file('x930509a.s',text,'gas-2.1.1/gas/testsuite/gas/ieee.fp', 91 ).
file('all.exp',text,'gas-2.1.1/gas/testsuite/gas/m68k', 187 ).
file('p2410.s',text,'gas-2.1.1/gas/testsuite/gas/m68k', 154 ).
file('p2663.s',text,'gas-2.1.1/gas/testsuite/gas/m68k', 335 ).
file('t2.s',text,'gas-2.1.1/gas/testsuite/gas/m68k', 72 ).
file('gas.exp',text,'gas-2.1.1/gas/testsuite/gas/m68k-coff', 395 ).
file('p2389.s',text,'gas-2.1.1/gas/testsuite/gas/m68k-coff', 632 ).
file('p2389a.s',text,'gas-2.1.1/gas/testsuite/gas/m68k-coff', 67 ).
file('p2430.s',text,'gas-2.1.1/gas/testsuite/gas/m68k-coff', 168 ).
file('p2430a.s',text,'gas-2.1.1/gas/testsuite/gas/m68k-coff', 29 ).
file('t1.s',text,'gas-2.1.1/gas/testsuite/gas/m68k-coff', 762 ).
file('gas.exp',text,'gas-2.1.1/gas/testsuite/gas/sparc-solaris', 311 ).
file('sol-cc.s',text,'gas-2.1.1/gas/testsuite/gas/sparc-solaris', 2504 ).
file('sol-gcc.s',text,'gas-2.1.1/gas/testsuite/gas/sparc-solaris', 1641 ).
file('doboth',exec,'gas-2.1.1/gas/testsuite/lib', 234 ).
file('doobjcmp',exec,'gas-2.1.1/gas/testsuite/lib', 1355 ).
file('dostriptest',exec,'gas-2.1.1/gas/testsuite/lib', 170 ).
file('dotest',exec,'gas-2.1.1/gas/testsuite/lib', 576 ).
file('dounsreloc',exec,'gas-2.1.1/gas/testsuite/lib', 324 ).
file('dounssym',exec,'gas-2.1.1/gas/testsuite/lib', 302 ).
file('gas-defs.exp',text,'gas-2.1.1/gas/testsuite/lib', 4726 ).
file('run',exec,'gas-2.1.1/gas/testsuite/lib', 23 ).
file('COPYING',text,'gas-2.1.1/include', 17982 ).
file('ChangeLog',text,'gas-2.1.1/include', 18052 ).
file('ansidecl.h',text,'gas-2.1.1/include', 4066 ).
file('aout',dir,'gas-2.1.1/include', 512 ).
file('bfd.h',text,'gas-2.1.1/include', 55101 ).
file('bout.h',text,'gas-2.1.1/include', 6929 ).
file('coff',dir,'gas-2.1.1/include', 512 ).
file('demangle.h',text,'gas-2.1.1/include', 2775 ).
file('dis-asm.h',text,'gas-2.1.1/include', 4187 ).
file('elf',dir,'gas-2.1.1/include', 512 ).
file('fopen-bin.h',text,'gas-2.1.1/include', 918 ).
file('fopen-same.h',text,'gas-2.1.1/include', 898 ).
file('gdbm.h',text,'gas-2.1.1/include', 2464 ).
file('getopt.h',text,'gas-2.1.1/include', 4333 ).
file('ieee-float.h',text,'gas-2.1.1/include', 2347 ).
file('ieee.h',text,'gas-2.1.1/include', 4220 ).
file('oasys.h',text,'gas-2.1.1/include', 3789 ).
file('obstack.h',text,'gas-2.1.1/include', 18564 ).
file('opcode',dir,'gas-2.1.1/include', 512 ).
file('wait.h',text,'gas-2.1.1/include', 1450 ).
file('ChangeLog',text,'gas-2.1.1/include/aout', 1966 ).
file('adobe.h',text,'gas-2.1.1/include/aout', 10416 ).
file('aout64.h',text,'gas-2.1.1/include/aout', 14023 ).
file('ar.h',text,'gas-2.1.1/include/aout', 761 ).
file('encap.h',text,'gas-2.1.1/include/aout', 4722 ).
file('host.h',text,'gas-2.1.1/include/aout', 577 ).
file('hp.h',text,'gas-2.1.1/include/aout', 2930 ).
file('hp300hpux.h',text,'gas-2.1.1/include/aout', 4386 ).
file('hppa.h',text,'gas-2.1.1/include/aout', 104 ).
file('ranlib.h',text,'gas-2.1.1/include/aout', 2463 ).
file('reloc.h',text,'gas-2.1.1/include/aout', 2227 ).
file('stab.def',text,'gas-2.1.1/include/aout', 11050 ).
file('stab_gnu.h',text,'gas-2.1.1/include/aout', 815 ).
file('sun4.h',text,'gas-2.1.1/include/aout', 1173 ).
file('ChangeLog',text,'gas-2.1.1/include/coff', 5686 ).
file('a29k.h',text,'gas-2.1.1/include/coff', 8709 ).
file('alpha.h',text,'gas-2.1.1/include/coff', 5884 ).
file('ecoff-ext.h',text,'gas-2.1.1/include/coff', 11050 ).
file('h8300.h',text,'gas-2.1.1/include/coff', 5140 ).
file('h8500.h',text,'gas-2.1.1/include/coff', 5140 ).
file('i386.h',text,'gas-2.1.1/include/coff', 5414 ).
file('i960.h',text,'gas-2.1.1/include/coff', 6705 ).
file('internal.h',text,'gas-2.1.1/include/coff', 18200 ).
file('m68k.h',text,'gas-2.1.1/include/coff', 5243 ).
file('m88k.h',text,'gas-2.1.1/include/coff', 6506 ).
file('mips.h',text,'gas-2.1.1/include/coff', 6146 ).
file('rs6000.h',text,'gas-2.1.1/include/coff', 6143 ).
file('sh.h',text,'gas-2.1.1/include/coff', 5127 ).
file('sym.h',text,'gas-2.1.1/include/coff', 16119 ).
file('symconst.h',text,'gas-2.1.1/include/coff', 6503 ).
file('we32k.h',text,'gas-2.1.1/include/coff', 5500 ).
file('z8k.h',text,'gas-2.1.1/include/coff', 5209 ).
file('ChangeLog',text,'gas-2.1.1/include/elf', 1487 ).
file('common.h',text,'gas-2.1.1/include/elf', 8054 ).
file('dwarf.h',text,'gas-2.1.1/include/elf', 10157 ).
file('external.h',text,'gas-2.1.1/include/elf', 4918 ).
file('internal.h',text,'gas-2.1.1/include/elf', 5326 ).
file('ChangeLog',text,'gas-2.1.1/include/opcode', 9110 ).
file('a29k.h',text,'gas-2.1.1/include/opcode', 10112 ).
file('arm.h',text,'gas-2.1.1/include/opcode', 11402 ).
file('convex.h',text,'gas-2.1.1/include/opcode', 42852 ).
file('h8300.h',text,'gas-2.1.1/include/opcode', 11658 ).
file('hppa.h',text,'gas-2.1.1/include/opcode', 18593 ).
file('i386.h',text,'gas-2.1.1/include/opcode', 32188 ).
file('i860.h',text,'gas-2.1.1/include/opcode', 27424 ).
file('i960.h',text,'gas-2.1.1/include/opcode', 18627 ).
file('m68k.h',text,'gas-2.1.1/include/opcode', 136524 ).
file('m68kmri.h',text,'gas-2.1.1/include/opcode', 251 ).
file('m88k.h',text,'gas-2.1.1/include/opcode', 32625 ).
file('mips.h',text,'gas-2.1.1/include/opcode', 15600 ).
file('np1.h',text,'gas-2.1.1/include/opcode', 18168 ).
file('ns32k.h',text,'gas-2.1.1/include/opcode', 25900 ).
file('pn.h',text,'gas-2.1.1/include/opcode', 12397 ).
file('pyr.h',text,'gas-2.1.1/include/opcode', 11238 ).
file('rs6k.h',text,'gas-2.1.1/include/opcode', 9232 ).
file('sparc.h',text,'gas-2.1.1/include/opcode', 4987 ).
file('tahoe.h',text,'gas-2.1.1/include/opcode', 7973 ).
file('vax.h',text,'gas-2.1.1/include/opcode', 13509 ).
file('COPYING.LIB',text,'gas-2.1.1/libiberty', 25265 ).
file('ChangeLog',text,'gas-2.1.1/libiberty', 25274 ).
file('Makefile',text,'gas-2.1.1/libiberty', 8225 ).
file('Makefile.in',text,'gas-2.1.1/libiberty', 7766 ).
file('README',text,'gas-2.1.1/libiberty', 5400 ).
file('alloca-botch.h',text,'gas-2.1.1/libiberty', 188 ).
file('alloca-norm.h',text,'gas-2.1.1/libiberty', 217 ).
file('alloca.c',text,'gas-2.1.1/libiberty', 5483 ).
file('argv.c',text,'gas-2.1.1/libiberty', 7387 ).
file('basename.c',text,'gas-2.1.1/libiberty', 1367 ).
file('bcmp.c',text,'gas-2.1.1/libiberty', 1567 ).
file('bcopy.c',text,'gas-2.1.1/libiberty', 1522 ).
file('bzero.c',text,'gas-2.1.1/libiberty', 1286 ).
file('concat.c',text,'gas-2.1.1/libiberty', 3087 ).
file('config',dir,'gas-2.1.1/libiberty', 512 ).
file('config.status',exec,'gas-2.1.1/libiberty', 229 ).
file('configure.bat',text,'gas-2.1.1/libiberty', 292 ).
file('configure.in',text,'gas-2.1.1/libiberty', 982 ).
file('copysign.c',text,'gas-2.1.1/libiberty', 2490 ).
file('cplus-dem.c',text,'gas-2.1.1/libiberty', 55204 ).
file('dummy.c',text,'gas-2.1.1/libiberty', 1368 ).
file('fdmatch.c',text,'gas-2.1.1/libiberty', 2041 ).
file('functions.def',text,'gas-2.1.1/libiberty', 2656 ).
file('getcwd.c',text,'gas-2.1.1/libiberty', 1705 ).
file('getopt.c',text,'gas-2.1.1/libiberty', 19056 ).
file('getopt1.c',text,'gas-2.1.1/libiberty', 3598 ).
file('getpagesize.c',text,'gas-2.1.1/libiberty', 2161 ).
file('ieee-float.c',exec,'gas-2.1.1/libiberty', 4775 ).
file('index.c',text,'gas-2.1.1/libiberty', 143 ).
file('insque.c',text,'gas-2.1.1/libiberty', 1703 ).
file('memchr.c',text,'gas-2.1.1/libiberty', 1185 ).
file('memcmp.c',text,'gas-2.1.1/libiberty', 1548 ).
file('memcpy.c',text,'gas-2.1.1/libiberty', 1280 ).
file('memmove.c',text,'gas-2.1.1/libiberty', 1355 ).
file('memset.c',text,'gas-2.1.1/libiberty', 1124 ).
file('msdos.c',text,'gas-2.1.1/libiberty', 206 ).
file('obstack.c',text,'gas-2.1.1/libiberty', 13365 ).
file('random.c',text,'gas-2.1.1/libiberty', 12800 ).
file('rename.c',text,'gas-2.1.1/libiberty', 1110 ).
file('rindex.c',text,'gas-2.1.1/libiberty', 147 ).
file('sigsetmask.c',text,'gas-2.1.1/libiberty', 1415 ).
file('spaces.c',text,'gas-2.1.1/libiberty', 1545 ).
file('strchr.c',text,'gas-2.1.1/libiberty', 1343 ).
file('strdup.c',text,'gas-2.1.1/libiberty', 172 ).
file('strerror.c',text,'gas-2.1.1/libiberty', 20370 ).
file('strrchr.c',text,'gas-2.1.1/libiberty', 1360 ).
file('strsignal.c',text,'gas-2.1.1/libiberty', 14678 ).
file('strstr.c',text,'gas-2.1.1/libiberty', 1717 ).
file('strtod.c',text,'gas-2.1.1/libiberty', 2605 ).
file('strtol.c',text,'gas-2.1.1/libiberty', 1196 ).
file('strtoul.c',text,'gas-2.1.1/libiberty', 1778 ).
file('tmpnam.c',text,'gas-2.1.1/libiberty', 619 ).
file('vfork.c',text,'gas-2.1.1/libiberty', 916 ).
file('vfprintf.c',text,'gas-2.1.1/libiberty', 184 ).
file('vprintf.c',text,'gas-2.1.1/libiberty', 160 ).
file('vsprintf.c',text,'gas-2.1.1/libiberty', 1501 ).
file('mh-a68bsd',text,'gas-2.1.1/libiberty/config', 70 ).
file('mh-aix',text,'gas-2.1.1/libiberty/config', 442 ).
file('mh-apollo68',text,'gas-2.1.1/libiberty/config', 68 ).
file('mh-hpux',text,'gas-2.1.1/libiberty/config', 24 ).
file('mh-irix4',text,'gas-2.1.1/libiberty/config', 65 ).
file('mh-ncr3000',text,'gas-2.1.1/libiberty/config', 741 ).
file('mh-sysv',text,'gas-2.1.1/libiberty/config', 12 ).
file('mh-sysv4',text,'gas-2.1.1/libiberty/config', 51 ).
file('ChangeLog',text,'gas-2.1.1/opcodes', 7824 ).
file('Makefile',text,'gas-2.1.1/opcodes', 5247 ).
file('Makefile.in',text,'gas-2.1.1/opcodes', 4762 ).
file('a29k-dis.c',text,'gas-2.1.1/opcodes', 9267 ).
file('alpha-dis.c',text,'gas-2.1.1/opcodes', 3688 ).
file('alpha-opc.h',text,'gas-2.1.1/opcodes', 16809 ).
file('config.status',exec,'gas-2.1.1/opcodes', 239 ).
file('configure.in',text,'gas-2.1.1/opcodes', 740 ).
file('dis-buf.c',text,'gas-2.1.1/opcodes', 2290 ).
file('h8500-dis.c',text,'gas-2.1.1/opcodes', 7557 ).
file('h8500-opc.h',text,'gas-2.1.1/opcodes', 282120 ).
file('hppa-dis.c',text,'gas-2.1.1/opcodes', 17109 ).
file('i386-dis.c',text,'gas-2.1.1/opcodes', 37793 ).
file('i960-dis.c',text,'gas-2.1.1/opcodes', 20254 ).
file('m68881-ext.c',text,'gas-2.1.1/opcodes', 157 ).
file('m68k-dis.c',text,'gas-2.1.1/opcodes', 23928 ).
file('mips-dis.c',text,'gas-2.1.1/opcodes', 6184 ).
file('sh-dis.c',text,'gas-2.1.1/opcodes', 5204 ).
file('sh-opc.h',text,'gas-2.1.1/opcodes', 13518 ).
file('sparc-dis.c',text,'gas-2.1.1/opcodes', 13403 ).
file('sparc-opc.c',text,'gas-2.1.1/opcodes', 43447 ).
file('z8k-dis.c',text,'gas-2.1.1/opcodes', 13658 ).
file('z8k-opc.h',text,'gas-2.1.1/opcodes', 120797 ).
file('z8kgen.c',text,'gas-2.1.1/opcodes', 50115 ).
file('gpl.texinfo',text,'gas-2.1.1/texinfo', 18545 ).
file('tex3patch',exec,'gas-2.1.1/texinfo', 1762 ).
file('texinfo.tex',text,'gas-2.1.1/texinfo', 129710 ).
file('ACKNOWLEDGMENT',text,'gawk-2.15.2', 1004 ).
file('COPYING',text,'gawk-2.15.2', 17983 ).
file('FUTURES',text,'gawk-2.15.2', 2741 ).
file('LIMITATIONS',text,'gawk-2.15.2', 562 ).
file('Makefile',text,'gawk-2.15.2', 7850 ).
file('Makefile.bsd44',text,'gawk-2.15.2', 304 ).
file('Makefile.in',text,'gawk-2.15.2', 7850 ).
file('NEWS',text,'gawk-2.15.2', 39782 ).
file('PORTS',text,'gawk-2.15.2', 1074 ).
file('POSIX',text,'gawk-2.15.2', 3041 ).
file('PROBLEMS',text,'gawk-2.15.2', 250 ).
file('README',text,'gawk-2.15.2', 4065 ).
file('README.VMS',text,'gawk-2.15.2', 5233 ).
file('README.amiga',text,'gawk-2.15.2', 3374 ).
file('README.atari',text,'gawk-2.15.2', 1961 ).
file('README.dos',text,'gawk-2.15.2', 838 ).
file('README.hpux8x',text,'gawk-2.15.2', 230 ).
file('README.ibmrt-aos',text,'gawk-2.15.2', 698 ).
file('README.rs6000',text,'gawk-2.15.2', 1194 ).
file('README.sun386i',text,'gawk-2.15.2', 1465 ).
file('README.ultrix',text,'gawk-2.15.2', 1006 ).
file('alloca.c',text,'gawk-2.15.2', 5432 ).
file('alloca.s',text,'gawk-2.15.2', 10194 ).
file('array.c',text,'gawk-2.15.2', 6776 ).
file('atari',dir,'gawk-2.15.2', 512 ).
file('awk.h',text,'gawk-2.15.2', 21160 ).
file('awk.y',text,'gawk-2.15.2', 39924 ).
file('awktab.c',text,'gawk-2.15.2', 102277 ).
file('builtin.c',text,'gawk-2.15.2', 22723 ).
file('config',dir,'gawk-2.15.2', 1024 ).
file('config.h',text,'gawk-2.15.2', 7578 ).
file('config.in',text,'gawk-2.15.2', 7511 ).
file('configure',exec,'gawk-2.15.2', 766 ).
file('dfa.c',text,'gawk-2.15.2', 61384 ).
file('dfa.h',text,'gawk-2.15.2', 22415 ).
file('eval.c',text,'gawk-2.15.2', 30528 ).
file('field.c',text,'gawk-2.15.2', 15582 ).
file('gawk.1',text,'gawk-2.15.2', 40394 ).
file('gawk.info',text,'gawk-2.15.2', 5657 ).
file('gawk.info-1',text,'gawk-2.15.2', 48248 ).
file('gawk.info-2',text,'gawk-2.15.2', 50881 ).
file('gawk.info-3',text,'gawk-2.15.2', 49723 ).
file('gawk.info-4',text,'gawk-2.15.2', 50175 ).
file('gawk.info-5',text,'gawk-2.15.2', 48775 ).
file('gawk.info-6',text,'gawk-2.15.2', 48923 ).
file('gawk.info-7',text,'gawk-2.15.2', 50434 ).
file('gawk.info-8',text,'gawk-2.15.2', 40885 ).
file('gawk.info-9',text,'gawk-2.15.2', 18667 ).
file('gawk.texi',text,'gawk-2.15.2', 422884 ).
file('getopt.c',text,'gawk-2.15.2', 18566 ).
file('getopt.h',text,'gawk-2.15.2', 4345 ).
file('getopt1.c',text,'gawk-2.15.2', 3598 ).
file('io.c',text,'gawk-2.15.2', 26507 ).
file('iop.c',text,'gawk-2.15.2', 7487 ).
file('main.c',text,'gawk-2.15.2', 17619 ).
file('missing',dir,'gawk-2.15.2', 512 ).
file('missing.c',text,'gawk-2.15.2', 1822 ).
file('msg.c',text,'gawk-2.15.2', 2115 ).
file('mungeconf',exec,'gawk-2.15.2', 464 ).
file('node.c',text,'gawk-2.15.2', 8052 ).
file('patchlevel.h',text,'gawk-2.15.2', 21 ).
file('pc',dir,'gawk-2.15.2', 512 ).
file('protos.h',text,'gawk-2.15.2', 3757 ).
file('re.c',text,'gawk-2.15.2', 4226 ).
file('regex.c',text,'gawk-2.15.2', 85996 ).
file('regex.h',text,'gawk-2.15.2', 9676 ).
file('support',dir,'gawk-2.15.2', 512 ).
file('test',dir,'gawk-2.15.2', 2048 ).
file('version.c',text,'gawk-2.15.2', 1718 ).
file('vms',dir,'gawk-2.15.2', 512 ).
file('Makefile.st',text,'gawk-2.15.2/atari', 4248 ).
file('mkconf.g',text,'gawk-2.15.2/atari', 635 ).
file('mkscrpt.sed',text,'gawk-2.15.2/atari', 404 ).
file('stack.c',text,'gawk-2.15.2/atari', 248 ).
file('system.c',text,'gawk-2.15.2/atari', 2404 ).
file('tmpnam.c',text,'gawk-2.15.2/atari', 1122 ).
file('apollo',text,'gawk-2.15.2/config', 150 ).
file('atari',text,'gawk-2.15.2/config', 281 ).
file('bsd42',text,'gawk-2.15.2/config', 303 ).
file('bsd43',text,'gawk-2.15.2/config', 303 ).
file('bsd43r',text,'gawk-2.15.2/config', 76 ).
file('bsd43t',text,'gawk-2.15.2/config', 273 ).
file('bsd44',text,'gawk-2.15.2/config', 96 ).
file('convex',text,'gawk-2.15.2/config', 166 ).
file('cray',text,'gawk-2.15.2/config', 180 ).
file('cray2-50',text,'gawk-2.15.2/config', 146 ).
file('cray2-60',text,'gawk-2.15.2/config', 136 ).
file('cray60',text,'gawk-2.15.2/config', 104 ).
file('hpux7.0',text,'gawk-2.15.2/config', 127 ).
file('hpux8x',text,'gawk-2.15.2/config', 70 ).
file('ibmrt-aos',text,'gawk-2.15.2/config', 350 ).
file('interactive2.2',text,'gawk-2.15.2/config', 151 ).
file('linux',text,'gawk-2.15.2/config', 87 ).
file('mach',text,'gawk-2.15.2/config', 217 ).
file('msc60',text,'gawk-2.15.2/config', 168 ).
file('msdos',text,'gawk-2.15.2/config', 165 ).
file('news',text,'gawk-2.15.2/config', 113 ).
file('next20',text,'gawk-2.15.2/config', 129 ).
file('next21',text,'gawk-2.15.2/config', 83 ).
file('osf1',text,'gawk-2.15.2/config', 70 ).
file('riscos452',text,'gawk-2.15.2/config', 130 ).
file('rs6000',text,'gawk-2.15.2/config', 99 ).
file('sequent',text,'gawk-2.15.2/config', 317 ).
file('sgi',text,'gawk-2.15.2/config', 111 ).
file('sgi33',text,'gawk-2.15.2/config', 105 ).
file('sgi33.cc',text,'gawk-2.15.2/config', 120 ).
file('sgi405',text,'gawk-2.15.2/config', 105 ).
file('sgi405.cc',text,'gawk-2.15.2/config', 125 ).
file('sunos3',text,'gawk-2.15.2/config', 155 ).
file('sunos40',text,'gawk-2.15.2/config', 145 ).
file('sunos41',text,'gawk-2.15.2/config', 87 ).
file('sysv2',text,'gawk-2.15.2/config', 140 ).
file('sysv3',text,'gawk-2.15.2/config', 139 ).
file('sysv4',text,'gawk-2.15.2/config', 96 ).
file('ultrix31',text,'gawk-2.15.2/config', 153 ).
file('ultrix40',text,'gawk-2.15.2/config', 53 ).
file('ultrix41',text,'gawk-2.15.2/config', 91 ).
file('v10config.h',text,'gawk-2.15.2/config', 7638 ).
file('vms-conf.h',text,'gawk-2.15.2/config', 9266 ).
file('vms-posix',text,'gawk-2.15.2/config', 240 ).
file('memcmp.c',text,'gawk-2.15.2/missing', 337 ).
file('memcpy.c',text,'gawk-2.15.2/missing', 261 ).
file('memset.c',text,'gawk-2.15.2/missing', 261 ).
file('random.c',text,'gawk-2.15.2/missing', 13063 ).
file('strchr.c',text,'gawk-2.15.2/missing', 579 ).
file('strerror.c',text,'gawk-2.15.2/missing', 1168 ).
file('strftime.3',text,'gawk-2.15.2/missing', 7705 ).
file('strftime.c',text,'gawk-2.15.2/missing', 17082 ).
file('strncasecmp.c',text,'gawk-2.15.2/missing', 3810 ).
file('strtod.c',text,'gawk-2.15.2/missing', 2050 ).
file('system.c',text,'gawk-2.15.2/missing', 1056 ).
file('tzset.c',text,'gawk-2.15.2/missing', 663 ).
file('config.h',text,'gawk-2.15.2/pc', 7570 ).
file('make.bat',text,'gawk-2.15.2/pc', 1637 ).
file('names.lnk',text,'gawk-2.15.2/pc', 180 ).
file('popen.c',text,'gawk-2.15.2/pc', 2046 ).
file('popen.h',text,'gawk-2.15.2/pc', 134 ).
file('texindex.c',text,'gawk-2.15.2/support', 38475 ).
file('texinfo.tex',text,'gawk-2.15.2/support', 103913 ).
file('Makefile',text,'gawk-2.15.2/test', 3446 ).
file('anchgsub.awk',text,'gawk-2.15.2/test', 35 ).
file('anchgsub.good',text,'gawk-2.15.2/test', 37 ).
file('anchgsub.in',text,'gawk-2.15.2/test', 40 ).
file('argarray.awk',text,'gawk-2.15.2/test', 365 ).
file('argarray.good',text,'gawk-2.15.2/test', 170 ).
file('arrayref',text,'gawk-2.15.2/test', 198 ).
file('arrayref.good',text,'gawk-2.15.2/test', 4 ).
file('asgext.awk',text,'gawk-2.15.2/test', 30 ).
file('asgext.good',text,'gawk-2.15.2/test', 27 ).
file('asgext.in',text,'gawk-2.15.2/test', 16 ).
file('awkpath.good',text,'gawk-2.15.2/test', 10 ).
file('compare.awk',text,'gawk-2.15.2/test', 183 ).
file('compare.good',text,'gawk-2.15.2/test', 15 ).
file('compare.in',text,'gawk-2.15.2/test', 9 ).
file('csi1.out',text,'gawk-2.15.2/test', 11908 ).
file('data',text,'gawk-2.15.2/test', 359 ).
file('fieldwdth.good',text,'gawk-2.15.2/test', 4 ).
file('fontdata.txt',text,'gawk-2.15.2/test', 1195 ).
file('fsbs.good',text,'gawk-2.15.2/test', 4 ).
file('fsbs.in',text,'gawk-2.15.2/test', 4 ).
file('fsrs.awk',text,'gawk-2.15.2/test', 111 ).
file('fsrs.good',text,'gawk-2.15.2/test', 22 ).
file('fsrs.in',text,'gawk-2.15.2/test', 25 ).
file('fstabplus',text,'gawk-2.15.2/test', 48 ).
file('fstabplus.good',text,'gawk-2.15.2/test', 4 ).
file('getline.awk',text,'gawk-2.15.2/test', 40 ).
file('getline.good',text,'gawk-2.15.2/test', 80 ).
file('header.awk',text,'gawk-2.15.2/test', 92 ).
file('igncfs.awk',text,'gawk-2.15.2/test', 132 ).
file('igncfs.good',text,'gawk-2.15.2/test', 51 ).
file('igncfs.in',text,'gawk-2.15.2/test', 44 ).
file('ignrcase.good',text,'gawk-2.15.2/test', 3 ).
file('include.awk',text,'gawk-2.15.2/test', 278 ).
file('inftest.awk',text,'gawk-2.15.2/test', 101 ).
file('inftest.good',text,'gawk-2.15.2/test', 1406 ).
file('lastnpages',text,'gawk-2.15.2/test', 1531 ).
file('lib',dir,'gawk-2.15.2/test', 512 ).
file('longwrds.awk',text,'gawk-2.15.2/test', 455 ).
file('longwrds.good',text,'gawk-2.15.2/test', 275 ).
file('manpage',text,'gawk-2.15.2/test', 4142 ).
file('manyfiles.awk',text,'gawk-2.15.2/test', 26 ).
file('messages.awk',text,'gawk-2.15.2/test', 365 ).
file('negexp.good',text,'gawk-2.15.2/test', 5 ).
file('nfset.awk',text,'gawk-2.15.2/test', 19 ).
file('nfset.good',text,'gawk-2.15.2/test', 42 ).
file('nfset.in',text,'gawk-2.15.2/test', 40 ).
file('numfunc.awk',text,'gawk-2.15.2/test', 303 ).
file('out1.good',text,'gawk-2.15.2/test', 20 ).
file('out2.good',text,'gawk-2.15.2/test', 46 ).
file('out3.good',text,'gawk-2.15.2/test', 13 ).
file('plus-minus',text,'gawk-2.15.2/test', 191 ).
file('posix',exec,'gawk-2.15.2/test', 1553 ).
file('posix.good',text,'gawk-2.15.2/test', 429 ).
file('poundbang',exec,'gawk-2.15.2/test', 101 ).
file('poundbang.good',text,'gawk-2.15.2/test', 31 ).
file('rand.awk',text,'gawk-2.15.2/test', 97 ).
file('reg',dir,'gawk-2.15.2/test', 512 ).
file('regtest',exec,'gawk-2.15.2/test', 265 ).
file('reparse.awk',text,'gawk-2.15.2/test', 67 ).
file('reparse.good',text,'gawk-2.15.2/test', 18 ).
file('reparse.in',text,'gawk-2.15.2/test', 10 ).
file('reverse.awk',text,'gawk-2.15.2/test', 225 ).
file('rs.data',text,'gawk-2.15.2/test', 21 ).
file('rs.good',text,'gawk-2.15.2/test', 13 ).
file('splitargv.awk',text,'gawk-2.15.2/test', 144 ).
file('splitargv.good',text,'gawk-2.15.2/test', 144 ).
file('splitargv.in',text,'gawk-2.15.2/test', 144 ).
file('sqrt.awk',text,'gawk-2.15.2/test', 106 ).
file('swaplns.awk',text,'gawk-2.15.2/test', 66 ).
file('swaplns.good',text,'gawk-2.15.2/test', 359 ).
file('up_down.awk',text,'gawk-2.15.2/test', 223 ).
file('zap_cpp.awk',text,'gawk-2.15.2/test', 258 ).
file('awkpath.awk',text,'gawk-2.15.2/test/lib', 28 ).
file('exp-eq.awk',text,'gawk-2.15.2/test/reg', 22 ).
file('exp-eq.good',text,'gawk-2.15.2/test/reg', 7 ).
file('exp-eq.in',text,'gawk-2.15.2/test/reg', 6 ).
file('exp.awk',text,'gawk-2.15.2/test/reg', 47 ).
file('exp.good',text,'gawk-2.15.2/test/reg', 79 ).
file('func.awk',text,'gawk-2.15.2/test/reg', 25 ).
file('func.good',text,'gawk-2.15.2/test/reg', 58 ).
file('func2.awk',text,'gawk-2.15.2/test/reg', 49 ).
file('func2.good',text,'gawk-2.15.2/test/reg', 121 ).
file('log.awk',text,'gawk-2.15.2/test/reg', 42 ).
file('log.good',text,'gawk-2.15.2/test/reg', 118 ).
file('descrip.mms',text,'gawk-2.15.2/vms', 7327 ).
file('fcntl.h',text,'gawk-2.15.2/vms', 263 ).
file('gawk.cld',text,'gawk-2.15.2/vms', 2193 ).
file('gawk.hlp',text,'gawk-2.15.2/vms', 61060 ).
file('unixlib.h',text,'gawk-2.15.2/vms', 660 ).
file('varargs.h',text,'gawk-2.15.2/vms', 1119 ).
file('vms.h',text,'gawk-2.15.2/vms', 2961 ).
file('vms_args.c',text,'gawk-2.15.2/vms', 15331 ).
file('vms_cli.c',text,'gawk-2.15.2/vms', 3238 ).
file('vms_fwrite.c',text,'gawk-2.15.2/vms', 8013 ).
file('vms_gawk.c',text,'gawk-2.15.2/vms', 8800 ).
file('vms_misc.c',text,'gawk-2.15.2/vms', 5828 ).
file('vms_popen.c',text,'gawk-2.15.2/vms', 12316 ).
file('vmsbuild.com',text,'gawk-2.15.2/vms', 2549 ).
file('COPYING',text,'gcc-2.4.5', 17982 ).
file('COPYING.LIB',text,'gcc-2.4.5', 25265 ).
file('ChangeLog',text,'gcc-2.4.5', 390298 ).
file('ChangeLog.1',text,'gcc-2.4.5', 353521 ).
file('ChangeLog.2',text,'gcc-2.4.5', 292385 ).
file('ChangeLog.3',text,'gcc-2.4.5', 338487 ).
file('ChangeLog.4',text,'gcc-2.4.5', 427191 ).
file('ChangeLog.5',text,'gcc-2.4.5', 336095 ).
file('ChangeLog.6',text,'gcc-2.4.5', 313790 ).
file('INSTALL',text,'gcc-2.4.5', 57100 ).
file('Makefile',text,'gcc-2.4.5', 74905 ).
file('Makefile.in',text,'gcc-2.4.5', 75032 ).
file('NEWS',text,'gcc-2.4.5', 5080 ).
file('OChangeLog',text,'gcc-2.4.5', 261226 ).
file('PROBLEMS',text,'gcc-2.4.5', 4877 ).
file('PROJECTS',text,'gcc-2.4.5', 18497 ).
file('README',text,'gcc-2.4.5', 1278 ).
file('README-fixinc',text,'gcc-2.4.5', 495 ).
file('README.ALTOS',text,'gcc-2.4.5', 1851 ).
file('README.APOLLO',text,'gcc-2.4.5', 3060 ).
file('README.DWARF',text,'gcc-2.4.5', 29709 ).
file('README.MIPS',text,'gcc-2.4.5', 680 ).
file('README.NS32K',text,'gcc-2.4.5', 6384 ).
file('README.RS6000',text,'gcc-2.4.5', 2393 ).
file('README.TRAD',text,'gcc-2.4.5', 1922 ).
file('README.X11',text,'gcc-2.4.5', 13994 ).
file('SERVICE',text,'gcc-2.4.5', 32911 ).
file('TAGS',text,'gcc-2.4.5', 217195 ).
file('TESTS.FLUNK',text,'gcc-2.4.5', 1341 ).
file('alloca.c',text,'gcc-2.4.5', 13704 ).
file('assert.h',text,'gcc-2.4.5', 1397 ).
file('basic-block.h',text,'gcc-2.4.5', 2311 ).
file('build-make',text,'gcc-2.4.5', 824 ).
file('byteorder.h',text,'gcc-2.4.5', 3124 ).
file('c++',exec,'gcc-2.4.5', 2462 ).
file('c-aux-info.c',text,'gcc-2.4.5', 22035 ).
file('c-common.c',text,'gcc-2.4.5', 36775 ).
file('c-convert.c',text,'gcc-2.4.5', 3453 ).
file('c-decl.c',text,'gcc-2.4.5', 208730 ).
file('c-iterate.c',text,'gcc-2.4.5', 16027 ).
file('c-lang.c',text,'gcc-2.4.5', 2101 ).
file('c-lex.c',text,'gcc-2.4.5', 54392 ).
file('c-lex.h',text,'gcc-2.4.5', 1961 ).
file('c-parse.c',text,'gcc-2.4.5', 131108 ).
file('c-parse.gperf',text,'gcc-2.4.5', 2231 ).
file('c-parse.h',text,'gcc-2.4.5', 1317 ).
file('c-parse.in',text,'gcc-2.4.5', 72680 ).
file('c-parse.y',text,'gcc-2.4.5', 55027 ).
file('c-pragma.c',text,'gcc-2.4.5', 4494 ).
file('c-tree.h',text,'gcc-2.4.5', 19099 ).
file('c-typeck.c',text,'gcc-2.4.5', 180044 ).
file('caller-save.c',text,'gcc-2.4.5', 24406 ).
file('calls.c',text,'gcc-2.4.5', 94184 ).
file('cccp.1',text,'gcc-2.4.5', 15426 ).
file('cccp.c',text,'gcc-2.4.5', 247353 ).
file('cexp.c',text,'gcc-2.4.5', 49793 ).
file('cexp.y',text,'gcc-2.4.5', 23178 ).
file('collect2.c',text,'gcc-2.4.5', 56407 ).
file('combine.c',text,'gcc-2.4.5', 337038 ).
file('conditions.h',text,'gcc-2.4.5', 4498 ).
file('config',dir,'gcc-2.4.5', 1024 ).
file('config.status',exec,'gcc-2.4.5', 162 ).
file('config.sub',exec,'gcc-2.4.5', 13793 ).
file('configure',exec,'gcc-2.4.5', 37221 ).
file('configure.bat',text,'gcc-2.4.5', 2950 ).
file('convert.c',text,'gcc-2.4.5', 14231 ).
file('convert.h',text,'gcc-2.4.5', 983 ).
file('cp-call.c',text,'gcc-2.4.5', 85163 ).
file('cp-class.c',text,'gcc-2.4.5', 134803 ).
file('cp-class.h',text,'gcc-2.4.5', 4036 ).
file('cp-cvt.c',text,'gcc-2.4.5', 53861 ).
file('cp-decl.c',text,'gcc-2.4.5', 367038 ).
file('cp-decl.h',text,'gcc-2.4.5', 2405 ).
file('cp-decl2.c',text,'gcc-2.4.5', 73395 ).
file('cp-dem.c',text,'gcc-2.4.5', 27039 ).
file('cp-edsel.c',text,'gcc-2.4.5', 19997 ).
file('cp-except.c',text,'gcc-2.4.5', 37402 ).
file('cp-expr.c',text,'gcc-2.4.5', 6646 ).
file('cp-gc.c',text,'gcc-2.4.5', 24756 ).
file('cp-hash.h',text,'gcc-2.4.5', 6051 ).
file('cp-init.c',text,'gcc-2.4.5', 126267 ).
file('cp-input.c',text,'gcc-2.4.5', 4343 ).
file('cp-lex.c',text,'gcc-2.4.5', 115043 ).
file('cp-lex.h',text,'gcc-2.4.5', 3085 ).
file('cp-method.c',text,'gcc-2.4.5', 66108 ).
file('cp-parse.c',text,'gcc-2.4.5', 295407 ).
file('cp-parse.h',text,'gcc-2.4.5', 1899 ).
file('cp-parse.y',text,'gcc-2.4.5', 116236 ).
file('cp-pt.c',text,'gcc-2.4.5', 59507 ).
file('cp-ptree.c',text,'gcc-2.4.5', 4874 ).
file('cp-search.c',text,'gcc-2.4.5', 124992 ).
file('cp-spew.c',text,'gcc-2.4.5', 28683 ).
file('cp-tree.c',text,'gcc-2.4.5', 47211 ).
file('cp-tree.def',text,'gcc-2.4.5', 3730 ).
file('cp-tree.h',text,'gcc-2.4.5', 88758 ).
file('cp-type2.c',text,'gcc-2.4.5', 48987 ).
file('cp-typeck.c',text,'gcc-2.4.5', 198952 ).
file('cp-xref.c',text,'gcc-2.4.5', 17631 ).
file('cpp.1',text,'gcc-2.4.5', 16 ).
file('cpp.aux',text,'gcc-2.4.5', 3246 ).
file('cpp.cps',text,'gcc-2.4.5', 1028 ).
file('cpp.fns',text,'gcc-2.4.5', 2365 ).
file('cpp.info',text,'gcc-2.4.5', 2013 ).
file('cpp.info-1',text,'gcc-2.4.5', 50641 ).
file('cpp.info-2',text,'gcc-2.4.5', 49102 ).
file('cpp.info-3',text,'gcc-2.4.5', 4898 ).
file('cpp.texi',text,'gcc-2.4.5', 101837 ).
file('cross-make',text,'gcc-2.4.5', 701 ).
file('cross-test.c',text,'gcc-2.4.5', 1473 ).
file('crtstuff.c',text,'gcc-2.4.5', 6705 ).
file('cse.c',text,'gcc-2.4.5', 253388 ).
file('dbxout.c',text,'gcc-2.4.5', 74052 ).
file('dbxstclass.h',text,'gcc-2.4.5', 427 ).
file('defaults.h',text,'gcc-2.4.5', 4432 ).
file('doschk.c',text,'gcc-2.4.5', 6831 ).
file('dostage2',exec,'gcc-2.4.5', 92 ).
file('dostage3',exec,'gcc-2.4.5', 93 ).
file('dwarf.h',text,'gcc-2.4.5', 10065 ).
file('dwarfout.c',text,'gcc-2.4.5', 188367 ).
file('emit-rtl.c',text,'gcc-2.4.5', 82973 ).
file('enquire.c',text,'gcc-2.4.5', 78191 ).
file('explow.c',text,'gcc-2.4.5', 29397 ).
file('expmed.c',text,'gcc-2.4.5', 101166 ).
file('expr.c',text,'gcc-2.4.5', 252994 ).
file('expr.h',text,'gcc-2.4.5', 29715 ).
file('extend.texi',text,'gcc-2.4.5', 90879 ).
file('final.c',text,'gcc-2.4.5', 73251 ).
file('fixcpp',text,'gcc-2.4.5', 2356 ).
file('fixinc.mips',text,'gcc-2.4.5', 34873 ).
file('fixinc.ps2',exec,'gcc-2.4.5', 34802 ).
file('fixinc.sco',exec,'gcc-2.4.5', 7839 ).
file('fixinc.svr4',exec,'gcc-2.4.5', 33452 ).
file('fixincludes',exec,'gcc-2.4.5', 35923 ).
file('fixlimits.h',text,'gcc-2.4.5', 25 ).
file('flags.h',text,'gcc-2.4.5', 10713 ).
file('floatlib.c',text,'gcc-2.4.5', 11412 ).
file('flow.c',text,'gcc-2.4.5', 85315 ).
file('fold-const.c',text,'gcc-2.4.5', 135226 ).
file('function.c',text,'gcc-2.4.5', 156103 ).
file('function.h',text,'gcc-2.4.5', 5441 ).
file('g++',exec,'gcc-2.4.5', 2462 ).
file('g++.1',text,'gcc-2.4.5', 18382 ).
file('g++int.texi',text,'gcc-2.4.5', 23165 ).
file('gbl-ctors.h',text,'gcc-2.4.5', 2895 ).
file('gcc.1',text,'gcc-2.4.5', 103942 ).
file('gcc.aux',text,'gcc-2.4.5', 16389 ).
file('gcc.c',text,'gcc-2.4.5', 115673 ).
file('gcc.cps',text,'gcc-2.4.5', 73997 ).
file('gcc.hlp',text,'gcc-2.4.5', 14325 ).
file('gcc.info',text,'gcc-2.4.5', 7647 ).
file('gcc.info-1',text,'gcc-2.4.5', 49437 ).
file('gcc.info-10',text,'gcc-2.4.5', 48647 ).
file('gcc.info-11',text,'gcc-2.4.5', 40438 ).
file('gcc.info-12',text,'gcc-2.4.5', 51175 ).
file('gcc.info-13',text,'gcc-2.4.5', 48861 ).
file('gcc.info-14',text,'gcc-2.4.5', 48927 ).
file('gcc.info-15',text,'gcc-2.4.5', 46867 ).
file('gcc.info-16',text,'gcc-2.4.5', 48707 ).
file('gcc.info-17',text,'gcc-2.4.5', 47434 ).
file('gcc.info-18',text,'gcc-2.4.5', 49785 ).
file('gcc.info-19',text,'gcc-2.4.5', 51074 ).
file('gcc.info-2',text,'gcc-2.4.5', 49860 ).
file('gcc.info-20',text,'gcc-2.4.5', 46038 ).
file('gcc.info-21',text,'gcc-2.4.5', 7060 ).
file('gcc.info-22',text,'gcc-2.4.5', 96843 ).
file('gcc.info-3',text,'gcc-2.4.5', 49746 ).
file('gcc.info-4',text,'gcc-2.4.5', 22186 ).
file('gcc.info-5',text,'gcc-2.4.5', 42910 ).
file('gcc.info-6',text,'gcc-2.4.5', 51009 ).
file('gcc.info-7',text,'gcc-2.4.5', 47055 ).
file('gcc.info-8',text,'gcc-2.4.5', 49187 ).
file('gcc.info-9',text,'gcc-2.4.5', 49974 ).
file('gcc.texi',text,'gcc-2.4.5', 165848 ).
file('genattr.c',text,'gcc-2.4.5', 12258 ).
file('genattrtab.c',text,'gcc-2.4.5', 158342 ).
file('gencodes.c',text,'gcc-2.4.5', 3496 ).
file('genconfig.c',text,'gcc-2.4.5', 8846 ).
file('genemit.c',text,'gcc-2.4.5', 21292 ).
file('genextract.c',text,'gcc-2.4.5', 12840 ).
file('genflags.c',text,'gcc-2.4.5', 6657 ).
file('genopinit.c',text,'gcc-2.4.5', 11827 ).
file('genoutput.c',text,'gcc-2.4.5', 27284 ).
file('genpeep.c',text,'gcc-2.4.5', 12288 ).
file('genrecog.c',text,'gcc-2.4.5', 54269 ).
file('getopt.c',text,'gcc-2.4.5', 20876 ).
file('getopt.h',text,'gcc-2.4.5', 4412 ).
file('getopt1.c',text,'gcc-2.4.5', 4142 ).
file('getpwd.c',text,'gcc-2.4.5', 2067 ).
file('glimits.h',text,'gcc-2.4.5', 2098 ).
file('global.c',text,'gcc-2.4.5', 53253 ).
file('gmon.c',text,'gcc-2.4.5', 8765 ).
file('gplus.gperf',text,'gcc-2.4.5', 2455 ).
file('gstab.h',text,'gcc-2.4.5', 259 ).
file('gstdarg.h',text,'gcc-2.4.5', 4429 ).
file('gstddef.h',text,'gcc-2.4.5', 5833 ).
file('gsyms.h',text,'gcc-2.4.5', 1035 ).
file('gsyslimits.h',text,'gcc-2.4.5', 239 ).
file('gvarargs.h',text,'gcc-2.4.5', 4203 ).
file('halfpic.c',text,'gcc-2.4.5', 8762 ).
file('halfpic.h',text,'gcc-2.4.5', 3121 ).
file('hard-reg-set.h',text,'gcc-2.4.5', 9532 ).
file('input.h',text,'gcc-2.4.5', 1479 ).
file('install.sh',exec,'gcc-2.4.5', 2186 ).
file('install.texi',text,'gcc-2.4.5', 59806 ).
file('integrate.c',text,'gcc-2.4.5', 94647 ).
file('integrate.h',text,'gcc-2.4.5', 5131 ).
file('invoke.texi',text,'gcc-2.4.5', 135245 ).
file('jump.c',text,'gcc-2.4.5', 126387 ).
file('just-fixinc',text,'gcc-2.4.5', 774 ).
file('libgcc1.c',text,'gcc-2.4.5', 11524 ).
file('libgcc2.c',text,'gcc-2.4.5', 39023 ).
file('limitx.h',text,'gcc-2.4.5', 455 ).
file('limity.h',text,'gcc-2.4.5', 143 ).
file('listing',exec,'gcc-2.4.5', 3900 ).
file('local-alloc.c',text,'gcc-2.4.5', 70099 ).
file('longlong.h',text,'gcc-2.4.5', 29703 ).
file('loop.c',text,'gcc-2.4.5', 202246 ).
file('loop.h',text,'gcc-2.4.5', 7557 ).
file('machmode.def',text,'gcc-2.4.5', 4897 ).
file('machmode.h',text,'gcc-2.4.5', 5090 ).
file('make-cc1.com',text,'gcc-2.4.5', 11532 ).
file('make-cccp.com',text,'gcc-2.4.5', 1511 ).
file('make-gcc.com',text,'gcc-2.4.5', 1059 ).
file('make-l2.com',text,'gcc-2.4.5', 2822 ).
file('math-3300.h',text,'gcc-2.4.5', 8504 ).
file('math-68881.h',text,'gcc-2.4.5', 9411 ).
file('md.texi',text,'gcc-2.4.5', 156035 ).
file('mips-tdump.c',text,'gcc-2.4.5', 42030 ).
file('mips-tfile.c',text,'gcc-2.4.5', 160167 ).
file('move-if-change',exec,'gcc-2.4.5', 229 ).
file('objc',dir,'gcc-2.4.5', 512 ).
file('objc-act.c',text,'gcc-2.4.5', 215240 ).
file('objc-act.h',text,'gcc-2.4.5', 4385 ).
file('objc-parse.c',text,'gcc-2.4.5', 180342 ).
file('objc-parse.y',text,'gcc-2.4.5', 70949 ).
file('objc-tree.def',text,'gcc-2.4.5', 1589 ).
file('obstack.c',text,'gcc-2.4.5', 13947 ).
file('obstack.h',text,'gcc-2.4.5', 18583 ).
file('optabs.c',text,'gcc-2.4.5', 107876 ).
file('output.h',text,'gcc-2.4.5', 5671 ).
file('pcp.h',text,'gcc-2.4.5', 3123 ).
file('populate',text,'gcc-2.4.5', 5587 ).
file('print-rtl.c',text,'gcc-2.4.5', 7524 ).
file('print-tree.c',text,'gcc-2.4.5', 17871 ).
file('proto.h',text,'gcc-2.4.5', 137 ).
file('protoize.c',text,'gcc-2.4.5', 146838 ).
file('pself.c',text,'gcc-2.4.5', 97 ).
file('pself1.c',text,'gcc-2.4.5', 68 ).
file('pself2.c',text,'gcc-2.4.5', 98 ).
file('pself3.c',text,'gcc-2.4.5', 65 ).
file('real.c',text,'gcc-2.4.5', 96403 ).
file('real.h',text,'gcc-2.4.5', 12743 ).
file('recog.c',text,'gcc-2.4.5', 55533 ).
file('recog.h',text,'gcc-2.4.5', 3638 ).
file('reg-stack.c',text,'gcc-2.4.5', 85233 ).
file('regclass.c',text,'gcc-2.4.5', 48974 ).
file('regs.h',text,'gcc-2.4.5', 5286 ).
file('reload.c',text,'gcc-2.4.5', 183530 ).
file('reload.h',text,'gcc-2.4.5', 9098 ).
file('reload1.c',text,'gcc-2.4.5', 230696 ).
file('reorg.c',text,'gcc-2.4.5', 132165 ).
file('rtl.c',text,'gcc-2.4.5', 21666 ).
file('rtl.def',text,'gcc-2.4.5', 32611 ).
file('rtl.h',text,'gcc-2.4.5', 37347 ).
file('rtl.texi',text,'gcc-2.4.5', 114462 ).
file('rtlanal.c',text,'gcc-2.4.5', 39761 ).
file('sched.c',text,'gcc-2.4.5', 139697 ).
file('sdbout.c',text,'gcc-2.4.5', 41994 ).
file('sort-protos',text,'gcc-2.4.5', 293 ).
file('stab.def',text,'gcc-2.4.5', 8981 ).
file('stack.h',text,'gcc-2.4.5', 1445 ).
file('stmt.c',text,'gcc-2.4.5', 150132 ).
file('stor-layout.c',text,'gcc-2.4.5', 36738 ).
file('stupid.c',text,'gcc-2.4.5', 16548 ).
file('sys-protos.h',text,'gcc-2.4.5', 76770 ).
file('sys-types.h',text,'gcc-2.4.5', 4300 ).
file('texinfo.tex',text,'gcc-2.4.5', 130562 ).
file('tm.texi',text,'gcc-2.4.5', 250796 ).
file('toplev.c',text,'gcc-2.4.5', 95276 ).
file('tree.c',text,'gcc-2.4.5', 92070 ).
file('tree.def',text,'gcc-2.4.5', 29875 ).
file('tree.h',text,'gcc-2.4.5', 56706 ).
file('typeclass.h',text,'gcc-2.4.5', 467 ).
file('unprotoize.c',text,'gcc-2.4.5', 22 ).
file('unroll.c',text,'gcc-2.4.5', 110503 ).
file('va-alpha.h',text,'gcc-2.4.5', 2470 ).
file('va-clipper.h',text,'gcc-2.4.5', 1362 ).
file('va-i860.h',text,'gcc-2.4.5', 6371 ).
file('va-i960.h',text,'gcc-2.4.5', 2074 ).
file('va-m88k.h',text,'gcc-2.4.5', 2543 ).
file('va-mips.h',text,'gcc-2.4.5', 2178 ).
file('va-pa.h',text,'gcc-2.4.5', 1383 ).
file('va-pyr.h',text,'gcc-2.4.5', 4045 ).
file('va-sparc.h',text,'gcc-2.4.5', 2807 ).
file('va-spur.h',text,'gcc-2.4.5', 1662 ).
file('varasm.c',text,'gcc-2.4.5', 84946 ).
file('version.c',text,'gcc-2.4.5', 32 ).
file('vmsconfig.com',text,'gcc-2.4.5', 9349 ).
file('xcoffout.c',text,'gcc-2.4.5', 12026 ).
file('xcoffout.h',text,'gcc-2.4.5', 6007 ).
file('README',text,'gcc-2.4.5/config', 262 ).
file('a29k',dir,'gcc-2.4.5/config', 512 ).
file('alpha',dir,'gcc-2.4.5/config', 512 ).
file('aoutos.h',text,'gcc-2.4.5/config', 3107 ).
file('arm',dir,'gcc-2.4.5/config', 512 ).
file('clipper',dir,'gcc-2.4.5/config', 512 ).
file('convex',dir,'gcc-2.4.5/config', 512 ).
file('elxsi',dir,'gcc-2.4.5/config', 512 ).
file('fx80',dir,'gcc-2.4.5/config', 512 ).
file('gmicro',dir,'gcc-2.4.5/config', 512 ).
file('h8300',dir,'gcc-2.4.5/config', 512 ).
file('i386',dir,'gcc-2.4.5/config', 1536 ).
file('i860',dir,'gcc-2.4.5/config', 512 ).
file('i960',dir,'gcc-2.4.5/config', 512 ).
file('m68k',dir,'gcc-2.4.5/config', 1536 ).
file('m88k',dir,'gcc-2.4.5/config', 512 ).
file('mips',dir,'gcc-2.4.5/config', 1536 ).
file('nextstep.c',text,'gcc-2.4.5/config', 2459 ).
file('nextstep.h',text,'gcc-2.4.5/config', 19122 ).
file('ns32k',dir,'gcc-2.4.5/config', 512 ).
file('pa',dir,'gcc-2.4.5/config', 512 ).
file('pyr',dir,'gcc-2.4.5/config', 512 ).
file('romp',dir,'gcc-2.4.5/config', 512 ).
file('rs6000',dir,'gcc-2.4.5/config', 512 ).
file('sh',dir,'gcc-2.4.5/config', 512 ).
file('sparc',dir,'gcc-2.4.5/config', 512 ).
file('spur',dir,'gcc-2.4.5/config', 512 ).
file('svr3.h',text,'gcc-2.4.5/config', 12279 ).
file('svr4.h',text,'gcc-2.4.5/config', 28010 ).
file('t-libc-ok',text,'gcc-2.4.5/config', 21 ).
file('t-svr3',text,'gcc-2.4.5/config', 176 ).
file('t-svr4',text,'gcc-2.4.5/config', 176 ).
file('tahoe',dir,'gcc-2.4.5/config', 512 ).
file('vax',dir,'gcc-2.4.5/config', 512 ).
file('we32k',dir,'gcc-2.4.5/config', 512 ).
file('x-svr4',text,'gcc-2.4.5/config', 303 ).
file('xm-svr3.h',text,'gcc-2.4.5/config', 1057 ).
file('xm-svr4.h',text,'gcc-2.4.5/config', 1182 ).
file('a29k.c',text,'gcc-2.4.5/config/a29k', 38058 ).
file('a29k.h',text,'gcc-2.4.5/config/a29k', 60981 ).
file('a29k.md',text,'gcc-2.4.5/config/a29k', 84247 ).
file('unix.h',text,'gcc-2.4.5/config/a29k', 2920 ).
file('x-unix',text,'gcc-2.4.5/config/a29k', 66 ).
file('xm-a29k.h',text,'gcc-2.4.5/config/a29k', 1450 ).
file('xm-unix.h',text,'gcc-2.4.5/config/a29k', 1507 ).
file('alpha.c',text,'gcc-2.4.5/config/alpha', 38871 ).
file('alpha.h',text,'gcc-2.4.5/config/alpha', 63634 ).
file('alpha.md',text,'gcc-2.4.5/config/alpha', 98408 ).
file('x-alpha',text,'gcc-2.4.5/config/alpha', 34 ).
file('xm-alpha.h',text,'gcc-2.4.5/config/alpha', 2060 ).
file('arm.c',text,'gcc-2.4.5/config/arm', 35901 ).
file('arm.h',text,'gcc-2.4.5/config/arm', 48687 ).
file('arm.md',text,'gcc-2.4.5/config/arm', 38210 ).
file('xm-arm.h',text,'gcc-2.4.5/config/arm', 1514 ).
file('clipper.c',text,'gcc-2.4.5/config/clipper', 10555 ).
file('clipper.h',text,'gcc-2.4.5/config/clipper', 38433 ).
file('clipper.md',text,'gcc-2.4.5/config/clipper', 35122 ).
file('clix.h',text,'gcc-2.4.5/config/clipper', 4504 ).
file('t-clix',text,'gcc-2.4.5/config/clipper', 34 ).
file('x-clix',text,'gcc-2.4.5/config/clipper', 18 ).
file('xm-clix.h',text,'gcc-2.4.5/config/clipper', 1037 ).
file('convex.c',text,'gcc-2.4.5/config/convex', 15405 ).
file('convex.h',text,'gcc-2.4.5/config/convex', 52074 ).
file('convex.md',text,'gcc-2.4.5/config/convex', 56330 ).
file('convex1.h',text,'gcc-2.4.5/config/convex', 90 ).
file('convex2.h',text,'gcc-2.4.5/config/convex', 90 ).
file('convex32.h',text,'gcc-2.4.5/config/convex', 93 ).
file('convex34.h',text,'gcc-2.4.5/config/convex', 93 ).
file('convex38.h',text,'gcc-2.4.5/config/convex', 93 ).
file('fixinc.convex',text,'gcc-2.4.5/config/convex', 9234 ).
file('x-convex',text,'gcc-2.4.5/config/convex', 141 ).
file('xm-convex.h',text,'gcc-2.4.5/config/convex', 1797 ).
file('elxsi.c',text,'gcc-2.4.5/config/elxsi', 3621 ).
file('elxsi.h',text,'gcc-2.4.5/config/elxsi', 35582 ).
file('elxsi.md',text,'gcc-2.4.5/config/elxsi', 39836 ).
file('x-elxsi',text,'gcc-2.4.5/config/elxsi', 247 ).
file('xm-elxsi.h',text,'gcc-2.4.5/config/elxsi', 1362 ).
file('fx80.c',text,'gcc-2.4.5/config/fx80', 8868 ).
file('fx80.h',text,'gcc-2.4.5/config/fx80', 55047 ).
file('fx80.md',text,'gcc-2.4.5/config/fx80', 70639 ).
file('xm-fx80.h',text,'gcc-2.4.5/config/fx80', 1412 ).
file('gmicro.c',text,'gcc-2.4.5/config/gmicro', 20730 ).
file('gmicro.h',text,'gcc-2.4.5/config/gmicro', 57448 ).
file('gmicro.md',text,'gcc-2.4.5/config/gmicro', 68485 ).
file('h8300.c',text,'gcc-2.4.5/config/h8300', 20126 ).
file('h8300.h',text,'gcc-2.4.5/config/h8300', 40316 ).
file('h8300.md',text,'gcc-2.4.5/config/h8300', 36256 ).
file('t-h8300',text,'gcc-2.4.5/config/h8300', 208 ).
file('xm-h8300.h',text,'gcc-2.4.5/config/h8300', 1424 ).
file('386bsd.h',text,'gcc-2.4.5/config/i386', 2317 ).
file('aix386.h',text,'gcc-2.4.5/config/i386', 2191 ).
file('aix386ng.h',text,'gcc-2.4.5/config/i386', 4184 ).
file('att.h',text,'gcc-2.4.5/config/i386', 3524 ).
file('bsd.h',text,'gcc-2.4.5/config/i386', 4481 ).
file('gas.h',text,'gcc-2.4.5/config/i386', 5174 ).
file('go32.h',text,'gcc-2.4.5/config/i386', 219 ).
file('gstabs.h',text,'gcc-2.4.5/config/i386', 190 ).
file('i386.c',text,'gcc-2.4.5/config/i386', 50389 ).
file('i386.h',text,'gcc-2.4.5/config/i386', 58378 ).
file('i386.md',text,'gcc-2.4.5/config/i386', 123954 ).
file('i386iscgas.h',text,'gcc-2.4.5/config/i386', 2415 ).
file('isc.h',text,'gcc-2.4.5/config/i386', 1980 ).
file('isccoff.h',text,'gcc-2.4.5/config/i386', 409 ).
file('iscdbx.h',text,'gcc-2.4.5/config/i386', 1491 ).
file('linux.h',text,'gcc-2.4.5/config/i386', 2763 ).
file('mach.h',text,'gcc-2.4.5/config/i386', 714 ).
file('next.c',text,'gcc-2.4.5/config/i386', 234 ).
file('next.h',text,'gcc-2.4.5/config/i386', 6390 ).
file('osfelf.h',text,'gcc-2.4.5/config/i386', 2252 ).
file('osfrose.h',text,'gcc-2.4.5/config/i386', 20093 ).
file('perform.h',text,'gcc-2.4.5/config/i386', 2936 ).
file('sco.h',text,'gcc-2.4.5/config/i386', 2862 ).
file('sco4.h',text,'gcc-2.4.5/config/i386', 2963 ).
file('sco4dbx.h',text,'gcc-2.4.5/config/i386', 2486 ).
file('scodbx.h',text,'gcc-2.4.5/config/i386', 2941 ).
file('seq-gas.h',text,'gcc-2.4.5/config/i386', 1378 ).
file('sequent.h',text,'gcc-2.4.5/config/i386', 4904 ).
file('sol2-c1.asm',text,'gcc-2.4.5/config/i386', 5257 ).
file('sol2-ci.asm',text,'gcc-2.4.5/config/i386', 1969 ).
file('sol2-cn.asm',text,'gcc-2.4.5/config/i386', 1854 ).
file('sol2.h',text,'gcc-2.4.5/config/i386', 1471 ).
file('sun.h',text,'gcc-2.4.5/config/i386', 2385 ).
file('sun386.h',text,'gcc-2.4.5/config/i386', 4505 ).
file('svr3.ifile',text,'gcc-2.4.5/config/i386', 1410 ).
file('svr3dbx.h',text,'gcc-2.4.5/config/i386', 3691 ).
file('svr3gas.h',text,'gcc-2.4.5/config/i386', 9629 ).
file('svr3z.ifile',text,'gcc-2.4.5/config/i386', 1426 ).
file('sysv3.h',text,'gcc-2.4.5/config/i386', 3969 ).
file('sysv4.h',text,'gcc-2.4.5/config/i386', 8428 ).
file('t-aix',text,'gcc-2.4.5/config/i386', 306 ).
file('t-isc',text,'gcc-2.4.5/config/i386', 245 ).
file('t-iscscodbx',text,'gcc-2.4.5/config/i386', 68 ).
file('t-next',text,'gcc-2.4.5/config/i386', 190 ).
file('t-osfrose',text,'gcc-2.4.5/config/i386', 100 ).
file('t-sco',text,'gcc-2.4.5/config/i386', 245 ).
file('t-sol2',text,'gcc-2.4.5/config/i386', 1147 ).
file('t-svr3dbx',text,'gcc-2.4.5/config/i386', 306 ).
file('unix.h',text,'gcc-2.4.5/config/i386', 4703 ).
file('v3gas.h',text,'gcc-2.4.5/config/i386', 2756 ).
file('x-aix',text,'gcc-2.4.5/config/i386', 501 ).
file('x-isc',text,'gcc-2.4.5/config/i386', 84 ).
file('x-isc3',text,'gcc-2.4.5/config/i386', 169 ).
file('x-linux',text,'gcc-2.4.5/config/i386', 46 ).
file('x-ncr3000',text,'gcc-2.4.5/config/i386', 1294 ).
file('x-next',text,'gcc-2.4.5/config/i386', 186 ).
file('x-osfrose',text,'gcc-2.4.5/config/i386', 903 ).
file('x-sco',text,'gcc-2.4.5/config/i386', 153 ).
file('x-sco4',text,'gcc-2.4.5/config/i386', 124 ).
file('x-sysv3',text,'gcc-2.4.5/config/i386', 10 ).
file('xm-aix.h',text,'gcc-2.4.5/config/i386', 1076 ).
file('xm-dos.h',text,'gcc-2.4.5/config/i386', 304 ).
file('xm-i386.h',text,'gcc-2.4.5/config/i386', 1450 ).
file('xm-isc.h',text,'gcc-2.4.5/config/i386', 142 ).
file('xm-linux.h',text,'gcc-2.4.5/config/i386', 514 ).
file('xm-next.h',text,'gcc-2.4.5/config/i386', 131 ).
file('xm-sco.h',text,'gcc-2.4.5/config/i386', 548 ).
file('xm-sun.h',text,'gcc-2.4.5/config/i386', 949 ).
file('xm-sysv3.h',text,'gcc-2.4.5/config/i386', 120 ).
file('xm-sysv4.h',text,'gcc-2.4.5/config/i386', 384 ).
file('bsd-gas.h',text,'gcc-2.4.5/config/i860', 83 ).
file('bsd.h',text,'gcc-2.4.5/config/i860', 1070 ).
file('fx2800.h',text,'gcc-2.4.5/config/i860', 11068 ).
file('i860.c',text,'gcc-2.4.5/config/i860', 62064 ).
file('i860.h',text,'gcc-2.4.5/config/i860', 54812 ).
file('i860.md',text,'gcc-2.4.5/config/i860', 65263 ).
file('mach.h',text,'gcc-2.4.5/config/i860', 525 ).
file('sysv3.h',text,'gcc-2.4.5/config/i860', 5785 ).
file('sysv4.h',text,'gcc-2.4.5/config/i860', 6862 ).
file('t-fx2800',text,'gcc-2.4.5/config/i860', 309 ).
file('x-fx2800',text,'gcc-2.4.5/config/i860', 253 ).
file('x-sysv3',text,'gcc-2.4.5/config/i860', 16 ).
file('x-sysv4',text,'gcc-2.4.5/config/i860', 2403 ).
file('xm-fx2800.h',text,'gcc-2.4.5/config/i860', 325 ).
file('xm-i860.h',text,'gcc-2.4.5/config/i860', 1435 ).
file('xm-sysv3.h',text,'gcc-2.4.5/config/i860', 120 ).
file('xm-sysv4.h',text,'gcc-2.4.5/config/i860', 120 ).
file('i960.c',text,'gcc-2.4.5/config/i960', 58321 ).
file('i960.h',text,'gcc-2.4.5/config/i960', 57183 ).
file('i960.md',text,'gcc-2.4.5/config/i960', 77994 ).
file('xm-i960.h',text,'gcc-2.4.5/config/i960', 1432 ).
file('local.h',text,'gcc-2.4.5/config/m6811', 43425 ).
file('m6811.c',text,'gcc-2.4.5/config/m6811', 11183 ).
file('m6811.md',text,'gcc-2.4.5/config/m6811', 58079 ).
file('xm-local.h',text,'gcc-2.4.5/config/m6811', 1577 ).
file('3b1.h',text,'gcc-2.4.5/config/m68k', 17341 ).
file('3b1g.h',text,'gcc-2.4.5/config/m68k', 2504 ).
file('altos3068.h',text,'gcc-2.4.5/config/m68k', 4227 ).
file('amix.h',text,'gcc-2.4.5/config/m68k', 5477 ).
file('apollo68.h',text,'gcc-2.4.5/config/m68k', 6632 ).
file('crds.h',text,'gcc-2.4.5/config/m68k', 22328 ).
file('ctix.h',text,'gcc-2.4.5/config/m68k', 1698 ).
file('dpx2.h',text,'gcc-2.4.5/config/m68k', 3949 ).
file('dpx2.ifile',text,'gcc-2.4.5/config/m68k', 1768 ).
file('dpx2g.h',text,'gcc-2.4.5/config/m68k', 2214 ).
file('hp2bsd.h',text,'gcc-2.4.5/config/m68k', 2684 ).
file('hp310.h',text,'gcc-2.4.5/config/m68k', 964 ).
file('hp310g.h',text,'gcc-2.4.5/config/m68k', 365 ).
file('hp320.h',text,'gcc-2.4.5/config/m68k', 22784 ).
file('hp320g.h',text,'gcc-2.4.5/config/m68k', 371 ).
file('hp3bsd.h',text,'gcc-2.4.5/config/m68k', 1267 ).
file('hp3bsd44.h',text,'gcc-2.4.5/config/m68k', 1475 ).
file('isi-nfp.h',text,'gcc-2.4.5/config/m68k', 209 ).
file('isi.h',text,'gcc-2.4.5/config/m68k', 3112 ).
file('m68k.c',text,'gcc-2.4.5/config/m68k', 58018 ).
file('m68k.h',text,'gcc-2.4.5/config/m68k', 67262 ).
file('m68k.md',text,'gcc-2.4.5/config/m68k', 164331 ).
file('m68kv4.h',text,'gcc-2.4.5/config/m68k', 10609 ).
file('mot3300.h',text,'gcc-2.4.5/config/m68k', 29003 ).
file('news.h',text,'gcc-2.4.5/config/m68k', 16802 ).
file('news3.h',text,'gcc-2.4.5/config/m68k', 147 ).
file('news3gas.h',text,'gcc-2.4.5/config/m68k', 150 ).
file('newsgas.h',text,'gcc-2.4.5/config/m68k', 246 ).
file('next.c',text,'gcc-2.4.5/config/m68k', 234 ).
file('next.h',text,'gcc-2.4.5/config/m68k', 6309 ).
file('pbb.h',text,'gcc-2.4.5/config/m68k', 5330 ).
file('plexus.h',text,'gcc-2.4.5/config/m68k', 3215 ).
file('sgs.h',text,'gcc-2.4.5/config/m68k', 15895 ).
file('sun2.h',text,'gcc-2.4.5/config/m68k', 2486 ).
file('sun2o4.h',text,'gcc-2.4.5/config/m68k', 5953 ).
file('sun3.h',text,'gcc-2.4.5/config/m68k', 9180 ).
file('sun3mach.h',text,'gcc-2.4.5/config/m68k', 488 ).
file('sun3n.h',text,'gcc-2.4.5/config/m68k', 199 ).
file('sun3n3.h',text,'gcc-2.4.5/config/m68k', 88 ).
file('sun3o3.h',text,'gcc-2.4.5/config/m68k', 87 ).
file('t-hp320g',text,'gcc-2.4.5/config/m68k', 219 ).
file('t-next',text,'gcc-2.4.5/config/m68k', 190 ).
file('tower-as.h',text,'gcc-2.4.5/config/m68k', 23956 ).
file('tower.h',text,'gcc-2.4.5/config/m68k', 3426 ).
file('x-alloca-c',text,'gcc-2.4.5/config/m68k', 16 ).
file('x-amix',text,'gcc-2.4.5/config/m68k', 963 ).
file('x-apollo68',text,'gcc-2.4.5/config/m68k', 672 ).
file('x-crds',text,'gcc-2.4.5/config/m68k', 239 ).
file('x-dpx2',text,'gcc-2.4.5/config/m68k', 629 ).
file('x-hp2bsd',text,'gcc-2.4.5/config/m68k', 139 ).
file('x-hp320',text,'gcc-2.4.5/config/m68k', 499 ).
file('x-hp320g',text,'gcc-2.4.5/config/m68k', 485 ).
file('x-hp3bsd44',text,'gcc-2.4.5/config/m68k', 39 ).
file('x-next',text,'gcc-2.4.5/config/m68k', 186 ).
file('x-tower',text,'gcc-2.4.5/config/m68k', 472 ).
file('xm-3b1.h',text,'gcc-2.4.5/config/m68k', 189 ).
file('xm-altos3068.h',text,'gcc-2.4.5/config/m68k', 145 ).
file('xm-amix.h',text,'gcc-2.4.5/config/m68k', 1518 ).
file('xm-crds.h',text,'gcc-2.4.5/config/m68k', 363 ).
file('xm-hp320.h',text,'gcc-2.4.5/config/m68k', 461 ).
file('xm-m68k.h',text,'gcc-2.4.5/config/m68k', 1550 ).
file('xm-m68kv.h',text,'gcc-2.4.5/config/m68k', 290 ).
file('xm-mot3300.h',text,'gcc-2.4.5/config/m68k', 260 ).
file('xm-next.h',text,'gcc-2.4.5/config/m68k', 131 ).
file('xm-plexus.h',text,'gcc-2.4.5/config/m68k', 352 ).
file('xm-tower.h',text,'gcc-2.4.5/config/m68k', 69 ).
file('dgux.h',text,'gcc-2.4.5/config/m88k', 8570 ).
file('dgux.ld',text,'gcc-2.4.5/config/m88k', 2243 ).
file('dolph.h',text,'gcc-2.4.5/config/m88k', 1425 ).
file('luna.h',text,'gcc-2.4.5/config/m88k', 2090 ).
file('m88k-move.sh',exec,'gcc-2.4.5/config/m88k', 10583 ).
file('m88k.c',text,'gcc-2.4.5/config/m88k', 86819 ).
file('m88k.h',text,'gcc-2.4.5/config/m88k', 99417 ).
file('m88k.md',text,'gcc-2.4.5/config/m88k', 108224 ).
file('sysv3.h',text,'gcc-2.4.5/config/m88k', 5181 ).
file('sysv4.h',text,'gcc-2.4.5/config/m88k', 2264 ).
file('t-dgux',text,'gcc-2.4.5/config/m88k', 682 ).
file('t-dgux-gas',text,'gcc-2.4.5/config/m88k', 705 ).
file('t-luna',text,'gcc-2.4.5/config/m88k', 344 ).
file('t-luna-gas',text,'gcc-2.4.5/config/m88k', 367 ).
file('t-m88k',text,'gcc-2.4.5/config/m88k', 506 ).
file('t-m88k-gas',text,'gcc-2.4.5/config/m88k', 529 ).
file('t-sysv4',text,'gcc-2.4.5/config/m88k', 560 ).
file('tekXD88.h',text,'gcc-2.4.5/config/m88k', 1862 ).
file('tekXD88.ld',text,'gcc-2.4.5/config/m88k', 1352 ).
file('x-dgux',text,'gcc-2.4.5/config/m88k', 78 ).
file('x-dolph',text,'gcc-2.4.5/config/m88k', 118 ).
file('x-sysv4',text,'gcc-2.4.5/config/m88k', 385 ).
file('x-tekXD88',text,'gcc-2.4.5/config/m88k', 378 ).
file('xm-m88k.h',text,'gcc-2.4.5/config/m88k', 2095 ).
file('xm-sysv3.h',text,'gcc-2.4.5/config/m88k', 996 ).
file('bsd-4-gdb.h',text,'gcc-2.4.5/config/mips', 898 ).
file('bsd-4.h',text,'gcc-2.4.5/config/mips', 2066 ).
file('bsd-5-gdb.h',text,'gcc-2.4.5/config/mips', 910 ).
file('bsd-5.h',text,'gcc-2.4.5/config/mips', 2177 ).
file('dec-gosf1.h',text,'gcc-2.4.5/config/mips', 945 ).
file('dec-osf1.h',text,'gcc-2.4.5/config/mips', 1706 ).
file('iris3-gdb.h',text,'gcc-2.4.5/config/mips', 894 ).
file('iris3.h',text,'gcc-2.4.5/config/mips', 4927 ).
file('iris4-gdb.h',text,'gcc-2.4.5/config/mips', 917 ).
file('iris4.h',text,'gcc-2.4.5/config/mips', 1457 ).
file('iris4gl.h',text,'gcc-2.4.5/config/mips', 664 ).
file('iris4loser.h',text,'gcc-2.4.5/config/mips', 663 ).
file('mips-4-gdb.h',text,'gcc-2.4.5/config/mips', 901 ).
file('mips-5-gdb.h',text,'gcc-2.4.5/config/mips', 915 ).
file('mips-5.h',text,'gcc-2.4.5/config/mips', 1112 ).
file('mips.c',text,'gcc-2.4.5/config/mips', 125204 ).
file('mips.h',text,'gcc-2.4.5/config/mips', 128131 ).
file('mips.md',text,'gcc-2.4.5/config/mips', 124782 ).
file('news4-gdb.h',text,'gcc-2.4.5/config/mips', 903 ).
file('news4.h',text,'gcc-2.4.5/config/mips', 2307 ).
file('news5-gdb.h',text,'gcc-2.4.5/config/mips', 912 ).
file('news5.h',text,'gcc-2.4.5/config/mips', 2460 ).
file('nws3250v4.h',text,'gcc-2.4.5/config/mips', 2225 ).
file('osfrose.h',text,'gcc-2.4.5/config/mips', 5681 ).
file('svr3-4-gdb.h',text,'gcc-2.4.5/config/mips', 904 ).
file('svr3-4.h',text,'gcc-2.4.5/config/mips', 2511 ).
file('svr3-5-gdb.h',text,'gcc-2.4.5/config/mips', 916 ).
file('svr3-5.h',text,'gcc-2.4.5/config/mips', 2622 ).
file('svr4-4-gdb.h',text,'gcc-2.4.5/config/mips', 906 ).
file('svr4-4.h',text,'gcc-2.4.5/config/mips', 2506 ).
file('svr4-5-gdb.h',text,'gcc-2.4.5/config/mips', 918 ).
file('svr4-5.h',text,'gcc-2.4.5/config/mips', 2673 ).
file('t-mips',text,'gcc-2.4.5/config/mips', 733 ).
file('t-mips-gas',text,'gcc-2.4.5/config/mips', 140 ).
file('t-osfrose',text,'gcc-2.4.5/config/mips', 430 ).
file('t-ultrix',text,'gcc-2.4.5/config/mips', 774 ).
file('ultrix-gdb.h',text,'gcc-2.4.5/config/mips', 910 ).
file('ultrix.h',text,'gcc-2.4.5/config/mips', 2843 ).
file('x-dec-osf1',text,'gcc-2.4.5/config/mips', 637 ).
file('x-iris',text,'gcc-2.4.5/config/mips', 876 ).
file('x-mips',text,'gcc-2.4.5/config/mips', 681 ).
file('x-nws3250v4',text,'gcc-2.4.5/config/mips', 581 ).
file('x-osfrose',text,'gcc-2.4.5/config/mips', 983 ).
file('x-sony',text,'gcc-2.4.5/config/mips', 595 ).
file('x-sysv',text,'gcc-2.4.5/config/mips', 683 ).
file('x-ultrix',text,'gcc-2.4.5/config/mips', 612 ).
file('xm-iris3.h',text,'gcc-2.4.5/config/mips', 146 ).
file('xm-iris4.h',text,'gcc-2.4.5/config/mips', 444 ).
file('xm-mips.h',text,'gcc-2.4.5/config/mips', 2473 ).
file('xm-news.h',text,'gcc-2.4.5/config/mips', 206 ).
file('xm-nws3250v4.h',text,'gcc-2.4.5/config/mips', 178 ).
file('xm-sysv.h',text,'gcc-2.4.5/config/mips', 1241 ).
file('encore.h',text,'gcc-2.4.5/config/ns32k', 6182 ).
file('genix.h',text,'gcc-2.4.5/config/ns32k', 6206 ).
file('merlin.h',text,'gcc-2.4.5/config/ns32k', 7556 ).
file('ns32k.c',text,'gcc-2.4.5/config/ns32k', 16630 ).
file('ns32k.h',text,'gcc-2.4.5/config/ns32k', 49714 ).
file('ns32k.md',text,'gcc-2.4.5/config/ns32k', 68305 ).
file('pc532-mach.h',text,'gcc-2.4.5/config/ns32k', 1110 ).
file('pc532-min.h',text,'gcc-2.4.5/config/ns32k', 1478 ).
file('pc532.h',text,'gcc-2.4.5/config/ns32k', 2369 ).
file('sequent.h',text,'gcc-2.4.5/config/ns32k', 2350 ).
file('tek6000.h',text,'gcc-2.4.5/config/ns32k', 7837 ).
file('tek6100.h',text,'gcc-2.4.5/config/ns32k', 153 ).
file('tek6200.h',text,'gcc-2.4.5/config/ns32k', 152 ).
file('x-genix',text,'gcc-2.4.5/config/ns32k', 145 ).
file('xm-genix.h',text,'gcc-2.4.5/config/ns32k', 195 ).
file('xm-ns32k.h',text,'gcc-2.4.5/config/ns32k', 1534 ).
file('xm-pc532-min.h',text,'gcc-2.4.5/config/ns32k', 1043 ).
file('pa-ghpux.h',text,'gcc-2.4.5/config/pa', 1423 ).
file('pa-gux7.h',text,'gcc-2.4.5/config/pa', 1366 ).
file('pa-hpux.h',text,'gcc-2.4.5/config/pa', 1504 ).
file('pa-hpux7.h',text,'gcc-2.4.5/config/pa', 1447 ).
file('pa-utahmach.h',text,'gcc-2.4.5/config/pa', 1503 ).
file('pa.c',text,'gcc-2.4.5/config/pa', 93406 ).
file('pa.h',text,'gcc-2.4.5/config/pa', 69211 ).
file('pa.md',text,'gcc-2.4.5/config/pa', 93961 ).
file('pa1-ghpux.h',text,'gcc-2.4.5/config/pa', 1102 ).
file('pa1-hpux.h',text,'gcc-2.4.5/config/pa', 1098 ).
file('pa1-oldas.h',text,'gcc-2.4.5/config/pa', 1073 ).
file('pa1-utahmach.h',text,'gcc-2.4.5/config/pa', 1518 ).
file('pa1.h',text,'gcc-2.4.5/config/pa', 1007 ).
file('t-pa',text,'gcc-2.4.5/config/pa', 81 ).
file('x-pa',text,'gcc-2.4.5/config/pa', 100 ).
file('x-pa-hpux',text,'gcc-2.4.5/config/pa', 16 ).
file('xm-pa.h',text,'gcc-2.4.5/config/pa', 1732 ).
file('xm-pahpux.h',text,'gcc-2.4.5/config/pa', 1952 ).
file('pyr.c',text,'gcc-2.4.5/config/pyr', 21766 ).
file('pyr.h',text,'gcc-2.4.5/config/pyr', 55664 ).
file('pyr.md',text,'gcc-2.4.5/config/pyr', 39168 ).
file('x-pyr',text,'gcc-2.4.5/config/pyr', 31 ).
file('xm-pyr.h',text,'gcc-2.4.5/config/pyr', 1458 ).
file('romp.c',text,'gcc-2.4.5/config/romp', 49388 ).
file('romp.h',text,'gcc-2.4.5/config/romp', 63046 ).
file('romp.md',text,'gcc-2.4.5/config/romp', 85671 ).
file('x-mach',text,'gcc-2.4.5/config/romp', 153 ).
file('x-romp',text,'gcc-2.4.5/config/romp', 423 ).
file('xm-romp.h',text,'gcc-2.4.5/config/romp', 1861 ).
file('aix32.h',text,'gcc-2.4.5/config/rs6000', 1032 ).
file('mach.h',text,'gcc-2.4.5/config/rs6000', 1470 ).
file('rs6000.c',text,'gcc-2.4.5/config/rs6000', 50546 ).
file('rs6000.h',text,'gcc-2.4.5/config/rs6000', 69915 ).
file('rs6000.md',text,'gcc-2.4.5/config/rs6000', 150546 ).
file('x-mach',text,'gcc-2.4.5/config/rs6000', 157 ).
file('x-rs6000',text,'gcc-2.4.5/config/rs6000', 330 ).
file('xm-mach.h',text,'gcc-2.4.5/config/rs6000', 887 ).
file('xm-rs6000.h',text,'gcc-2.4.5/config/rs6000', 1776 ).
file('sh.c',text,'gcc-2.4.5/config/sh', 28338 ).
file('sh.h',text,'gcc-2.4.5/config/sh', 39324 ).
file('sh.md',text,'gcc-2.4.5/config/sh', 34491 ).
file('t-sh',text,'gcc-2.4.5/config/sh', 493 ).
file('xm-sh.h',text,'gcc-2.4.5/config/sh', 1367 ).
file('gmon-sol2.c',text,'gcc-2.4.5/config/sparc', 11368 ).
file('lite.h',text,'gcc-2.4.5/config/sparc', 1143 ).
file('pbd.h',text,'gcc-2.4.5/config/sparc', 6563 ).
file('sol2-c1.asm',text,'gcc-2.4.5/config/sparc', 2782 ).
file('sol2-ci.asm',text,'gcc-2.4.5/config/sparc', 2163 ).
file('sol2-cn.asm',text,'gcc-2.4.5/config/sparc', 1999 ).
file('sol2.h',text,'gcc-2.4.5/config/sparc', 7765 ).
file('sparc.c',text,'gcc-2.4.5/config/sparc', 99132 ).
file('sparc.h',text,'gcc-2.4.5/config/sparc', 70379 ).
file('sparc.md',text,'gcc-2.4.5/config/sparc', 106076 ).
file('sun4o3.h',text,'gcc-2.4.5/config/sparc', 404 ).
file('sysv4.h',text,'gcc-2.4.5/config/sparc', 7713 ).
file('t-sol2',text,'gcc-2.4.5/config/sparc', 779 ).
file('x-sysv4',text,'gcc-2.4.5/config/sparc', 32 ).
file('xm-pbd.h',text,'gcc-2.4.5/config/sparc', 255 ).
file('xm-sol2.h',text,'gcc-2.4.5/config/sparc', 348 ).
file('xm-sparc.h',text,'gcc-2.4.5/config/sparc', 1824 ).
file('xm-sysv4.h',text,'gcc-2.4.5/config/sparc', 1786 ).
file('spur.c',text,'gcc-2.4.5/config/spur', 9463 ).
file('spur.h',text,'gcc-2.4.5/config/spur', 39109 ).
file('spur.md',text,'gcc-2.4.5/config/spur', 32080 ).
file('xm-spur.h',text,'gcc-2.4.5/config/spur', 1414 ).
file('harris.h',text,'gcc-2.4.5/config/tahoe', 2672 ).
file('tahoe.c',text,'gcc-2.4.5/config/tahoe', 14072 ).
file('tahoe.h',text,'gcc-2.4.5/config/tahoe', 30244 ).
file('tahoe.md',text,'gcc-2.4.5/config/tahoe', 54771 ).
file('xm-tahoe.h',text,'gcc-2.4.5/config/tahoe', 1774 ).
file('ultrix.h',text,'gcc-2.4.5/config/vax', 459 ).
file('vax.c',text,'gcc-2.4.5/config/vax', 17800 ).
file('vax.h',text,'gcc-2.4.5/config/vax', 47111 ).
file('vax.md',text,'gcc-2.4.5/config/vax', 58941 ).
file('vaxv.h',text,'gcc-2.4.5/config/vax', 2077 ).
file('vms.h',text,'gcc-2.4.5/config/vax', 14206 ).
file('x-vax',text,'gcc-2.4.5/config/vax', 131 ).
file('xm-vax.h',text,'gcc-2.4.5/config/vax', 1554 ).
file('xm-vaxv.h',text,'gcc-2.4.5/config/vax', 189 ).
file('xm-vms.h',text,'gcc-2.4.5/config/vax', 6791 ).
file('we32k.c',text,'gcc-2.4.5/config/we32k', 4062 ).
file('we32k.h',text,'gcc-2.4.5/config/we32k', 36855 ).
file('we32k.md',text,'gcc-2.4.5/config/we32k', 35561 ).
file('x-we32k',text,'gcc-2.4.5/config/we32k', 16 ).
file('xm-we32k.h',text,'gcc-2.4.5/config/we32k', 1687 ).
file('Makefile',text,'gcc-2.4.5/objc', 3073 ).
file('Object.h',text,'gcc-2.4.5/objc', 3402 ).
file('Object.m',text,'gcc-2.4.5/objc', 7620 ).
file('Protocol.h',text,'gcc-2.4.5/objc', 1843 ).
file('Protocol.m',text,'gcc-2.4.5/objc', 3418 ).
file('README',text,'gcc-2.4.5/objc', 4725 ).
file('archive.c',text,'gcc-2.4.5/objc', 35259 ).
file('class.c',text,'gcc-2.4.5/objc', 12123 ).
file('hash.c',text,'gcc-2.4.5/objc', 6832 ).
file('hash.h',text,'gcc-2.4.5/objc', 6165 ).
file('init.c',text,'gcc-2.4.5/objc', 8517 ).
file('list.h',text,'gcc-2.4.5/objc', 3442 ).
file('misc.c',text,'gcc-2.4.5/objc', 1999 ).
file('mutex.h',text,'gcc-2.4.5/objc', 2676 ).
file('objc-api.h',text,'gcc-2.4.5/objc', 15155 ).
file('objc.h',text,'gcc-2.4.5/objc', 5955 ).
file('objects.c',text,'gcc-2.4.5/objc', 2452 ).
file('runtime.h',text,'gcc-2.4.5/objc', 2767 ).
file('sarray.c',text,'gcc-2.4.5/objc', 11994 ).
file('sarray.h',text,'gcc-2.4.5/objc', 5931 ).
file('selector.c',text,'gcc-2.4.5/objc', 4201 ).
file('sendmsg.c',text,'gcc-2.4.5/objc', 13400 ).
file('todo',text,'gcc-2.4.5/objc', 189 ).
file('typedstream.h',text,'gcc-2.4.5/objc', 4239 ).
file('xforward.c',text,'gcc-2.4.5/objc', 1886 ).
file('COPYING',text,'gdb-4.10.pl1', 17982 ).
file('COPYING.LIB',text,'gdb-4.10.pl1', 25265 ).
file('Makefile.in',text,'gdb-4.10.pl1', 29849 ).
file('README',text,'gdb-4.10.pl1', 1190 ).
file('bfd',dir,'gdb-4.10.pl1', 3072 ).
file('config',dir,'gdb-4.10.pl1', 512 ).
file('config.guess',exec,'gdb-4.10.pl1', 5597 ).
file('config.sub',exec,'gdb-4.10.pl1', 17202 ).
file('configure',exec,'gdb-4.10.pl1', 37782 ).
file('configure.in',text,'gdb-4.10.pl1', 8296 ).
file('etc',dir,'gdb-4.10.pl1', 512 ).
file('gdb',dir,'gdb-4.10.pl1', 5120 ).
file('glob',dir,'gdb-4.10.pl1', 512 ).
file('include',dir,'gdb-4.10.pl1', 512 ).
file('install.sh',exec,'gdb-4.10.pl1', 3768 ).
file('libiberty',dir,'gdb-4.10.pl1', 1536 ).
file('mmalloc',dir,'gdb-4.10.pl1', 512 ).
file('move-if-change',exec,'gdb-4.10.pl1', 129 ).
file('opcodes',dir,'gdb-4.10.pl1', 1024 ).
file('readline',dir,'gdb-4.10.pl1', 1024 ).
file('sim',dir,'gdb-4.10.pl1', 512 ).
file('texinfo',dir,'gdb-4.10.pl1', 512 ).
file('COPYING',text,'gdb-4.10.pl1/bfd', 17982 ).
file('ChangeLog',text,'gdb-4.10.pl1/bfd', 203102 ).
file('Makefile.in',text,'gdb-4.10.pl1/bfd', 18228 ).
file('PORTING',text,'gdb-4.10.pl1/bfd', 2154 ).
file('README.hppaelf',text,'gdb-4.10.pl1/bfd', 774 ).
file('TODO',text,'gdb-4.10.pl1/bfd', 1363 ).
file('VERSION',text,'gdb-4.10.pl1/bfd', 4 ).
file('aix386-core.c',text,'gdb-4.10.pl1/bfd', 11233 ).
file('aout-adobe.c',text,'gdb-4.10.pl1/bfd', 16201 ).
file('aout-encap.c',text,'gdb-4.10.pl1/bfd', 6873 ).
file('aout-target.h',text,'gdb-4.10.pl1/bfd', 12809 ).
file('aout32.c',text,'gdb-4.10.pl1/bfd', 898 ).
file('aout64.c',text,'gdb-4.10.pl1/bfd', 901 ).
file('aoutf1.h',text,'gdb-4.10.pl1/bfd', 19499 ).
file('aoutx.h',text,'gdb-4.10.pl1/bfd', 77612 ).
file('archive.c',text,'gdb-4.10.pl1/bfd', 48029 ).
file('archures.c',text,'gdb-4.10.pl1/bfd', 16282 ).
file('bfd-in.h',text,'gdb-4.10.pl1/bfd', 14700 ).
file('bfd-in2.h',text,'gdb-4.10.pl1/bfd', 58082 ).
file('bfd.c',text,'gdb-4.10.pl1/bfd', 17005 ).
file('bout.c',text,'gdb-4.10.pl1/bfd', 39596 ).
file('cache.c',text,'gdb-4.10.pl1/bfd', 7120 ).
file('coff-a29k.c',text,'gdb-4.10.pl1/bfd', 10497 ).
file('coff-alpha.c',text,'gdb-4.10.pl1/bfd', 121922 ).
file('coff-h8300.c',text,'gdb-4.10.pl1/bfd', 8126 ).
file('coff-h8500.c',text,'gdb-4.10.pl1/bfd', 9001 ).
file('coff-i386.c',text,'gdb-4.10.pl1/bfd', 12568 ).
file('coff-i960.c',text,'gdb-4.10.pl1/bfd', 7294 ).
file('coff-m68k.c',text,'gdb-4.10.pl1/bfd', 4987 ).
file('coff-m88k.c',text,'gdb-4.10.pl1/bfd', 4643 ).
file('coff-mips.c',text,'gdb-4.10.pl1/bfd', 130747 ).
file('coff-msym.c',text,'gdb-4.10.pl1/bfd', 29715 ).
file('coff-rs6000.c',text,'gdb-4.10.pl1/bfd', 9115 ).
file('coff-sh.c',text,'gdb-4.10.pl1/bfd', 5041 ).
file('coff-u68k.c',text,'gdb-4.10.pl1/bfd', 1338 ).
file('coff-we32k.c',text,'gdb-4.10.pl1/bfd', 3541 ).
file('coff-z8k.c',text,'gdb-4.10.pl1/bfd', 6957 ).
file('coffcode.h',text,'gdb-4.10.pl1/bfd', 64400 ).
file('coffgen.c',text,'gdb-4.10.pl1/bfd', 41588 ).
file('coffswap.h',text,'gdb-4.10.pl1/bfd', 21957 ).
file('config',dir,'gdb-4.10.pl1/bfd', 2048 ).
file('configure.bat',text,'gdb-4.10.pl1/bfd', 338 ).
file('configure.host',text,'gdb-4.10.pl1/bfd', 2749 ).
file('configure.in',text,'gdb-4.10.pl1/bfd', 5911 ).
file('core.c',text,'gdb-4.10.pl1/bfd', 2518 ).
file('cpu-a29k.c',text,'gdb-4.10.pl1/bfd', 1330 ).
file('cpu-alpha.c',text,'gdb-4.10.pl1/bfd', 1280 ).
file('cpu-h8300.c',text,'gdb-4.10.pl1/bfd', 6451 ).
file('cpu-h8500.c',text,'gdb-4.10.pl1/bfd', 5350 ).
file('cpu-hppa.c',text,'gdb-4.10.pl1/bfd', 1303 ).
file('cpu-i386.c',text,'gdb-4.10.pl1/bfd', 1280 ).
file('cpu-i960.c',text,'gdb-4.10.pl1/bfd', 4519 ).
file('cpu-m68k.c',text,'gdb-4.10.pl1/bfd', 1592 ).
file('cpu-m88k.c',text,'gdb-4.10.pl1/bfd', 1357 ).
file('cpu-mips.c',text,'gdb-4.10.pl1/bfd', 1680 ).
file('cpu-rs6000.c',text,'gdb-4.10.pl1/bfd', 1370 ).
file('cpu-sh.c',text,'gdb-4.10.pl1/bfd', 1984 ).
file('cpu-sparc.c',text,'gdb-4.10.pl1/bfd', 1280 ).
file('cpu-vax.c',text,'gdb-4.10.pl1/bfd', 1321 ).
file('cpu-we32k.c',text,'gdb-4.10.pl1/bfd', 1361 ).
file('cpu-z8k.c',text,'gdb-4.10.pl1/bfd', 5252 ).
file('ctor.c',text,'gdb-4.10.pl1/bfd', 5275 ).
file('demo64.c',text,'gdb-4.10.pl1/bfd', 975 ).
file('doc',dir,'gdb-4.10.pl1/bfd', 512 ).
file('elf.c',text,'gdb-4.10.pl1/bfd', 6232 ).
file('elf32-gen.c',text,'gdb-4.10.pl1/bfd', 1367 ).
file('elf32-hppa.c',text,'gdb-4.10.pl1/bfd', 85811 ).
file('elf32-hppa.h',text,'gdb-4.10.pl1/bfd', 18035 ).
file('elf32-i386.c',text,'gdb-4.10.pl1/bfd', 3611 ).
file('elf32-i860.c',text,'gdb-4.10.pl1/bfd', 1163 ).
file('elf32-m68k.c',text,'gdb-4.10.pl1/bfd', 1164 ).
file('elf32-m88k.c',text,'gdb-4.10.pl1/bfd', 1254 ).
file('elf32-mips.c',text,'gdb-4.10.pl1/bfd', 1353 ).
file('elf32-sparc.c',text,'gdb-4.10.pl1/bfd', 7120 ).
file('elf32-target.h',text,'gdb-4.10.pl1/bfd', 8025 ).
file('elf32.c',text,'gdb-4.10.pl1/bfd', 865 ).
file('elf64-gen.c',text,'gdb-4.10.pl1/bfd', 1367 ).
file('elf64-target.h',text,'gdb-4.10.pl1/bfd', 8057 ).
file('elf64.c',text,'gdb-4.10.pl1/bfd', 864 ).
file('elfcode.h',text,'gdb-4.10.pl1/bfd', 106525 ).
file('filemode.c',text,'gdb-4.10.pl1/bfd', 4489 ).
file('format.c',text,'gdb-4.10.pl1/bfd', 7016 ).
file('gen-aout.c',text,'gdb-4.10.pl1/bfd', 2592 ).
file('host-aout.c',text,'gdb-4.10.pl1/bfd', 2292 ).
file('hosts',dir,'gdb-4.10.pl1/bfd', 1024 ).
file('hp300bsd.c',text,'gdb-4.10.pl1/bfd', 1315 ).
file('hp300hpux.c',text,'gdb-4.10.pl1/bfd', 26297 ).
file('hppa.c',text,'gdb-4.10.pl1/bfd', 23905 ).
file('hppa_stubs.h',text,'gdb-4.10.pl1/bfd', 3204 ).
file('hpux-core.c',text,'gdb-4.10.pl1/bfd', 9871 ).
file('i386aout.c',text,'gdb-4.10.pl1/bfd', 2075 ).
file('i386bsd.c',text,'gdb-4.10.pl1/bfd', 1514 ).
file('i386linux.c',text,'gdb-4.10.pl1/bfd', 1538 ).
file('i386lynx.c',text,'gdb-4.10.pl1/bfd', 1204 ).
file('ieee.c',text,'gdb-4.10.pl1/bfd', 73766 ).
file('init.c',text,'gdb-4.10.pl1/bfd', 1780 ).
file('libaout.h',text,'gdb-4.10.pl1/bfd', 12822 ).
file('libbfd-in.h',text,'gdb-4.10.pl1/bfd', 8017 ).
file('libbfd.c',text,'gdb-4.10.pl1/bfd', 20193 ).
file('libbfd.h',text,'gdb-4.10.pl1/bfd', 9743 ).
file('libcoff-in.h',text,'gdb-4.10.pl1/bfd', 4633 ).
file('libcoff.h',text,'gdb-4.10.pl1/bfd', 11993 ).
file('libecoff.h',text,'gdb-4.10.pl1/bfd', 3391 ).
file('libelf.h',text,'gdb-4.10.pl1/bfd', 9227 ).
file('libhppa.h',text,'gdb-4.10.pl1/bfd', 5096 ).
file('libieee.h',text,'gdb-4.10.pl1/bfd', 3422 ).
file('libnlm.h',text,'gdb-4.10.pl1/bfd', 6171 ).
file('liboasys.h',text,'gdb-4.10.pl1/bfd', 2225 ).
file('mipsbsd.c',text,'gdb-4.10.pl1/bfd', 12552 ).
file('newsos3.c',text,'gdb-4.10.pl1/bfd', 1448 ).
file('nlm-target.h',text,'gdb-4.10.pl1/bfd', 9050 ).
file('nlm.c',text,'gdb-4.10.pl1/bfd', 1627 ).
file('nlm32-gen.c',text,'gdb-4.10.pl1/bfd', 1289 ).
file('nlm32-i386.c',text,'gdb-4.10.pl1/bfd', 5811 ).
file('nlm32.c',text,'gdb-4.10.pl1/bfd', 892 ).
file('nlm64-gen.c',text,'gdb-4.10.pl1/bfd', 1289 ).
file('nlm64.c',text,'gdb-4.10.pl1/bfd', 892 ).
file('nlmcode.h',text,'gdb-4.10.pl1/bfd', 61133 ).
file('oasys.c',text,'gdb-4.10.pl1/bfd', 35854 ).
file('opncls.c',text,'gdb-4.10.pl1/bfd', 12336 ).
file('reloc.c',text,'gdb-4.10.pl1/bfd', 33458 ).
file('reloc16.c',text,'gdb-4.10.pl1/bfd', 9952 ).
file('rs6000-core.c',text,'gdb-4.10.pl1/bfd', 12103 ).
file('sco-core.c',text,'gdb-4.10.pl1/bfd', 10639 ).
file('seclet.c',text,'gdb-4.10.pl1/bfd', 4935 ).
file('seclet.h',text,'gdb-4.10.pl1/bfd', 1376 ).
file('section.c',text,'gdb-4.10.pl1/bfd', 23993 ).
file('srec.c',text,'gdb-4.10.pl1/bfd', 23379 ).
file('stab-syms.c',text,'gdb-4.10.pl1/bfd', 2168 ).
file('sunos.c',text,'gdb-4.10.pl1/bfd', 959 ).
file('syms.c',text,'gdb-4.10.pl1/bfd', 14324 ).
file('targets.c',text,'gdb-4.10.pl1/bfd', 18952 ).
file('tekhex.c',text,'gdb-4.10.pl1/bfd', 24485 ).
file('trad-core.c',text,'gdb-4.10.pl1/bfd', 12948 ).
file('a29k-aout.mt',text,'gdb-4.10.pl1/bfd/config', 122 ).
file('a29k-coff.mt',text,'gdb-4.10.pl1/bfd/config', 124 ).
file('adobe.mt',text,'gdb-4.10.pl1/bfd/config', 90 ).
file('alphaosf.mh',text,'gdb-4.10.pl1/bfd/config', 54 ).
file('alphaosf.mt',text,'gdb-4.10.pl1/bfd/config', 148 ).
file('amix.mh',text,'gdb-4.10.pl1/bfd/config', 114 ).
file('apollov68.mh',text,'gdb-4.10.pl1/bfd/config', 307 ).
file('bigmips.mt',text,'gdb-4.10.pl1/bfd/config', 78 ).
file('decstation.mh',text,'gdb-4.10.pl1/bfd/config', 48 ).
file('decstation.mt',text,'gdb-4.10.pl1/bfd/config', 82 ).
file('delta68.mh',text,'gdb-4.10.pl1/bfd/config', 80 ).
file('delta88.mh',text,'gdb-4.10.pl1/bfd/config', 44 ).
file('dgux.mh',text,'gdb-4.10.pl1/bfd/config', 46 ).
file('dpx2.mh',text,'gdb-4.10.pl1/bfd/config', 43 ).
file('go32.mh',text,'gdb-4.10.pl1/bfd/config', 54 ).
file('h8300-coff.mt',text,'gdb-4.10.pl1/bfd/config', 248 ).
file('h8500-coff.mt',text,'gdb-4.10.pl1/bfd/config', 206 ).
file('harris.mh',text,'gdb-4.10.pl1/bfd/config', 53 ).
file('hp300.mh',text,'gdb-4.10.pl1/bfd/config', 78 ).
file('hp300bsd.mh',text,'gdb-4.10.pl1/bfd/config', 43 ).
file('hp300bsd.mt',text,'gdb-4.10.pl1/bfd/config', 99 ).
file('hp300hpux.mt',text,'gdb-4.10.pl1/bfd/config', 43 ).
file('hppa-elf.mt',text,'gdb-4.10.pl1/bfd/config', 48 ).
file('hppabsd.mh',text,'gdb-4.10.pl1/bfd/config', 80 ).
file('hppabsd.mt',text,'gdb-4.10.pl1/bfd/config', 38 ).
file('hppahpux.mh',text,'gdb-4.10.pl1/bfd/config', 69 ).
file('hppahpux.mt',text,'gdb-4.10.pl1/bfd/config', 38 ).
file('hppaosf.mh',text,'gdb-4.10.pl1/bfd/config', 150 ).
file('i386-aout.mt',text,'gdb-4.10.pl1/bfd/config', 77 ).
file('i386-bsd.mt',text,'gdb-4.10.pl1/bfd/config', 87 ).
file('i386-coff.mt',text,'gdb-4.10.pl1/bfd/config', 77 ).
file('i386-elf.mt',text,'gdb-4.10.pl1/bfd/config', 81 ).
file('i386-linux.mt',text,'gdb-4.10.pl1/bfd/config', 90 ).
file('i386-lynx.mt',text,'gdb-4.10.pl1/bfd/config', 108 ).
file('i386-nlm.mt',text,'gdb-4.10.pl1/bfd/config', 123 ).
file('i386-sco.mt',text,'gdb-4.10.pl1/bfd/config', 187 ).
file('i386aix.mh',text,'gdb-4.10.pl1/bfd/config', 150 ).
file('i386bsd.mh',text,'gdb-4.10.pl1/bfd/config', 43 ).
file('i386isc.mh',text,'gdb-4.10.pl1/bfd/config', 37 ).
file('i386linux.mh',text,'gdb-4.10.pl1/bfd/config', 61 ).
file('i386v.mh',text,'gdb-4.10.pl1/bfd/config', 37 ).
file('i386v4.mh',text,'gdb-4.10.pl1/bfd/config', 100 ).
file('i860-elf.mt',text,'gdb-4.10.pl1/bfd/config', 80 ).
file('i960-bout.mt',text,'gdb-4.10.pl1/bfd/config', 180 ).
file('i960-coff.mt',text,'gdb-4.10.pl1/bfd/config', 195 ).
file('irix3.mh',text,'gdb-4.10.pl1/bfd/config', 47 ).
file('irix4.mh',text,'gdb-4.10.pl1/bfd/config', 62 ).
file('m68k-aout.mt',text,'gdb-4.10.pl1/bfd/config', 80 ).
file('m68k-coff.mt',text,'gdb-4.10.pl1/bfd/config', 79 ).
file('m68k-elf.mt',text,'gdb-4.10.pl1/bfd/config', 85 ).
file('m88k-coff.mt',text,'gdb-4.10.pl1/bfd/config', 142 ).
file('mipsbsd.mh',text,'gdb-4.10.pl1/bfd/config', 43 ).
file('mipsdecbsd.mt',text,'gdb-4.10.pl1/bfd/config', 104 ).
file('ncr3000.mh',text,'gdb-4.10.pl1/bfd/config', 642 ).
file('news.mh',text,'gdb-4.10.pl1/bfd/config', 43 ).
file('news.mt',text,'gdb-4.10.pl1/bfd/config', 61 ).
file('noop.mt',text,'gdb-4.10.pl1/bfd/config', 63 ).
file('riscos.mh',text,'gdb-4.10.pl1/bfd/config', 60 ).
file('riscos.mt',text,'gdb-4.10.pl1/bfd/config', 106 ).
file('rs6000.mh',text,'gdb-4.10.pl1/bfd/config', 328 ).
file('rs6000.mt',text,'gdb-4.10.pl1/bfd/config', 79 ).
file('rtbsd.mh',text,'gdb-4.10.pl1/bfd/config', 251 ).
file('sh-coff.mt',text,'gdb-4.10.pl1/bfd/config', 225 ).
file('solaris2.mh',text,'gdb-4.10.pl1/bfd/config', 72 ).
file('sparc-aout.mt',text,'gdb-4.10.pl1/bfd/config', 120 ).
file('sparc-elf.mt',text,'gdb-4.10.pl1/bfd/config', 134 ).
file('sparc-ll.mh',text,'gdb-4.10.pl1/bfd/config', 70 ).
file('st2000.mt',text,'gdb-4.10.pl1/bfd/config', 250 ).
file('stratus.mh',text,'gdb-4.10.pl1/bfd/config', 89 ).
file('symmetry.mh',text,'gdb-4.10.pl1/bfd/config', 95 ).
file('sysv4.mh',text,'gdb-4.10.pl1/bfd/config', 12 ).
file('tahoe.mh',text,'gdb-4.10.pl1/bfd/config', 63 ).
file('tahoe.mt',text,'gdb-4.10.pl1/bfd/config', 125 ).
file('u68k-coff.mt',text,'gdb-4.10.pl1/bfd/config', 103 ).
file('ultra3.mh',text,'gdb-4.10.pl1/bfd/config', 53 ).
file('vax.mt',text,'gdb-4.10.pl1/bfd/config', 118 ).
file('vaxbsd.mh',text,'gdb-4.10.pl1/bfd/config', 63 ).
file('vaxult.mh',text,'gdb-4.10.pl1/bfd/config', 63 ).
file('vaxult2.mh',text,'gdb-4.10.pl1/bfd/config', 62 ).
file('we32k.mt',text,'gdb-4.10.pl1/bfd/config', 77 ).
file('z8k-coff.mt',text,'gdb-4.10.pl1/bfd/config', 228 ).
file('ChangeLog',text,'gdb-4.10.pl1/bfd/doc', 4663 ).
file('Makefile.in',text,'gdb-4.10.pl1/bfd/doc', 9690 ).
file('bfd.texinfo',text,'gdb-4.10.pl1/bfd/doc', 10487 ).
file('bfdsumm.texi',text,'gdb-4.10.pl1/bfd/doc', 7795 ).
file('chew.c',text,'gdb-4.10.pl1/bfd/doc', 22681 ).
file('configure.in',text,'gdb-4.10.pl1/bfd/doc', 302 ).
file('doc.str',text,'gdb-4.10.pl1/bfd/doc', 1503 ).
file('proto.str',text,'gdb-4.10.pl1/bfd/doc', 545 ).
file('alphaosf.h',text,'gdb-4.10.pl1/bfd/hosts', 544 ).
file('amix.h',text,'gdb-4.10.pl1/bfd/hosts', 1194 ).
file('apollo68.h',text,'gdb-4.10.pl1/bfd/hosts', 543 ).
file('apollov68.h',text,'gdb-4.10.pl1/bfd/hosts', 843 ).
file('decstation.h',text,'gdb-4.10.pl1/bfd/hosts', 807 ).
file('delta68.h',text,'gdb-4.10.pl1/bfd/hosts', 376 ).
file('delta88.h',text,'gdb-4.10.pl1/bfd/hosts', 2023 ).
file('dgux.h',text,'gdb-4.10.pl1/bfd/hosts', 599 ).
file('dose.h',text,'gdb-4.10.pl1/bfd/hosts', 299 ).
file('dpx2.h',text,'gdb-4.10.pl1/bfd/hosts', 336 ).
file('go32.h',text,'gdb-4.10.pl1/bfd/hosts', 337 ).
file('harris.h',text,'gdb-4.10.pl1/bfd/hosts', 547 ).
file('hp300.h',text,'gdb-4.10.pl1/bfd/hosts', 707 ).
file('hp300bsd.h',text,'gdb-4.10.pl1/bfd/hosts', 1158 ).
file('hppabsd.h',text,'gdb-4.10.pl1/bfd/hosts', 791 ).
file('hppahpux.h',text,'gdb-4.10.pl1/bfd/hosts', 608 ).
file('i386aix.h',text,'gdb-4.10.pl1/bfd/hosts', 1103 ).
file('i386bsd.h',text,'gdb-4.10.pl1/bfd/hosts', 1151 ).
file('i386isc.h',text,'gdb-4.10.pl1/bfd/hosts', 964 ).
file('i386linux.h',text,'gdb-4.10.pl1/bfd/hosts', 614 ).
file('i386lynx.h',text,'gdb-4.10.pl1/bfd/hosts', 616 ).
file('i386mach.h',text,'gdb-4.10.pl1/bfd/hosts', 748 ).
file('i386v.h',text,'gdb-4.10.pl1/bfd/hosts', 802 ).
file('i386v4.h',text,'gdb-4.10.pl1/bfd/hosts', 1181 ).
file('irix3.h',text,'gdb-4.10.pl1/bfd/hosts', 413 ).
file('irix4.h',text,'gdb-4.10.pl1/bfd/hosts', 433 ).
file('miniframe.h',text,'gdb-4.10.pl1/bfd/hosts', 279 ).
file('mipsbsd.h',text,'gdb-4.10.pl1/bfd/hosts', 689 ).
file('ncr3000.h',text,'gdb-4.10.pl1/bfd/hosts', 1181 ).
file('news.h',text,'gdb-4.10.pl1/bfd/hosts', 310 ).
file('riscos.h',text,'gdb-4.10.pl1/bfd/hosts', 539 ).
file('rs6000.h',text,'gdb-4.10.pl1/bfd/hosts', 341 ).
file('rtbsd.h',text,'gdb-4.10.pl1/bfd/hosts', 368 ).
file('solaris2.h',text,'gdb-4.10.pl1/bfd/hosts', 119 ).
file('sparc-ll.h',text,'gdb-4.10.pl1/bfd/hosts', 2639 ).
file('sparc.h',text,'gdb-4.10.pl1/bfd/hosts', 297 ).
file('std-host.h',text,'gdb-4.10.pl1/bfd/hosts', 1880 ).
file('stratus.h',text,'gdb-4.10.pl1/bfd/hosts', 1185 ).
file('sun3.h',text,'gdb-4.10.pl1/bfd/hosts', 1391 ).
file('symmetry.h',text,'gdb-4.10.pl1/bfd/hosts', 554 ).
file('sysv4.h',text,'gdb-4.10.pl1/bfd/hosts', 1208 ).
file('tahoe.h',text,'gdb-4.10.pl1/bfd/hosts', 587 ).
file('ultra3.h',text,'gdb-4.10.pl1/bfd/hosts', 346 ).
file('vaxbsd.h',text,'gdb-4.10.pl1/bfd/hosts', 753 ).
file('vaxult.h',text,'gdb-4.10.pl1/bfd/hosts', 555 ).
file('vaxult2.h',text,'gdb-4.10.pl1/bfd/hosts', 876 ).
file('we32k.h',text,'gdb-4.10.pl1/bfd/hosts', 662 ).
file('ChangeLog',text,'gdb-4.10.pl1/config', 1926 ).
file('mh-a68bsd',text,'gdb-4.10.pl1/config', 90 ).
file('mh-aix',text,'gdb-4.10.pl1/config', 172 ).
file('mh-aix386',text,'gdb-4.10.pl1/config', 12 ).
file('mh-alphaosf',text,'gdb-4.10.pl1/config', 82 ).
file('mh-apollo68',text,'gdb-4.10.pl1/config', 91 ).
file('mh-decstation',text,'gdb-4.10.pl1/config', 185 ).
file('mh-delta88',text,'gdb-4.10.pl1/config', 17 ).
file('mh-dgux',text,'gdb-4.10.pl1/config', 76 ).
file('mh-hpux',text,'gdb-4.10.pl1/config', 180 ).
file('mh-irix4',text,'gdb-4.10.pl1/config', 273 ).
file('mh-linux',text,'gdb-4.10.pl1/config', 118 ).
file('mh-lynxos',text,'gdb-4.10.pl1/config', 78 ).
file('mh-ncr3000',text,'gdb-4.10.pl1/config', 710 ).
file('mh-riscos',text,'gdb-4.10.pl1/config', 640 ).
file('mh-sco',text,'gdb-4.10.pl1/config', 215 ).
file('mh-solaris',text,'gdb-4.10.pl1/config', 472 ).
file('mh-sun',text,'gdb-4.10.pl1/config', 116 ).
file('mh-sun3',text,'gdb-4.10.pl1/config', 268 ).
file('mh-sysv',text,'gdb-4.10.pl1/config', 98 ).
file('mh-sysv4',text,'gdb-4.10.pl1/config', 188 ).
file('mh-unixware',text,'gdb-4.10.pl1/config', 277 ).
file('mh-vaxult2',text,'gdb-4.10.pl1/config', 73 ).
file('Makefile.in',text,'gdb-4.10.pl1/etc', 1888 ).
file('cfg-paper.texi',text,'gdb-4.10.pl1/etc', 29217 ).
file('configure.in',text,'gdb-4.10.pl1/etc', 378 ).
file('configure.man',text,'gdb-4.10.pl1/etc', 3188 ).
file('configure.texi',text,'gdb-4.10.pl1/etc', 73534 ).
file('make-stds.texi',text,'gdb-4.10.pl1/etc', 18223 ).
file('standards.texi',text,'gdb-4.10.pl1/etc', 47797 ).
file('29k-share',dir,'gdb-4.10.pl1/gdb', 512 ).
file('COPYING',text,'gdb-4.10.pl1/gdb', 17982 ).
file('ChangeLog',text,'gdb-4.10.pl1/gdb', 175737 ).
file('ChangeLog-3.x',text,'gdb-4.10.pl1/gdb', 184506 ).
file('ChangeLog-9091',text,'gdb-4.10.pl1/gdb', 336721 ).
file('ChangeLog-92',text,'gdb-4.10.pl1/gdb', 277152 ).
file('Makefile.in',text,'gdb-4.10.pl1/gdb', 51411 ).
file('NEWS',text,'gdb-4.10.pl1/gdb', 37523 ).
file('Projects',text,'gdb-4.10.pl1/gdb', 3801 ).
file('README',text,'gdb-4.10.pl1/gdb', 21436 ).
file('TAGS',text,'gdb-4.10.pl1/gdb', 124871 ).
file('TODO',text,'gdb-4.10.pl1/gdb', 11619 ).
file('a29k-pinsn.c',text,'gdb-4.10.pl1/gdb', 1322 ).
file('a29k-tdep.c',text,'gdb-4.10.pl1/gdb', 24624 ).
file('a68v-nat.c',text,'gdb-4.10.pl1/gdb', 4539 ).
file('altos-xdep.c',text,'gdb-4.10.pl1/gdb', 4399 ).
file('arm-convert.s',text,'gdb-4.10.pl1/gdb', 200 ).
file('arm-pinsn.c',text,'gdb-4.10.pl1/gdb', 6823 ).
file('arm-tdep.c',text,'gdb-4.10.pl1/gdb', 11149 ).
file('arm-xdep.c',text,'gdb-4.10.pl1/gdb', 8097 ).
file('blockframe.c',text,'gdb-4.10.pl1/gdb', 21181 ).
file('breakpoint.c',text,'gdb-4.10.pl1/gdb', 85595 ).
file('breakpoint.h',text,'gdb-4.10.pl1/gdb', 12109 ).
file('buildsym.c',text,'gdb-4.10.pl1/gdb', 25159 ).
file('buildsym.h',text,'gdb-4.10.pl1/gdb', 7322 ).
file('c-exp.tab.c',text,'gdb-4.10.pl1/gdb', 69740 ).
file('c-exp.y',text,'gdb-4.10.pl1/gdb', 41770 ).
file('c-lang.c',text,'gdb-4.10.pl1/gdb', 13349 ).
file('c-lang.h',text,'gdb-4.10.pl1/gdb', 1161 ).
file('c-typeprint.c',text,'gdb-4.10.pl1/gdb', 20991 ).
file('c-valprint.c',text,'gdb-4.10.pl1/gdb', 10948 ).
file('call-cmds.h',text,'gdb-4.10.pl1/gdb', 1004 ).
file('ch-exp.tab.c',text,'gdb-4.10.pl1/gdb', 72558 ).
file('ch-exp.y',text,'gdb-4.10.pl1/gdb', 45983 ).
file('ch-lang.c',text,'gdb-4.10.pl1/gdb', 10129 ).
file('ch-lang.h',text,'gdb-4.10.pl1/gdb', 1181 ).
file('ch-typeprint.c',text,'gdb-4.10.pl1/gdb', 6188 ).
file('ch-valprint.c',text,'gdb-4.10.pl1/gdb', 9120 ).
file('coffread.c',text,'gdb-4.10.pl1/gdb', 63431 ).
file('command.c',text,'gdb-4.10.pl1/gdb', 34984 ).
file('command.h',text,'gdb-4.10.pl1/gdb', 8615 ).
file('complaints.c',text,'gdb-4.10.pl1/gdb', 4458 ).
file('complaints.h',text,'gdb-4.10.pl1/gdb', 1538 ).
file('config',dir,'gdb-4.10.pl1/gdb', 512 ).
file('configure.in',text,'gdb-4.10.pl1/gdb', 9394 ).
file('convex-pinsn.c',text,'gdb-4.10.pl1/gdb', 7908 ).
file('convex-tdep.c',text,'gdb-4.10.pl1/gdb', 25791 ).
file('convex-xdep.c',text,'gdb-4.10.pl1/gdb', 25595 ).
file('copying.awk',text,'gdb-4.10.pl1/gdb', 2355 ).
file('copying.c',text,'gdb-4.10.pl1/gdb', 23200 ).
file('core-svr4.c',text,'gdb-4.10.pl1/gdb', 3020 ).
file('core.c',text,'gdb-4.10.pl1/gdb', 5766 ).
file('coredep.c',text,'gdb-4.10.pl1/gdb', 3635 ).
file('corelow.c',text,'gdb-4.10.pl1/gdb', 7484 ).
file('cp-valprint.c',text,'gdb-4.10.pl1/gdb', 12845 ).
file('dbxread.c',text,'gdb-4.10.pl1/gdb', 73095 ).
file('defs.h',text,'gdb-4.10.pl1/gdb', 23033 ).
file('delta68-nat.c',text,'gdb-4.10.pl1/gdb', 2431 ).
file('demangle.c',text,'gdb-4.10.pl1/gdb', 6447 ).
file('doc',dir,'gdb-4.10.pl1/gdb', 512 ).
file('dpx2-nat.c',text,'gdb-4.10.pl1/gdb', 2491 ).
file('dwarfread.c',text,'gdb-4.10.pl1/gdb', 110321 ).
file('elfread.c',text,'gdb-4.10.pl1/gdb', 20542 ).
file('environ.c',text,'gdb-4.10.pl1/gdb', 4553 ).
file('environ.h',text,'gdb-4.10.pl1/gdb', 1738 ).
file('eval.c',text,'gdb-4.10.pl1/gdb', 35430 ).
file('exec.c',text,'gdb-4.10.pl1/gdb', 13440 ).
file('expprint.c',text,'gdb-4.10.pl1/gdb', 19673 ).
file('expression.h',text,'gdb-4.10.pl1/gdb', 11070 ).
file('findvar.c',text,'gdb-4.10.pl1/gdb', 25156 ).
file('fork-child.c',text,'gdb-4.10.pl1/gdb', 8860 ).
file('frame.h',text,'gdb-4.10.pl1/gdb', 8682 ).
file('gcc.patch',text,'gdb-4.10.pl1/gdb', 2101 ).
file('gdb-stabs.h',text,'gdb-4.10.pl1/gdb', 3256 ).
file('gdb.1',text,'gdb-4.10.pl1/gdb', 8042 ).
file('gdb.info',text,'gdb-4.10.pl1/gdb', 5593 ).
file('gdb.info-1',text,'gdb-4.10.pl1/gdb', 50673 ).
file('gdb.info-2',text,'gdb-4.10.pl1/gdb', 48798 ).
file('gdb.info-3',text,'gdb-4.10.pl1/gdb', 48247 ).
file('gdb.info-4',text,'gdb-4.10.pl1/gdb', 50111 ).
file('gdb.info-5',text,'gdb-4.10.pl1/gdb', 49632 ).
file('gdb.info-6',text,'gdb-4.10.pl1/gdb', 46863 ).
file('gdb.info-7',text,'gdb-4.10.pl1/gdb', 46089 ).
file('gdb.info-8',text,'gdb-4.10.pl1/gdb', 33695 ).
file('gdbcmd.h',text,'gdb-4.10.pl1/gdb', 2889 ).
file('gdbcore.h',text,'gdb-4.10.pl1/gdb', 3656 ).
file('gdbserver',dir,'gdb-4.10.pl1/gdb', 512 ).
file('gdbtypes.c',text,'gdb-4.10.pl1/gdb', 41245 ).
file('gdbtypes.h',text,'gdb-4.10.pl1/gdb', 23991 ).
file('go32-xdep.c',text,'gdb-4.10.pl1/gdb', 1112 ).
file('gould-pinsn.c',text,'gdb-4.10.pl1/gdb', 6737 ).
file('gould-xdep.c',text,'gdb-4.10.pl1/gdb', 3501 ).
file('h8300-tdep.c',text,'gdb-4.10.pl1/gdb', 11969 ).
file('h8500-tdep.c',text,'gdb-4.10.pl1/gdb', 18880 ).
file('hp300ux-nat.c',text,'gdb-4.10.pl1/gdb', 6955 ).
file('hppa-pinsn.c',text,'gdb-4.10.pl1/gdb', 1293 ).
file('hppa-tdep.c',text,'gdb-4.10.pl1/gdb', 27069 ).
file('hppab-nat.c',text,'gdb-4.10.pl1/gdb', 9672 ).
file('hppah-nat.c',text,'gdb-4.10.pl1/gdb', 8547 ).
file('i386-pinsn.c',text,'gdb-4.10.pl1/gdb', 1190 ).
file('i386-stub.c',text,'gdb-4.10.pl1/gdb', 26501 ).
file('i386-tdep.c',text,'gdb-4.10.pl1/gdb', 15595 ).
file('i386aix-nat.c',text,'gdb-4.10.pl1/gdb', 7725 ).
file('i386b-nat.c',text,'gdb-4.10.pl1/gdb', 1775 ).
file('i386lynx-nat.c',text,'gdb-4.10.pl1/gdb', 7047 ).
file('i386lynx-tdep.c',text,'gdb-4.10.pl1/gdb', 1655 ).
file('i386mach-nat.c',text,'gdb-4.10.pl1/gdb', 4344 ).
file('i386v-nat.c',text,'gdb-4.10.pl1/gdb', 4449 ).
file('i386v4-nat.c',text,'gdb-4.10.pl1/gdb', 3998 ).
file('i387-tdep.c',text,'gdb-4.10.pl1/gdb', 3337 ).
file('i960-pinsn.c',text,'gdb-4.10.pl1/gdb', 4637 ).
file('i960-tdep.c',text,'gdb-4.10.pl1/gdb', 21479 ).
file('infcmd.c',text,'gdb-4.10.pl1/gdb', 37272 ).
file('inferior.h',text,'gdb-4.10.pl1/gdb', 10687 ).
file('inflow.c',text,'gdb-4.10.pl1/gdb', 14488 ).
file('infptrace.c',text,'gdb-4.10.pl1/gdb', 11135 ).
file('infrun.c',text,'gdb-4.10.pl1/gdb', 54231 ).
file('inftarg.c',text,'gdb-4.10.pl1/gdb', 7823 ).
file('irix4-nat.c',text,'gdb-4.10.pl1/gdb', 4838 ).
file('isi-xdep.c',text,'gdb-4.10.pl1/gdb', 841 ).
file('kdb-start.c',text,'gdb-4.10.pl1/gdb', 1067 ).
file('language.c',text,'gdb-4.10.pl1/gdb', 34920 ).
file('language.h',text,'gdb-4.10.pl1/gdb', 12722 ).
file('m2-exp.tab.c',text,'gdb-4.10.pl1/gdb', 53536 ).
file('m2-exp.y',text,'gdb-4.10.pl1/gdb', 27493 ).
file('m2-lang.c',text,'gdb-4.10.pl1/gdb', 13138 ).
file('m2-lang.h',text,'gdb-4.10.pl1/gdb', 1176 ).
file('m2-typeprint.c',text,'gdb-4.10.pl1/gdb', 1491 ).
file('m2-valprint.c',text,'gdb-4.10.pl1/gdb', 1511 ).
file('m68k-pinsn.c',text,'gdb-4.10.pl1/gdb', 1201 ).
file('m68k-stub.c',text,'gdb-4.10.pl1/gdb', 34020 ).
file('m68k-tdep.c',text,'gdb-4.10.pl1/gdb', 14929 ).
file('m88k-nat.c',text,'gdb-4.10.pl1/gdb', 6445 ).
file('m88k-pinsn.c',text,'gdb-4.10.pl1/gdb', 1325 ).
file('m88k-tdep.c',text,'gdb-4.10.pl1/gdb', 27135 ).
file('main.c',text,'gdb-4.10.pl1/gdb', 73262 ).
file('maint.c',text,'gdb-4.10.pl1/gdb', 6573 ).
file('mem-break.c',text,'gdb-4.10.pl1/gdb', 3022 ).
file('minimon.h',text,'gdb-4.10.pl1/gdb', 15534 ).
file('minsyms.c',text,'gdb-4.10.pl1/gdb', 20292 ).
file('mips-nat.c',text,'gdb-4.10.pl1/gdb', 6832 ).
file('mips-pinsn.c',text,'gdb-4.10.pl1/gdb', 1378 ).
file('mips-tdep.c',text,'gdb-4.10.pl1/gdb', 29830 ).
file('mipsread.c',text,'gdb-4.10.pl1/gdb', 95937 ).
file('monitor.h',text,'gdb-4.10.pl1/gdb', 2570 ).
file('munch',exec,'gdb-4.10.pl1/gdb', 1496 ).
file('news-xdep.c',text,'gdb-4.10.pl1/gdb', 1571 ).
file('nindy-share',dir,'gdb-4.10.pl1/gdb', 512 ).
file('nindy-tdep.c',text,'gdb-4.10.pl1/gdb', 2405 ).
file('nlmread.c',text,'gdb-4.10.pl1/gdb', 8936 ).
file('ns32k-opcode.h',text,'gdb-4.10.pl1/gdb', 11478 ).
file('ns32k-pinsn.c',text,'gdb-4.10.pl1/gdb', 12727 ).
file('objfiles.c',text,'gdb-4.10.pl1/gdb', 22216 ).
file('objfiles.h',text,'gdb-4.10.pl1/gdb', 16645 ).
file('paread.c',text,'gdb-4.10.pl1/gdb', 15493 ).
file('parse.c',text,'gdb-4.10.pl1/gdb', 19415 ).
file('parser-defs.h',text,'gdb-4.10.pl1/gdb', 5129 ).
file('partial-stab.h',text,'gdb-4.10.pl1/gdb', 18070 ).
file('printcmd.c',text,'gdb-4.10.pl1/gdb', 56474 ).
file('procfs.c',text,'gdb-4.10.pl1/gdb', 77509 ).
file('putenv.c',text,'gdb-4.10.pl1/gdb', 2819 ).
file('pyr-pinsn.c',text,'gdb-4.10.pl1/gdb', 9311 ).
file('pyr-tdep.c',text,'gdb-4.10.pl1/gdb', 4398 ).
file('pyr-xdep.c',text,'gdb-4.10.pl1/gdb', 11004 ).
file('refcard.ps',text,'gdb-4.10.pl1/gdb', 57126 ).
file('regex.c',text,'gdb-4.10.pl1/gdb', 44547 ).
file('regex.h',text,'gdb-4.10.pl1/gdb', 7861 ).
file('rem-multi.shar',text,'gdb-4.10.pl1/gdb', 30696 ).
file('remote-adapt.c',text,'gdb-4.10.pl1/gdb', 35505 ).
file('remote-bug.c',text,'gdb-4.10.pl1/gdb', 30481 ).
file('remote-eb.c',text,'gdb-4.10.pl1/gdb', 24761 ).
file('remote-es.c',text,'gdb-4.10.pl1/gdb', 49797 ).
file('remote-hms.c',text,'gdb-4.10.pl1/gdb', 29196 ).
file('remote-mips.c',text,'gdb-4.10.pl1/gdb', 37037 ).
file('remote-mm.c',text,'gdb-4.10.pl1/gdb', 49582 ).
file('remote-monitor.c',text,'gdb-4.10.pl1/gdb', 28944 ).
file('remote-nindy.c',text,'gdb-4.10.pl1/gdb', 25067 ).
file('remote-sim.c',text,'gdb-4.10.pl1/gdb', 6073 ).
file('remote-st.c',text,'gdb-4.10.pl1/gdb', 18973 ).
file('remote-udi.c',text,'gdb-4.10.pl1/gdb', 42578 ).
file('remote-vx.c',text,'gdb-4.10.pl1/gdb', 35276 ).
file('remote-z8k.c',text,'gdb-4.10.pl1/gdb', 7651 ).
file('remote.c',text,'gdb-4.10.pl1/gdb', 29323 ).
file('rs6000-nat.c',text,'gdb-4.10.pl1/gdb', 7233 ).
file('rs6000-pinsn.c',text,'gdb-4.10.pl1/gdb', 8676 ).
file('rs6000-tdep.c',text,'gdb-4.10.pl1/gdb', 35766 ).
file('saber.suppress',text,'gdb-4.10.pl1/gdb', 13069 ).
file('ser-go32.c',text,'gdb-4.10.pl1/gdb', 7519 ).
file('ser-tcp.c',text,'gdb-4.10.pl1/gdb', 7010 ).
file('ser-unix.c',text,'gdb-4.10.pl1/gdb', 17159 ).
file('serial.c',text,'gdb-4.10.pl1/gdb', 5243 ).
file('serial.h',text,'gdb-4.10.pl1/gdb', 6124 ).
file('sh-tdep.c',text,'gdb-4.10.pl1/gdb', 5896 ).
file('signals.h',text,'gdb-4.10.pl1/gdb', 1157 ).
file('solib.c',text,'gdb-4.10.pl1/gdb', 38626 ).
file('solib.h',text,'gdb-4.10.pl1/gdb', 1884 ).
file('source.c',text,'gdb-4.10.pl1/gdb', 36456 ).
file('sparc-nat.c',text,'gdb-4.10.pl1/gdb', 10093 ).
file('sparc-pinsn.c',text,'gdb-4.10.pl1/gdb', 1190 ).
file('sparc-stub.c',text,'gdb-4.10.pl1/gdb', 19855 ).
file('sparc-tdep.c',text,'gdb-4.10.pl1/gdb', 24285 ).
file('sparclite',dir,'gdb-4.10.pl1/gdb', 512 ).
file('stabsread.c',text,'gdb-4.10.pl1/gdb', 105719 ).
file('stabsread.h',text,'gdb-4.10.pl1/gdb', 5565 ).
file('stack.c',text,'gdb-4.10.pl1/gdb', 37060 ).
file('standalone.c',text,'gdb-4.10.pl1/gdb', 11796 ).
file('stuff.c',text,'gdb-4.10.pl1/gdb', 5235 ).
file('sun3-nat.c',text,'gdb-4.10.pl1/gdb', 4292 ).
file('sun386-nat.c',text,'gdb-4.10.pl1/gdb', 8250 ).
file('symfile.c',text,'gdb-4.10.pl1/gdb', 41652 ).
file('symfile.h',text,'gdb-4.10.pl1/gdb', 8066 ).
file('symm-nat.c',text,'gdb-4.10.pl1/gdb', 8723 ).
file('symm-tdep.c',text,'gdb-4.10.pl1/gdb', 11745 ).
file('symmisc.c',text,'gdb-4.10.pl1/gdb', 21521 ).
file('symtab.c',text,'gdb-4.10.pl1/gdb', 84168 ).
file('symtab.h',text,'gdb-4.10.pl1/gdb', 35245 ).
file('tahoe-pinsn.c',text,'gdb-4.10.pl1/gdb', 5641 ).
file('target.c',text,'gdb-4.10.pl1/gdb', 19255 ).
file('target.h',text,'gdb-4.10.pl1/gdb', 16635 ).
file('terminal.h',text,'gdb-4.10.pl1/gdb', 1443 ).
file('thread.c',text,'gdb-4.10.pl1/gdb', 4903 ).
file('thread.h',text,'gdb-4.10.pl1/gdb', 1206 ).
file('typeprint.c',text,'gdb-4.10.pl1/gdb', 7461 ).
file('typeprint.h',text,'gdb-4.10.pl1/gdb', 920 ).
file('ultra3-nat.c',text,'gdb-4.10.pl1/gdb', 9597 ).
file('ultra3-xdep.c',text,'gdb-4.10.pl1/gdb', 2692 ).
file('umax-xdep.c',text,'gdb-4.10.pl1/gdb', 3597 ).
file('utils.c',text,'gdb-4.10.pl1/gdb', 37348 ).
file('valarith.c',text,'gdb-4.10.pl1/gdb', 26143 ).
file('valops.c',text,'gdb-4.10.pl1/gdb', 51313 ).
file('valprint.c',text,'gdb-4.10.pl1/gdb', 30859 ).
file('valprint.h',text,'gdb-4.10.pl1/gdb', 1690 ).
file('value.h',text,'gdb-4.10.pl1/gdb', 15334 ).
file('values.c',text,'gdb-4.10.pl1/gdb', 43223 ).
file('vax-pinsn.c',text,'gdb-4.10.pl1/gdb', 5458 ).
file('vx-share',dir,'gdb-4.10.pl1/gdb', 512 ).
file('xcoffexec.c',text,'gdb-4.10.pl1/gdb', 28590 ).
file('xcoffread.c',text,'gdb-4.10.pl1/gdb', 61542 ).
file('xcoffsolib.c',text,'gdb-4.10.pl1/gdb', 5028 ).
file('xcoffsolib.h',text,'gdb-4.10.pl1/gdb', 2073 ).
file('z8k-tdep.c',text,'gdb-4.10.pl1/gdb', 10230 ).
file('README',text,'gdb-4.10.pl1/gdb/29k-share', 525 ).
file('udi',dir,'gdb-4.10.pl1/gdb/29k-share', 512 ).
file('udi_soc',text,'gdb-4.10.pl1/gdb/29k-share', 531 ).
file('udiids.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 1845 ).
file('udip2soc.c',text,'gdb-4.10.pl1/gdb/29k-share/udi', 39676 ).
file('udiphcfg.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 1138 ).
file('udiphunix.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 3190 ).
file('udiproc.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 8169 ).
file('udipt29k.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 2792 ).
file('udiptcfg.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 803 ).
file('udisoc.h',text,'gdb-4.10.pl1/gdb/29k-share/udi', 4683 ).
file('udr.c',text,'gdb-4.10.pl1/gdb/29k-share/udi', 11088 ).
file('a29k',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('alpha',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('arm',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('convex',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('gould',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('h8300',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('h8500',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('i386',dir,'gdb-4.10.pl1/gdb/config', 1536 ).
file('i960',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('m68k',dir,'gdb-4.10.pl1/gdb/config', 2048 ).
file('m88k',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('mips',dir,'gdb-4.10.pl1/gdb/config', 1024 ).
file('nm-sysv4.h',text,'gdb-4.10.pl1/gdb/config', 1073 ).
file('nm-trash.h',text,'gdb-4.10.pl1/gdb/config', 113 ).
file('none',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('ns32k',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('pa',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('pyr',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('romp',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('rs6000',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('sh',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('sparc',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('tahoe',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('tm-sunos.h',text,'gdb-4.10.pl1/gdb/config', 945 ).
file('tm-sysv4.h',text,'gdb-4.10.pl1/gdb/config', 2280 ).
file('vax',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('xm-sysv4.h',text,'gdb-4.10.pl1/gdb/config', 1473 ).
file('z8k',dir,'gdb-4.10.pl1/gdb/config', 512 ).
file('a29k-kern.mt',text,'gdb-4.10.pl1/gdb/config/a29k', 555 ).
file('a29k-udi.mt',text,'gdb-4.10.pl1/gdb/config/a29k', 246 ).
file('a29k.mt',text,'gdb-4.10.pl1/gdb/config/a29k', 372 ).
file('nm-ultra3.h',text,'gdb-4.10.pl1/gdb/config/a29k', 1144 ).
file('tm-a29k.h',text,'gdb-4.10.pl1/gdb/config/a29k', 28394 ).
file('tm-ultra3.h',text,'gdb-4.10.pl1/gdb/config/a29k', 9035 ).
file('ultra3.mh',text,'gdb-4.10.pl1/gdb/config/a29k', 280 ).
file('ultra3.mt',text,'gdb-4.10.pl1/gdb/config/a29k', 179 ).
file('xm-ultra3.h',text,'gdb-4.10.pl1/gdb/config/a29k', 1700 ).
file('alpha-osf1.mh',text,'gdb-4.10.pl1/gdb/config/alpha', 259 ).
file('xm-alpha.h',text,'gdb-4.10.pl1/gdb/config/alpha', 1412 ).
file('arm.mh',text,'gdb-4.10.pl1/gdb/config/arm', 140 ).
file('arm.mt',text,'gdb-4.10.pl1/gdb/config/arm', 105 ).
file('tm-arm.h',text,'gdb-4.10.pl1/gdb/config/arm', 14616 ).
file('xm-arm.h',text,'gdb-4.10.pl1/gdb/config/arm', 2961 ).
file('Convex.notes',text,'gdb-4.10.pl1/gdb/config/convex', 7013 ).
file('convex.mh',text,'gdb-4.10.pl1/gdb/config/convex', 73 ).
file('convex.mt',text,'gdb-4.10.pl1/gdb/config/convex', 90 ).
file('tm-convex.h',text,'gdb-4.10.pl1/gdb/config/convex', 20312 ).
file('xm-convex.h',text,'gdb-4.10.pl1/gdb/config/convex', 1473 ).
file('np1.mh',text,'gdb-4.10.pl1/gdb/config/gould', 95 ).
file('np1.mt',text,'gdb-4.10.pl1/gdb/config/gould', 70 ).
file('pn.mh',text,'gdb-4.10.pl1/gdb/config/gould', 107 ).
file('pn.mt',text,'gdb-4.10.pl1/gdb/config/gould', 75 ).
file('tm-np1.h',text,'gdb-4.10.pl1/gdb/config/gould', 19064 ).
file('tm-pn.h',text,'gdb-4.10.pl1/gdb/config/gould', 15313 ).
file('xm-np1.h',text,'gdb-4.10.pl1/gdb/config/gould', 3472 ).
file('xm-pn.h',text,'gdb-4.10.pl1/gdb/config/gould', 3208 ).
file('h8300hms.mt',text,'gdb-4.10.pl1/gdb/config/h8300', 151 ).
file('tm-h8300.h',text,'gdb-4.10.pl1/gdb/config/h8300', 9501 ).
file('h8500hms.mt',text,'gdb-4.10.pl1/gdb/config/h8500', 151 ).
file('tm-h8500.h',text,'gdb-4.10.pl1/gdb/config/h8500', 9520 ).
file('go32.mh',text,'gdb-4.10.pl1/gdb/config/i386', 129 ).
file('i386aix.mh',text,'gdb-4.10.pl1/gdb/config/i386', 267 ).
file('i386aix.mt',text,'gdb-4.10.pl1/gdb/config/i386', 281 ).
file('i386aout.mt',text,'gdb-4.10.pl1/gdb/config/i386', 96 ).
file('i386bsd.mh',text,'gdb-4.10.pl1/gdb/config/i386', 206 ).
file('i386bsd.mt',text,'gdb-4.10.pl1/gdb/config/i386', 90 ).
file('i386lynx.mh',text,'gdb-4.10.pl1/gdb/config/i386', 192 ).
file('i386lynx.mt',text,'gdb-4.10.pl1/gdb/config/i386', 115 ).
file('i386mach.mh',text,'gdb-4.10.pl1/gdb/config/i386', 154 ).
file('i386nw.mt',text,'gdb-4.10.pl1/gdb/config/i386', 93 ).
file('i386sco.mh',text,'gdb-4.10.pl1/gdb/config/i386', 480 ).
file('i386sco4.mh',text,'gdb-4.10.pl1/gdb/config/i386', 532 ).
file('i386sol2.mh',text,'gdb-4.10.pl1/gdb/config/i386', 687 ).
file('i386sol2.mt',text,'gdb-4.10.pl1/gdb/config/i386', 110 ).
file('i386v.mh',text,'gdb-4.10.pl1/gdb/config/i386', 238 ).
file('i386v.mt',text,'gdb-4.10.pl1/gdb/config/i386', 112 ).
file('i386v32.mh',text,'gdb-4.10.pl1/gdb/config/i386', 245 ).
file('i386v4.mh',text,'gdb-4.10.pl1/gdb/config/i386', 653 ).
file('i386v4.mt',text,'gdb-4.10.pl1/gdb/config/i386', 110 ).
file('linux.mh',text,'gdb-4.10.pl1/gdb/config/i386', 242 ).
file('linux.mt',text,'gdb-4.10.pl1/gdb/config/i386', 99 ).
file('ncr3000.mh',text,'gdb-4.10.pl1/gdb/config/i386', 1151 ).
file('ncr3000.mt',text,'gdb-4.10.pl1/gdb/config/i386', 110 ).
file('nm-i386aix.h',text,'gdb-4.10.pl1/gdb/config/i386', 1456 ).
file('nm-i386bsd.h',text,'gdb-4.10.pl1/gdb/config/i386', 1309 ).
file('nm-i386lynx.h',text,'gdb-4.10.pl1/gdb/config/i386', 1995 ).
file('nm-i386mach.h',text,'gdb-4.10.pl1/gdb/config/i386', 1009 ).
file('nm-i386sco.h',text,'gdb-4.10.pl1/gdb/config/i386', 1542 ).
file('nm-i386sco4.h',text,'gdb-4.10.pl1/gdb/config/i386', 1234 ).
file('nm-i386v.h',text,'gdb-4.10.pl1/gdb/config/i386', 1425 ).
file('nm-i386v4.h',text,'gdb-4.10.pl1/gdb/config/i386', 875 ).
file('nm-linux.h',text,'gdb-4.10.pl1/gdb/config/i386', 1055 ).
file('nm-sun386.h',text,'gdb-4.10.pl1/gdb/config/i386', 1008 ).
file('nm-symmetry.h',text,'gdb-4.10.pl1/gdb/config/i386', 1628 ).
file('ptx.mh',text,'gdb-4.10.pl1/gdb/config/i386', 232 ).
file('sun386.mh',text,'gdb-4.10.pl1/gdb/config/i386', 140 ).
file('sun386.mt',text,'gdb-4.10.pl1/gdb/config/i386', 99 ).
file('symmetry.mh',text,'gdb-4.10.pl1/gdb/config/i386', 150 ).
file('symmetry.mt',text,'gdb-4.10.pl1/gdb/config/i386', 143 ).
file('tm-i386aix.h',text,'gdb-4.10.pl1/gdb/config/i386', 5321 ).
file('tm-i386bsd.h',text,'gdb-4.10.pl1/gdb/config/i386', 1080 ).
file('tm-i386lynx.h',text,'gdb-4.10.pl1/gdb/config/i386', 1195 ).
file('tm-i386nw.h',text,'gdb-4.10.pl1/gdb/config/i386', 1195 ).
file('tm-i386v.h',text,'gdb-4.10.pl1/gdb/config/i386', 9549 ).
file('tm-i386v4.h',text,'gdb-4.10.pl1/gdb/config/i386', 2475 ).
file('tm-linux.h',text,'gdb-4.10.pl1/gdb/config/i386', 1032 ).
file('tm-sun386.h',text,'gdb-4.10.pl1/gdb/config/i386', 10099 ).
file('tm-symmetry.h',text,'gdb-4.10.pl1/gdb/config/i386', 12446 ).
file('xm-go32.h',text,'gdb-4.10.pl1/gdb/config/i386', 1019 ).
file('xm-i386aix.h',text,'gdb-4.10.pl1/gdb/config/i386', 1074 ).
file('xm-i386bsd.h',text,'gdb-4.10.pl1/gdb/config/i386', 1134 ).
file('xm-i386lynx.h',text,'gdb-4.10.pl1/gdb/config/i386', 1147 ).
file('xm-i386mach.h',text,'gdb-4.10.pl1/gdb/config/i386', 1494 ).
file('xm-i386sco.h',text,'gdb-4.10.pl1/gdb/config/i386', 1584 ).
file('xm-i386v.h',text,'gdb-4.10.pl1/gdb/config/i386', 1813 ).
file('xm-i386v32.h',text,'gdb-4.10.pl1/gdb/config/i386', 972 ).
file('xm-i386v4.h',text,'gdb-4.10.pl1/gdb/config/i386', 1818 ).
file('xm-linux.h',text,'gdb-4.10.pl1/gdb/config/i386', 1036 ).
file('xm-sun386.h',text,'gdb-4.10.pl1/gdb/config/i386', 1265 ).
file('xm-symmetry.h',text,'gdb-4.10.pl1/gdb/config/i386', 4336 ).
file('nindy960.mt',text,'gdb-4.10.pl1/gdb/config/i960', 288 ).
file('tm-i960.h',text,'gdb-4.10.pl1/gdb/config/i960', 14083 ).
file('tm-nindy960.h',text,'gdb-4.10.pl1/gdb/config/i960', 3512 ).
file('tm-vx960.h',text,'gdb-4.10.pl1/gdb/config/i960', 1839 ).
file('vxworks960.mt',text,'gdb-4.10.pl1/gdb/config/i960', 332 ).
file('3b1.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 467 ).
file('3b1.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 76 ).
file('altos.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 173 ).
file('altos.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 99 ).
file('amix.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 495 ).
file('amix.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 109 ).
file('apollo68b.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 152 ).
file('apollo68v.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 276 ).
file('delta68.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 190 ).
file('delta68.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 101 ).
file('dpx2.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 233 ).
file('dpx2.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 108 ).
file('es1800.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 133 ).
file('hp300bsd.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 216 ).
file('hp300bsd.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 114 ).
file('hp300hpux.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 445 ).
file('hp300hpux.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 229 ).
file('isi.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 243 ).
file('isi.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 94 ).
file('m68k-fp.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 110 ).
file('m68k-nofp.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 115 ).
file('monitor.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 440 ).
file('news.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 209 ).
file('news.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 128 ).
file('news1000.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 165 ).
file('nm-apollo68b.h',text,'gdb-4.10.pl1/gdb/config/m68k', 862 ).
file('nm-apollo68v.h',text,'gdb-4.10.pl1/gdb/config/m68k', 832 ).
file('nm-delta68.h',text,'gdb-4.10.pl1/gdb/config/m68k', 845 ).
file('nm-dpx2.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1124 ).
file('nm-hp300bsd.h',text,'gdb-4.10.pl1/gdb/config/m68k', 3792 ).
file('nm-hp300hpux.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1751 ).
file('nm-news.h',text,'gdb-4.10.pl1/gdb/config/m68k', 862 ).
file('nm-sun2.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1329 ).
file('nm-sun3.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1018 ).
file('os68k.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 100 ).
file('st2000.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 391 ).
file('sun2os3.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 151 ).
file('sun2os3.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 263 ).
file('sun2os4.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 151 ).
file('sun2os4.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 106 ).
file('sun3os3.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 151 ).
file('sun3os3.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 263 ).
file('sun3os4.mh',text,'gdb-4.10.pl1/gdb/config/m68k', 154 ).
file('sun3os4.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 126 ).
file('tm-3b1.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1061 ).
file('tm-altos.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1925 ).
file('tm-amix.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2415 ).
file('tm-delta68.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1266 ).
file('tm-dpx2.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1178 ).
file('tm-es1800.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1966 ).
file('tm-hp300bsd.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1742 ).
file('tm-hp300hpux.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1053 ).
file('tm-isi.h',text,'gdb-4.10.pl1/gdb/config/m68k', 6537 ).
file('tm-m68k-fp.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2001 ).
file('tm-m68k-nofp.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1979 ).
file('tm-m68k.h',text,'gdb-4.10.pl1/gdb/config/m68k', 15439 ).
file('tm-monitor.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1300 ).
file('tm-news.h',text,'gdb-4.10.pl1/gdb/config/m68k', 3089 ).
file('tm-os68k.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1834 ).
file('tm-st2000.h',text,'gdb-4.10.pl1/gdb/config/m68k', 895 ).
file('tm-sun2.h',text,'gdb-4.10.pl1/gdb/config/m68k', 855 ).
file('tm-sun2os4.h',text,'gdb-4.10.pl1/gdb/config/m68k', 795 ).
file('tm-sun3.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2240 ).
file('tm-sun3os4.h',text,'gdb-4.10.pl1/gdb/config/m68k', 877 ).
file('tm-vx68.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2915 ).
file('vxworks68.mt',text,'gdb-4.10.pl1/gdb/config/m68k', 143 ).
file('xm-3b1.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2779 ).
file('xm-altos.h',text,'gdb-4.10.pl1/gdb/config/m68k', 5511 ).
file('xm-amix.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1826 ).
file('xm-apollo68b.h',text,'gdb-4.10.pl1/gdb/config/m68k', 889 ).
file('xm-apollo68v.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1684 ).
file('xm-delta68.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1523 ).
file('xm-dpx2.h',text,'gdb-4.10.pl1/gdb/config/m68k', 924 ).
file('xm-hp300bsd.h',text,'gdb-4.10.pl1/gdb/config/m68k', 3466 ).
file('xm-hp300hpux.h',text,'gdb-4.10.pl1/gdb/config/m68k', 5485 ).
file('xm-isi.h',text,'gdb-4.10.pl1/gdb/config/m68k', 3370 ).
file('xm-m68k.h',text,'gdb-4.10.pl1/gdb/config/m68k', 909 ).
file('xm-news.h',text,'gdb-4.10.pl1/gdb/config/m68k', 4421 ).
file('xm-news1000.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1081 ).
file('xm-sun2.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2776 ).
file('xm-sun3.h',text,'gdb-4.10.pl1/gdb/config/m68k', 2878 ).
file('xm-sun3os4.h',text,'gdb-4.10.pl1/gdb/config/m68k', 1911 ).
file('delta88.mh',text,'gdb-4.10.pl1/gdb/config/m88k', 360 ).
file('delta88.mt',text,'gdb-4.10.pl1/gdb/config/m88k', 84 ).
file('m88k.mh',text,'gdb-4.10.pl1/gdb/config/m88k', 153 ).
file('m88k.mt',text,'gdb-4.10.pl1/gdb/config/m88k', 130 ).
file('nm-m88k.h',text,'gdb-4.10.pl1/gdb/config/m88k', 968 ).
file('tm-delta88.h',text,'gdb-4.10.pl1/gdb/config/m88k', 996 ).
file('tm-m88k.h',text,'gdb-4.10.pl1/gdb/config/m88k', 12966 ).
file('xm-delta88.h',text,'gdb-4.10.pl1/gdb/config/m88k', 1556 ).
file('xm-m88k.h',text,'gdb-4.10.pl1/gdb/config/m88k', 1750 ).
file('bigmips.mt',text,'gdb-4.10.pl1/gdb/config/mips', 117 ).
file('decstation.mh',text,'gdb-4.10.pl1/gdb/config/mips', 383 ).
file('decstation.mt',text,'gdb-4.10.pl1/gdb/config/mips', 119 ).
file('idt.mt',text,'gdb-4.10.pl1/gdb/config/mips', 111 ).
file('idtl.mt',text,'gdb-4.10.pl1/gdb/config/mips', 108 ).
file('irix3.mh',text,'gdb-4.10.pl1/gdb/config/mips', 187 ).
file('irix3.mt',text,'gdb-4.10.pl1/gdb/config/mips', 82 ).
file('irix4.mh',text,'gdb-4.10.pl1/gdb/config/mips', 306 ).
file('littlemips.mh',text,'gdb-4.10.pl1/gdb/config/mips', 140 ).
file('littlemips.mt',text,'gdb-4.10.pl1/gdb/config/mips', 119 ).
file('news-mips.mh',text,'gdb-4.10.pl1/gdb/config/mips', 177 ).
file('nm-irix3.h',text,'gdb-4.10.pl1/gdb/config/mips', 1461 ).
file('nm-irix4.h',text,'gdb-4.10.pl1/gdb/config/mips', 1228 ).
file('nm-mips.h',text,'gdb-4.10.pl1/gdb/config/mips', 1438 ).
file('nm-news-mips.h',text,'gdb-4.10.pl1/gdb/config/mips', 1351 ).
file('nm-riscos.h',text,'gdb-4.10.pl1/gdb/config/mips', 2426 ).
file('riscos.mh',text,'gdb-4.10.pl1/gdb/config/mips', 523 ).
file('tm-bigmips.h',text,'gdb-4.10.pl1/gdb/config/mips', 810 ).
file('tm-irix3.h',text,'gdb-4.10.pl1/gdb/config/mips', 2956 ).
file('tm-mips.h',text,'gdb-4.10.pl1/gdb/config/mips', 15858 ).
file('xm-irix3.h',text,'gdb-4.10.pl1/gdb/config/mips', 1398 ).
file('xm-irix4.h',text,'gdb-4.10.pl1/gdb/config/mips', 1054 ).
file('xm-makeva.h',text,'gdb-4.10.pl1/gdb/config/mips', 1409 ).
file('xm-mips.h',text,'gdb-4.10.pl1/gdb/config/mips', 2424 ).
file('xm-news-mips.h',text,'gdb-4.10.pl1/gdb/config/mips', 1201 ).
file('xm-riscos.h',text,'gdb-4.10.pl1/gdb/config/mips', 1016 ).
file('nm-none.h',text,'gdb-4.10.pl1/gdb/config/none', 792 ).
file('none.mh',text,'gdb-4.10.pl1/gdb/config/none', 177 ).
file('none.mt',text,'gdb-4.10.pl1/gdb/config/none', 158 ).
file('tm-none.h',text,'gdb-4.10.pl1/gdb/config/none', 964 ).
file('xm-none.h',text,'gdb-4.10.pl1/gdb/config/none', 792 ).
file('merlin.mh',text,'gdb-4.10.pl1/gdb/config/ns32k', 368 ).
file('merlin.mt',text,'gdb-4.10.pl1/gdb/config/ns32k', 87 ).
file('nm-umax.h',text,'gdb-4.10.pl1/gdb/config/ns32k', 2063 ).
file('tm-merlin.h',text,'gdb-4.10.pl1/gdb/config/ns32k', 11568 ).
file('tm-umax.h',text,'gdb-4.10.pl1/gdb/config/ns32k', 13161 ).
file('umax.mh',text,'gdb-4.10.pl1/gdb/config/ns32k', 149 ).
file('umax.mt',text,'gdb-4.10.pl1/gdb/config/ns32k', 78 ).
file('xm-merlin.h',text,'gdb-4.10.pl1/gdb/config/ns32k', 2572 ).
file('xm-umax.h',text,'gdb-4.10.pl1/gdb/config/ns32k', 936 ).
file('hppabsd.mh',text,'gdb-4.10.pl1/gdb/config/pa', 190 ).
file('hppabsd.mt',text,'gdb-4.10.pl1/gdb/config/pa', 90 ).
file('hppahpux.mh',text,'gdb-4.10.pl1/gdb/config/pa', 265 ).
file('hppahpux.mt',text,'gdb-4.10.pl1/gdb/config/pa', 97 ).
file('nm-hppab.h',text,'gdb-4.10.pl1/gdb/config/pa', 1467 ).
file('nm-hppah.h',text,'gdb-4.10.pl1/gdb/config/pa', 1379 ).
file('tm-hppa.h',text,'gdb-4.10.pl1/gdb/config/pa', 20871 ).
file('tm-hppab.h',text,'gdb-4.10.pl1/gdb/config/pa', 249 ).
file('tm-hppah.h',text,'gdb-4.10.pl1/gdb/config/pa', 1210 ).
file('xm-hppab.h',text,'gdb-4.10.pl1/gdb/config/pa', 1761 ).
file('xm-hppah.h',text,'gdb-4.10.pl1/gdb/config/pa', 2178 ).
file('xm-pa.h',text,'gdb-4.10.pl1/gdb/config/pa', 988 ).
file('pyramid.mh',text,'gdb-4.10.pl1/gdb/config/pyr', 241 ).
file('pyramid.mt',text,'gdb-4.10.pl1/gdb/config/pyr', 188 ).
file('tm-pyr.h',text,'gdb-4.10.pl1/gdb/config/pyr', 19666 ).
file('xm-pyr.h',text,'gdb-4.10.pl1/gdb/config/pyr', 3782 ).
file('rtbsd.mh',text,'gdb-4.10.pl1/gdb/config/romp', 406 ).
file('xm-rtbsd.h',text,'gdb-4.10.pl1/gdb/config/romp', 1612 ).
file('nm-rs6000.h',text,'gdb-4.10.pl1/gdb/config/rs6000', 1083 ).
file('rs6000.mh',text,'gdb-4.10.pl1/gdb/config/rs6000', 1434 ).
file('rs6000.mt',text,'gdb-4.10.pl1/gdb/config/rs6000', 869 ).
file('tm-rs6000.h',text,'gdb-4.10.pl1/gdb/config/rs6000', 20567 ).
file('xm-rs6000.h',text,'gdb-4.10.pl1/gdb/config/rs6000', 3383 ).
file('sh.mt',text,'gdb-4.10.pl1/gdb/config/sh', 200 ).
file('tm-sh.h',text,'gdb-4.10.pl1/gdb/config/sh', 7436 ).
file('nm-sun4os4.h',text,'gdb-4.10.pl1/gdb/config/sparc', 1004 ).
file('sparc-em.mt',text,'gdb-4.10.pl1/gdb/config/sparc', 91 ).
file('sparclite.mt',text,'gdb-4.10.pl1/gdb/config/sparc', 107 ).
file('sun4os4.mh',text,'gdb-4.10.pl1/gdb/config/sparc', 202 ).
file('sun4os4.mt',text,'gdb-4.10.pl1/gdb/config/sparc', 124 ).
file('sun4sol2.mh',text,'gdb-4.10.pl1/gdb/config/sparc', 1001 ).
file('sun4sol2.mt',text,'gdb-4.10.pl1/gdb/config/sparc', 112 ).
file('tm-sparc.h',text,'gdb-4.10.pl1/gdb/config/sparc', 23935 ).
file('tm-sparclite.h',text,'gdb-4.10.pl1/gdb/config/sparc', 824 ).
file('tm-spc-em.h',text,'gdb-4.10.pl1/gdb/config/sparc', 1655 ).
file('tm-sun4os4.h',text,'gdb-4.10.pl1/gdb/config/sparc', 1668 ).
file('tm-sun4sol2.h',text,'gdb-4.10.pl1/gdb/config/sparc', 1864 ).
file('tm-vxsparc.h',text,'gdb-4.10.pl1/gdb/config/sparc', 1199 ).
file('vxsparc.mt',text,'gdb-4.10.pl1/gdb/config/sparc', 148 ).
file('xm-sparc.h',text,'gdb-4.10.pl1/gdb/config/sparc', 1348 ).
file('xm-sun4os4.h',text,'gdb-4.10.pl1/gdb/config/sparc', 2547 ).
file('xm-sun4sol2.h',text,'gdb-4.10.pl1/gdb/config/sparc', 2873 ).
file('tahoe.mh',text,'gdb-4.10.pl1/gdb/config/tahoe', 160 ).
file('tahoe.mt',text,'gdb-4.10.pl1/gdb/config/tahoe', 99 ).
file('tm-tahoe.h',text,'gdb-4.10.pl1/gdb/config/tahoe', 10793 ).
file('xm-tahoe.h',text,'gdb-4.10.pl1/gdb/config/tahoe', 4696 ).
file('nm-vax.h',text,'gdb-4.10.pl1/gdb/config/vax', 1191 ).
file('tm-vax.h',text,'gdb-4.10.pl1/gdb/config/vax', 13581 ).
file('vax.mt',text,'gdb-4.10.pl1/gdb/config/vax', 88 ).
file('vaxbsd.mh',text,'gdb-4.10.pl1/gdb/config/vax', 498 ).
file('vaxult.mh',text,'gdb-4.10.pl1/gdb/config/vax', 181 ).
file('vaxult2.mh',text,'gdb-4.10.pl1/gdb/config/vax', 182 ).
file('xm-vax.h',text,'gdb-4.10.pl1/gdb/config/vax', 2954 ).
file('xm-vaxbsd.h',text,'gdb-4.10.pl1/gdb/config/vax', 449 ).
file('xm-vaxult.h',text,'gdb-4.10.pl1/gdb/config/vax', 131 ).
file('xm-vaxult2.h',text,'gdb-4.10.pl1/gdb/config/vax', 360 ).
file('tm-z8k.h',text,'gdb-4.10.pl1/gdb/config/z8k', 10263 ).
file('z8ksim.mt',text,'gdb-4.10.pl1/gdb/config/z8k', 131 ).
file('ChangeLog',text,'gdb-4.10.pl1/gdb/doc', 20705 ).
file('GDBvn.texi',text,'gdb-4.10.pl1/gdb/doc', 16 ).
file('Makefile.in',text,'gdb-4.10.pl1/gdb/doc', 10435 ).
file('all-cfg.texi',text,'gdb-4.10.pl1/gdb/doc', 2702 ).
file('configure.in',text,'gdb-4.10.pl1/gdb/doc', 86 ).
file('gdb-cfg.texi',text,'gdb-4.10.pl1/gdb/doc', 2702 ).
file('gdb.texinfo',text,'gdb-4.10.pl1/gdb/doc', 314709 ).
file('gdbint.texinfo',text,'gdb-4.10.pl1/gdb/doc', 89914 ).
file('h8-cfg.texi',text,'gdb-4.10.pl1/gdb/doc', 864 ).
file('lpsrc.sed',text,'gdb-4.10.pl1/gdb/doc', 526 ).
file('psrc.sed',text,'gdb-4.10.pl1/gdb/doc', 481 ).
file('refcard.dvi',text,'gdb-4.10.pl1/gdb/doc', 21800 ).
file('refcard.tex',text,'gdb-4.10.pl1/gdb/doc', 21076 ).
file('remote.texi',text,'gdb-4.10.pl1/gdb/doc', 48179 ).
file('stabs.texinfo',text,'gdb-4.10.pl1/gdb/doc', 130928 ).
file('Makefile.in',text,'gdb-4.10.pl1/gdb/gdbserver', 26516 ).
file('README',text,'gdb-4.10.pl1/gdb/gdbserver', 3799 ).
file('configure.in',text,'gdb-4.10.pl1/gdb/gdbserver', 8899 ).
file('remote-gutils.c',text,'gdb-4.10.pl1/gdb/gdbserver', 2230 ).
file('remote-inflow.c',text,'gdb-4.10.pl1/gdb/gdbserver', 8675 ).
file('remote-server.c',text,'gdb-4.10.pl1/gdb/gdbserver', 3346 ).
file('remote-utils.c',text,'gdb-4.10.pl1/gdb/gdbserver', 8979 ).
file('server.h',text,'gdb-4.10.pl1/gdb/gdbserver', 1300 ).
file('Makefile',text,'gdb-4.10.pl1/gdb/nindy-share', 3713 ).
file('Onindy.c',text,'gdb-4.10.pl1/gdb/nindy-share', 21748 ).
file('README',text,'gdb-4.10.pl1/gdb/nindy-share', 206 ).
file('VERSION',text,'gdb-4.10.pl1/gdb/nindy-share', 4 ).
file('b.out.h',text,'gdb-4.10.pl1/gdb/nindy-share', 5901 ).
file('block_io.h',text,'gdb-4.10.pl1/gdb/nindy-share', 2520 ).
file('coff.h',text,'gdb-4.10.pl1/gdb/nindy-share', 9783 ).
file('env.h',text,'gdb-4.10.pl1/gdb/nindy-share', 313 ).
file('nindy.c',text,'gdb-4.10.pl1/gdb/nindy-share', 31335 ).
file('stop.h',text,'gdb-4.10.pl1/gdb/nindy-share', 3267 ).
file('ttyflush.c',text,'gdb-4.10.pl1/gdb/nindy-share', 1478 ).
file('Makefile.in',text,'gdb-4.10.pl1/gdb/sparclite', 4700 ).
file('README',text,'gdb-4.10.pl1/gdb/sparclite', 832 ).
file('aload.c',text,'gdb-4.10.pl1/gdb/sparclite', 4612 ).
file('configure.in',text,'gdb-4.10.pl1/gdb/sparclite', 98 ).
file('crt0.s',text,'gdb-4.10.pl1/gdb/sparclite', 816 ).
file('hello.c',text,'gdb-4.10.pl1/gdb/sparclite', 1136 ).
file('salib.c',text,'gdb-4.10.pl1/gdb/sparclite', 6383 ).
file('README',text,'gdb-4.10.pl1/gdb/vx-share', 374 ).
file('dbgRpcLib.h',text,'gdb-4.10.pl1/gdb/vx-share', 738 ).
file('ptrace.h',text,'gdb-4.10.pl1/gdb/vx-share', 433 ).
file('vxTypes.h',text,'gdb-4.10.pl1/gdb/vx-share', 1670 ).
file('vxWorks.h',text,'gdb-4.10.pl1/gdb/vx-share', 4489 ).
file('wait.h',text,'gdb-4.10.pl1/gdb/vx-share', 1286 ).
file('xdr_ld.c',text,'gdb-4.10.pl1/gdb/vx-share', 2039 ).
file('xdr_ld.h',text,'gdb-4.10.pl1/gdb/vx-share', 796 ).
file('xdr_ptrace.c',text,'gdb-4.10.pl1/gdb/vx-share', 2642 ).
file('xdr_ptrace.h',text,'gdb-4.10.pl1/gdb/vx-share', 1477 ).
file('xdr_rdb.c',text,'gdb-4.10.pl1/gdb/vx-share', 4427 ).
file('xdr_rdb.h',text,'gdb-4.10.pl1/gdb/vx-share', 2595 ).
file('ChangeLog',text,'gdb-4.10.pl1/glob', 3151 ).
file('Makefile.in',text,'gdb-4.10.pl1/glob', 2310 ).
file('config',dir,'gdb-4.10.pl1/glob', 512 ).
file('configure.in',text,'gdb-4.10.pl1/glob', 940 ).
file('glob.c',text,'gdb-4.10.pl1/glob', 14726 ).
file('glob.texinfo',text,'gdb-4.10.pl1/glob', 22 ).
file('sysdep-a68v.h',text,'gdb-4.10.pl1/glob', 226 ).
file('sysdep-aix.h',text,'gdb-4.10.pl1/glob', 141 ).
file('sysdep-irix.h',text,'gdb-4.10.pl1/glob', 228 ).
file('sysdep-norm.h',text,'gdb-4.10.pl1/glob', 483 ).
file('sysdep-obsd.h',text,'gdb-4.10.pl1/glob', 245 ).
file('sysdep-sco.h',text,'gdb-4.10.pl1/glob', 243 ).
file('tilde.c',text,'gdb-4.10.pl1/glob', 9263 ).
file('mh-aix',text,'gdb-4.10.pl1/glob/config', 41 ).
file('mh-apollo68v',text,'gdb-4.10.pl1/glob/config', 123 ).
file('mh-sysv4',text,'gdb-4.10.pl1/glob/config', 224 ).
file('COPYING',text,'gdb-4.10.pl1/include', 17982 ).
file('ChangeLog',text,'gdb-4.10.pl1/include', 20581 ).
file('ansidecl.h',text,'gdb-4.10.pl1/include', 4294 ).
file('aout',dir,'gdb-4.10.pl1/include', 512 ).
file('bout.h',text,'gdb-4.10.pl1/include', 6929 ).
file('coff',dir,'gdb-4.10.pl1/include', 512 ).
file('demangle.h',text,'gdb-4.10.pl1/include', 2775 ).
file('dis-asm.h',text,'gdb-4.10.pl1/include', 6887 ).
file('elf',dir,'gdb-4.10.pl1/include', 512 ).
file('fopen-bin.h',text,'gdb-4.10.pl1/include', 918 ).
file('fopen-same.h',text,'gdb-4.10.pl1/include', 898 ).
file('gdbm.h',text,'gdb-4.10.pl1/include', 2464 ).
file('getopt.h',text,'gdb-4.10.pl1/include', 4333 ).
file('ieee-float.h',text,'gdb-4.10.pl1/include', 2347 ).
file('ieee.h',text,'gdb-4.10.pl1/include', 4220 ).
file('nlm',dir,'gdb-4.10.pl1/include', 512 ).
file('oasys.h',text,'gdb-4.10.pl1/include', 3863 ).
file('obstack.h',text,'gdb-4.10.pl1/include', 18564 ).
file('opcode',dir,'gdb-4.10.pl1/include', 512 ).
file('wait.h',text,'gdb-4.10.pl1/include', 1457 ).
file('ChangeLog',text,'gdb-4.10.pl1/include/aout', 2586 ).
file('adobe.h',text,'gdb-4.10.pl1/include/aout', 10416 ).
file('aout64.h',text,'gdb-4.10.pl1/include/aout', 16117 ).
file('ar.h',text,'gdb-4.10.pl1/include/aout', 761 ).
file('dynix3.h',text,'gdb-4.10.pl1/include/aout', 13778 ).
file('encap.h',text,'gdb-4.10.pl1/include/aout', 4722 ).
file('host.h',text,'gdb-4.10.pl1/include/aout', 577 ).
file('hp.h',text,'gdb-4.10.pl1/include/aout', 2930 ).
file('hp300hpux.h',text,'gdb-4.10.pl1/include/aout', 4386 ).
file('hppa.h',text,'gdb-4.10.pl1/include/aout', 104 ).
file('ranlib.h',text,'gdb-4.10.pl1/include/aout', 2463 ).
file('reloc.h',text,'gdb-4.10.pl1/include/aout', 2227 ).
file('stab.def',text,'gdb-4.10.pl1/include/aout', 10785 ).
file('stab_gnu.h',text,'gdb-4.10.pl1/include/aout', 815 ).
file('sun4.h',text,'gdb-4.10.pl1/include/aout', 1345 ).
file('ChangeLog',text,'gdb-4.10.pl1/include/coff', 6465 ).
file('a29k.h',text,'gdb-4.10.pl1/include/coff', 8709 ).
file('alpha.h',text,'gdb-4.10.pl1/include/coff', 5884 ).
file('ecoff-ext.h',text,'gdb-4.10.pl1/include/coff', 11050 ).
file('h8300.h',text,'gdb-4.10.pl1/include/coff', 5233 ).
file('h8500.h',text,'gdb-4.10.pl1/include/coff', 5140 ).
file('i386.h',text,'gdb-4.10.pl1/include/coff', 5567 ).
file('i960.h',text,'gdb-4.10.pl1/include/coff', 6705 ).
file('internal.h',text,'gdb-4.10.pl1/include/coff', 18549 ).
file('m68k.h',text,'gdb-4.10.pl1/include/coff', 5454 ).
file('m88k.h',text,'gdb-4.10.pl1/include/coff', 6536 ).
file('mips.h',text,'gdb-4.10.pl1/include/coff', 6227 ).
file('rs6000.h',text,'gdb-4.10.pl1/include/coff', 6143 ).
file('sh.h',text,'gdb-4.10.pl1/include/coff', 5127 ).
file('sym.h',text,'gdb-4.10.pl1/include/coff', 16030 ).
file('symconst.h',text,'gdb-4.10.pl1/include/coff', 6405 ).
file('we32k.h',text,'gdb-4.10.pl1/include/coff', 5500 ).
file('z8k.h',text,'gdb-4.10.pl1/include/coff', 5209 ).
file('ChangeLog',text,'gdb-4.10.pl1/include/elf', 1944 ).
file('common.h',text,'gdb-4.10.pl1/include/elf', 8162 ).
file('dwarf.h',text,'gdb-4.10.pl1/include/elf', 10157 ).
file('external.h',text,'gdb-4.10.pl1/include/elf', 8168 ).
file('internal.h',text,'gdb-4.10.pl1/include/elf', 7458 ).
file('ChangeLog',text,'gdb-4.10.pl1/include/nlm', 650 ).
file('common.h',text,'gdb-4.10.pl1/include/nlm', 3040 ).
file('external.h',text,'gdb-4.10.pl1/include/nlm', 7978 ).
file('internal.h',text,'gdb-4.10.pl1/include/nlm', 10059 ).
file('ChangeLog',text,'gdb-4.10.pl1/include/opcode', 12389 ).
file('a29k.h',text,'gdb-4.10.pl1/include/opcode', 10112 ).
file('arm.h',text,'gdb-4.10.pl1/include/opcode', 11402 ).
file('convex.h',text,'gdb-4.10.pl1/include/opcode', 42852 ).
file('h8300.h',text,'gdb-4.10.pl1/include/opcode', 23066 ).
file('hppa.h',text,'gdb-4.10.pl1/include/opcode', 20694 ).
file('i386.h',text,'gdb-4.10.pl1/include/opcode', 32186 ).
file('i860.h',text,'gdb-4.10.pl1/include/opcode', 27424 ).
file('i960.h',text,'gdb-4.10.pl1/include/opcode', 18627 ).
file('m68k.h',text,'gdb-4.10.pl1/include/opcode', 136524 ).
file('m68kmri.h',text,'gdb-4.10.pl1/include/opcode', 251 ).
file('m88k.h',text,'gdb-4.10.pl1/include/opcode', 32626 ).
file('mips.h',text,'gdb-4.10.pl1/include/opcode', 11514 ).
file('np1.h',text,'gdb-4.10.pl1/include/opcode', 18168 ).
file('ns32k.h',text,'gdb-4.10.pl1/include/opcode', 25900 ).
file('pn.h',text,'gdb-4.10.pl1/include/opcode', 12397 ).
file('pyr.h',text,'gdb-4.10.pl1/include/opcode', 11238 ).
file('rs6k.h',text,'gdb-4.10.pl1/include/opcode', 9232 ).
file('sparc.h',text,'gdb-4.10.pl1/include/opcode', 5055 ).
file('tahoe.h',text,'gdb-4.10.pl1/include/opcode', 7973 ).
file('vax.h',text,'gdb-4.10.pl1/include/opcode', 13509 ).
file('COPYING.LIB',text,'gdb-4.10.pl1/libiberty', 25265 ).
file('ChangeLog',text,'gdb-4.10.pl1/libiberty', 27949 ).
file('Makefile.in',text,'gdb-4.10.pl1/libiberty', 7340 ).
file('README',text,'gdb-4.10.pl1/libiberty', 5400 ).
file('alloca-botch.h',text,'gdb-4.10.pl1/libiberty', 188 ).
file('alloca-norm.h',text,'gdb-4.10.pl1/libiberty', 217 ).
file('alloca.c',text,'gdb-4.10.pl1/libiberty', 5483 ).
file('argv.c',text,'gdb-4.10.pl1/libiberty', 7387 ).
file('basename.c',text,'gdb-4.10.pl1/libiberty', 1367 ).
file('bcmp.c',text,'gdb-4.10.pl1/libiberty', 1567 ).
file('bcopy.c',text,'gdb-4.10.pl1/libiberty', 1522 ).
file('bzero.c',text,'gdb-4.10.pl1/libiberty', 1286 ).
file('concat.c',text,'gdb-4.10.pl1/libiberty', 3087 ).
file('config',dir,'gdb-4.10.pl1/libiberty', 512 ).
file('config.table',text,'gdb-4.10.pl1/libiberty', 717 ).
file('configure.bat',text,'gdb-4.10.pl1/libiberty', 292 ).
file('configure.in',text,'gdb-4.10.pl1/libiberty', 420 ).
file('copysign.c',text,'gdb-4.10.pl1/libiberty', 2490 ).
file('cplus-dem.c',text,'gdb-4.10.pl1/libiberty', 57894 ).
file('dummy.c',text,'gdb-4.10.pl1/libiberty', 1368 ).
file('fdmatch.c',text,'gdb-4.10.pl1/libiberty', 2041 ).
file('functions.def',text,'gdb-4.10.pl1/libiberty', 2656 ).
file('getcwd.c',text,'gdb-4.10.pl1/libiberty', 1705 ).
file('getopt.c',text,'gdb-4.10.pl1/libiberty', 19056 ).
file('getopt1.c',text,'gdb-4.10.pl1/libiberty', 3598 ).
file('getpagesize.c',text,'gdb-4.10.pl1/libiberty', 2161 ).
file('ieee-float.c',exec,'gdb-4.10.pl1/libiberty', 4887 ).
file('index.c',text,'gdb-4.10.pl1/libiberty', 143 ).
file('insque.c',text,'gdb-4.10.pl1/libiberty', 1703 ).
file('memchr.c',text,'gdb-4.10.pl1/libiberty', 1185 ).
file('memcmp.c',text,'gdb-4.10.pl1/libiberty', 1548 ).
file('memcpy.c',text,'gdb-4.10.pl1/libiberty', 1280 ).
file('memmove.c',text,'gdb-4.10.pl1/libiberty', 1355 ).
file('memset.c',text,'gdb-4.10.pl1/libiberty', 1124 ).
file('msdos.c',text,'gdb-4.10.pl1/libiberty', 206 ).
file('obstack.c',text,'gdb-4.10.pl1/libiberty', 13365 ).
file('random.c',text,'gdb-4.10.pl1/libiberty', 12800 ).
file('rename.c',text,'gdb-4.10.pl1/libiberty', 1110 ).
file('rindex.c',text,'gdb-4.10.pl1/libiberty', 147 ).
file('sigsetmask.c',text,'gdb-4.10.pl1/libiberty', 1415 ).
file('spaces.c',text,'gdb-4.10.pl1/libiberty', 1545 ).
file('strchr.c',text,'gdb-4.10.pl1/libiberty', 1343 ).
file('strdup.c',text,'gdb-4.10.pl1/libiberty', 172 ).
file('strerror.c',text,'gdb-4.10.pl1/libiberty', 20370 ).
file('strrchr.c',text,'gdb-4.10.pl1/libiberty', 1360 ).
file('strsignal.c',text,'gdb-4.10.pl1/libiberty', 14824 ).
file('strstr.c',text,'gdb-4.10.pl1/libiberty', 1717 ).
file('strtod.c',text,'gdb-4.10.pl1/libiberty', 2605 ).
file('strtol.c',text,'gdb-4.10.pl1/libiberty', 1196 ).
file('strtoul.c',text,'gdb-4.10.pl1/libiberty', 1778 ).
file('tmpnam.c',text,'gdb-4.10.pl1/libiberty', 619 ).
file('vfork.c',text,'gdb-4.10.pl1/libiberty', 916 ).
file('vfprintf.c',text,'gdb-4.10.pl1/libiberty', 184 ).
file('vprintf.c',text,'gdb-4.10.pl1/libiberty', 160 ).
file('vsprintf.c',text,'gdb-4.10.pl1/libiberty', 1501 ).
file('mh-a68bsd',text,'gdb-4.10.pl1/libiberty/config', 70 ).
file('mh-aix',text,'gdb-4.10.pl1/libiberty/config', 442 ).
file('mh-apollo68',text,'gdb-4.10.pl1/libiberty/config', 68 ).
file('mh-hpbsd',text,'gdb-4.10.pl1/libiberty/config', 35 ).
file('mh-hpux',text,'gdb-4.10.pl1/libiberty/config', 24 ).
file('mh-irix4',text,'gdb-4.10.pl1/libiberty/config', 65 ).
file('mh-lynxos',text,'gdb-4.10.pl1/libiberty/config', 32 ).
file('mh-ncr3000',text,'gdb-4.10.pl1/libiberty/config', 741 ).
file('mh-sysv',text,'gdb-4.10.pl1/libiberty/config', 12 ).
file('mh-sysv4',text,'gdb-4.10.pl1/libiberty/config', 51 ).
file('ChangeLog',text,'gdb-4.10.pl1/mmalloc', 5720 ).
file('Makefile.in',text,'gdb-4.10.pl1/mmalloc', 3964 ).
file('TODO',text,'gdb-4.10.pl1/mmalloc', 934 ).
file('attach.c',text,'gdb-4.10.pl1/mmalloc', 7115 ).
file('config',dir,'gdb-4.10.pl1/mmalloc', 512 ).
file('configure.in',text,'gdb-4.10.pl1/mmalloc', 624 ).
file('detach.c',text,'gdb-4.10.pl1/mmalloc', 2229 ).
file('keys.c',text,'gdb-4.10.pl1/mmalloc', 2112 ).
file('mcalloc.c',text,'gdb-4.10.pl1/mmalloc', 1714 ).
file('mfree.c',text,'gdb-4.10.pl1/mmalloc', 7393 ).
file('mmalloc.c',text,'gdb-4.10.pl1/mmalloc', 9882 ).
file('mmalloc.h',text,'gdb-4.10.pl1/mmalloc', 12218 ).
file('mmalloc.texi',text,'gdb-4.10.pl1/mmalloc', 9751 ).
file('mmap-sup.c',text,'gdb-4.10.pl1/mmalloc', 4458 ).
file('mmcheck.c',text,'gdb-4.10.pl1/mmalloc', 6002 ).
file('mmemalign.c',text,'gdb-4.10.pl1/mmalloc', 1749 ).
file('mmstats.c',text,'gdb-4.10.pl1/mmalloc', 1673 ).
file('mmtrace.awk',text,'gdb-4.10.pl1/mmalloc', 860 ).
file('mmtrace.c',text,'gdb-4.10.pl1/mmalloc', 4384 ).
file('mrealloc.c',text,'gdb-4.10.pl1/mmalloc', 4563 ).
file('mvalloc.c',text,'gdb-4.10.pl1/mmalloc', 1315 ).
file('sbrk-sup.c',text,'gdb-4.10.pl1/mmalloc', 3282 ).
file('mh-irix4',text,'gdb-4.10.pl1/mmalloc/config', 41 ).
file('mh-ncr3000',text,'gdb-4.10.pl1/mmalloc/config', 697 ).
file('mh-sunos4',text,'gdb-4.10.pl1/mmalloc/config', 70 ).
file('mh-sysv4',text,'gdb-4.10.pl1/mmalloc/config', 193 ).
file('ChangeLog',text,'gdb-4.10.pl1/opcodes', 13644 ).
file('Makefile.in',text,'gdb-4.10.pl1/opcodes', 5225 ).
file('a29k-dis.c',text,'gdb-4.10.pl1/opcodes', 9267 ).
file('alpha-dis.c',text,'gdb-4.10.pl1/opcodes', 3688 ).
file('alpha-opc.h',text,'gdb-4.10.pl1/opcodes', 16809 ).
file('configure.in',text,'gdb-4.10.pl1/opcodes', 739 ).
file('dis-buf.c',text,'gdb-4.10.pl1/opcodes', 2290 ).
file('h8300-dis.c',text,'gdb-4.10.pl1/opcodes', 7410 ).
file('h8500-dis.c',text,'gdb-4.10.pl1/opcodes', 7549 ).
file('h8500-opc.h',text,'gdb-4.10.pl1/opcodes', 282119 ).
file('hppa-dis.c',text,'gdb-4.10.pl1/opcodes', 17533 ).
file('i386-dis.c',text,'gdb-4.10.pl1/opcodes', 37793 ).
file('i960-dis.c',text,'gdb-4.10.pl1/opcodes', 20254 ).
file('m68881-ext.c',text,'gdb-4.10.pl1/opcodes', 157 ).
file('m68k-dis.c',text,'gdb-4.10.pl1/opcodes', 26863 ).
file('m88k-dis.c',text,'gdb-4.10.pl1/opcodes', 8590 ).
file('mips-dis.c',text,'gdb-4.10.pl1/opcodes', 6449 ).
file('mips-opc.c',text,'gdb-4.10.pl1/opcodes', 20130 ).
file('sh-dis.c',text,'gdb-4.10.pl1/opcodes', 5253 ).
file('sh-opc.h',text,'gdb-4.10.pl1/opcodes', 14290 ).
file('sparc-dis.c',text,'gdb-4.10.pl1/opcodes', 14369 ).
file('sparc-opc.c',text,'gdb-4.10.pl1/opcodes', 44959 ).
file('z8k-dis.c',text,'gdb-4.10.pl1/opcodes', 13658 ).
file('z8k-opc.h',text,'gdb-4.10.pl1/opcodes', 120797 ).
file('z8kgen.c',text,'gdb-4.10.pl1/opcodes', 50115 ).
file('COPYING',text,'gdb-4.10.pl1/readline', 17982 ).
file('ChangeLog',text,'gdb-4.10.pl1/readline', 16691 ).
file('Makefile.in',text,'gdb-4.10.pl1/readline', 5578 ).
file('chardefs.h',text,'gdb-4.10.pl1/readline', 1434 ).
file('config',dir,'gdb-4.10.pl1/readline', 512 ).
file('configure.bat',text,'gdb-4.10.pl1/readline', 383 ).
file('configure.in',text,'gdb-4.10.pl1/readline', 1961 ).
file('doc',dir,'gdb-4.10.pl1/readline', 512 ).
file('emacs_keymap.c',text,'gdb-4.10.pl1/readline', 18445 ).
file('examples',dir,'gdb-4.10.pl1/readline', 512 ).
file('funmap.c',text,'gdb-4.10.pl1/readline', 8016 ).
file('history.c',text,'gdb-4.10.pl1/readline', 40138 ).
file('history.h',text,'gdb-4.10.pl1/readline', 4844 ).
file('keymaps.c',text,'gdb-4.10.pl1/readline', 4122 ).
file('keymaps.h',text,'gdb-4.10.pl1/readline', 1537 ).
file('readline.c',text,'gdb-4.10.pl1/readline', 158290 ).
file('readline.h',text,'gdb-4.10.pl1/readline', 8670 ).
file('sysdep-aix.h',text,'gdb-4.10.pl1/readline', 141 ).
file('sysdep-irix.h',text,'gdb-4.10.pl1/readline', 228 ).
file('sysdep-norm.h',text,'gdb-4.10.pl1/readline', 1088 ).
file('sysdep-obsd.h',text,'gdb-4.10.pl1/readline', 268 ).
file('sysdep-sco.h',text,'gdb-4.10.pl1/readline', 243 ).
file('sysdep-sysv4.h',text,'gdb-4.10.pl1/readline', 675 ).
file('vi_keymap.c',text,'gdb-4.10.pl1/readline', 17634 ).
file('vi_mode.c',text,'gdb-4.10.pl1/readline', 24437 ).
file('mh-apollo68v',text,'gdb-4.10.pl1/readline/config', 112 ).
file('mh-isc',text,'gdb-4.10.pl1/readline/config', 111 ).
file('mh-ncr3000',text,'gdb-4.10.pl1/readline/config', 816 ).
file('mh-posix',text,'gdb-4.10.pl1/readline/config', 109 ).
file('mh-sco',text,'gdb-4.10.pl1/readline/config', 130 ).
file('mh-sco4',text,'gdb-4.10.pl1/readline/config', 112 ).
file('mh-sysv',text,'gdb-4.10.pl1/readline/config', 215 ).
file('mh-sysv4',text,'gdb-4.10.pl1/readline/config', 270 ).
file('ChangeLog',text,'gdb-4.10.pl1/readline/doc', 373 ).
file('Makefile.in',text,'gdb-4.10.pl1/readline/doc', 2622 ).
file('configure.in',text,'gdb-4.10.pl1/readline/doc', 94 ).
file('hist.texinfo',text,'gdb-4.10.pl1/readline/doc', 3440 ).
file('hstech.texinfo',text,'gdb-4.10.pl1/readline/doc', 10512 ).
file('hsuser.texinfo',text,'gdb-4.10.pl1/readline/doc', 4753 ).
file('inc-hist.texi',text,'gdb-4.10.pl1/readline/doc', 4771 ).
file('rlman.texinfo',text,'gdb-4.10.pl1/readline/doc', 3417 ).
file('rltech.texinfo',text,'gdb-4.10.pl1/readline/doc', 32512 ).
file('rluser.texinfo',text,'gdb-4.10.pl1/readline/doc', 19951 ).
file('Inputrc',text,'gdb-4.10.pl1/readline/examples', 1503 ).
file('Makefile',text,'gdb-4.10.pl1/readline/examples', 248 ).
file('configure.bat',text,'gdb-4.10.pl1/readline/examples', 308 ).
file('fileman.c',text,'gdb-4.10.pl1/readline/examples', 9914 ).
file('manexamp.c',text,'gdb-4.10.pl1/readline/examples', 2465 ).
file('ChangeLog',text,'gdb-4.10.pl1/sim', 1596 ).
file('Makefile.in',text,'gdb-4.10.pl1/sim', 5001 ).
file('configure.in',text,'gdb-4.10.pl1/sim', 977 ).
file('h8300',dir,'gdb-4.10.pl1/sim', 512 ).
file('h8500',dir,'gdb-4.10.pl1/sim', 512 ).
file('sh',dir,'gdb-4.10.pl1/sim', 512 ).
file('z8k',dir,'gdb-4.10.pl1/sim', 512 ).
file('ChangeLog',text,'gdb-4.10.pl1/sim/h8300', 2231 ).
file('Makefile.in',text,'gdb-4.10.pl1/sim/h8300', 4266 ).
file('compile.c',text,'gdb-4.10.pl1/sim/h8300', 31187 ).
file('configure.in',text,'gdb-4.10.pl1/sim/h8300', 790 ).
file('h8300.mt',text,'gdb-4.10.pl1/sim/h8300', 58 ).
file('inst.h',text,'gdb-4.10.pl1/sim/h8300', 841 ).
file('run.c',text,'gdb-4.10.pl1/sim/h8300', 1766 ).
file('writecode.c',text,'gdb-4.10.pl1/sim/h8300', 19663 ).
file('ChangeLog',text,'gdb-4.10.pl1/sim/h8500', 1119 ).
file('Makefile.in',text,'gdb-4.10.pl1/sim/h8500', 4259 ).
file('compile.c',text,'gdb-4.10.pl1/sim/h8500', 48438 ).
file('configure.in',text,'gdb-4.10.pl1/sim/h8500', 788 ).
file('h8500.mt',text,'gdb-4.10.pl1/sim/h8500', 57 ).
file('inst.h',text,'gdb-4.10.pl1/sim/h8500', 1298 ).
file('run.c',text,'gdb-4.10.pl1/sim/h8500', 2116 ).
file('ChangeLog',text,'gdb-4.10.pl1/sim/sh', 1074 ).
file('Makefile.in',text,'gdb-4.10.pl1/sim/sh', 4506 ).
file('configure.in',text,'gdb-4.10.pl1/sim/sh', 786 ).
file('gencode.c',text,'gdb-4.10.pl1/sim/sh', 20432 ).
file('interp.c',text,'gdb-4.10.pl1/sim/sh', 14137 ).
file('run.c',text,'gdb-4.10.pl1/sim/sh', 2380 ).
file('sh.mt',text,'gdb-4.10.pl1/sim/sh', 49 ).
file('ChangeLog',text,'gdb-4.10.pl1/sim/z8k', 1089 ).
file('Makefile.in',text,'gdb-4.10.pl1/sim/z8k', 4928 ).
file('comped1.c',text,'gdb-4.10.pl1/sim/z8k', 953 ).
file('comped2.c',text,'gdb-4.10.pl1/sim/z8k', 870 ).
file('comped3.c',text,'gdb-4.10.pl1/sim/z8k', 875 ).
file('compedb3.c',text,'gdb-4.10.pl1/sim/z8k', 911 ).
file('configure.in',text,'gdb-4.10.pl1/sim/z8k', 785 ).
file('iface.c',text,'gdb-4.10.pl1/sim/z8k', 2666 ).
file('inlines.h',text,'gdb-4.10.pl1/sim/z8k', 11794 ).
file('list.c',text,'gdb-4.10.pl1/sim/z8k', 308654 ).
file('mem.c',text,'gdb-4.10.pl1/sim/z8k', 2535 ).
file('mem.h',text,'gdb-4.10.pl1/sim/z8k', 404 ).
file('quick.c',text,'gdb-4.10.pl1/sim/z8k', 17128 ).
file('run.c',text,'gdb-4.10.pl1/sim/z8k', 2078 ).
file('sim.h',text,'gdb-4.10.pl1/sim/z8k', 689 ).
file('support.c',text,'gdb-4.10.pl1/sim/z8k', 10266 ).
file('syscall.h',text,'gdb-4.10.pl1/sim/z8k', 1204 ).
file('tm.h',text,'gdb-4.10.pl1/sim/z8k', 6445 ).
file('writecode.c',text,'gdb-4.10.pl1/sim/z8k', 36673 ).
file('z8k.mt',text,'gdb-4.10.pl1/sim/z8k', 51 ).
file('gpl.texinfo',text,'gdb-4.10.pl1/texinfo', 18545 ).
file('tex3patch',exec,'gdb-4.10.pl1/texinfo', 1762 ).
file('texinfo.tex',text,'gdb-4.10.pl1/texinfo', 131666 ).
file('COPYING',text,'glibc-1.06', 17982 ).
file('COPYING.LIB',text,'glibc-1.06', 25265 ).
file('ChangeLog',text,'glibc-1.06', 181017 ).
file('INSTALL',text,'glibc-1.06', 32227 ).
file('Make-dist',text,'glibc-1.06', 6635 ).
file('MakeTAGS',text,'glibc-1.06', 3084 ).
file('Makeconfig',text,'glibc-1.06', 9407 ).
file('Makefile',text,'glibc-1.06', 6433 ).
file('Makerules',text,'glibc-1.06', 11684 ).
file('NEWS',text,'glibc-1.06', 7176 ).
file('NOTES',text,'glibc-1.06', 3433 ).
file('README',text,'glibc-1.06', 2193 ).
file('Rules',text,'glibc-1.06', 3390 ).
file('TAGS',text,'glibc-1.06', 12512 ).
file('__mkdir.S',text,'glibc-1.06', 41 ).
file('__rmdir.S',text,'glibc-1.06', 41 ).
file('a.out.h',text,'glibc-1.06', 24 ).
file('aclocal.m4',text,'glibc-1.06', 508 ).
file('alloca.h',text,'glibc-1.06', 27 ).
file('ansidecl.h',text,'glibc-1.06', 3001 ).
file('arpa',dir,'glibc-1.06', 512 ).
file('assert',dir,'glibc-1.06', 512 ).
file('assert.h',text,'glibc-1.06', 27 ).
file('bytesex.h',text,'glibc-1.06', 26 ).
file('conf',dir,'glibc-1.06', 512 ).
file('config.guess',exec,'glibc-1.06', 4427 ).
file('config.make',text,'glibc-1.06', 584 ).
file('config.status',exec,'glibc-1.06', 231 ).
file('config.sub',exec,'glibc-1.06', 13774 ).
file('configure',exec,'glibc-1.06', 17159 ).
file('configure.in',text,'glibc-1.06', 14005 ).
file('ctype',dir,'glibc-1.06', 512 ).
file('ctype.h',text,'glibc-1.06', 25 ).
file('depend-',text,'glibc-1.06', 116 ).
file('det_endian',exec,'glibc-1.06', 24576 ).
file('dirent',dir,'glibc-1.06', 1024 ).
file('dirent.h',text,'glibc-1.06', 27 ).
file('endian.h',text,'glibc-1.06', 27 ).
file('errno.h',text,'glibc-1.06', 1122 ).
file('errnos',text,'glibc-1.06', 760 ).
file('errnos.h',text,'glibc-1.06', 2266 ).
file('fcntl.h',text,'glibc-1.06', 22 ).
file('features.h',text,'glibc-1.06', 4971 ).
file('fnmatch.h',text,'glibc-1.06', 27 ).
file('ftw.h',text,'glibc-1.06', 20 ).
file('getopt.h',text,'glibc-1.06', 26 ).
file('glob.h',text,'glibc-1.06', 24 ).
file('gnu',dir,'glibc-1.06', 512 ).
file('gnu-stabs.h',text,'glibc-1.06', 2908 ).
file('gnulib',dir,'glibc-1.06', 512 ).
file('grp',dir,'glibc-1.06', 1024 ).
file('grp.h',text,'glibc-1.06', 21 ).
file('hurd',dir,'glibc-1.06', 1024 ).
file('inet',dir,'glibc-1.06', 2560 ).
file('io',dir,'glibc-1.06', 3584 ).
file('ioctls',text,'glibc-1.06', 2308 ).
file('ioctls.h',text,'glibc-1.06', 6897 ).
file('libc.a',text,'glibc-1.06', 1746240 ).
file('limits.h',text,'glibc-1.06', 2964 ).
file('local_lim.h',text,'glibc-1.06', 236 ).
file('locale',dir,'glibc-1.06', 1024 ).
file('locale.h',text,'glibc-1.06', 27 ).
file('localeinfo.h',text,'glibc-1.06', 31 ).
file('mach',dir,'glibc-1.06', 512 ).
file('make-errnos',exec,'glibc-1.06', 24576 ).
file('make-errnos.c',text,'glibc-1.06', 6771 ).
file('make-ioctls',exec,'glibc-1.06', 32768 ).
file('make-ioctls.c',text,'glibc-1.06', 15954 ).
file('malloc',dir,'glibc-1.06', 1536 ).
file('malloc.h',text,'glibc-1.06', 27 ).
file('manual',dir,'glibc-1.06', 2048 ).
file('math',dir,'glibc-1.06', 2560 ).
file('math.h',text,'glibc-1.06', 23 ).
file('memory.h',text,'glibc-1.06', 27 ).
file('misc',dir,'glibc-1.06', 3072 ).
file('mk-local_lim',exec,'glibc-1.06', 24576 ).
file('mk-stdiolim',exec,'glibc-1.06', 24576 ).
file('mkinstalldirs',exec,'glibc-1.06', 616 ).
file('munch-init.c',text,'glibc-1.06', 946 ).
file('munch-init.dep',text,'glibc-1.06', 57 ).
file('munch-tmpl.c',text,'glibc-1.06', 962 ).
file('munch.awk',text,'glibc-1.06', 396 ).
file('netdb.h',text,'glibc-1.06', 24 ).
file('netinet',dir,'glibc-1.06', 512 ).
file('nlist.h',text,'glibc-1.06', 24 ).
file('obstack.h',text,'glibc-1.06', 28 ).
file('param.h.c',text,'glibc-1.06', 4890 ).
file('param.h.dep',text,'glibc-1.06', 240 ).
file('paths.h',text,'glibc-1.06', 24 ).
file('posix',dir,'glibc-1.06', 4096 ).
file('posix1_lim.h',text,'glibc-1.06', 30 ).
file('posix2_lim.h',text,'glibc-1.06', 30 ).
file('printf.h',text,'glibc-1.06', 26 ).
file('protocols',dir,'glibc-1.06', 512 ).
file('pwd',dir,'glibc-1.06', 1024 ).
file('pwd.h',text,'glibc-1.06', 21 ).
file('resolv.h',text,'glibc-1.06', 25 ).
file('resource',dir,'glibc-1.06', 1024 ).
file('set-init.c',text,'glibc-1.06', 1225 ).
file('setjmp',dir,'glibc-1.06', 1024 ).
file('setjmp.h',text,'glibc-1.06', 27 ).
file('sgtty.h',text,'glibc-1.06', 24 ).
file('signal',dir,'glibc-1.06', 2048 ).
file('signal.h',text,'glibc-1.06', 27 ).
file('socket',dir,'glibc-1.06', 1024 ).
file('stab.def',text,'glibc-1.06', 25 ).
file('stab.h',text,'glibc-1.06', 23 ).
file('start.dep',text,'glibc-1.06', 604 ).
file('stdarg.h',text,'glibc-1.06', 4429 ).
file('stddef.h',text,'glibc-1.06', 5732 ).
file('stdio',dir,'glibc-1.06', 6144 ).
file('stdio.h',text,'glibc-1.06', 25 ).
file('stdio_lim.h',text,'glibc-1.06', 157 ).
file('stdlib',dir,'glibc-1.06', 2560 ).
file('stdlib.h',text,'glibc-1.06', 27 ).
file('string',dir,'glibc-1.06', 2560 ).
file('string.h',text,'glibc-1.06', 27 ).
file('strings.h',text,'glibc-1.06', 28 ).
file('sys',dir,'glibc-1.06', 512 ).
file('syscall.h',text,'glibc-1.06', 3507 ).
file('sysd-Makefile',text,'glibc-1.06', 6382 ).
file('sysd-dirs',text,'glibc-1.06', 33 ).
file('sysdep.dep',text,'glibc-1.06', 45 ).
file('sysdeps',dir,'glibc-1.06', 512 ).
file('tar.h',text,'glibc-1.06', 23 ).
file('termios',dir,'glibc-1.06', 1024 ).
file('termios.h',text,'glibc-1.06', 29 ).
file('time',dir,'glibc-1.06', 2560 ).
file('time.h',text,'glibc-1.06', 23 ).
file('ttyent.h',text,'glibc-1.06', 25 ).
file('unistd.h',text,'glibc-1.06', 26 ).
file('utime.h',text,'glibc-1.06', 25 ).
file('va-alpha.h',text,'glibc-1.06', 2470 ).
file('va-i860.h',text,'glibc-1.06', 6371 ).
file('va-i960.h',text,'glibc-1.06', 2074 ).
file('va-m88k.h',text,'glibc-1.06', 2543 ).
file('va-mips.h',text,'glibc-1.06', 2178 ).
file('va-pa.h',text,'glibc-1.06', 1383 ).
file('va-pyr.h',text,'glibc-1.06', 4045 ).
file('va-sparc.h',text,'glibc-1.06', 2807 ).
file('va-spur.h',text,'glibc-1.06', 1662 ).
file('version.c',text,'glibc-1.06', 1412 ).
file('version.dep',text,'glibc-1.06', 279 ).
file('wordexp.h',text,'glibc-1.06', 27 ).
file('ftp.h',text,'glibc-1.06/arpa', 27 ).
file('inet.h',text,'glibc-1.06/arpa', 28 ).
file('nameser.h',text,'glibc-1.06/arpa', 31 ).
file('telnet.h',text,'glibc-1.06/arpa', 30 ).
file('tftp.h',text,'glibc-1.06/arpa', 28 ).
file('Makefile',text,'glibc-1.06/assert', 955 ).
file('TAGS',text,'glibc-1.06/assert', 131 ).
file('assert.c',text,'glibc-1.06/assert', 1616 ).
file('assert.dep',text,'glibc-1.06/assert', 398 ).
file('assert.h',text,'glibc-1.06/assert', 1600 ).
file('depend-assert',text,'glibc-1.06/assert', 28 ).
file('portability.h',text,'glibc-1.06/conf', 286 ).
file('Makefile',text,'glibc-1.06/ctype', 1015 ).
file('TAGS',text,'glibc-1.06/ctype', 1103 ).
file('ctype-extn.c',text,'glibc-1.06/ctype', 1248 ).
file('ctype-extn.dep',text,'glibc-1.06/ctype', 121 ).
file('ctype-glue.c',text,'glibc-1.06/ctype', 1952 ).
file('ctype-glue.dep',text,'glibc-1.06/ctype', 58 ).
file('ctype-info.c',text,'glibc-1.06/ctype', 1244 ).
file('ctype-info.dep',text,'glibc-1.06/ctype', 348 ).
file('ctype.c',text,'glibc-1.06/ctype', 1432 ).
file('ctype.dep',text,'glibc-1.06/ctype', 102 ).
file('ctype.h',text,'glibc-1.06/ctype', 5013 ).
file('depend-ctype',text,'glibc-1.06/ctype', 155 ).
file('glue-ctype',exec,'glibc-1.06/ctype', 24576 ).
file('test_ctype.c',text,'glibc-1.06/ctype', 2125 ).
file('test_ctype.dep',text,'glibc-1.06/ctype', 585 ).
file('Makefile',text,'glibc-1.06/dirent', 1110 ).
file('TAGS',text,'glibc-1.06/dirent', 1731 ).
file('__getdents.dep',text,'glibc-1.06/dirent', 141 ).
file('alphasort.c',text,'glibc-1.06/dirent', 1039 ).
file('alphasort.dep',text,'glibc-1.06/dirent', 255 ).
file('closedir.dep',text,'glibc-1.06/dirent', 422 ).
file('depend-dirent',text,'glibc-1.06/dirent', 325 ).
file('dirent.h',text,'glibc-1.06/dirent', 4130 ).
file('getdents.c',text,'glibc-1.06/dirent', 1139 ).
file('getdents.dep',text,'glibc-1.06/dirent', 301 ).
file('list.c',text,'glibc-1.06/dirent', 1572 ).
file('list.dep',text,'glibc-1.06/dirent', 449 ).
file('opendir.dep',text,'glibc-1.06/dirent', 851 ).
file('readdir.dep',text,'glibc-1.06/dirent', 597 ).
file('rewinddir.dep',text,'glibc-1.06/dirent', 401 ).
file('scandir.c',text,'glibc-1.06/dirent', 2082 ).
file('scandir.dep',text,'glibc-1.06/dirent', 307 ).
file('seekdir.dep',text,'glibc-1.06/dirent', 353 ).
file('telldir.dep',text,'glibc-1.06/dirent', 418 ).
file('types.h',text,'glibc-1.06/gnu', 29 ).
file('Makefile',text,'glibc-1.06/gnulib', 934 ).
file('depend-gnulib',text,'glibc-1.06/gnulib', 154 ).
file('mul.dep',text,'glibc-1.06/gnulib', 64 ).
file('rem.dep',text,'glibc-1.06/gnulib', 135 ).
file('sdiv.dep',text,'glibc-1.06/gnulib', 138 ).
file('udiv.dep',text,'glibc-1.06/gnulib', 138 ).
file('umul.dep',text,'glibc-1.06/gnulib', 67 ).
file('urem.dep',text,'glibc-1.06/gnulib', 138 ).
file('Makefile',text,'glibc-1.06/grp', 1047 ).
file('TAGS',text,'glibc-1.06/grp', 708 ).
file('depend-grp',text,'glibc-1.06/grp', 271 ).
file('fgetgrent.c',text,'glibc-1.06/grp', 1221 ).
file('fgetgrent.dep',text,'glibc-1.06/grp', 435 ).
file('getgrent.c',text,'glibc-1.06/grp', 1516 ).
file('getgrent.dep',text,'glibc-1.06/grp', 312 ).
file('getgrgid.c',text,'glibc-1.06/grp', 1417 ).
file('getgrgid.dep',text,'glibc-1.06/grp', 312 ).
file('getgrnam.c',text,'glibc-1.06/grp', 1419 ).
file('getgrnam.dep',text,'glibc-1.06/grp', 343 ).
file('grp.h',text,'glibc-1.06/grp', 2881 ).
file('grpopen.c',text,'glibc-1.06/grp', 1003 ).
file('grpopen.dep',text,'glibc-1.06/grp', 309 ).
file('grpread.c',text,'glibc-1.06/grp', 3167 ).
file('grpread.dep',text,'glibc-1.06/grp', 429 ).
file('initgroups.c',text,'glibc-1.06/grp', 1869 ).
file('initgroups.dep',text,'glibc-1.06/grp', 563 ).
file('setgroups.dep',text,'glibc-1.06/grp', 137 ).
file('testgrp.c',text,'glibc-1.06/grp', 1038 ).
file('testgrp.dep',text,'glibc-1.06/grp', 516 ).
file('Makefile',text,'glibc-1.06/hurd', 2265 ).
file('TAGS',text,'glibc-1.06/hurd', 2804 ).
file('__fopenport.c',text,'glibc-1.06/hurd', 1150 ).
file('__getdport.c',text,'glibc-1.06/hurd', 1472 ).
file('__pid2task.c',text,'glibc-1.06/hurd', 1087 ).
file('__setauth.c',text,'glibc-1.06/hurd', 3090 ).
file('__task2pid.c',text,'glibc-1.06/hurd', 1045 ).
file('dead_recv.c',text,'glibc-1.06/hurd', 1129 ).
file('dtable.c',text,'glibc-1.06/hurd', 5865 ).
file('fchroot.c',text,'glibc-1.06/hurd', 1263 ).
file('fopenport.c',text,'glibc-1.06/hurd', 888 ).
file('getdport.c',text,'glibc-1.06/hurd', 886 ).
file('getuids.c',text,'glibc-1.06/hurd', 1631 ).
file('getumask.c',text,'glibc-1.06/hurd', 893 ).
file('hurd.h',text,'glibc-1.06/hurd', 16057 ).
file('hurdauth.c',text,'glibc-1.06/hurd', 2789 ).
file('hurdfrob.c',text,'glibc-1.06/hurd', 3276 ).
file('hurdid.c',text,'glibc-1.06/hurd', 1070 ).
file('hurdinit.c',text,'glibc-1.06/hurd', 3670 ).
file('hurdintr.awk',text,'glibc-1.06/hurd', 582 ).
file('hurdpath.c',text,'glibc-1.06/hurd', 4290 ).
file('hurdpid.c',text,'glibc-1.06/hurd', 1406 ).
file('hurdports.c',text,'glibc-1.06/hurd', 1831 ).
file('hurdsig.c',text,'glibc-1.06/hurd', 15724 ).
file('hurdsock.c',text,'glibc-1.06/hurd', 2520 ).
file('hurdsyms.c',text,'glibc-1.06/hurd', 1037 ).
file('intr_rpc.awk',text,'glibc-1.06/hurd', 517 ).
file('msgportdemux.c',text,'glibc-1.06/hurd', 1592 ).
file('openport.c',text,'glibc-1.06/hurd', 1325 ).
file('pid2task.c',text,'glibc-1.06/hurd', 886 ).
file('setauth.c',text,'glibc-1.06/hurd', 884 ).
file('setdtsz.c',text,'glibc-1.06/hurd', 2560 ).
file('task2pid.c',text,'glibc-1.06/hurd', 886 ).
file('vpprintf.c',text,'glibc-1.06/hurd', 1839 ).
file('Makefile',text,'glibc-1.06/inet', 1511 ).
file('TAGS',text,'glibc-1.06/inet', 15938 ).
file('arpa',dir,'glibc-1.06/inet', 512 ).
file('depend-inet',text,'glibc-1.06/inet', 908 ).
file('gethstnmad.c',text,'glibc-1.06/inet', 12041 ).
file('gethstnmad.dep',text,'glibc-1.06/inet', 883 ).
file('getnetbyad.c',text,'glibc-1.06/inet', 2288 ).
file('getnetbyad.dep',text,'glibc-1.06/inet', 537 ).
file('getnetbynm.c',text,'glibc-1.06/inet', 2399 ).
file('getnetbynm.dep',text,'glibc-1.06/inet', 568 ).
file('getnetent.c',text,'glibc-1.06/inet', 3349 ).
file('getnetent.dep',text,'glibc-1.06/inet', 710 ).
file('getproto.c',text,'glibc-1.06/inet', 2257 ).
file('getproto.dep',text,'glibc-1.06/inet', 531 ).
file('getprtent.c',text,'glibc-1.06/inet', 3365 ).
file('getprtent.dep',text,'glibc-1.06/inet', 770 ).
file('getprtname.c',text,'glibc-1.06/inet', 2417 ).
file('getprtname.dep',text,'glibc-1.06/inet', 568 ).
file('getservent.c',text,'glibc-1.06/inet', 3395 ).
file('getservent.dep',text,'glibc-1.06/inet', 773 ).
file('getsrvbynm.c',text,'glibc-1.06/inet', 2495 ).
file('getsrvbynm.dep',text,'glibc-1.06/inet', 568 ).
file('getsrvbypt.c',text,'glibc-1.06/inet', 2356 ).
file('getsrvbypt.dep',text,'glibc-1.06/inet', 568 ).
file('herror.c',text,'glibc-1.06/inet', 4144 ).
file('herror.dep',text,'glibc-1.06/inet', 678 ).
file('htonl.dep',text,'glibc-1.06/inet', 229 ).
file('htons.dep',text,'glibc-1.06/inet', 229 ).
file('inet_addr.c',text,'glibc-1.06/inet', 5446 ).
file('inet_addr.dep',text,'glibc-1.06/inet', 746 ).
file('inet_lnaof.c',text,'glibc-1.06/inet', 2392 ).
file('inet_lnaof.dep',text,'glibc-1.06/inet', 599 ).
file('inet_mkadr.c',text,'glibc-1.06/inet', 2579 ).
file('inet_mkadr.dep',text,'glibc-1.06/inet', 599 ).
file('inet_net.c',text,'glibc-1.06/inet', 3007 ).
file('inet_net.dep',text,'glibc-1.06/inet', 621 ).
file('inet_netof.c',text,'glibc-1.06/inet', 2428 ).
file('inet_netof.dep',text,'glibc-1.06/inet', 599 ).
file('inet_ntoa.c',text,'glibc-1.06/inet', 2380 ).
file('inet_ntoa.dep',text,'glibc-1.06/inet', 665 ).
file('netdb.h',text,'glibc-1.06/inet', 5934 ).
file('netinet',dir,'glibc-1.06/inet', 512 ).
file('ntohl.dep',text,'glibc-1.06/inet', 229 ).
file('ntohs.dep',text,'glibc-1.06/inet', 229 ).
file('protocols',dir,'glibc-1.06/inet', 512 ).
file('rcmd.c',text,'glibc-1.06/inet', 8554 ).
file('rcmd.dep',text,'glibc-1.06/inet', 1005 ).
file('res_comp.c',text,'glibc-1.06/inet', 9290 ).
file('res_comp.dep',text,'glibc-1.06/inet', 798 ).
file('res_debug.c',text,'glibc-1.06/inet', 17157 ).
file('res_debug.dep',text,'glibc-1.06/inet', 815 ).
file('res_init.c',text,'glibc-1.06/inet', 7946 ).
file('res_init.dep',text,'glibc-1.06/inet', 812 ).
file('res_mkqry.c',text,'glibc-1.06/inet', 7620 ).
file('res_mkqry.dep',text,'glibc-1.06/inet', 801 ).
file('res_query.c',text,'glibc-1.06/inet', 9587 ).
file('res_query.dep',text,'glibc-1.06/inet', 880 ).
file('res_send.c',text,'glibc-1.06/inet', 12657 ).
file('res_send.dep',text,'glibc-1.06/inet', 903 ).
file('resolv.h',text,'glibc-1.06/inet', 6725 ).
file('rexec.c',text,'glibc-1.06/inet', 3090 ).
file('rexec.dep',text,'glibc-1.06/inet', 676 ).
file('sethostent.c',text,'glibc-1.06/inet', 2251 ).
file('sethostent.dep',text,'glibc-1.06/inet', 696 ).
file('sys',dir,'glibc-1.06/inet', 512 ).
file('ftp.h',text,'glibc-1.06/inet/arpa', 3646 ).
file('inet.h',text,'glibc-1.06/inet/arpa', 3677 ).
file('nameser.h',text,'glibc-1.06/inet/arpa', 10895 ).
file('telnet.h',text,'glibc-1.06/inet/arpa', 10035 ).
file('tftp.h',text,'glibc-1.06/inet/arpa', 2960 ).
file('in.h',text,'glibc-1.06/inet/netinet', 6551 ).
file('routed.h',text,'glibc-1.06/inet/protocols', 3999 ).
file('rwhod.h',text,'glibc-1.06/inet/protocols', 2687 ).
file('talkd.h',text,'glibc-1.06/inet/protocols', 4860 ).
file('timed.h',text,'glibc-1.06/inet/protocols', 3992 ).
file('bitypes.h',text,'glibc-1.06/inet/sys', 3236 ).
file('Makefile',text,'glibc-1.06/io', 1566 ).
file('TAGS',text,'glibc-1.06/io', 19612 ).
file('__access.dep',text,'glibc-1.06/io', 134 ).
file('__chdir.dep',text,'glibc-1.06/io', 124 ).
file('__chmod.dep',text,'glibc-1.06/io', 124 ).
file('__chown.dep',text,'glibc-1.06/io', 124 ).
file('__close.dep',text,'glibc-1.06/io', 124 ).
file('__dup.dep',text,'glibc-1.06/io', 118 ).
file('__dup2.dep',text,'glibc-1.06/io', 128 ).
file('__fchmod.dep',text,'glibc-1.06/io', 134 ).
file('__fchown.dep',text,'glibc-1.06/io', 134 ).
file('__fcntl.dep',text,'glibc-1.06/io', 124 ).
file('__flock.dep',text,'glibc-1.06/io', 128 ).
file('__fstat.dep',text,'glibc-1.06/io', 124 ).
file('__isatty.dep',text,'glibc-1.06/io', 397 ).
file('__link.dep',text,'glibc-1.06/io', 121 ).
file('__lseek.dep',text,'glibc-1.06/io', 124 ).
file('__lstat.dep',text,'glibc-1.06/io', 131 ).
file('__mkdir.dep',text,'glibc-1.06/io', 131 ).
file('__open.dep',text,'glibc-1.06/io', 121 ).
file('__pipe.dep',text,'glibc-1.06/io', 127 ).
file('__read.dep',text,'glibc-1.06/io', 121 ).
file('__readlink.dep',text,'glibc-1.06/io', 140 ).
file('__rmdir.dep',text,'glibc-1.06/io', 131 ).
file('__stat.dep',text,'glibc-1.06/io', 121 ).
file('__symlink.dep',text,'glibc-1.06/io', 137 ).
file('__umask.dep',text,'glibc-1.06/io', 124 ).
file('__unlink.dep',text,'glibc-1.06/io', 127 ).
file('__write.dep',text,'glibc-1.06/io', 124 ).
file('access.c',text,'glibc-1.06/io', 1025 ).
file('access.dep',text,'glibc-1.06/io', 258 ).
file('chdir.c',text,'glibc-1.06/io', 997 ).
file('chdir.dep',text,'glibc-1.06/io', 255 ).
file('chmod.c',text,'glibc-1.06/io', 1035 ).
file('chmod.dep',text,'glibc-1.06/io', 190 ).
file('chown.c',text,'glibc-1.06/io', 1074 ).
file('chown.dep',text,'glibc-1.06/io', 255 ).
file('close.c',text,'glibc-1.06/io', 982 ).
file('close.dep',text,'glibc-1.06/io', 255 ).
file('creat.c',text,'glibc-1.06/io', 1070 ).
file('creat.dep',text,'glibc-1.06/io', 262 ).
file('depend-io',text,'glibc-1.06/io', 1792 ).
file('dup.c',text,'glibc-1.06/io', 994 ).
file('dup.dep',text,'glibc-1.06/io', 306 ).
file('dup2.c',text,'glibc-1.06/io', 1039 ).
file('dup2.dep',text,'glibc-1.06/io', 332 ).
file('fchdir.dep',text,'glibc-1.06/io', 282 ).
file('fchmod.c',text,'glibc-1.06/io', 1046 ).
file('fchmod.dep',text,'glibc-1.06/io', 270 ).
file('fchown.c',text,'glibc-1.06/io', 1083 ).
file('fchown.dep',text,'glibc-1.06/io', 323 ).
file('fcntl.c',text,'glibc-1.06/io', 1037 ).
file('fcntl.dep',text,'glibc-1.06/io', 237 ).
file('fcntl.h',text,'glibc-1.06/io', 2485 ).
file('flock.c',text,'glibc-1.06/io', 1031 ).
file('flock.dep',text,'glibc-1.06/io', 237 ).
file('fstat.c',text,'glibc-1.06/io', 1015 ).
file('fstat.dep',text,'glibc-1.06/io', 190 ).
file('ftw.c',text,'glibc-1.06/io', 4250 ).
file('ftw.dep',text,'glibc-1.06/io', 585 ).
file('ftw.h',text,'glibc-1.06/io', 1445 ).
file('getcwd.dep',text,'glibc-1.06/io', 585 ).
file('getdirname.c',text,'glibc-1.06/io', 1491 ).
file('getdirname.dep',text,'glibc-1.06/io', 422 ).
file('getwd.c',text,'glibc-1.06/io', 1487 ).
file('getwd.dep',text,'glibc-1.06/io', 470 ).
file('isatty.c',text,'glibc-1.06/io', 987 ).
file('isatty.dep',text,'glibc-1.06/io', 258 ).
file('link.c',text,'glibc-1.06/io', 1020 ).
file('link.dep',text,'glibc-1.06/io', 252 ).
file('lseek.c',text,'glibc-1.06/io', 1053 ).
file('lseek.dep',text,'glibc-1.06/io', 320 ).
file('lstat.c',text,'glibc-1.06/io', 1036 ).
file('lstat.dep',text,'glibc-1.06/io', 190 ).
file('mkdir.c',text,'glibc-1.06/io', 1034 ).
file('mkdir.dep',text,'glibc-1.06/io', 190 ).
file('mkfifo.dep',text,'glibc-1.06/io', 298 ).
file('open.c',text,'glibc-1.06/io', 1385 ).
file('open.dep',text,'glibc-1.06/io', 212 ).
file('pipe.c',text,'glibc-1.06/io', 1016 ).
file('pipe.dep',text,'glibc-1.06/io', 275 ).
file('pwd',exec,'glibc-1.06/io', 286720 ).
file('pwd.c',text,'glibc-1.06/io', 1119 ).
file('pwd.dep',text,'glibc-1.06/io', 457 ).
file('read.c',text,'glibc-1.06/io', 1058 ).
file('read.dep',text,'glibc-1.06/io', 252 ).
file('readlink.c',text,'glibc-1.06/io', 1065 ).
file('readlink.dep',text,'glibc-1.06/io', 264 ).
file('rmdir.c',text,'glibc-1.06/io', 997 ).
file('rmdir.dep',text,'glibc-1.06/io', 255 ).
file('stat.c',text,'glibc-1.06/io', 1038 ).
file('stat.dep',text,'glibc-1.06/io', 187 ).
file('symlink.c',text,'glibc-1.06/io', 1032 ).
file('symlink.dep',text,'glibc-1.06/io', 261 ).
file('sys',dir,'glibc-1.06/io', 512 ).
file('ttyname.dep',text,'glibc-1.06/io', 698 ).
file('umask.c',text,'glibc-1.06/io', 1008 ).
file('umask.dep',text,'glibc-1.06/io', 190 ).
file('unlink.c',text,'glibc-1.06/io', 1000 ).
file('unlink.dep',text,'glibc-1.06/io', 258 ).
file('utime.dep',text,'glibc-1.06/io', 368 ).
file('utime.h',text,'glibc-1.06/io', 1469 ).
file('write.c',text,'glibc-1.06/io', 1059 ).
file('write.dep',text,'glibc-1.06/io', 255 ).
file('stat.h',text,'glibc-1.06/io/sys', 5326 ).
file('C-collate.c',text,'glibc-1.06/locale', 190 ).
file('C-collate.dep',text,'glibc-1.06/locale', 307 ).
file('C-ctype.c',text,'glibc-1.06/locale', 373 ).
file('C-ctype.dep',text,'glibc-1.06/locale', 301 ).
file('C-ctype_ct.c',text,'glibc-1.06/locale', 9828 ).
file('C-ctype_ct.dep',text,'glibc-1.06/locale', 342 ).
file('C-ctype_mb.c',text,'glibc-1.06/locale', 139 ).
file('C-ctype_mb.dep',text,'glibc-1.06/locale', 310 ).
file('C-monetary.c',text,'glibc-1.06/locale', 398 ).
file('C-monetary.dep',text,'glibc-1.06/locale', 310 ).
file('C-numeric.c',text,'glibc-1.06/locale', 222 ).
file('C-numeric.dep',text,'glibc-1.06/locale', 307 ).
file('C-response.c',text,'glibc-1.06/locale', 243 ).
file('C-response.dep',text,'glibc-1.06/locale', 310 ).
file('C-time.c',text,'glibc-1.06/locale', 1126 ).
file('C-time.dep',text,'glibc-1.06/locale', 298 ).
file('Makefile',text,'glibc-1.06/locale', 1078 ).
file('TAGS',text,'glibc-1.06/locale', 504 ).
file('depend-locale',text,'glibc-1.06/locale', 310 ).
file('locale.h',text,'glibc-1.06/locale', 3686 ).
file('localeconv.c',text,'glibc-1.06/locale', 2119 ).
file('localeconv.dep',text,'glibc-1.06/locale', 325 ).
file('localeinfo.h',text,'glibc-1.06/locale', 4987 ).
file('setlocale.c',text,'glibc-1.06/locale', 1605 ).
file('setlocale.dep',text,'glibc-1.06/locale', 376 ).
file('Machrules',text,'glibc-1.06/mach', 4545 ).
file('Makefile',text,'glibc-1.06/mach', 5345 ).
file('TAGS',text,'glibc-1.06/mach', 971 ).
file('__msg.c',text,'glibc-1.06/mach', 2538 ).
file('__msg_dest.c',text,'glibc-1.06/mach', 3979 ).
file('__msg_server.c',text,'glibc-1.06/mach', 3797 ).
file('bootprivport.c',text,'glibc-1.06/mach', 2119 ).
file('devstream.c',text,'glibc-1.06/mach', 3863 ).
file('hello.c',text,'glibc-1.06/mach', 1457 ).
file('mach.h',text,'glibc-1.06/mach', 1480 ).
file('mach_init.c',text,'glibc-1.06/mach', 1193 ).
file('mach_init.h',text,'glibc-1.06/mach', 1766 ).
file('mig_strncpy.c',text,'glibc-1.06/mach', 298 ).
file('mig_support.c',text,'glibc-1.06/mach', 1875 ).
file('mig_syms.c',text,'glibc-1.06/mach', 161 ).
file('msg.c',text,'glibc-1.06/mach', 161 ).
file('msg_destroy.c',text,'glibc-1.06/mach', 101 ).
file('msg_server.c',text,'glibc-1.06/mach', 98 ).
file('msg_server_t.c',text,'glibc-1.06/mach', 122 ).
file('shortcut.awk',text,'glibc-1.06/mach', 685 ).
file('syscalls.awk',text,'glibc-1.06/mach', 119 ).
file('vm_page_size.c',text,'glibc-1.06/mach', 69 ).
file('Makefile',text,'glibc-1.06/malloc', 3044 ).
file('OChangeLog',text,'glibc-1.06/malloc', 1118 ).
file('TAGS',text,'glibc-1.06/malloc', 4858 ).
file('TODO',text,'glibc-1.06/malloc', 1284 ).
file('calloc.c',text,'glibc-1.06/malloc', 1321 ).
file('calloc.dep',text,'glibc-1.06/malloc', 498 ).
file('cfree.c',text,'glibc-1.06/malloc', 1131 ).
file('cfree.dep',text,'glibc-1.06/malloc', 528 ).
file('depend-malloc',text,'glibc-1.06/malloc', 397 ).
file('dist-Makefile',text,'glibc-1.06/malloc', 1741 ).
file('dist-README',text,'glibc-1.06/malloc', 658 ).
file('free.c',text,'glibc-1.06/malloc', 6320 ).
file('free.dep',text,'glibc-1.06/malloc', 492 ).
file('getpagesize.h',text,'glibc-1.06/malloc', 610 ).
file('malloc',dir,'glibc-1.06/malloc', 512 ).
file('malloc.c',text,'glibc-1.06/malloc', 9021 ).
file('malloc.dep',text,'glibc-1.06/malloc', 498 ).
file('malloc.h',text,'glibc-1.06/malloc', 7872 ).
file('mcheck-init.c',text,'glibc-1.06/malloc', 1145 ).
file('mcheck.c',text,'glibc-1.06/malloc', 3620 ).
file('mcheck.dep',text,'glibc-1.06/malloc', 498 ).
file('mem-limits.h',text,'glibc-1.06/malloc', 2836 ).
file('memalign.c',text,'glibc-1.06/malloc', 1721 ).
file('memalign.dep',text,'glibc-1.06/malloc', 504 ).
file('morecore.dep',text,'glibc-1.06/malloc', 524 ).
file('mstats.c',text,'glibc-1.06/malloc', 1346 ).
file('mstats.dep',text,'glibc-1.06/malloc', 498 ).
file('mtrace.awk',text,'glibc-1.06/malloc', 860 ).
file('mtrace.c',text,'glibc-1.06/malloc', 4134 ).
file('mtrace.dep',text,'glibc-1.06/malloc', 587 ).
file('obstack.c',text,'glibc-1.06/malloc', 14075 ).
file('obstack.dep',text,'glibc-1.06/malloc', 297 ).
file('obstack.h',text,'glibc-1.06/malloc', 18711 ).
file('ralloc.c',text,'glibc-1.06/malloc', 12864 ).
file('ralloc.dep',text,'glibc-1.06/malloc', 271 ).
file('realloc.c',text,'glibc-1.06/malloc', 4340 ).
file('realloc.dep',text,'glibc-1.06/malloc', 501 ).
file('valloc.c',text,'glibc-1.06/malloc', 1409 ).
file('valloc.dep',text,'glibc-1.06/malloc', 498 ).
file('vm-limit.c',text,'glibc-1.06/malloc', 3453 ).
file('vm-limit.dep',text,'glibc-1.06/malloc', 157 ).
file('gmalloc-head.c',text,'glibc-1.06/malloc/malloc', 176 ).
file('COPYING.LIB',text,'glibc-1.06/manual', 25265 ).
file('Makefile',text,'glibc-1.06/manual', 4941 ).
file('TAGS',text,'glibc-1.06/manual', 11964 ).
file('add.c.texi',text,'glibc-1.06/manual', 549 ).
file('arith.texi',text,'glibc-1.06/manual', 22350 ).
file('atexit.c.texi',text,'glibc-1.06/manual', 162 ).
file('chapters',text,'glibc-1.06/manual', 427 ).
file('chapters-incl',text,'glibc-1.06/manual', 488 ).
file('child.texi',text,'glibc-1.06/manual', 28456 ).
file('conf.texi',text,'glibc-1.06/manual', 35358 ).
file('ctype.texi',text,'glibc-1.06/manual', 8007 ).
file('db.c.texi',text,'glibc-1.06/manual', 1297 ).
file('dir.c.texi',text,'glibc-1.06/manual', 354 ).
file('errno.texi',text,'glibc-1.06/manual', 27027 ).
file('examples',dir,'glibc-1.06/manual', 1024 ).
file('fileclient.c.texi',text,'glibc-1.06/manual', 1234 ).
file('fileserver.c.texi',text,'glibc-1.06/manual', 973 ).
file('filesys.texi',text,'glibc-1.06/manual', 64677 ).
file('header.texi',text,'glibc-1.06/manual', 562 ).
file('inetclient.c.texi',text,'glibc-1.06/manual', 1190 ).
file('inetserver.c.texi',text,'glibc-1.06/manual', 2154 ).
file('intro.texi',text,'glibc-1.06/manual', 30833 ).
file('io.texi',text,'glibc-1.06/manual', 16638 ).
file('isockaddr.c.texi',text,'glibc-1.06/manual', 527 ).
file('job.texi',text,'glibc-1.06/manual', 44782 ).
file('lang.texi',text,'glibc-1.06/manual', 45593 ).
file('lgpl.texinfo',text,'glibc-1.06/manual', 25677 ).
file('libc.cp',text,'glibc-1.06/manual', 45868 ).
file('libc.cps',text,'glibc-1.06/manual', 28217 ).
file('libc.fn',text,'glibc-1.06/manual', 19385 ).
file('libc.fns',text,'glibc-1.06/manual', 14847 ).
file('libc.pg',text,'glibc-1.06/manual', 9345 ).
file('libc.pgs',text,'glibc-1.06/manual', 2812 ).
file('libc.texinfo',text,'glibc-1.06/manual', 43124 ).
file('libc.tp',text,'glibc-1.06/manual', 3281 ).
file('libc.tps',text,'glibc-1.06/manual', 1898 ).
file('libc.vr',text,'glibc-1.06/manual', 22681 ).
file('libc.vrs',text,'glibc-1.06/manual', 15952 ).
file('llio.texi',text,'glibc-1.06/manual', 63660 ).
file('locale.texi',text,'glibc-1.06/manual', 23418 ).
file('longopt.c.texi',text,'glibc-1.06/manual', 1942 ).
file('maint.texi',text,'glibc-1.06/manual', 35028 ).
file('makefsock.c.texi',text,'glibc-1.06/manual', 869 ).
file('makeisock.c.texi',text,'glibc-1.06/manual', 623 ).
file('math.texi',text,'glibc-1.06/manual', 18534 ).
file('mbyte.texi',text,'glibc-1.06/manual', 27868 ).
file('memopen.c.texi',text,'glibc-1.06/manual', 255 ).
file('memory.texi',text,'glibc-1.06/manual', 63103 ).
file('memstream.c.texi',text,'glibc-1.06/manual', 329 ).
file('move-if-change',exec,'glibc-1.06/manual', 229 ).
file('pattern.texi',text,'glibc-1.06/manual', 43148 ).
file('pipe.c.texi',text,'glibc-1.06/manual', 1235 ).
file('pipe.texi',text,'glibc-1.06/manual', 8076 ).
file('popen.c.texi',text,'glibc-1.06/manual', 527 ).
file('rprintf.c.texi',text,'glibc-1.06/manual', 1042 ).
file('search.c.texi',text,'glibc-1.06/manual', 1720 ).
file('search.texi',text,'glibc-1.06/manual', 6457 ).
file('select.c.texi',text,'glibc-1.06/manual', 661 ).
file('setjmp.c.texi',text,'glibc-1.06/manual', 441 ).
file('setjmp.texi',text,'glibc-1.06/manual', 8880 ).
file('sigh1.c.texi',text,'glibc-1.06/manual', 702 ).
file('signal.texi',text,'glibc-1.06/manual', 120902 ).
file('sigusr.c.texi',text,'glibc-1.06/manual', 1236 ).
file('socket.texi',text,'glibc-1.06/manual', 101109 ).
file('stamp-summary',text,'glibc-1.06/manual', 29 ).
file('startup.texi',text,'glibc-1.06/manual', 33398 ).
file('stdio.texi',text,'glibc-1.06/manual', 138463 ).
file('stpcpy.c.texi',text,'glibc-1.06/manual', 145 ).
file('strftime.c.texi',text,'glibc-1.06/manual', 644 ).
file('string.texi',text,'glibc-1.06/manual', 37335 ).
file('strncat.c.texi',text,'glibc-1.06/manual', 246 ).
file('summary.awk',text,'glibc-1.06/manual', 3012 ).
file('summary.texi',text,'glibc-1.06/manual', 119670 ).
file('sysinfo.texi',text,'glibc-1.06/manual', 6320 ).
file('terminal.texi',text,'glibc-1.06/manual', 60901 ).
file('termios.c.texi',text,'glibc-1.06/manual', 1144 ).
file('testopt.c.texi',text,'glibc-1.06/manual', 942 ).
file('texinfo.tex',text,'glibc-1.06/manual', 131253 ).
file('time.texi',text,'glibc-1.06/manual', 50778 ).
file('users.texi',text,'glibc-1.06/manual', 35620 ).
file('add.c',text,'glibc-1.06/manual/examples', 520 ).
file('atexit.c',text,'glibc-1.06/manual/examples', 158 ).
file('db.c',text,'glibc-1.06/manual/examples', 1269 ).
file('dir.c',text,'glibc-1.06/manual/examples', 358 ).
file('fileclient.c',text,'glibc-1.06/manual/examples', 1198 ).
file('fileserver.c',text,'glibc-1.06/manual/examples', 945 ).
file('inetclient.c',text,'glibc-1.06/manual/examples', 1165 ).
file('inetserver.c',text,'glibc-1.06/manual/examples', 2085 ).
file('isockaddr.c',text,'glibc-1.06/manual/examples', 523 ).
file('longopt.c',text,'glibc-1.06/manual/examples', 1881 ).
file('makefsock.c',text,'glibc-1.06/manual/examples', 853 ).
file('makeisock.c',text,'glibc-1.06/manual/examples', 607 ).
file('memopen.c',text,'glibc-1.06/manual/examples', 253 ).
file('memstream.c',text,'glibc-1.06/manual/examples', 327 ).
file('pipe.c',text,'glibc-1.06/manual/examples', 1194 ).
file('popen.c',text,'glibc-1.06/manual/examples', 527 ).
file('rprintf.c',text,'glibc-1.06/manual/examples', 1009 ).
file('search.c',text,'glibc-1.06/manual/examples', 1661 ).
file('select.c',text,'glibc-1.06/manual/examples', 642 ).
file('setjmp.c',text,'glibc-1.06/manual/examples', 435 ).
file('sigh1.c',text,'glibc-1.06/manual/examples', 671 ).
file('sigusr.c',text,'glibc-1.06/manual/examples', 1196 ).
file('stpcpy.c',text,'glibc-1.06/manual/examples', 143 ).
file('strftime.c',text,'glibc-1.06/manual/examples', 630 ).
file('strncat.c',text,'glibc-1.06/manual/examples', 244 ).
file('termios.c',text,'glibc-1.06/manual/examples', 1112 ).
file('testopt.c',text,'glibc-1.06/manual/examples', 962 ).
file('Makefile',text,'glibc-1.06/math', 3247 ).
file('TAGS',text,'glibc-1.06/math', 10868 ).
file('__copysign.dep',text,'glibc-1.06/math', 323 ).
file('__drem.dep',text,'glibc-1.06/math', 375 ).
file('__expm1.dep',text,'glibc-1.06/math', 297 ).
file('__finite.c',text,'glibc-1.06/math', 1037 ).
file('__finite.dep',text,'glibc-1.06/math', 265 ).
file('__infnan.dep',text,'glibc-1.06/math', 381 ).
file('__isinf.dep',text,'glibc-1.06/math', 314 ).
file('__isnan.dep',text,'glibc-1.06/math', 314 ).
file('__logb.dep',text,'glibc-1.06/math', 375 ).
file('__rint.dep',text,'glibc-1.06/math', 294 ).
file('__scalb.c',text,'glibc-1.06/math', 1003 ).
file('__scalb.dep',text,'glibc-1.06/math', 277 ).
file('acosh.dep',text,'glibc-1.06/math', 291 ).
file('asincos.dep',text,'glibc-1.06/math', 54 ).
file('asinh.dep',text,'glibc-1.06/math', 291 ).
file('atan.dep',text,'glibc-1.06/math', 272 ).
file('atan2.dep',text,'glibc-1.06/math', 291 ).
file('atanh.dep',text,'glibc-1.06/math', 291 ).
file('cabs.dep',text,'glibc-1.06/math', 288 ).
file('cbrt.dep',text,'glibc-1.06/math', 94 ).
file('ceil.dep',text,'glibc-1.06/math', 288 ).
file('copysign.c',text,'glibc-1.06/math', 1014 ).
file('copysign.dep',text,'glibc-1.06/math', 280 ).
file('cosh.dep',text,'glibc-1.06/math', 288 ).
file('depend-math',text,'glibc-1.06/math', 1337 ).
file('drem.c',text,'glibc-1.06/math', 1004 ).
file('drem.dep',text,'glibc-1.06/math', 268 ).
file('exp.dep',text,'glibc-1.06/math', 285 ).
file('exp__E.dep',text,'glibc-1.06/math', 294 ).
file('expm1.c',text,'glibc-1.06/math', 983 ).
file('expm1.dep',text,'glibc-1.06/math', 271 ).
file('fabs.dep',text,'glibc-1.06/math', 272 ).
file('finite.c',text,'glibc-1.06/math', 996 ).
file('finite.dep',text,'glibc-1.06/math', 274 ).
file('floor.dep',text,'glibc-1.06/math', 291 ).
file('fmod.dep',text,'glibc-1.06/math', 45 ).
file('frexp.dep',text,'glibc-1.06/math', 290 ).
file('infnan.c',text,'glibc-1.06/math', 996 ).
file('infnan.dep',text,'glibc-1.06/math', 274 ).
file('isinf.c',text,'glibc-1.06/math', 992 ).
file('isinf.dep',text,'glibc-1.06/math', 271 ).
file('isnan.c',text,'glibc-1.06/math', 992 ).
file('isnan.dep',text,'glibc-1.06/math', 271 ).
file('ldexp.dep',text,'glibc-1.06/math', 372 ).
file('log.dep',text,'glibc-1.06/math', 285 ).
file('log10.dep',text,'glibc-1.06/math', 275 ).
file('log1p.dep',text,'glibc-1.06/math', 291 ).
file('log__L.dep',text,'glibc-1.06/math', 294 ).
file('logb.c',text,'glibc-1.06/math', 979 ).
file('logb.dep',text,'glibc-1.06/math', 268 ).
file('math.h',text,'glibc-1.06/math', 9985 ).
file('modf.dep',text,'glibc-1.06/math', 272 ).
file('pow.dep',text,'glibc-1.06/math', 461 ).
file('rint.c',text,'glibc-1.06/math', 979 ).
file('rint.dep',text,'glibc-1.06/math', 268 ).
file('scalb.c',text,'glibc-1.06/math', 999 ).
file('scalb.dep',text,'glibc-1.06/math', 271 ).
file('sincos.dep',text,'glibc-1.06/math', 324 ).
file('sinh.dep',text,'glibc-1.06/math', 288 ).
file('sqrt.dep',text,'glibc-1.06/math', 270 ).
file('tan.dep',text,'glibc-1.06/math', 315 ).
file('tanh.dep',text,'glibc-1.06/math', 45 ).
file('test-math.c',text,'glibc-1.06/math', 1384 ).
file('test-math.dep',text,'glibc-1.06/math', 533 ).
file('Makefile',text,'glibc-1.06/misc', 1923 ).
file('TAGS',text,'glibc-1.06/misc', 43617 ).
file('__brk.dep',text,'glibc-1.06/misc', 124 ).
file('__getdtsz.dep',text,'glibc-1.06/misc', 134 ).
file('__gethstnm.dep',text,'glibc-1.06/misc', 138 ).
file('__getpgsz.dep',text,'glibc-1.06/misc', 134 ).
file('__ioctl.dep',text,'glibc-1.06/misc', 124 ).
file('__mknod.dep',text,'glibc-1.06/misc', 124 ).
file('__sbrk.dep',text,'glibc-1.06/misc', 62 ).
file('__select.dep',text,'glibc-1.06/misc', 134 ).
file('__setregid.dep',text,'glibc-1.06/misc', 137 ).
file('__setreuid.dep',text,'glibc-1.06/misc', 137 ).
file('__utimes.dep',text,'glibc-1.06/misc', 131 ).
file('a.out.h',text,'glibc-1.06/misc', 9981 ).
file('acct.dep',text,'glibc-1.06/misc', 115 ).
file('brk.c',text,'glibc-1.06/misc', 999 ).
file('brk.dep',text,'glibc-1.06/misc', 52 ).
file('bsd-compat.c',text,'glibc-1.06/misc', 1291 ).
file('chflags.dep',text,'glibc-1.06/misc', 233 ).
file('chroot.dep',text,'glibc-1.06/misc', 121 ).
file('data_start.c',text,'glibc-1.06/misc', 110 ).
file('data_start.dep',text,'glibc-1.06/misc', 44 ).
file('depend-misc',text,'glibc-1.06/misc', 1646 ).
file('fchflags.dep',text,'glibc-1.06/misc', 236 ).
file('fsync.dep',text,'glibc-1.06/misc', 118 ).
file('ftruncate.dep',text,'glibc-1.06/misc', 137 ).
file('getdtsz.c',text,'glibc-1.06/misc', 1009 ).
file('getdtsz.dep',text,'glibc-1.06/misc', 240 ).
file('gethostid.dep',text,'glibc-1.06/misc', 135 ).
file('gethstnm.c',text,'glibc-1.06/misc', 1073 ).
file('gethstnm.dep',text,'glibc-1.06/misc', 243 ).
file('getpass.c',text,'glibc-1.06/misc', 2365 ).
file('getpass.dep',text,'glibc-1.06/misc', 475 ).
file('getpgsz.c',text,'glibc-1.06/misc', 1024 ).
file('getpgsz.dep',text,'glibc-1.06/misc', 240 ).
file('getttyent.c',text,'glibc-1.06/misc', 4790 ).
file('getttyent.dep',text,'glibc-1.06/misc', 329 ).
file('getusersh.c',text,'glibc-1.06/misc', 3798 ).
file('getusersh.dep',text,'glibc-1.06/misc', 919 ).
file('gtty.dep',text,'glibc-1.06/misc', 201 ).
file('insremque.c',text,'glibc-1.06/misc', 1545 ).
file('insremque.dep',text,'glibc-1.06/misc', 67 ).
file('ioctl.c',text,'glibc-1.06/misc', 1236 ).
file('ioctl.dep',text,'glibc-1.06/misc', 196 ).
file('mknod.c',text,'glibc-1.06/misc', 1063 ).
file('mknod.dep',text,'glibc-1.06/misc', 191 ).
file('mkstemp.dep',text,'glibc-1.06/misc', 506 ).
file('mktemp.dep',text,'glibc-1.06/misc', 496 ).
file('nlist.dep',text,'glibc-1.06/misc', 407 ).
file('nlist.h',text,'glibc-1.06/misc', 1411 ).
file('paths.h',text,'glibc-1.06/misc', 2688 ).
file('progname.c',text,'glibc-1.06/misc', 1410 ).
file('progname.dep',text,'glibc-1.06/misc', 38 ).
file('ptrace.dep',text,'glibc-1.06/misc', 121 ).
file('readv.dep',text,'glibc-1.06/misc', 125 ).
file('reboot.dep',text,'glibc-1.06/misc', 261 ).
file('sbrk.c',text,'glibc-1.06/misc', 1019 ).
file('sbrk.dep',text,'glibc-1.06/misc', 55 ).
file('select.c',text,'glibc-1.06/misc', 1215 ).
file('select.dep',text,'glibc-1.06/misc', 239 ).
file('setegid.dep',text,'glibc-1.06/misc', 310 ).
file('seteuid.dep',text,'glibc-1.06/misc', 310 ).
file('sethostid.dep',text,'glibc-1.06/misc', 156 ).
file('sethostnam.dep',text,'glibc-1.06/misc', 138 ).
file('setregid.c',text,'glibc-1.06/misc', 1036 ).
file('setregid.dep',text,'glibc-1.06/misc', 243 ).
file('setreuid.c',text,'glibc-1.06/misc', 1036 ).
file('setreuid.dep',text,'glibc-1.06/misc', 243 ).
file('sgtty.h',text,'glibc-1.06/misc', 1235 ).
file('sstk.dep',text,'glibc-1.06/misc', 111 ).
file('stab.def',text,'glibc-1.06/misc', 8981 ).
file('stab.h',text,'glibc-1.06/misc', 259 ).
file('stty.dep',text,'glibc-1.06/misc', 201 ).
file('swapon.dep',text,'glibc-1.06/misc', 128 ).
file('sync.dep',text,'glibc-1.06/misc', 115 ).
file('sys',dir,'glibc-1.06/misc', 512 ).
file('syslog.c',text,'glibc-1.06/misc', 6050 ).
file('syslog.dep',text,'glibc-1.06/misc', 951 ).
file('truncate.dep',text,'glibc-1.06/misc', 134 ).
file('ttyent.h',text,'glibc-1.06/misc', 2876 ).
file('ttyslot.c',text,'glibc-1.06/misc', 2461 ).
file('ttyslot.dep',text,'glibc-1.06/misc', 387 ).
file('ualarm.dep',text,'glibc-1.06/misc', 208 ).
file('usleep.dep',text,'glibc-1.06/misc', 340 ).
file('utimes.c',text,'glibc-1.06/misc', 1051 ).
file('utimes.dep',text,'glibc-1.06/misc', 203 ).
file('vhangup.dep',text,'glibc-1.06/misc', 131 ).
file('writev.dep',text,'glibc-1.06/misc', 128 ).
file('cdefs.h',text,'glibc-1.06/misc/sys', 2474 ).
file('dir.h',text,'glibc-1.06/misc/sys', 957 ).
file('file.h',text,'glibc-1.06/misc/sys', 1760 ).
file('ioctl.h',text,'glibc-1.06/misc/sys', 3850 ).
file('ptrace.h',text,'glibc-1.06/misc/sys', 4629 ).
file('syslog.h',text,'glibc-1.06/misc/sys', 6401 ).
file('uio.h',text,'glibc-1.06/misc/sys', 1929 ).
file('in.h',text,'glibc-1.06/netinet', 24 ).
file('Makefile',text,'glibc-1.06/posix', 2657 ).
file('TAGS',text,'glibc-1.06/posix', 19818 ).
file('__execve.dep',text,'glibc-1.06/posix', 127 ).
file('__fork.dep',text,'glibc-1.06/posix', 127 ).
file('__fpathcon.dep',text,'glibc-1.06/posix', 380 ).
file('__getegid.dep',text,'glibc-1.06/posix', 130 ).
file('__geteuid.dep',text,'glibc-1.06/posix', 130 ).
file('__getgid.dep',text,'glibc-1.06/posix', 127 ).
file('__getgrps.dep',text,'glibc-1.06/posix', 561 ).
file('__getpgrp.dep',text,'glibc-1.06/posix', 137 ).
file('__getpid.dep',text,'glibc-1.06/posix', 127 ).
file('__getppid.dep',text,'glibc-1.06/posix', 130 ).
file('__getuid.dep',text,'glibc-1.06/posix', 127 ).
file('__pathconf.dep',text,'glibc-1.06/posix', 253 ).
file('__setgid.dep',text,'glibc-1.06/posix', 363 ).
file('__setpgrp.dep',text,'glibc-1.06/posix', 137 ).
file('__setsid.dep',text,'glibc-1.06/posix', 463 ).
file('__setuid.dep',text,'glibc-1.06/posix', 363 ).
file('__sysconf.dep',text,'glibc-1.06/posix', 493 ).
file('__times.dep',text,'glibc-1.06/posix', 296 ).
file('__vfork.dep',text,'glibc-1.06/posix', 130 ).
file('__wait.dep',text,'glibc-1.06/posix', 455 ).
file('__wait3.dep',text,'glibc-1.06/posix', 391 ).
file('__wait4.dep',text,'glibc-1.06/posix', 404 ).
file('__waitpid.dep',text,'glibc-1.06/posix', 399 ).
file('_exit.dep',text,'glibc-1.06/posix', 118 ).
file('alarm.dep',text,'glibc-1.06/posix', 281 ).
file('bsd_getgrp.dep',text,'glibc-1.06/posix', 140 ).
file('confstr.c',text,'glibc-1.06/posix', 1558 ).
file('confstr.dep',text,'glibc-1.06/posix', 289 ).
file('depend-posix',text,'glibc-1.06/posix', 2058 ).
file('environ.c',text,'glibc-1.06/posix', 950 ).
file('environ.dep',text,'glibc-1.06/posix', 49 ).
file('execl.c',text,'glibc-1.06/posix', 1430 ).
file('execl.dep',text,'glibc-1.06/posix', 225 ).
file('execle.c',text,'glibc-1.06/posix', 1453 ).
file('execle.dep',text,'glibc-1.06/posix', 228 ).
file('execlp.c',text,'glibc-1.06/posix', 1396 ).
file('execlp.dep',text,'glibc-1.06/posix', 228 ).
file('execv.c',text,'glibc-1.06/posix', 1118 ).
file('execv.dep',text,'glibc-1.06/posix', 195 ).
file('execve.c',text,'glibc-1.06/posix', 1056 ).
file('execve.dep',text,'glibc-1.06/posix', 217 ).
file('execvp.c',text,'glibc-1.06/posix', 3051 ).
file('execvp.dep',text,'glibc-1.06/posix', 582 ).
file('fnmatch.c',text,'glibc-1.06/posix', 4878 ).
file('fnmatch.dep',text,'glibc-1.06/posix', 151 ).
file('fnmatch.h',text,'glibc-1.06/posix', 2158 ).
file('fork.c',text,'glibc-1.06/posix', 977 ).
file('fork.dep',text,'glibc-1.06/posix', 211 ).
file('fpathcon.c',text,'glibc-1.06/posix', 1029 ).
file('fpathcon.dep',text,'glibc-1.06/posix', 223 ).
file('getconf',exec,'glibc-1.06/posix', 401408 ).
file('getconf.c',text,'glibc-1.06/posix', 3533 ).
file('getconf.dep',text,'glibc-1.06/posix', 454 ).
file('getegid.c',text,'glibc-1.06/posix', 982 ).
file('getegid.dep',text,'glibc-1.06/posix', 220 ).
file('geteuid.c',text,'glibc-1.06/posix', 982 ).
file('geteuid.dep',text,'glibc-1.06/posix', 220 ).
file('getgid.c',text,'glibc-1.06/posix', 978 ).
file('getgid.dep',text,'glibc-1.06/posix', 217 ).
file('getgrps.c',text,'glibc-1.06/posix', 1042 ).
file('getgrps.dep',text,'glibc-1.06/posix', 220 ).
file('getlogin.dep',text,'glibc-1.06/posix', 608 ).
file('getopt.c',text,'glibc-1.06/posix', 20971 ).
file('getopt.dep',text,'glibc-1.06/posix', 352 ).
file('getopt.h',text,'glibc-1.06/posix', 4507 ).
file('getopt1.c',text,'glibc-1.06/posix', 4237 ).
file('getopt1.dep',text,'glibc-1.06/posix', 324 ).
file('getpgrp.dep',text,'glibc-1.06/posix', 298 ).
file('getpid.c',text,'glibc-1.06/posix', 985 ).
file('getpid.dep',text,'glibc-1.06/posix', 217 ).
file('getppid.c',text,'glibc-1.06/posix', 982 ).
file('getppid.dep',text,'glibc-1.06/posix', 220 ).
file('getuid.c',text,'glibc-1.06/posix', 978 ).
file('getuid.dep',text,'glibc-1.06/posix', 217 ).
file('glob.c',text,'glibc-1.06/posix', 16168 ).
file('glob.dep',text,'glibc-1.06/posix', 350 ).
file('glob.h',text,'glibc-1.06/posix', 3645 ).
file('gnu',dir,'glibc-1.06/posix', 512 ).
file('init-posix.dep',text,'glibc-1.06/posix', 182 ).
file('pathconf.c',text,'glibc-1.06/posix', 1039 ).
file('pathconf.dep',text,'glibc-1.06/posix', 223 ).
file('pause.dep',text,'glibc-1.06/posix', 385 ).
file('posix1_lim.h',text,'glibc-1.06/posix', 2441 ).
file('posix2_lim.h',text,'glibc-1.06/posix', 2597 ).
file('setgid.c',text,'glibc-1.06/posix', 1000 ).
file('setgid.dep',text,'glibc-1.06/posix', 217 ).
file('setlogin.dep',text,'glibc-1.06/posix', 247 ).
file('setpgid.c',text,'glibc-1.06/posix', 1051 ).
file('setpgid.dep',text,'glibc-1.06/posix', 259 ).
file('setpgrp.c',text,'glibc-1.06/posix', 1023 ).
file('setpgrp.dep',text,'glibc-1.06/posix', 220 ).
file('setsid.c',text,'glibc-1.06/posix', 985 ).
file('setsid.dep',text,'glibc-1.06/posix', 217 ).
file('setuid.c',text,'glibc-1.06/posix', 1000 ).
file('setuid.dep',text,'glibc-1.06/posix', 217 ).
file('sleep.dep',text,'glibc-1.06/posix', 407 ).
file('sys',dir,'glibc-1.06/posix', 512 ).
file('sys_wait4.dep',text,'glibc-1.06/posix', 145 ).
file('sysconf.c',text,'glibc-1.06/posix', 1002 ).
file('sysconf.dep',text,'glibc-1.06/posix', 220 ).
file('tar.h',text,'glibc-1.06/posix', 3747 ).
file('testfnm.args',text,'glibc-1.06/posix', 10 ).
file('testfnm.c',text,'glibc-1.06/posix', 219 ).
file('testfnm.dep',text,'glibc-1.06/posix', 249 ).
file('times.c',text,'glibc-1.06/posix', 1000 ).
file('times.dep',text,'glibc-1.06/posix', 150 ).
file('tstgetopt.args',text,'glibc-1.06/posix', 15 ).
file('tstgetopt.c',text,'glibc-1.06/posix', 811 ).
file('tstgetopt.dep',text,'glibc-1.06/posix', 336 ).
file('uname.dep',text,'glibc-1.06/posix', 162 ).
file('unistd.h',text,'glibc-1.06/posix', 21176 ).
file('vfork.dep',text,'glibc-1.06/posix', 132 ).
file('wait.c',text,'glibc-1.06/posix', 991 ).
file('wait.dep',text,'glibc-1.06/posix', 247 ).
file('wait3.c',text,'glibc-1.06/posix', 1078 ).
file('wait3.dep',text,'glibc-1.06/posix', 305 ).
file('wait4.c',text,'glibc-1.06/posix', 1103 ).
file('wait4.dep',text,'glibc-1.06/posix', 305 ).
file('waitpid.c',text,'glibc-1.06/posix', 1061 ).
file('waitpid.dep',text,'glibc-1.06/posix', 311 ).
file('wordexp.h',text,'glibc-1.06/posix', 2382 ).
file('types.h',text,'glibc-1.06/posix/gnu', 2952 ).
file('times.h',text,'glibc-1.06/posix/sys', 1705 ).
file('types.h',text,'glibc-1.06/posix/sys', 2698 ).
file('unistd.h',text,'glibc-1.06/posix/sys', 20 ).
file('utsname.h',text,'glibc-1.06/posix/sys', 1695 ).
file('wait.h',text,'glibc-1.06/posix/sys', 4920 ).
file('routed.h',text,'glibc-1.06/protocols', 30 ).
file('rwhod.h',text,'glibc-1.06/protocols', 29 ).
file('talkd.h',text,'glibc-1.06/protocols', 29 ).
file('timed.h',text,'glibc-1.06/protocols', 29 ).
file('Makefile',text,'glibc-1.06/pwd', 1010 ).
file('TAGS',text,'glibc-1.06/pwd', 499 ).
file('depend-pwd',text,'glibc-1.06/pwd', 236 ).
file('fgetpwent.c',text,'glibc-1.06/pwd', 1229 ).
file('fgetpwent.dep',text,'glibc-1.06/pwd', 545 ).
file('getpw.c',text,'glibc-1.06/pwd', 1499 ).
file('getpw.dep',text,'glibc-1.06/pwd', 326 ).
file('getpwent.c',text,'glibc-1.06/pwd', 1527 ).
file('getpwent.dep',text,'glibc-1.06/pwd', 312 ).
file('getpwnam.c',text,'glibc-1.06/pwd', 1424 ).
file('getpwnam.dep',text,'glibc-1.06/pwd', 343 ).
file('getpwuid.c',text,'glibc-1.06/pwd', 1402 ).
file('getpwuid.dep',text,'glibc-1.06/pwd', 312 ).
file('putpwent.c',text,'glibc-1.06/pwd', 1371 ).
file('putpwent.dep',text,'glibc-1.06/pwd', 335 ).
file('pwd.h',text,'glibc-1.06/pwd', 2654 ).
file('pwdopen.c',text,'glibc-1.06/pwd', 1008 ).
file('pwdopen.dep',text,'glibc-1.06/pwd', 309 ).
file('pwdread.c',text,'glibc-1.06/pwd', 2826 ).
file('pwdread.dep',text,'glibc-1.06/pwd', 605 ).
file('Makefile',text,'glibc-1.06/resource', 1047 ).
file('TAGS',text,'glibc-1.06/resource', 2201 ).
file('__getrusag.dep',text,'glibc-1.06/resource', 140 ).
file('depend-resource',text,'glibc-1.06/resource', 293 ).
file('getprio.dep',text,'glibc-1.06/resource', 128 ).
file('getrlimit.dep',text,'glibc-1.06/resource', 137 ).
file('getrusage.c',text,'glibc-1.06/resource', 1073 ).
file('getrusage.dep',text,'glibc-1.06/resource', 250 ).
file('nice.dep',text,'glibc-1.06/resource', 359 ).
file('setprio.dep',text,'glibc-1.06/resource', 128 ).
file('setrlimit.dep',text,'glibc-1.06/resource', 137 ).
file('sys',dir,'glibc-1.06/resource', 512 ).
file('ulimit.dep',text,'glibc-1.06/resource', 438 ).
file('vlimit.dep',text,'glibc-1.06/resource', 287 ).
file('vtimes.dep',text,'glibc-1.06/resource', 260 ).
file('resource.h',text,'glibc-1.06/resource/sys', 6257 ).
file('vlimit.h',text,'glibc-1.06/resource/sys', 1914 ).
file('vtimes.h',text,'glibc-1.06/resource/sys', 2501 ).
file('Makefile',text,'glibc-1.06/setjmp', 1069 ).
file('TAGS',text,'glibc-1.06/setjmp', 2036 ).
file('__longjmp.dep',text,'glibc-1.06/setjmp', 202 ).
file('_longjmp.c',text,'glibc-1.06/setjmp', 1055 ).
file('_longjmp.dep',text,'glibc-1.06/setjmp', 302 ).
file('_setjmp.c',text,'glibc-1.06/setjmp', 1006 ).
file('_setjmp.dep',text,'glibc-1.06/setjmp', 299 ).
file('depend-setjmp',text,'glibc-1.06/setjmp', 244 ).
file('longjmp.c',text,'glibc-1.06/setjmp', 1030 ).
file('longjmp.dep',text,'glibc-1.06/setjmp', 299 ).
file('setjmp.dep',text,'glibc-1.06/setjmp', 122 ).
file('setjmp.h',text,'glibc-1.06/setjmp', 4042 ).
file('sigjmp_save.c',text,'glibc-1.06/setjmp', 1222 ).
file('sigjmp_save.dep',text,'glibc-1.06/setjmp', 370 ).
file('siglongjmp.c',text,'glibc-1.06/setjmp', 1315 ).
file('siglongjmp.dep',text,'glibc-1.06/setjmp', 367 ).
file('tst-setjmp.c',text,'glibc-1.06/setjmp', 1531 ).
file('tst-setjmp.dep',text,'glibc-1.06/setjmp', 529 ).
file('Makefile',text,'glibc-1.06/signal', 1405 ).
file('TAGS',text,'glibc-1.06/signal', 11229 ).
file('__kill.dep',text,'glibc-1.06/signal', 121 ).
file('__sigact.dep',text,'glibc-1.06/signal', 419 ).
file('__sigblock.dep',text,'glibc-1.06/signal', 137 ).
file('__sigpause.dep',text,'glibc-1.06/signal', 137 ).
file('__sigproc.dep',text,'glibc-1.06/signal', 349 ).
file('__sigret.dep',text,'glibc-1.06/signal', 135 ).
file('__sigstmsk.dep',text,'glibc-1.06/signal', 137 ).
file('__sigvec.dep',text,'glibc-1.06/signal', 135 ).
file('depend-signal',text,'glibc-1.06/signal', 992 ).
file('gsignal.c',text,'glibc-1.06/signal', 990 ).
file('gsignal.dep',text,'glibc-1.06/signal', 315 ).
file('kill.c',text,'glibc-1.06/signal', 1014 ).
file('kill.dep',text,'glibc-1.06/signal', 306 ).
file('killpg.dep',text,'glibc-1.06/signal', 125 ).
file('raise.dep',text,'glibc-1.06/signal', 403 ).
file('sigaction.c',text,'glibc-1.06/signal', 1109 ).
file('sigaction.dep',text,'glibc-1.06/signal', 321 ).
file('sigaddset.c',text,'glibc-1.06/signal', 1079 ).
file('sigaddset.dep',text,'glibc-1.06/signal', 391 ).
file('sigaltstack.dep',text,'glibc-1.06/signal', 351 ).
file('sigblock.c',text,'glibc-1.06/signal', 1001 ).
file('sigblock.dep',text,'glibc-1.06/signal', 318 ).
file('sigdelset.c',text,'glibc-1.06/signal', 1079 ).
file('sigdelset.dep',text,'glibc-1.06/signal', 391 ).
file('sigempty.c',text,'glibc-1.06/signal', 1034 ).
file('sigempty.dep',text,'glibc-1.06/signal', 384 ).
file('sigfillset.c',text,'glibc-1.06/signal', 1028 ).
file('sigfillset.dep',text,'glibc-1.06/signal', 394 ).
file('sigintr.dep',text,'glibc-1.06/signal', 317 ).
file('sigismem.c',text,'glibc-1.06/signal', 1110 ).
file('sigismem.dep',text,'glibc-1.06/signal', 384 ).
file('signal.dep',text,'glibc-1.06/signal', 337 ).
file('signal.h',text,'glibc-1.06/signal', 7835 ).
file('sigpause.c',text,'glibc-1.06/signal', 1001 ).
file('sigpause.dep',text,'glibc-1.06/signal', 318 ).
file('sigpending.dep',text,'glibc-1.06/signal', 348 ).
file('sigproc.c',text,'glibc-1.06/signal', 1090 ).
file('sigproc.dep',text,'glibc-1.06/signal', 315 ).
file('sigret.c',text,'glibc-1.06/signal', 1035 ).
file('sigret.dep',text,'glibc-1.06/signal', 312 ).
file('sigsetmask.c',text,'glibc-1.06/signal', 1009 ).
file('sigsetmask.dep',text,'glibc-1.06/signal', 324 ).
file('sigsetops.h',text,'glibc-1.06/signal', 1211 ).
file('sigstack.dep',text,'glibc-1.06/signal', 131 ).
file('sigsuspend.dep',text,'glibc-1.06/signal', 448 ).
file('sigtramp.dep',text,'glibc-1.06/signal', 414 ).
file('sigvec.c',text,'glibc-1.06/signal', 1091 ).
file('sigvec.dep',text,'glibc-1.06/signal', 312 ).
file('ssignal.c',text,'glibc-1.06/signal', 1047 ).
file('ssignal.dep',text,'glibc-1.06/signal', 315 ).
file('sys',dir,'glibc-1.06/signal', 512 ).
file('tst-signal.c',text,'glibc-1.06/signal', 727 ).
file('tst-signal.dep',text,'glibc-1.06/signal', 576 ).
file('signal.h',text,'glibc-1.06/signal/sys', 20 ).
file('Makefile',text,'glibc-1.06/socket', 1137 ).
file('TAGS',text,'glibc-1.06/socket', 4539 ).
file('accept.dep',text,'glibc-1.06/socket', 126 ).
file('bind.dep',text,'glibc-1.06/socket', 120 ).
file('connect.dep',text,'glibc-1.06/socket', 129 ).
file('depend-socket',text,'glibc-1.06/socket', 497 ).
file('getpeernam.dep',text,'glibc-1.06/socket', 138 ).
file('getsocknam.dep',text,'glibc-1.06/socket', 138 ).
file('getsockopt.dep',text,'glibc-1.06/socket', 138 ).
file('listen.dep',text,'glibc-1.06/socket', 126 ).
file('recv.dep',text,'glibc-1.06/socket', 120 ).
file('recvfrom.dep',text,'glibc-1.06/socket', 132 ).
file('recvmsg.dep',text,'glibc-1.06/socket', 129 ).
file('send.dep',text,'glibc-1.06/socket', 120 ).
file('sendmsg.dep',text,'glibc-1.06/socket', 129 ).
file('sendto.dep',text,'glibc-1.06/socket', 126 ).
file('setsockopt.dep',text,'glibc-1.06/socket', 138 ).
file('shutdown.dep',text,'glibc-1.06/socket', 132 ).
file('socket.dep',text,'glibc-1.06/socket', 126 ).
file('socketpair.dep',text,'glibc-1.06/socket', 138 ).
file('sys',dir,'glibc-1.06/socket', 512 ).
file('socket.h',text,'glibc-1.06/socket/sys', 10006 ).
file('un.h',text,'glibc-1.06/socket/sys', 1087 ).
file('Makefile',text,'glibc-1.06/stdio', 2116 ).
file('TAGS',text,'glibc-1.06/stdio', 10794 ).
file('__getdelim.c',text,'glibc-1.06/stdio', 3992 ).
file('__getdelim.dep',text,'glibc-1.06/stdio', 584 ).
file('__getline.c',text,'glibc-1.06/stdio', 1117 ).
file('__getline.dep',text,'glibc-1.06/stdio', 289 ).
file('__vfscanf.c',text,'glibc-1.06/stdio', 11930 ).
file('__vfscanf.dep',text,'glibc-1.06/stdio', 652 ).
file('__vsscanf.c',text,'glibc-1.06/stdio', 1665 ).
file('__vsscanf.dep',text,'glibc-1.06/stdio', 343 ).
file('asprintf.c',text,'glibc-1.06/stdio', 1246 ).
file('asprintf.dep',text,'glibc-1.06/stdio', 282 ).
file('bug1.c',text,'glibc-1.06/stdio', 572 ).
file('bug1.dep',text,'glibc-1.06/stdio', 301 ).
file('bug1.input',text,'glibc-1.06/stdio', 3 ).
file('bug2.c',text,'glibc-1.06/stdio', 223 ).
file('bug2.dep',text,'glibc-1.06/stdio', 270 ).
file('bug3.c',text,'glibc-1.06/stdio', 765 ).
file('bug3.dep',text,'glibc-1.06/stdio', 301 ).
file('bug4.c',text,'glibc-1.06/stdio', 877 ).
file('bug4.dep',text,'glibc-1.06/stdio', 397 ).
file('bug5.c',text,'glibc-1.06/stdio', 1235 ).
file('bug5.dep',text,'glibc-1.06/stdio', 455 ).
file('bug6.c',text,'glibc-1.06/stdio', 505 ).
file('bug6.dep',text,'glibc-1.06/stdio', 270 ).
file('bug6.input',text,'glibc-1.06/stdio', 9 ).
file('clearerr.c',text,'glibc-1.06/stdio', 1029 ).
file('clearerr.dep',text,'glibc-1.06/stdio', 309 ).
file('ctermid.dep',text,'glibc-1.06/stdio', 331 ).
file('cuserid.dep',text,'glibc-1.06/stdio', 353 ).
file('defs.dep',text,'glibc-1.06/stdio', 387 ).
file('depend-stdio',text,'glibc-1.06/stdio', 2804 ).
file('dprintf.c',text,'glibc-1.06/stdio', 1168 ).
file('dprintf.dep',text,'glibc-1.06/stdio', 279 ).
file('errlist.c',text,'glibc-1.06/stdio', 2876 ).
file('errlist.dep',text,'glibc-1.06/stdio', 61 ).
file('fclose.c',text,'glibc-1.06/stdio', 1926 ).
file('fclose.dep',text,'glibc-1.06/stdio', 400 ).
file('fdopen.dep',text,'glibc-1.06/stdio', 392 ).
file('feof.c',text,'glibc-1.06/stdio', 1108 ).
file('feof.dep',text,'glibc-1.06/stdio', 293 ).
file('ferror.c',text,'glibc-1.06/stdio', 1116 ).
file('ferror.dep',text,'glibc-1.06/stdio', 299 ).
file('fflush.c',text,'glibc-1.06/stdio', 1454 ).
file('fflush.dep',text,'glibc-1.06/stdio', 365 ).
file('fgetc.c',text,'glibc-1.06/stdio', 1099 ).
file('fgetc.dep',text,'glibc-1.06/stdio', 296 ).
file('fgetpos.c',text,'glibc-1.06/stdio', 1186 ).
file('fgetpos.dep',text,'glibc-1.06/stdio', 306 ).
file('fgets.c',text,'glibc-1.06/stdio', 2956 ).
file('fgets.dep',text,'glibc-1.06/stdio', 331 ).
file('fileno.c',text,'glibc-1.06/stdio', 1221 ).
file('fileno.dep',text,'glibc-1.06/stdio', 299 ).
file('fmemopen.c',text,'glibc-1.06/stdio', 3245 ).
file('fmemopen.dep',text,'glibc-1.06/stdio', 406 ).
file('fopen.c',text,'glibc-1.06/stdio', 2568 ).
file('fopen.dep',text,'glibc-1.06/stdio', 425 ).
file('fopncook.c',text,'glibc-1.06/stdio', 1433 ).
file('fopncook.dep',text,'glibc-1.06/stdio', 282 ).
file('fprintf.c',text,'glibc-1.06/stdio', 1187 ).
file('fprintf.dep',text,'glibc-1.06/stdio', 279 ).
file('fputc.c',text,'glibc-1.06/stdio', 1119 ).
file('fputc.dep',text,'glibc-1.06/stdio', 296 ).
file('fputs.c',text,'glibc-1.06/stdio', 1196 ).
file('fputs.dep',text,'glibc-1.06/stdio', 331 ).
file('fread.c',text,'glibc-1.06/stdio', 3436 ).
file('fread.dep',text,'glibc-1.06/stdio', 331 ).
file('freopen.c',text,'glibc-1.06/stdio', 1488 ).
file('freopen.dep',text,'glibc-1.06/stdio', 306 ).
file('fscanf.c',text,'glibc-1.06/stdio', 1195 ).
file('fscanf.dep',text,'glibc-1.06/stdio', 276 ).
file('fseek.c',text,'glibc-1.06/stdio', 5191 ).
file('fseek.dep',text,'glibc-1.06/stdio', 296 ).
file('fsetpos.c',text,'glibc-1.06/stdio', 1137 ).
file('fsetpos.dep',text,'glibc-1.06/stdio', 306 ).
file('ftell.c',text,'glibc-1.06/stdio', 1524 ).
file('ftell.dep',text,'glibc-1.06/stdio', 296 ).
file('fwrite.c',text,'glibc-1.06/stdio', 5067 ).
file('fwrite.dep',text,'glibc-1.06/stdio', 334 ).
file('getc.c',text,'glibc-1.06/stdio', 91 ).
file('getc.dep',text,'glibc-1.06/stdio', 307 ).
file('getchar.c',text,'glibc-1.06/stdio', 972 ).
file('getchar.dep',text,'glibc-1.06/stdio', 279 ).
file('getdelim.c',text,'glibc-1.06/stdio', 1151 ).
file('getdelim.dep',text,'glibc-1.06/stdio', 301 ).
file('getline.c',text,'glibc-1.06/stdio', 1094 ).
file('getline.dep',text,'glibc-1.06/stdio', 298 ).
file('gets.c',text,'glibc-1.06/stdio', 1573 ).
file('gets.dep',text,'glibc-1.06/stdio', 328 ).
file('getw.c',text,'glibc-1.06/stdio', 1067 ).
file('getw.dep',text,'glibc-1.06/stdio', 270 ).
file('glue.c',text,'glibc-1.06/stdio', 3474 ).
file('glue.dep',text,'glibc-1.06/stdio', 293 ).
file('internals.c',text,'glibc-1.06/stdio', 17995 ).
file('internals.dep',text,'glibc-1.06/stdio', 409 ).
file('libc_fatal.dep',text,'glibc-1.06/stdio', 405 ).
file('make_errlist',exec,'glibc-1.06/stdio', 24576 ).
file('make_siglist',exec,'glibc-1.06/stdio', 24576 ).
file('memstream.c',text,'glibc-1.06/stdio', 4809 ).
file('memstream.dep',text,'glibc-1.06/stdio', 409 ).
file('newstream.c',text,'glibc-1.06/stdio', 1677 ).
file('newstream.dep',text,'glibc-1.06/stdio', 378 ).
file('obstream.c',text,'glibc-1.06/stdio', 5536 ).
file('obstream.dep',text,'glibc-1.06/stdio', 350 ).
file('perror.c',text,'glibc-1.06/stdio', 1556 ).
file('perror.dep',text,'glibc-1.06/stdio', 299 ).
file('pipestream.dep',text,'glibc-1.06/stdio', 911 ).
file('printf-prs.c',text,'glibc-1.06/stdio', 4289 ).
file('printf-prs.dep',text,'glibc-1.06/stdio', 538 ).
file('printf.c',text,'glibc-1.06/stdio', 1146 ).
file('printf.dep',text,'glibc-1.06/stdio', 276 ).
file('printf.h',text,'glibc-1.06/stdio', 3800 ).
file('printf_fp.dep',text,'glibc-1.06/stdio', 938 ).
file('psignal.c',text,'glibc-1.06/stdio', 1588 ).
file('psignal.dep',text,'glibc-1.06/stdio', 449 ).
file('putc.c',text,'glibc-1.06/stdio', 91 ).
file('putc.dep',text,'glibc-1.06/stdio', 307 ).
file('putchar.c',text,'glibc-1.06/stdio', 987 ).
file('putchar.dep',text,'glibc-1.06/stdio', 279 ).
file('puts.c',text,'glibc-1.06/stdio', 1077 ).
file('puts.dep',text,'glibc-1.06/stdio', 305 ).
file('putw.c',text,'glibc-1.06/stdio', 1084 ).
file('putw.dep',text,'glibc-1.06/stdio', 270 ).
file('reg-printf.c',text,'glibc-1.06/stdio', 1566 ).
file('reg-printf.dep',text,'glibc-1.06/stdio', 369 ).
file('remove.c',text,'glibc-1.06/stdio', 1006 ).
file('remove.dep',text,'glibc-1.06/stdio', 387 ).
file('rename.dep',text,'glibc-1.06/stdio', 128 ).
file('rewind.c',text,'glibc-1.06/stdio', 1096 ).
file('rewind.dep',text,'glibc-1.06/stdio', 276 ).
file('scanf.c',text,'glibc-1.06/stdio', 1151 ).
file('scanf.dep',text,'glibc-1.06/stdio', 273 ).
file('setbuf.c',text,'glibc-1.06/stdio', 1135 ).
file('setbuf.dep',text,'glibc-1.06/stdio', 276 ).
file('setbuffer.c',text,'glibc-1.06/stdio', 1144 ).
file('setbuffer.dep',text,'glibc-1.06/stdio', 289 ).
file('setlinebuf.c',text,'glibc-1.06/stdio', 1034 ).
file('setlinebuf.dep',text,'glibc-1.06/stdio', 288 ).
file('setvbuf.c',text,'glibc-1.06/stdio', 2561 ).
file('setvbuf.dep',text,'glibc-1.06/stdio', 368 ).
file('siglist.c',text,'glibc-1.06/stdio', 917 ).
file('siglist.dep',text,'glibc-1.06/stdio', 61 ).
file('snprintf.c',text,'glibc-1.06/stdio', 1257 ).
file('snprintf.dep',text,'glibc-1.06/stdio', 282 ).
file('sprintf.c',text,'glibc-1.06/stdio', 1172 ).
file('sprintf.dep',text,'glibc-1.06/stdio', 279 ).
file('sscanf.c',text,'glibc-1.06/stdio', 1176 ).
file('sscanf.dep',text,'glibc-1.06/stdio', 276 ).
file('stdio.h',text,'glibc-1.06/stdio', 21922 ).
file('stdio_init.dep',text,'glibc-1.06/stdio', 466 ).
file('syms-stdio.c',text,'glibc-1.06/stdio', 1017 ).
file('syms-stdio.dep',text,'glibc-1.06/stdio', 73 ).
file('sysd-stdio.dep',text,'glibc-1.06/stdio', 658 ).
file('tempnam.c',text,'glibc-1.06/stdio', 1725 ).
file('tempnam.dep',text,'glibc-1.06/stdio', 403 ).
file('tempname.dep',text,'glibc-1.06/stdio', 652 ).
file('test-fseek.c',text,'glibc-1.06/stdio', 1202 ).
file('test-fseek.dep',text,'glibc-1.06/stdio', 288 ).
file('test-popen.c',text,'glibc-1.06/stdio', 1119 ).
file('test-popen.dep',text,'glibc-1.06/stdio', 377 ).
file('test_rdwr.c',text,'glibc-1.06/stdio', 2948 ).
file('test_rdwr.dep',text,'glibc-1.06/stdio', 409 ).
file('tmpfile.c',text,'glibc-1.06/stdio', 1521 ).
file('tmpfile.dep',text,'glibc-1.06/stdio', 279 ).
file('tmpnam.c',text,'glibc-1.06/stdio', 1231 ).
file('tmpnam.dep',text,'glibc-1.06/stdio', 311 ).
file('tst-printf.c',text,'glibc-1.06/stdio', 6153 ).
file('tst-printf.dep',text,'glibc-1.06/stdio', 476 ).
file('tstgetln.c',text,'glibc-1.06/stdio', 1239 ).
file('tstgetln.dep',text,'glibc-1.06/stdio', 282 ).
file('tstgetln.input',text,'glibc-1.06/stdio', 904 ).
file('tstscanf.c',text,'glibc-1.06/stdio', 2706 ).
file('tstscanf.dep',text,'glibc-1.06/stdio', 406 ).
file('tstscanf.input',text,'glibc-1.06/stdio', 139 ).
file('ungetc.c',text,'glibc-1.06/stdio', 1881 ).
file('ungetc.dep',text,'glibc-1.06/stdio', 299 ).
file('vasprintf.c',text,'glibc-1.06/stdio', 2566 ).
file('vasprintf.dep',text,'glibc-1.06/stdio', 409 ).
file('vdprintf.c',text,'glibc-1.06/stdio', 1677 ).
file('vdprintf.dep',text,'glibc-1.06/stdio', 516 ).
file('vfprintf.c',text,'glibc-1.06/stdio', 14857 ).
file('vfprintf.dep',text,'glibc-1.06/stdio', 881 ).
file('vfscanf.c',text,'glibc-1.06/stdio', 1088 ).
file('vfscanf.dep',text,'glibc-1.06/stdio', 298 ).
file('vprintf.c',text,'glibc-1.06/stdio', 1189 ).
file('vprintf.dep',text,'glibc-1.06/stdio', 279 ).
file('vscanf.c',text,'glibc-1.06/stdio', 1137 ).
file('vscanf.dep',text,'glibc-1.06/stdio', 276 ).
file('vsnprintf.c',text,'glibc-1.06/stdio', 1826 ).
file('vsnprintf.dep',text,'glibc-1.06/stdio', 320 ).
file('vsprintf.c',text,'glibc-1.06/stdio', 1545 ).
file('vsprintf.dep',text,'glibc-1.06/stdio', 516 ).
file('vsscanf.c',text,'glibc-1.06/stdio', 1095 ).
file('vsscanf.dep',text,'glibc-1.06/stdio', 298 ).
file('Makefile',text,'glibc-1.06/stdlib', 1489 ).
file('TAGS',text,'glibc-1.06/stdlib', 1209 ).
file('__random.c',text,'glibc-1.06/stdlib', 12518 ).
file('__random.dep',text,'glibc-1.06/stdlib', 338 ).
file('abort.c',text,'glibc-1.06/stdlib', 1760 ).
file('abort.dep',text,'glibc-1.06/stdlib', 606 ).
file('abs.c',text,'glibc-1.06/stdlib', 989 ).
file('abs.dep',text,'glibc-1.06/stdlib', 147 ).
file('alloca.h',text,'glibc-1.06/stdlib', 1344 ).
file('atexit.c',text,'glibc-1.06/stdlib', 1794 ).
file('atexit.dep',text,'glibc-1.06/stdlib', 163 ).
file('atof.c',text,'glibc-1.06/stdlib', 1008 ).
file('atof.dep',text,'glibc-1.06/stdlib', 150 ).
file('atoi.c',text,'glibc-1.06/stdlib', 1013 ).
file('atoi.dep',text,'glibc-1.06/stdlib', 150 ).
file('atol.c',text,'glibc-1.06/stdlib', 1016 ).
file('atol.dep',text,'glibc-1.06/stdlib', 150 ).
file('bsearch.c',text,'glibc-1.06/stdlib', 1614 ).
file('bsearch.dep',text,'glibc-1.06/stdlib', 159 ).
file('depend-stdlib',text,'glibc-1.06/stdlib', 1005 ).
file('div.c',text,'glibc-1.06/stdlib', 4021 ).
file('div.dep',text,'glibc-1.06/stdlib', 147 ).
file('exit.c',text,'glibc-1.06/stdlib', 1953 ).
file('exit.dep',text,'glibc-1.06/stdlib', 440 ).
file('exit.h',text,'glibc-1.06/stdlib', 1344 ).
file('getenv.dep',text,'glibc-1.06/stdlib', 340 ).
file('labs.c',text,'glibc-1.06/stdlib', 1002 ).
file('labs.dep',text,'glibc-1.06/stdlib', 150 ).
file('ldiv.c',text,'glibc-1.06/stdlib', 4035 ).
file('ldiv.dep',text,'glibc-1.06/stdlib', 150 ).
file('mblen.c',text,'glibc-1.06/stdlib', 1098 ).
file('mblen.dep',text,'glibc-1.06/stdlib', 153 ).
file('mbstowcs.c',text,'glibc-1.06/stdlib', 2020 ).
file('mbstowcs.dep',text,'glibc-1.06/stdlib', 194 ).
file('mbtowc.c',text,'glibc-1.06/stdlib', 2099 ).
file('mbtowc.dep',text,'glibc-1.06/stdlib', 621 ).
file('msort.c',text,'glibc-1.06/stdlib', 2530 ).
file('msort.dep',text,'glibc-1.06/stdlib', 184 ).
file('on_exit.c',text,'glibc-1.06/stdlib', 1202 ).
file('on_exit.dep',text,'glibc-1.06/stdlib', 166 ).
file('putenv.dep',text,'glibc-1.06/stdlib', 340 ).
file('qsort.c',text,'glibc-1.06/stdlib', 8152 ).
file('qsort.dep',text,'glibc-1.06/stdlib', 184 ).
file('rand.c',text,'glibc-1.06/stdlib', 989 ).
file('rand.dep',text,'glibc-1.06/stdlib', 150 ).
file('random.c',text,'glibc-1.06/stdlib', 1383 ).
file('random.dep',text,'glibc-1.06/stdlib', 171 ).
file('setenv.dep',text,'glibc-1.06/stdlib', 340 ).
file('srand.c',text,'glibc-1.06/stdlib', 1002 ).
file('srand.dep',text,'glibc-1.06/stdlib', 168 ).
file('stdlib.h',text,'glibc-1.06/stdlib', 9554 ).
file('strtod.c',text,'glibc-1.06/stdlib', 4498 ).
file('strtod.dep',text,'glibc-1.06/stdlib', 655 ).
file('strtol.c',text,'glibc-1.06/stdlib', 3731 ).
file('strtol.dep',text,'glibc-1.06/stdlib', 360 ).
file('strtoul.c',text,'glibc-1.06/stdlib', 865 ).
file('strtoul.dep',text,'glibc-1.06/stdlib', 374 ).
file('system.dep',text,'glibc-1.06/stdlib', 715 ).
file('testdiv.c',text,'glibc-1.06/stdlib', 1082 ).
file('testdiv.dep',text,'glibc-1.06/stdlib', 346 ).
file('testdiv.input',text,'glibc-1.06/stdlib', 11 ).
file('testmb.c',text,'glibc-1.06/stdlib', 646 ).
file('testmb.dep',text,'glibc-1.06/stdlib', 343 ).
file('testrand.c',text,'glibc-1.06/stdlib', 1466 ).
file('testrand.dep',text,'glibc-1.06/stdlib', 349 ).
file('testsort.c',text,'glibc-1.06/stdlib', 691 ).
file('testsort.dep',text,'glibc-1.06/stdlib', 384 ).
file('testsort.input',text,'glibc-1.06/stdlib', 206672 ).
file('tst-strtod.c',text,'glibc-1.06/stdlib', 2520 ).
file('tst-strtod.dep',text,'glibc-1.06/stdlib', 418 ).
file('tst-strtol.c',text,'glibc-1.06/stdlib', 3380 ).
file('tst-strtol.dep',text,'glibc-1.06/stdlib', 451 ).
file('wcstombs.c',text,'glibc-1.06/stdlib', 2129 ).
file('wcstombs.dep',text,'glibc-1.06/stdlib', 627 ).
file('wctomb.c',text,'glibc-1.06/stdlib', 1799 ).
file('wctomb.dep',text,'glibc-1.06/stdlib', 589 ).
file('Makefile',text,'glibc-1.06/string', 1411 ).
file('TAGS',text,'glibc-1.06/string', 5075 ).
file('__memccpy.dep',text,'glibc-1.06/string', 86 ).
file('bcmp.c',text,'glibc-1.06/string', 1043 ).
file('bcmp.dep',text,'glibc-1.06/string', 131 ).
file('bcopy.dep',text,'glibc-1.06/string', 251 ).
file('bzero.dep',text,'glibc-1.06/string', 222 ).
file('depend-string',text,'glibc-1.06/string', 1223 ).
file('endian.h',text,'glibc-1.06/string', 1555 ).
file('ffs.dep',text,'glibc-1.06/string', 308 ).
file('index.c',text,'glibc-1.06/string', 1034 ).
file('index.dep',text,'glibc-1.06/string', 146 ).
file('memccpy.c',text,'glibc-1.06/string', 1068 ).
file('memccpy.dep',text,'glibc-1.06/string', 140 ).
file('memchr.dep',text,'glibc-1.06/string', 141 ).
file('memcmp.dep',text,'glibc-1.06/string', 211 ).
file('memcpy.dep',text,'glibc-1.06/string', 225 ).
file('memfrob.c',text,'glibc-1.06/string', 1006 ).
file('memfrob.dep',text,'glibc-1.06/string', 125 ).
file('memmem.dep',text,'glibc-1.06/string', 141 ).
file('memmove.dep',text,'glibc-1.06/string', 228 ).
file('memory.h',text,'glibc-1.06/string', 990 ).
file('memset.dep',text,'glibc-1.06/string', 225 ).
file('rindex.c',text,'glibc-1.06/string', 1032 ).
file('rindex.dep',text,'glibc-1.06/string', 149 ).
file('stpcpy.dep',text,'glibc-1.06/string', 141 ).
file('strcasecmp.dep',text,'glibc-1.06/string', 181 ).
file('strcat.dep',text,'glibc-1.06/string', 225 ).
file('strchr.dep',text,'glibc-1.06/string', 141 ).
file('strcmp.dep',text,'glibc-1.06/string', 225 ).
file('strcoll.c',text,'glibc-1.06/string', 2235 ).
file('strcoll.dep',text,'glibc-1.06/string', 429 ).
file('strcpy.dep',text,'glibc-1.06/string', 225 ).
file('strcspn.dep',text,'glibc-1.06/string', 144 ).
file('strdup.c',text,'glibc-1.06/string', 1160 ).
file('strdup.dep',text,'glibc-1.06/string', 211 ).
file('strerror.c',text,'glibc-1.06/string', 1582 ).
file('strerror.dep',text,'glibc-1.06/string', 315 ).
file('strfry.c',text,'glibc-1.06/string', 1267 ).
file('strfry.dep',text,'glibc-1.06/string', 272 ).
file('string.h',text,'glibc-1.06/string', 6586 ).
file('strings.h',text,'glibc-1.06/string', 1025 ).
file('strlen.dep',text,'glibc-1.06/string', 141 ).
file('strncase.dep',text,'glibc-1.06/string', 175 ).
file('strncat.dep',text,'glibc-1.06/string', 228 ).
file('strncmp.dep',text,'glibc-1.06/string', 228 ).
file('strncpy.dep',text,'glibc-1.06/string', 228 ).
file('strpbrk.dep',text,'glibc-1.06/string', 144 ).
file('strrchr.dep',text,'glibc-1.06/string', 144 ).
file('strsep.dep',text,'glibc-1.06/string', 141 ).
file('strsignal.c',text,'glibc-1.06/string', 1551 ).
file('strsignal.dep',text,'glibc-1.06/string', 488 ).
file('strspn.dep',text,'glibc-1.06/string', 141 ).
file('strstr.dep',text,'glibc-1.06/string', 141 ).
file('strtok.c',text,'glibc-1.06/string', 1860 ).
file('strtok.dep',text,'glibc-1.06/string', 145 ).
file('strxfrm.c',text,'glibc-1.06/string', 2289 ).
file('strxfrm.dep',text,'glibc-1.06/string', 429 ).
file('swab.c',text,'glibc-1.06/string', 1071 ).
file('swab.dep',text,'glibc-1.06/string', 52 ).
file('testcopy.c',text,'glibc-1.06/string', 3010 ).
file('testcopy.dep',text,'glibc-1.06/string', 435 ).
file('tester.c',text,'glibc-1.06/string', 20232 ).
file('tester.dep',text,'glibc-1.06/string', 482 ).
file('wordcopy.dep',text,'glibc-1.06/string', 220 ).
file('bitypes.h',text,'glibc-1.06/sys', 30 ).
file('cdefs.h',text,'glibc-1.06/sys', 28 ).
file('dir.h',text,'glibc-1.06/sys', 26 ).
file('errno.h',text,'glibc-1.06/sys', 19 ).
file('file.h',text,'glibc-1.06/sys', 27 ).
file('ioctl.h',text,'glibc-1.06/sys', 28 ).
file('param.h',text,'glibc-1.06/sys', 5099 ).
file('ptrace.h',text,'glibc-1.06/sys', 29 ).
file('resource.h',text,'glibc-1.06/sys', 35 ).
file('signal.h',text,'glibc-1.06/sys', 31 ).
file('socket.h',text,'glibc-1.06/sys', 31 ).
file('stat.h',text,'glibc-1.06/sys', 25 ).
file('syslog.h',text,'glibc-1.06/sys', 29 ).
file('termios.h',text,'glibc-1.06/sys', 33 ).
file('time.h',text,'glibc-1.06/sys', 27 ).
file('times.h',text,'glibc-1.06/sys', 29 ).
file('ttydefaults.h',text,'glibc-1.06/sys', 37 ).
file('types.h',text,'glibc-1.06/sys', 29 ).
file('uio.h',text,'glibc-1.06/sys', 26 ).
file('un.h',text,'glibc-1.06/sys', 27 ).
file('unistd.h',text,'glibc-1.06/sys', 30 ).
file('utsname.h',text,'glibc-1.06/sys', 31 ).
file('vlimit.h',text,'glibc-1.06/sys', 33 ).
file('vtimes.h',text,'glibc-1.06/sys', 33 ).
file('wait.h',text,'glibc-1.06/sys', 28 ).
file('alpha',dir,'glibc-1.06/sysdeps', 512 ).
file('am29k',dir,'glibc-1.06/sysdeps', 512 ).
file('generic',dir,'glibc-1.06/sysdeps', 2048 ).
file('i386',dir,'glibc-1.06/sysdeps', 512 ).
file('i860',dir,'glibc-1.06/sysdeps', 512 ).
file('ieee754',dir,'glibc-1.06/sysdeps', 512 ).
file('m68k',dir,'glibc-1.06/sysdeps', 512 ).
file('m88k',dir,'glibc-1.06/sysdeps', 512 ).
file('mach',dir,'glibc-1.06/sysdeps', 512 ).
file('mips',dir,'glibc-1.06/sysdeps', 512 ).
file('posix',dir,'glibc-1.06/sysdeps', 1024 ).
file('rs6000',dir,'glibc-1.06/sysdeps', 512 ).
file('sparc',dir,'glibc-1.06/sysdeps', 512 ).
file('stub',dir,'glibc-1.06/sysdeps', 4608 ).
file('tahoe',dir,'glibc-1.06/sysdeps', 512 ).
file('unix',dir,'glibc-1.06/sysdeps', 1536 ).
file('vax',dir,'glibc-1.06/sysdeps', 1024 ).
file('Dist',text,'glibc-1.06/sysdeps/alpha', 13 ).
file('Implies',text,'glibc-1.06/sysdeps/alpha', 46 ).
file('Makefile',text,'glibc-1.06/sysdeps/alpha', 79 ).
file('__copysign.c',text,'glibc-1.06/sysdeps/alpha', 1008 ).
file('__longjmp.c',text,'glibc-1.06/sysdeps/alpha', 3000 ).
file('__math.h',text,'glibc-1.06/sysdeps/alpha', 1147 ).
file('bytesex.h',text,'glibc-1.06/sysdeps/alpha', 69 ).
file('fabs.c',text,'glibc-1.06/sysdeps/alpha', 933 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/alpha', 1871 ).
file('memchr.c',text,'glibc-1.06/sysdeps/alpha', 2210 ).
file('setjmp.S',text,'glibc-1.06/sysdeps/alpha', 1227 ).
file('setjmp_aux.c',text,'glibc-1.06/sysdeps/alpha', 2217 ).
file('strchr.c',text,'glibc-1.06/sysdeps/alpha', 2613 ).
file('strlen.c',text,'glibc-1.06/sysdeps/alpha', 1754 ).
file('ffs.c',text,'glibc-1.06/sysdeps/am29k', 1168 ).
file('Dist',text,'glibc-1.06/sysdeps/generic', 103 ).
file('Makefile',text,'glibc-1.06/sysdeps/generic', 1877 ).
file('__copysign.c',text,'glibc-1.06/sysdeps/generic', 1016 ).
file('__expm1.c',text,'glibc-1.06/sysdeps/generic', 5569 ).
file('__infnan.c',text,'glibc-1.06/sysdeps/generic', 1409 ).
file('__isnan.c',text,'glibc-1.06/sysdeps/generic', 985 ).
file('__lstat.c',text,'glibc-1.06/sysdeps/generic', 1067 ).
file('__memccpy.c',text,'glibc-1.06/sysdeps/generic', 1406 ).
file('__rint.c',text,'glibc-1.06/sysdeps/generic', 3099 ).
file('__vfork.c',text,'glibc-1.06/sysdeps/generic', 1031 ).
file('acos.c',text,'glibc-1.06/sysdeps/generic', 1277 ).
file('acosh.c',text,'glibc-1.06/sysdeps/generic', 3842 ).
file('asin.c',text,'glibc-1.06/sysdeps/generic', 1223 ).
file('asincos.c',text,'glibc-1.06/sysdeps/generic', 5816 ).
file('asinh.c',text,'glibc-1.06/sysdeps/generic', 3899 ).
file('atan.c',text,'glibc-1.06/sysdeps/generic', 977 ).
file('atan2.c',text,'glibc-1.06/sysdeps/generic', 10772 ).
file('atanh.c',text,'glibc-1.06/sysdeps/generic', 3227 ).
file('bcopy.c',text,'glibc-1.06/sysdeps/generic', 1039 ).
file('bzero.c',text,'glibc-1.06/sysdeps/generic', 2104 ).
file('cabs.c',text,'glibc-1.06/sysdeps/generic', 956 ).
file('ceil.c',text,'glibc-1.06/sysdeps/generic', 2559 ).
file('confname.h',text,'glibc-1.06/sysdeps/generic', 1974 ).
file('cosh.c',text,'glibc-1.06/sysdeps/generic', 5056 ).
file('det_endian.c',text,'glibc-1.06/sysdeps/generic', 1137 ).
file('exp.c',text,'glibc-1.06/sysdeps/generic', 5859 ).
file('exp__E.c',text,'glibc-1.06/sysdeps/generic', 5224 ).
file('fabs.c',text,'glibc-1.06/sysdeps/generic', 979 ).
file('ffs.c',text,'glibc-1.06/sysdeps/generic', 1760 ).
file('floor.c',text,'glibc-1.06/sysdeps/generic', 2787 ).
file('fmod.c',text,'glibc-1.06/sysdeps/generic', 4309 ).
file('frexp.c',text,'glibc-1.06/sysdeps/generic', 1483 ).
file('htonl.c',text,'glibc-1.06/sysdeps/generic', 1074 ).
file('htons.c',text,'glibc-1.06/sysdeps/generic', 1028 ).
file('hypot.c',text,'glibc-1.06/sysdeps/generic', 980 ).
file('ldexp.c',text,'glibc-1.06/sysdeps/generic', 1023 ).
file('log.c',text,'glibc-1.06/sysdeps/generic', 5402 ).
file('log10.c',text,'glibc-1.06/sysdeps/generic', 1101 ).
file('log1p.c',text,'glibc-1.06/sysdeps/generic', 5934 ).
file('log__L.c',text,'glibc-1.06/sysdeps/generic', 4588 ).
file('make_siglist.c',text,'glibc-1.06/sysdeps/generic', 1371 ).
file('mathimpl.h',text,'glibc-1.06/sysdeps/generic', 4194 ).
file('memchr.c',text,'glibc-1.06/sysdeps/generic', 4892 ).
file('memcmp.c',text,'glibc-1.06/sysdeps/generic', 7818 ).
file('memcopy.h',text,'glibc-1.06/sysdeps/generic', 5795 ).
file('memcpy.c',text,'glibc-1.06/sysdeps/generic', 1917 ).
file('memmem.c',text,'glibc-1.06/sysdeps/generic', 1489 ).
file('memmove.c',text,'glibc-1.06/sysdeps/generic', 3188 ).
file('memset.c',text,'glibc-1.06/sysdeps/generic', 2189 ).
file('modf.c',text,'glibc-1.06/sysdeps/generic', 1084 ).
file('ntohl.c',text,'glibc-1.06/sysdeps/generic', 1074 ).
file('ntohs.c',text,'glibc-1.06/sysdeps/generic', 1028 ).
file('pow.c',text,'glibc-1.06/sysdeps/generic', 8985 ).
file('printf_fp.c',text,'glibc-1.06/sysdeps/generic', 10514 ).
file('sigaction.h',text,'glibc-1.06/sysdeps/generic', 1943 ).
file('signame.c',text,'glibc-1.06/sysdeps/generic', 7018 ).
file('signame.h',text,'glibc-1.06/sysdeps/generic', 1854 ).
file('sigset.h',text,'glibc-1.06/sysdeps/generic', 1485 ).
file('sincos.c',text,'glibc-1.06/sysdeps/generic', 3085 ).
file('sinh.c',text,'glibc-1.06/sysdeps/generic', 4621 ).
file('speed.c',text,'glibc-1.06/sysdeps/generic', 1883 ).
file('stpcpy.c',text,'glibc-1.06/sysdeps/generic', 1142 ).
file('strcasecmp.c',text,'glibc-1.06/sysdeps/generic', 1456 ).
file('strcat.c',text,'glibc-1.06/sysdeps/generic', 1377 ).
file('strchr.c',text,'glibc-1.06/sysdeps/generic', 5090 ).
file('strcmp.c',text,'glibc-1.06/sysdeps/generic', 1427 ).
file('strcpy.c',text,'glibc-1.06/sysdeps/generic', 1174 ).
file('strcspn.c',text,'glibc-1.06/sysdeps/generic', 1225 ).
file('strlen.c',text,'glibc-1.06/sysdeps/generic', 4330 ).
file('strncase.c',text,'glibc-1.06/sysdeps/generic', 1534 ).
file('strncat.c',text,'glibc-1.06/sysdeps/generic', 1703 ).
file('strncmp.c',text,'glibc-1.06/sysdeps/generic', 1956 ).
file('strncpy.c',text,'glibc-1.06/sysdeps/generic', 1659 ).
file('strpbrk.c',text,'glibc-1.06/sysdeps/generic', 1153 ).
file('strrchr.c',text,'glibc-1.06/sysdeps/generic', 1276 ).
file('strsep.c',text,'glibc-1.06/sysdeps/generic', 1192 ).
file('strspn.c',text,'glibc-1.06/sysdeps/generic', 1316 ).
file('strstr.c',text,'glibc-1.06/sysdeps/generic', 1836 ).
file('tan.c',text,'glibc-1.06/sysdeps/generic', 2643 ).
file('tanh.c',text,'glibc-1.06/sysdeps/generic', 3711 ).
file('termbits.h',text,'glibc-1.06/sysdeps/generic', 7751 ).
file('trig.h',text,'glibc-1.06/sysdeps/generic', 8570 ).
file('uname.c',text,'glibc-1.06/sysdeps/generic', 1866 ).
file('utsnamelen.h',text,'glibc-1.06/sysdeps/generic', 151 ).
file('vfork.c',text,'glibc-1.06/sysdeps/generic', 981 ).
file('vlimit.c',text,'glibc-1.06/sysdeps/generic', 1740 ).
file('vtimes.c',text,'glibc-1.06/sysdeps/generic', 2261 ).
file('waitstatus.h',text,'glibc-1.06/sysdeps/generic', 3395 ).
file('wordcopy.c',text,'glibc-1.06/sysdeps/generic', 9829 ).
file('Implies',text,'glibc-1.06/sysdeps/i386', 8 ).
file('__longjmp.c',text,'glibc-1.06/sysdeps/i386', 1778 ).
file('bytesex.h',text,'glibc-1.06/sysdeps/i386', 68 ).
file('bzero.c',text,'glibc-1.06/sysdeps/i386', 2296 ).
file('ffs.c',text,'glibc-1.06/sysdeps/i386', 1450 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/i386', 195 ).
file('memchr.c',text,'glibc-1.06/sysdeps/i386', 1653 ).
file('memcopy.h',text,'glibc-1.06/sysdeps/i386', 3244 ).
file('memset.c',text,'glibc-1.06/sysdeps/i386', 2396 ).
file('setjmp.c',text,'glibc-1.06/sysdeps/i386', 1435 ).
file('strlen.c',text,'glibc-1.06/sysdeps/i386', 1209 ).
file('memcopy.h',text,'glibc-1.06/sysdeps/i860', 1264 ).
file('Dist',text,'glibc-1.06/sysdeps/ieee754', 20 ).
file('__copysign.c',text,'glibc-1.06/sysdeps/ieee754', 1113 ).
file('__drem.c',text,'glibc-1.06/sysdeps/ieee754', 3079 ).
file('__infnan.c',text,'glibc-1.06/sysdeps/ieee754', 1455 ).
file('__isinf.c',text,'glibc-1.06/sysdeps/ieee754', 1325 ).
file('__isnan.c',text,'glibc-1.06/sysdeps/ieee754', 1220 ).
file('__logb.c',text,'glibc-1.06/sysdeps/ieee754', 1469 ).
file('cabs.c',text,'glibc-1.06/sysdeps/ieee754', 6912 ).
file('cbrt.c',text,'glibc-1.06/sysdeps/ieee754', 4106 ).
file('frexp.c',text,'glibc-1.06/sysdeps/ieee754', 1219 ).
file('huge_val.h',text,'glibc-1.06/sysdeps/ieee754', 1606 ).
file('ieee754.h',text,'glibc-1.06/sysdeps/ieee754', 1425 ).
file('ldexp.c',text,'glibc-1.06/sysdeps/ieee754', 4152 ).
file('log10.c',text,'glibc-1.06/sysdeps/ieee754', 1074 ).
file('nan.h',text,'glibc-1.06/sysdeps/ieee754', 1426 ).
file('printf_fp.c',text,'glibc-1.06/sysdeps/ieee754', 2790 ).
file('sqrt.c',text,'glibc-1.06/sysdeps/ieee754', 2676 ).
file('support.c',text,'glibc-1.06/sysdeps/ieee754', 17742 ).
file('Implies',text,'glibc-1.06/sysdeps/m68k', 44 ).
file('Makefile',text,'glibc-1.06/sysdeps/m68k', 378 ).
file('__longjmp.c',text,'glibc-1.06/sysdeps/m68k', 2096 ).
file('ffs.c',text,'glibc-1.06/sysdeps/m68k', 1228 ).
file('fpu',dir,'glibc-1.06/sysdeps/m68k', 1024 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/m68k', 528 ).
file('memcopy.h',text,'glibc-1.06/sysdeps/m68k', 3661 ).
file('setjmp.c',text,'glibc-1.06/sysdeps/m68k', 1763 ).
file('Makefile',text,'glibc-1.06/sysdeps/m68k/fpu', 157 ).
file('__drem.c',text,'glibc-1.06/sysdeps/m68k/fpu', 992 ).
file('__expm1.c',text,'glibc-1.06/sysdeps/m68k/fpu', 56 ).
file('__isinf.c',text,'glibc-1.06/sysdeps/m68k/fpu', 997 ).
file('__isnan.c',text,'glibc-1.06/sysdeps/m68k/fpu', 42 ).
file('__logb.c',text,'glibc-1.06/sysdeps/m68k/fpu', 1253 ).
file('__math.h',text,'glibc-1.06/sysdeps/m68k/fpu', 4526 ).
file('__rint.c',text,'glibc-1.06/sysdeps/m68k/fpu', 54 ).
file('acos.c',text,'glibc-1.06/sysdeps/m68k/fpu', 997 ).
file('asin.c',text,'glibc-1.06/sysdeps/m68k/fpu', 36 ).
file('atan.c',text,'glibc-1.06/sysdeps/m68k/fpu', 36 ).
file('atan2.c',text,'glibc-1.06/sysdeps/m68k/fpu', 1802 ).
file('atanh.c',text,'glibc-1.06/sysdeps/m68k/fpu', 37 ).
file('ceil.c',text,'glibc-1.06/sysdeps/m68k/fpu', 38 ).
file('cos.c',text,'glibc-1.06/sysdeps/m68k/fpu', 35 ).
file('cosh.c',text,'glibc-1.06/sysdeps/m68k/fpu', 36 ).
file('exp.c',text,'glibc-1.06/sysdeps/m68k/fpu', 51 ).
file('fabs.c',text,'glibc-1.06/sysdeps/m68k/fpu', 51 ).
file('floor.c',text,'glibc-1.06/sysdeps/m68k/fpu', 54 ).
file('fmod.c',text,'glibc-1.06/sysdeps/m68k/fpu', 969 ).
file('frexp.c',text,'glibc-1.06/sysdeps/m68k/fpu', 996 ).
file('ldexp.c',text,'glibc-1.06/sysdeps/m68k/fpu', 974 ).
file('log.c',text,'glibc-1.06/sysdeps/m68k/fpu', 51 ).
file('log10.c',text,'glibc-1.06/sysdeps/m68k/fpu', 37 ).
file('log1p.c',text,'glibc-1.06/sysdeps/m68k/fpu', 37 ).
file('pow.c',text,'glibc-1.06/sysdeps/m68k/fpu', 967 ).
file('printf_fp.c',text,'glibc-1.06/sysdeps/m68k/fpu', 1207 ).
file('sin.c',text,'glibc-1.06/sysdeps/m68k/fpu', 35 ).
file('sinh.c',text,'glibc-1.06/sysdeps/m68k/fpu', 36 ).
file('sqrt.c',text,'glibc-1.06/sysdeps/m68k/fpu', 36 ).
file('switch',dir,'glibc-1.06/sysdeps/m68k/fpu', 512 ).
file('tan.c',text,'glibc-1.06/sysdeps/m68k/fpu', 35 ).
file('tanh.c',text,'glibc-1.06/sysdeps/m68k/fpu', 36 ).
file('68881-sw.h',text,'glibc-1.06/sysdeps/m68k/fpu/switch', 2550 ).
file('Dist',text,'glibc-1.06/sysdeps/m68k/fpu/switch', 20 ).
file('Makefile',text,'glibc-1.06/sysdeps/m68k/fpu/switch', 2006 ).
file('__math.h',text,'glibc-1.06/sysdeps/m68k/fpu/switch', 65 ).
file('switch.c',text,'glibc-1.06/sysdeps/m68k/fpu/switch', 3045 ).
file('ffs.c',text,'glibc-1.06/sysdeps/m88k', 1201 ).
file('Makefile',text,'glibc-1.06/sysdeps/mach', 87 ).
file('Subdirs',text,'glibc-1.06/sysdeps/mach', 5 ).
file('__getpgsz.c',text,'glibc-1.06/sysdeps/mach', 1023 ).
file('__gettod.c',text,'glibc-1.06/sysdeps/mach', 1377 ).
file('hurd',dir,'glibc-1.06/sysdeps/mach', 2560 ).
file('i386',dir,'glibc-1.06/sysdeps/mach', 512 ).
file('morecore.c',text,'glibc-1.06/sysdeps/mach', 1414 ).
file('Dist',text,'glibc-1.06/sysdeps/mach/hurd', 23 ).
file('Implies',text,'glibc-1.06/sysdeps/mach/hurd', 13 ).
file('Makefile',text,'glibc-1.06/sysdeps/mach/hurd', 1438 ).
file('Subdirs',text,'glibc-1.06/sysdeps/mach/hurd', 5 ).
file('__access.c',text,'glibc-1.06/sysdeps/mach/hurd', 2985 ).
file('__adjtime.c',text,'glibc-1.06/sysdeps/mach/hurd', 1550 ).
file('__brk.c',text,'glibc-1.06/sysdeps/mach/hurd', 3857 ).
file('__chdir.c',text,'glibc-1.06/sysdeps/mach/hurd', 1157 ).
file('__chmod.c',text,'glibc-1.06/sysdeps/mach/hurd', 1297 ).
file('__chown.c',text,'glibc-1.06/sysdeps/mach/hurd', 1329 ).
file('__close.c',text,'glibc-1.06/sysdeps/mach/hurd', 1718 ).
file('__dup.c',text,'glibc-1.06/sysdeps/mach/hurd', 2070 ).
file('__dup2.c',text,'glibc-1.06/sysdeps/mach/hurd', 2384 ).
file('__execve.c',text,'glibc-1.06/sysdeps/mach/hurd', 6185 ).
file('__fchmod.c',text,'glibc-1.06/sysdeps/mach/hurd', 1182 ).
file('__fchown.c',text,'glibc-1.06/sysdeps/mach/hurd', 1201 ).
file('__fcntl.c',text,'glibc-1.06/sysdeps/mach/hurd', 4536 ).
file('__flock.c',text,'glibc-1.06/sysdeps/mach/hurd', 1202 ).
file('__fork.c',text,'glibc-1.06/sysdeps/mach/hurd', 8620 ).
file('__fstat.c',text,'glibc-1.06/sysdeps/mach/hurd', 1153 ).
file('__getdents.c',text,'glibc-1.06/sysdeps/mach/hurd', 1321 ).
file('__getdtsz.c',text,'glibc-1.06/sysdeps/mach/hurd', 1214 ).
file('__getegid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1484 ).
file('__geteuid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1481 ).
file('__getgid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1348 ).
file('__getgrps.c',text,'glibc-1.06/sysdeps/mach/hurd', 1519 ).
file('__gethstnm.c',text,'glibc-1.06/sysdeps/mach/hurd', 1302 ).
file('__getpgrp.c',text,'glibc-1.06/sysdeps/mach/hurd', 1202 ).
file('__getpid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1004 ).
file('__getppid.c',text,'glibc-1.06/sysdeps/mach/hurd', 996 ).
file('__getuid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1347 ).
file('__ioctl.c',text,'glibc-1.06/sysdeps/mach/hurd', 4160 ).
file('__kill.c',text,'glibc-1.06/sysdeps/mach/hurd', 2493 ).
file('__link.c',text,'glibc-1.06/sysdeps/mach/hurd', 1730 ).
file('__lseek.c',text,'glibc-1.06/sysdeps/mach/hurd', 1185 ).
file('__lstat.c',text,'glibc-1.06/sysdeps/mach/hurd', 1242 ).
file('__mkdir.c',text,'glibc-1.06/sysdeps/mach/hurd', 1337 ).
file('__mknod.c',text,'glibc-1.06/sysdeps/mach/hurd', 2027 ).
file('__open.c',text,'glibc-1.06/sysdeps/mach/hurd', 3421 ).
file('__pipe.c',text,'glibc-1.06/sysdeps/mach/hurd', 2222 ).
file('__read.c',text,'glibc-1.06/sysdeps/mach/hurd', 2149 ).
file('__readlink.c',text,'glibc-1.06/sysdeps/mach/hurd', 1798 ).
file('__rmdir.c',text,'glibc-1.06/sysdeps/mach/hurd', 1287 ).
file('__sbrk.c',text,'glibc-1.06/sysdeps/mach/hurd', 1339 ).
file('__select.c',text,'glibc-1.06/sysdeps/mach/hurd', 5871 ).
file('__setgid.c',text,'glibc-1.06/sysdeps/mach/hurd', 2324 ).
file('__setpgrp.c',text,'glibc-1.06/sysdeps/mach/hurd', 1288 ).
file('__setregid.c',text,'glibc-1.06/sysdeps/mach/hurd', 2094 ).
file('__setreuid.c',text,'glibc-1.06/sysdeps/mach/hurd', 2074 ).
file('__setsid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1248 ).
file('__settod.c',text,'glibc-1.06/sysdeps/mach/hurd', 1447 ).
file('__setuid.c',text,'glibc-1.06/sysdeps/mach/hurd', 2295 ).
file('__sigact.c',text,'glibc-1.06/sysdeps/mach/hurd', 1823 ).
file('__sigproc.c',text,'glibc-1.06/sysdeps/mach/hurd', 1899 ).
file('__stat.c',text,'glibc-1.06/sysdeps/mach/hurd', 1272 ).
file('__symlink.c',text,'glibc-1.06/sysdeps/mach/hurd', 1704 ).
file('__umask.c',text,'glibc-1.06/sysdeps/mach/hurd', 1101 ).
file('__unlink.c',text,'glibc-1.06/sysdeps/mach/hurd', 1289 ).
file('__utimes.c',text,'glibc-1.06/sysdeps/mach/hurd', 1345 ).
file('__vfork.c',text,'glibc-1.06/sysdeps/mach/hurd', 5275 ).
file('__wait4.c',text,'glibc-1.06/sysdeps/mach/hurd', 1256 ).
file('__write.c',text,'glibc-1.06/sysdeps/mach/hurd', 1942 ).
file('_exit.c',text,'glibc-1.06/sysdeps/mach/hurd', 1381 ).
file('accept.c',text,'glibc-1.06/sysdeps/mach/hurd', 1787 ).
file('bind.c',text,'glibc-1.06/sysdeps/mach/hurd', 1408 ).
file('chflags.c',text,'glibc-1.06/sysdeps/mach/hurd', 1276 ).
file('chroot.c',text,'glibc-1.06/sysdeps/mach/hurd', 1210 ).
file('connect.c',text,'glibc-1.06/sysdeps/mach/hurd', 1587 ).
file('defs.c',text,'glibc-1.06/sysdeps/mach/hurd', 1844 ).
file('errlist.awk',text,'glibc-1.06/sysdeps/mach/hurd', 1724 ).
file('errnos.awk',text,'glibc-1.06/sysdeps/mach/hurd', 2016 ).
file('fchdir.c',text,'glibc-1.06/sysdeps/mach/hurd', 1259 ).
file('fchflags.c',text,'glibc-1.06/sysdeps/mach/hurd', 1161 ).
file('fdopen.c',text,'glibc-1.06/sysdeps/mach/hurd', 1375 ).
file('fsync.c',text,'glibc-1.06/sysdeps/mach/hurd', 1122 ).
file('ftruncate.c',text,'glibc-1.06/sysdeps/mach/hurd', 1149 ).
file('gethostid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1138 ).
file('getlogin.c',text,'glibc-1.06/sysdeps/mach/hurd', 1295 ).
file('getpeernam.c',text,'glibc-1.06/sysdeps/mach/hurd', 1464 ).
file('getprio.c',text,'glibc-1.06/sysdeps/mach/hurd', 1463 ).
file('getrlimit.c',text,'glibc-1.06/sysdeps/mach/hurd', 1681 ).
file('getsocknam.c',text,'glibc-1.06/sysdeps/mach/hurd', 1397 ).
file('getsockopt.c',text,'glibc-1.06/sysdeps/mach/hurd', 1450 ).
file('i386',dir,'glibc-1.06/sysdeps/mach/hurd', 512 ).
file('ioctls.h',text,'glibc-1.06/sysdeps/mach/hurd', 13405 ).
file('listen.c',text,'glibc-1.06/sysdeps/mach/hurd', 1264 ).
file('readv.c',text,'glibc-1.06/sysdeps/mach/hurd', 33 ).
file('reboot.c',text,'glibc-1.06/sysdeps/mach/hurd', 1284 ).
file('rename.c',text,'glibc-1.06/sysdeps/mach/hurd', 1545 ).
file('setegid.c',text,'glibc-1.06/sysdeps/mach/hurd', 2139 ).
file('seteuid.c',text,'glibc-1.06/sysdeps/mach/hurd', 2114 ).
file('setgroups.c',text,'glibc-1.06/sysdeps/mach/hurd', 1998 ).
file('sethostid.c',text,'glibc-1.06/sysdeps/mach/hurd', 1167 ).
file('sethostnam.c',text,'glibc-1.06/sysdeps/mach/hurd', 1233 ).
file('setlogin.c',text,'glibc-1.06/sysdeps/mach/hurd', 1122 ).
file('setrlimit.c',text,'glibc-1.06/sysdeps/mach/hurd', 1890 ).
file('setsockopt.c',text,'glibc-1.06/sysdeps/mach/hurd', 1378 ).
file('shutdown.c',text,'glibc-1.06/sysdeps/mach/hurd', 1347 ).
file('sigaltstack.c',text,'glibc-1.06/sysdeps/mach/hurd', 1705 ).
file('sigpending.c',text,'glibc-1.06/sysdeps/mach/hurd', 1348 ).
file('sigstack.c',text,'glibc-1.06/sysdeps/mach/hurd', 1412 ).
file('sigsuspend.c',text,'glibc-1.06/sysdeps/mach/hurd', 1722 ).
file('sleep.c',text,'glibc-1.06/sysdeps/mach/hurd', 1616 ).
file('socket.c',text,'glibc-1.06/sysdeps/mach/hurd', 1485 ).
file('socketpair.c',text,'glibc-1.06/sysdeps/mach/hurd', 1832 ).
file('start.c',text,'glibc-1.06/sysdeps/mach/hurd', 7053 ).
file('statbuf.h',text,'glibc-1.06/sysdeps/mach/hurd', 2835 ).
file('stdio_init.c',text,'glibc-1.06/sysdeps/mach/hurd', 12273 ).
file('sync.c',text,'glibc-1.06/sysdeps/mach/hurd', 1113 ).
file('sysd-stdio.c',text,'glibc-1.06/sysdeps/mach/hurd', 5001 ).
file('truncate.c',text,'glibc-1.06/sysdeps/mach/hurd', 1301 ).
file('uname.c',text,'glibc-1.06/sysdeps/mach/hurd', 1296 ).
file('usleep.c',text,'glibc-1.06/sysdeps/mach/hurd', 1438 ).
file('writev.c',text,'glibc-1.06/sysdeps/mach/hurd', 34 ).
file('Implies',text,'glibc-1.06/sysdeps/mach/hurd/i386', 5 ).
file('sigcontext.h',text,'glibc-1.06/sysdeps/mach/hurd/i386', 2680 ).
file('startsig.c',text,'glibc-1.06/sysdeps/mach/hurd/i386', 1412 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/mach/hurd/i386', 1892 ).
file('Implies',text,'glibc-1.06/sysdeps/mach/i386', 5 ).
file('start.c',text,'glibc-1.06/sysdeps/mach/i386', 2297 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/mach/i386', 1234 ).
file('Dist',text,'glibc-1.06/sysdeps/mips', 13 ).
file('Implies',text,'glibc-1.06/sysdeps/mips', 45 ).
file('Makefile',text,'glibc-1.06/sysdeps/mips', 79 ).
file('__longjmp.c',text,'glibc-1.06/sysdeps/mips', 2989 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/mips', 1459 ).
file('setjmp.S',text,'glibc-1.06/sysdeps/mips', 1068 ).
file('setjmp_aux.c',text,'glibc-1.06/sysdeps/mips', 2760 ).
file('sqrt.c',text,'glibc-1.06/sysdeps/mips', 1297 ).
file('Dist',text,'glibc-1.06/sysdeps/posix', 14 ).
file('Makefile',text,'glibc-1.06/sysdeps/posix', 1683 ).
file('__dup.c',text,'glibc-1.06/sysdeps/posix', 1050 ).
file('__dup2.c',text,'glibc-1.06/sysdeps/posix', 1395 ).
file('__flock.c',text,'glibc-1.06/sysdeps/posix', 1750 ).
file('__fpathcon.c',text,'glibc-1.06/sysdeps/posix', 2269 ).
file('__getdtsz.c',text,'glibc-1.06/sysdeps/posix', 1084 ).
file('__getpgsz.c',text,'glibc-1.06/sysdeps/posix', 1003 ).
file('__gettod.c',text,'glibc-1.06/sysdeps/posix', 1907 ).
file('__isatty.c',text,'glibc-1.06/sysdeps/posix', 1133 ).
file('__sigblock.c',text,'glibc-1.06/sysdeps/posix', 1503 ).
file('__sigpause.c',text,'glibc-1.06/sysdeps/posix', 1095 ).
file('__sigstmsk.c',text,'glibc-1.06/sysdeps/posix', 1525 ).
file('__sigvec.c',text,'glibc-1.06/sysdeps/posix', 3569 ).
file('__sysconf.c',text,'glibc-1.06/sysdeps/posix', 3451 ).
file('__wait.c',text,'glibc-1.06/sysdeps/posix', 1122 ).
file('__wait3.c',text,'glibc-1.06/sysdeps/posix', 1536 ).
file('clock.c',text,'glibc-1.06/sysdeps/posix', 1143 ).
file('ctermid.c',text,'glibc-1.06/sysdeps/posix', 1268 ).
file('cuserid.c',text,'glibc-1.06/sysdeps/posix', 1399 ).
file('defs.c',text,'glibc-1.06/sysdeps/posix', 2679 ).
file('fdopen.c',text,'glibc-1.06/sysdeps/posix', 1868 ).
file('getcwd.c',text,'glibc-1.06/sysdeps/posix', 7889 ).
file('getenv.c',text,'glibc-1.06/sysdeps/posix', 1315 ).
file('killpg.c',text,'glibc-1.06/sysdeps/posix', 1189 ).
file('libc_fatal.c',text,'glibc-1.06/sysdeps/posix', 1292 ).
file('mk-stdiolim.c',text,'glibc-1.06/sysdeps/posix', 1229 ).
file('mkstemp.c',text,'glibc-1.06/sysdeps/posix', 1966 ).
file('mktemp.c',text,'glibc-1.06/sysdeps/posix', 1918 ).
file('pipestream.c',text,'glibc-1.06/sysdeps/posix', 4892 ).
file('putenv.c',text,'glibc-1.06/sysdeps/posix', 2237 ).
file('raise.c',text,'glibc-1.06/sysdeps/posix', 988 ).
file('readv.c',text,'glibc-1.06/sysdeps/posix', 2138 ).
file('rename.c',text,'glibc-1.06/sysdeps/posix', 1414 ).
file('setenv.c',text,'glibc-1.06/sysdeps/posix', 2439 ).
file('sigintr.c',text,'glibc-1.06/sysdeps/posix', 1604 ).
file('signal.c',text,'glibc-1.06/sysdeps/posix', 1490 ).
file('sigsuspend.c',text,'glibc-1.06/sysdeps/posix', 1410 ).
file('sleep.c',text,'glibc-1.06/sysdeps/posix', 3324 ).
file('stdio_init.c',text,'glibc-1.06/sysdeps/posix', 2634 ).
file('sysd-stdio.c',text,'glibc-1.06/sysdeps/posix', 3212 ).
file('system.c',text,'glibc-1.06/sysdeps/posix', 3346 ).
file('tempname.c',text,'glibc-1.06/sysdeps/posix', 5024 ).
file('ttyname.c',text,'glibc-1.06/sysdeps/posix', 2285 ).
file('writev.c',text,'glibc-1.06/sysdeps/posix', 2031 ).
file('ffs.c',text,'glibc-1.06/sysdeps/rs6000', 1171 ).
file('memcopy.h',text,'glibc-1.06/sysdeps/rs6000', 3254 ).
file('DEFS.h',text,'glibc-1.06/sysdeps/sparc', 58 ).
file('Dist',text,'glibc-1.06/sysdeps/sparc', 30 ).
file('Implies',text,'glibc-1.06/sysdeps/sparc', 46 ).
file('Makefile',text,'glibc-1.06/sysdeps/sparc', 1841 ).
file('__longjmp.S',text,'glibc-1.06/sysdeps/sparc', 1670 ).
file('divrem.m4',text,'glibc-1.06/sysdeps/sparc', 5875 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/sparc', 383 ).
file('memcopy.h',text,'glibc-1.06/sysdeps/sparc', 899 ).
file('mul.S',text,'glibc-1.06/sysdeps/sparc', 3284 ).
file('rem.S',text,'glibc-1.06/sysdeps/sparc', 7416 ).
file('sdiv.S',text,'glibc-1.06/sysdeps/sparc', 7414 ).
file('setjmp.S',text,'glibc-1.06/sysdeps/sparc', 1034 ).
file('sqrt.c',text,'glibc-1.06/sysdeps/sparc', 1152 ).
file('udiv.S',text,'glibc-1.06/sysdeps/sparc', 6905 ).
file('umul.S',text,'glibc-1.06/sysdeps/sparc', 4586 ).
file('urem.S',text,'glibc-1.06/sysdeps/sparc', 6907 ).
file('__access.c',text,'glibc-1.06/sysdeps/stub', 1250 ).
file('__adjtime.c',text,'glibc-1.06/sysdeps/stub', 1375 ).
file('__brk.c',text,'glibc-1.06/sysdeps/stub', 1125 ).
file('__chdir.c',text,'glibc-1.06/sysdeps/stub', 1202 ).
file('__chmod.c',text,'glibc-1.06/sysdeps/stub', 1252 ).
file('__chown.c',text,'glibc-1.06/sysdeps/stub', 1279 ).
file('__close.c',text,'glibc-1.06/sysdeps/stub', 1153 ).
file('__drem.c',text,'glibc-1.06/sysdeps/stub', 1015 ).
file('__dup.c',text,'glibc-1.06/sysdeps/stub', 1142 ).
file('__dup2.c',text,'glibc-1.06/sysdeps/stub', 1361 ).
file('__execve.c',text,'glibc-1.06/sysdeps/stub', 1404 ).
file('__fchmod.c',text,'glibc-1.06/sysdeps/stub', 1256 ).
file('__fchown.c',text,'glibc-1.06/sysdeps/stub', 1285 ).
file('__fcntl.c',text,'glibc-1.06/sysdeps/stub', 1183 ).
file('__flock.c',text,'glibc-1.06/sysdeps/stub', 1086 ).
file('__fork.c',text,'glibc-1.06/sysdeps/stub', 1207 ).
file('__fpathcon.c',text,'glibc-1.06/sysdeps/stub', 1439 ).
file('__fstat.c',text,'glibc-1.06/sysdeps/stub', 1299 ).
file('__getdents.c',text,'glibc-1.06/sysdeps/stub', 1165 ).
file('__getdtsz.c',text,'glibc-1.06/sysdeps/stub', 1155 ).
file('__getegid.c',text,'glibc-1.06/sysdeps/stub', 1114 ).
file('__geteuid.c',text,'glibc-1.06/sysdeps/stub', 1114 ).
file('__getgid.c',text,'glibc-1.06/sysdeps/stub', 1097 ).
file('__getgrps.c',text,'glibc-1.06/sysdeps/stub', 1525 ).
file('__gethstnm.c',text,'glibc-1.06/sysdeps/stub', 1267 ).
file('__getitmr.c',text,'glibc-1.06/sysdeps/stub', 1314 ).
file('__getpgrp.c',text,'glibc-1.06/sysdeps/stub', 1113 ).
file('__getpgsz.c',text,'glibc-1.06/sysdeps/stub', 1072 ).
file('__getpid.c',text,'glibc-1.06/sysdeps/stub', 1096 ).
file('__getppid.c',text,'glibc-1.06/sysdeps/stub', 1104 ).
file('__getrusag.c',text,'glibc-1.06/sysdeps/stub', 1253 ).
file('__gettod.c',text,'glibc-1.06/sysdeps/stub', 1280 ).
file('__getuid.c',text,'glibc-1.06/sysdeps/stub', 1097 ).
file('__ioctl.c',text,'glibc-1.06/sysdeps/stub', 1239 ).
file('__isatty.c',text,'glibc-1.06/sysdeps/stub', 1003 ).
file('__isinf.c',text,'glibc-1.06/sysdeps/stub', 1112 ).
file('__kill.c',text,'glibc-1.06/sysdeps/stub', 1270 ).
file('__link.c',text,'glibc-1.06/sysdeps/stub', 1229 ).
file('__logb.c',text,'glibc-1.06/sysdeps/stub', 1013 ).
file('__longjmp.c',text,'glibc-1.06/sysdeps/stub', 1258 ).
file('__lseek.c',text,'glibc-1.06/sysdeps/stub', 1392 ).
file('__lstat.c',text,'glibc-1.06/sysdeps/stub', 1294 ).
file('__math.h',text,'glibc-1.06/sysdeps/stub', 240 ).
file('__mkdir.c',text,'glibc-1.06/sysdeps/stub', 1263 ).
file('__mknod.c',text,'glibc-1.06/sysdeps/stub', 1308 ).
file('__open.c',text,'glibc-1.06/sysdeps/stub', 1452 ).
file('__pathconf.c',text,'glibc-1.06/sysdeps/stub', 1136 ).
file('__pipe.c',text,'glibc-1.06/sysdeps/stub', 1379 ).
file('__read.c',text,'glibc-1.06/sysdeps/stub', 1358 ).
file('__readlink.c',text,'glibc-1.06/sysdeps/stub', 1303 ).
file('__rmdir.c',text,'glibc-1.06/sysdeps/stub', 1190 ).
file('__sbrk.c',text,'glibc-1.06/sysdeps/stub', 1233 ).
file('__select.c',text,'glibc-1.06/sysdeps/stub', 1475 ).
file('__setgid.c',text,'glibc-1.06/sysdeps/stub', 1276 ).
file('__setitmr.c',text,'glibc-1.06/sysdeps/stub', 1391 ).
file('__setpgrp.c',text,'glibc-1.06/sysdeps/stub', 1172 ).
file('__setregid.c',text,'glibc-1.06/sysdeps/stub', 1268 ).
file('__setreuid.c',text,'glibc-1.06/sysdeps/stub', 1266 ).
file('__setsid.c',text,'glibc-1.06/sysdeps/stub', 1249 ).
file('__settod.c',text,'glibc-1.06/sysdeps/stub', 1232 ).
file('__setuid.c',text,'glibc-1.06/sysdeps/stub', 1272 ).
file('__sigact.c',text,'glibc-1.06/sysdeps/stub', 1346 ).
file('__sigblock.c',text,'glibc-1.06/sysdeps/stub', 1115 ).
file('__sigpause.c',text,'glibc-1.06/sysdeps/stub', 1061 ).
file('__sigproc.c',text,'glibc-1.06/sysdeps/stub', 1543 ).
file('__sigret.c',text,'glibc-1.06/sysdeps/stub', 990 ).
file('__sigstmsk.c',text,'glibc-1.06/sysdeps/stub', 1065 ).
file('__sigvec.c',text,'glibc-1.06/sysdeps/stub', 1484 ).
file('__stat.c',text,'glibc-1.06/sysdeps/stub', 1243 ).
file('__symlink.c',text,'glibc-1.06/sysdeps/stub', 1235 ).
file('__sysconf.c',text,'glibc-1.06/sysdeps/stub', 1632 ).
file('__tcgetatr.c',text,'glibc-1.06/sysdeps/stub', 1315 ).
file('__times.c',text,'glibc-1.06/sysdeps/stub', 1407 ).
file('__umask.c',text,'glibc-1.06/sysdeps/stub', 1153 ).
file('__unlink.c',text,'glibc-1.06/sysdeps/stub', 1195 ).
file('__utimes.c',text,'glibc-1.06/sysdeps/stub', 1210 ).
file('__wait.c',text,'glibc-1.06/sysdeps/stub', 1215 ).
file('__wait3.c',text,'glibc-1.06/sysdeps/stub', 1607 ).
file('__wait4.c',text,'glibc-1.06/sysdeps/stub', 1182 ).
file('__waitpid.c',text,'glibc-1.06/sysdeps/stub', 1921 ).
file('__write.c',text,'glibc-1.06/sysdeps/stub', 1387 ).
file('_exit.c',text,'glibc-1.06/sysdeps/stub', 1219 ).
file('accept.c',text,'glibc-1.06/sysdeps/stub', 1427 ).
file('acct.c',text,'glibc-1.06/sysdeps/stub', 1287 ).
file('acos.c',text,'glibc-1.06/sysdeps/stub', 1087 ).
file('alarm.c',text,'glibc-1.06/sysdeps/stub', 1572 ).
file('asin.c',text,'glibc-1.06/sysdeps/stub', 1085 ).
file('atan2.c',text,'glibc-1.06/sysdeps/stub', 1089 ).
file('bind.c',text,'glibc-1.06/sysdeps/stub', 1183 ).
file('bytesex.h',text,'glibc-1.06/sysdeps/stub', 426 ).
file('cbrt.c',text,'glibc-1.06/sysdeps/stub', 1082 ).
file('chflags.c',text,'glibc-1.06/sysdeps/stub', 1220 ).
file('chroot.c',text,'glibc-1.06/sysdeps/stub', 1187 ).
file('clock.c',text,'glibc-1.06/sysdeps/stub', 1056 ).
file('closedir.c',text,'glibc-1.06/sysdeps/stub', 1158 ).
file('confstr.h',text,'glibc-1.06/sysdeps/stub', 146 ).
file('connect.c',text,'glibc-1.06/sysdeps/stub', 1366 ).
file('cos.c',text,'glibc-1.06/sysdeps/stub', 1078 ).
file('cosh.c',text,'glibc-1.06/sysdeps/stub', 1091 ).
file('ctermid.c',text,'glibc-1.06/sysdeps/stub', 1240 ).
file('cuserid.c',text,'glibc-1.06/sysdeps/stub', 1238 ).
file('defs.c',text,'glibc-1.06/sysdeps/stub', 1724 ).
file('direct.h',text,'glibc-1.06/sysdeps/stub', 177 ).
file('errlist.c',text,'glibc-1.06/sysdeps/stub', 1100 ).
file('errnos.h',text,'glibc-1.06/sysdeps/stub', 1130 ).
file('exp.c',text,'glibc-1.06/sysdeps/stub', 1072 ).
file('fchdir.c',text,'glibc-1.06/sysdeps/stub', 1112 ).
file('fchflags.c',text,'glibc-1.06/sysdeps/stub', 1221 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/stub', 3315 ).
file('fdopen.c',text,'glibc-1.06/sysdeps/stub', 1045 ).
file('fmod.c',text,'glibc-1.06/sysdeps/stub', 1123 ).
file('frexp.c',text,'glibc-1.06/sysdeps/stub', 1162 ).
file('fsync.c',text,'glibc-1.06/sysdeps/stub', 1107 ).
file('ftruncate.c',text,'glibc-1.06/sysdeps/stub', 1046 ).
file('getcwd.c',text,'glibc-1.06/sysdeps/stub', 1471 ).
file('getenv.c',text,'glibc-1.06/sysdeps/stub', 1125 ).
file('gethostid.c',text,'glibc-1.06/sysdeps/stub', 1105 ).
file('getlogin.c',text,'glibc-1.06/sysdeps/stub', 1218 ).
file('getpeernam.c',text,'glibc-1.06/sysdeps/stub', 1255 ).
file('getpgrp.c',text,'glibc-1.06/sysdeps/stub', 1087 ).
file('getprio.c',text,'glibc-1.06/sysdeps/stub', 1406 ).
file('getrlimit.c',text,'glibc-1.06/sysdeps/stub', 1252 ).
file('getsocknam.c',text,'glibc-1.06/sysdeps/stub', 1192 ).
file('getsockopt.c',text,'glibc-1.06/sysdeps/stub', 1386 ).
file('gtty.c',text,'glibc-1.06/sysdeps/stub', 1159 ).
file('huge_val.h',text,'glibc-1.06/sysdeps/stub', 1013 ).
file('init-posix.c',text,'glibc-1.06/sysdeps/stub', 1022 ).
file('ioctls.h',text,'glibc-1.06/sysdeps/stub', 44 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/stub', 94 ).
file('killpg.c',text,'glibc-1.06/sysdeps/stub', 1121 ).
file('libc_fatal.c',text,'glibc-1.06/sysdeps/stub', 1123 ).
file('listen.c',text,'glibc-1.06/sysdeps/stub', 1249 ).
file('local_lim.h',text,'glibc-1.06/sysdeps/stub', 169 ).
file('log.c',text,'glibc-1.06/sysdeps/stub', 1089 ).
file('log10.c',text,'glibc-1.06/sysdeps/stub', 1093 ).
file('mkfifo.c',text,'glibc-1.06/sysdeps/stub', 1269 ).
file('mkstemp.c',text,'glibc-1.06/sysdeps/stub', 1333 ).
file('mktemp.c',text,'glibc-1.06/sysdeps/stub', 1272 ).
file('morecore.c',text,'glibc-1.06/sysdeps/stub', 1309 ).
file('nan.h',text,'glibc-1.06/sysdeps/stub', 125 ).
file('nice.c',text,'glibc-1.06/sysdeps/stub', 1191 ).
file('nlist.c',text,'glibc-1.06/sysdeps/stub', 1353 ).
file('opendir.c',text,'glibc-1.06/sysdeps/stub', 1129 ).
file('pause.c',text,'glibc-1.06/sysdeps/stub', 1194 ).
file('pipestream.c',text,'glibc-1.06/sysdeps/stub', 1690 ).
file('posix_opt.h',text,'glibc-1.06/sysdeps/stub', 118 ).
file('ptrace.c',text,'glibc-1.06/sysdeps/stub', 2748 ).
file('putenv.c',text,'glibc-1.06/sysdeps/stub', 1196 ).
file('raise.c',text,'glibc-1.06/sysdeps/stub', 984 ).
file('readdir.c',text,'glibc-1.06/sysdeps/stub', 1132 ).
file('readv.c',text,'glibc-1.06/sysdeps/stub', 1353 ).
file('reboot.c',text,'glibc-1.06/sysdeps/stub', 1081 ).
file('recv.c',text,'glibc-1.06/sysdeps/stub', 1199 ).
file('recvfrom.c',text,'glibc-1.06/sysdeps/stub', 1339 ).
file('recvmsg.c',text,'glibc-1.06/sysdeps/stub', 1237 ).
file('rename.c',text,'glibc-1.06/sysdeps/stub', 1207 ).
file('rewinddir.c',text,'glibc-1.06/sysdeps/stub', 1159 ).
file('seekdir.c',text,'glibc-1.06/sysdeps/stub', 1212 ).
file('send.c',text,'glibc-1.06/sysdeps/stub', 1182 ).
file('sendmsg.c',text,'glibc-1.06/sysdeps/stub', 1233 ).
file('sendto.c',text,'glibc-1.06/sysdeps/stub', 1324 ).
file('setegid.c',text,'glibc-1.06/sysdeps/stub', 1118 ).
file('setenv.c',text,'glibc-1.06/sysdeps/stub', 1026 ).
file('seteuid.c',text,'glibc-1.06/sysdeps/stub', 1121 ).
file('setgroups.c',text,'glibc-1.06/sysdeps/stub', 1177 ).
file('sethostid.c',text,'glibc-1.06/sysdeps/stub', 1162 ).
file('sethostnam.c',text,'glibc-1.06/sysdeps/stub', 1217 ).
file('setjmp.c',text,'glibc-1.06/sysdeps/stub', 1156 ).
file('setlogin.c',text,'glibc-1.06/sysdeps/stub', 1018 ).
file('setprio.c',text,'glibc-1.06/sysdeps/stub', 1248 ).
file('setrlimit.c',text,'glibc-1.06/sysdeps/stub', 1300 ).
file('setsockopt.c',text,'glibc-1.06/sysdeps/stub', 1314 ).
file('shutdown.c',text,'glibc-1.06/sysdeps/stub', 1330 ).
file('sigaltstack.c',text,'glibc-1.06/sysdeps/stub', 1165 ).
file('sigcontext.h',text,'glibc-1.06/sysdeps/stub', 1029 ).
file('sigintr.c',text,'glibc-1.06/sysdeps/stub', 1180 ).
file('siglist.c',text,'glibc-1.06/sysdeps/stub', 1294 ).
file('signal.c',text,'glibc-1.06/sysdeps/stub', 1209 ).
file('signum.h',text,'glibc-1.06/sysdeps/stub', 2072 ).
file('sigpending.c',text,'glibc-1.06/sysdeps/stub', 1220 ).
file('sigstack.c',text,'glibc-1.06/sysdeps/stub', 1156 ).
file('sigsuspend.c',text,'glibc-1.06/sysdeps/stub', 1193 ).
file('sin.c',text,'glibc-1.06/sysdeps/stub', 1076 ).
file('sinh.c',text,'glibc-1.06/sysdeps/stub', 1089 ).
file('sleep.c',text,'glibc-1.06/sysdeps/stub', 1538 ).
file('socket.c',text,'glibc-1.06/sysdeps/stub', 1332 ).
file('socketpair.c',text,'glibc-1.06/sysdeps/stub', 1440 ).
file('sqrt.c',text,'glibc-1.06/sysdeps/stub', 1085 ).
file('sstk.c',text,'glibc-1.06/sysdeps/stub', 1058 ).
file('start.c',text,'glibc-1.06/sysdeps/stub', 329 ).
file('startsig.c',text,'glibc-1.06/sysdeps/stub', 1049 ).
file('statbuf.h',text,'glibc-1.06/sysdeps/stub', 2512 ).
file('stdio_init.c',text,'glibc-1.06/sysdeps/stub', 1156 ).
file('stdio_lim.h',text,'glibc-1.06/sysdeps/stub', 122 ).
file('stime.c',text,'glibc-1.06/sysdeps/stub', 1096 ).
file('stty.c',text,'glibc-1.06/sysdeps/stub', 1163 ).
file('swapon.c',text,'glibc-1.06/sysdeps/stub', 1187 ).
file('sync.c',text,'glibc-1.06/sysdeps/stub', 1103 ).
file('syscall.h',text,'glibc-1.06/sysdeps/stub', 136 ).
file('sysd-stdio.c',text,'glibc-1.06/sysdeps/stub', 2460 ).
file('sysdep.c',text,'glibc-1.06/sysdeps/stub', 114 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/stub', 200 ).
file('system.c',text,'glibc-1.06/sysdeps/stub', 1189 ).
file('tan.c',text,'glibc-1.06/sysdeps/stub', 1079 ).
file('tanh.c',text,'glibc-1.06/sysdeps/stub', 1092 ).
file('tcdrain.c',text,'glibc-1.06/sysdeps/stub', 1169 ).
file('tcflow.c',text,'glibc-1.06/sysdeps/stub', 1347 ).
file('tcflush.c',text,'glibc-1.06/sysdeps/stub', 1287 ).
file('tcgetpgrp.c',text,'glibc-1.06/sysdeps/stub', 1214 ).
file('tcsendbrk.c',text,'glibc-1.06/sysdeps/stub', 1181 ).
file('tcsetattr.c',text,'glibc-1.06/sysdeps/stub', 2151 ).
file('tcsetpgrp.c',text,'glibc-1.06/sysdeps/stub', 1238 ).
file('telldir.c',text,'glibc-1.06/sysdeps/stub', 1227 ).
file('tempname.c',text,'glibc-1.06/sysdeps/stub', 1531 ).
file('time.c',text,'glibc-1.06/sysdeps/stub', 1179 ).
file('truncate.c',text,'glibc-1.06/sysdeps/stub', 1040 ).
file('ttyname.c',text,'glibc-1.06/sysdeps/stub', 1254 ).
file('ualarm.c',text,'glibc-1.06/sysdeps/stub', 1274 ).
file('ulimit.c',text,'glibc-1.06/sysdeps/stub', 1481 ).
file('usleep.c',text,'glibc-1.06/sysdeps/stub', 1042 ).
file('utime.c',text,'glibc-1.06/sysdeps/stub', 1302 ).
file('vhangup.c',text,'glibc-1.06/sysdeps/stub', 1225 ).
file('waitflags.h',text,'glibc-1.06/sysdeps/stub', 1100 ).
file('writev.c',text,'glibc-1.06/sysdeps/stub', 1348 ).
file('Implies',text,'glibc-1.06/sysdeps/tahoe', 41 ).
file('Makefile',text,'glibc-1.06/sysdeps/tahoe', 271 ).
file('log10.c',text,'glibc-1.06/sysdeps/tahoe', 928 ).
file('Dist',text,'glibc-1.06/sysdeps/unix', 93 ).
file('Implies',text,'glibc-1.06/sysdeps/unix', 6 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix', 8234 ).
file('__chdir.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__chmod.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__chown.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__close.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__dup.S',text,'glibc-1.06/sysdeps/unix', 876 ).
file('__execve.S',text,'glibc-1.06/sysdeps/unix', 1121 ).
file('__fcntl.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__fork.S',text,'glibc-1.06/sysdeps/unix', 1031 ).
file('__fstat.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__getdents.c',text,'glibc-1.06/sysdeps/unix', 1147 ).
file('__getegid.S',text,'glibc-1.06/sysdeps/unix', 956 ).
file('__geteuid.S',text,'glibc-1.06/sysdeps/unix', 956 ).
file('__getgid.S',text,'glibc-1.06/sysdeps/unix', 879 ).
file('__getpgsz.c',text,'glibc-1.06/sysdeps/unix', 1176 ).
file('__getpid.S',text,'glibc-1.06/sysdeps/unix', 879 ).
file('__getppid.S',text,'glibc-1.06/sysdeps/unix', 956 ).
file('__getuid.S',text,'glibc-1.06/sysdeps/unix', 879 ).
file('__ioctl.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__kill.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('__link.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('__lseek.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__mknod.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__open.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('__read.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('__sbrk.c',text,'glibc-1.06/sysdeps/unix', 1265 ).
file('__setgid.S',text,'glibc-1.06/sysdeps/unix', 879 ).
file('__setuid.S',text,'glibc-1.06/sysdeps/unix', 879 ).
file('__stat.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('__umask.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('__unlink.S',text,'glibc-1.06/sysdeps/unix', 879 ).
file('__write.S',text,'glibc-1.06/sysdeps/unix', 878 ).
file('_exit.S',text,'glibc-1.06/sysdeps/unix', 904 ).
file('acct.S',text,'glibc-1.06/sysdeps/unix', 875 ).
file('bsd',dir,'glibc-1.06/sysdeps/unix', 1536 ).
file('chroot.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('closedir.c',text,'glibc-1.06/sysdeps/unix', 1252 ).
file('common',dir,'glibc-1.06/sysdeps/unix', 1024 ).
file('configure',text,'glibc-1.06/sysdeps/unix', 3547 ).
file('confstr.h',text,'glibc-1.06/sysdeps/unix', 32 ).
file('errnos-tmpl.c',text,'glibc-1.06/sysdeps/unix', 2341 ).
file('errnos.awk',text,'glibc-1.06/sysdeps/unix', 367 ).
file('fsync.S',text,'glibc-1.06/sysdeps/unix', 876 ).
file('getlogin.c',text,'glibc-1.06/sysdeps/unix', 2312 ).
file('i386',dir,'glibc-1.06/sysdeps/unix', 512 ).
file('inet',dir,'glibc-1.06/sysdeps/unix', 512 ).
file('ioctls-tmpl.c',text,'glibc-1.06/sysdeps/unix', 2013 ).
file('ioctls.awk',text,'glibc-1.06/sysdeps/unix', 265 ).
file('make_errlist.c',text,'glibc-1.06/sysdeps/unix', 1424 ).
file('mk-local_lim.c',text,'glibc-1.06/sysdeps/unix', 2606 ).
file('mkfifo.c',text,'glibc-1.06/sysdeps/unix', 1065 ).
file('morecore.c',text,'glibc-1.06/sysdeps/unix', 1330 ).
file('nlist.c',text,'glibc-1.06/sysdeps/unix', 2467 ).
file('opendir.c',text,'glibc-1.06/sysdeps/unix', 2179 ).
file('pipestream.c',text,'glibc-1.06/sysdeps/unix', 57 ).
file('ptrace.S',text,'glibc-1.06/sysdeps/unix', 877 ).
file('readdir.c',text,'glibc-1.06/sysdeps/unix', 2160 ).
file('rewinddir.c',text,'glibc-1.06/sysdeps/unix', 1122 ).
file('seekdir.c',text,'glibc-1.06/sysdeps/unix', 1155 ).
file('siglist.c',text,'glibc-1.06/sysdeps/unix', 859 ).
file('snarf-ioctls',exec,'glibc-1.06/sysdeps/unix', 1546 ).
file('sparc',dir,'glibc-1.06/sysdeps/unix', 512 ).
file('start.c',text,'glibc-1.06/sysdeps/unix', 2704 ).
file('sync.S',text,'glibc-1.06/sysdeps/unix', 875 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix', 2071 ).
file('system.c',text,'glibc-1.06/sysdeps/unix', 53 ).
file('sysv',dir,'glibc-1.06/sysdeps/unix', 1024 ).
file('telldir.c',text,'glibc-1.06/sysdeps/unix', 1287 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/bsd', 10 ).
file('Implies',text,'glibc-1.06/sysdeps/unix/bsd', 97 ).
file('__flock.S',text,'glibc-1.06/sysdeps/unix/bsd', 872 ).
file('__getdtsz.S',text,'glibc-1.06/sysdeps/unix/bsd', 886 ).
file('__getpgsz.S',text,'glibc-1.06/sysdeps/unix/bsd', 884 ).
file('__isatty.c',text,'glibc-1.06/sysdeps/unix/bsd', 1141 ).
file('__setgid.c',text,'glibc-1.06/sysdeps/unix/bsd', 1231 ).
file('__setregid.S',text,'glibc-1.06/sysdeps/unix/bsd', 881 ).
file('__setreuid.S',text,'glibc-1.06/sysdeps/unix/bsd', 881 ).
file('__setsid.c',text,'glibc-1.06/sysdeps/unix/bsd', 1586 ).
file('__settod.S',text,'glibc-1.06/sysdeps/unix/bsd', 885 ).
file('__setuid.c',text,'glibc-1.06/sysdeps/unix/bsd', 1227 ).
file('__sigact.c',text,'glibc-1.06/sysdeps/unix/bsd', 1891 ).
file('__sigblock.S',text,'glibc-1.06/sysdeps/unix/bsd', 881 ).
file('__sigpause.S',text,'glibc-1.06/sysdeps/unix/bsd', 881 ).
file('__sigproc.c',text,'glibc-1.06/sysdeps/unix/bsd', 1642 ).
file('__sigstmsk.S',text,'glibc-1.06/sysdeps/unix/bsd', 883 ).
file('__sigvec.S',text,'glibc-1.06/sysdeps/unix/bsd', 879 ).
file('__tcgetatr.c',text,'glibc-1.06/sysdeps/unix/bsd', 3544 ).
file('__times.c',text,'glibc-1.06/sysdeps/unix/bsd', 2105 ).
file('__utimes.S',text,'glibc-1.06/sysdeps/unix/bsd', 879 ).
file('alarm.c',text,'glibc-1.06/sysdeps/unix/bsd', 2061 ).
file('bsd4.4',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('bsdtty.h',text,'glibc-1.06/sysdeps/unix/bsd', 2907 ).
file('clock.c',text,'glibc-1.06/sysdeps/unix/bsd', 1415 ).
file('confstr.h',text,'glibc-1.06/sysdeps/unix/bsd', 41 ).
file('direct.h',text,'glibc-1.06/sysdeps/unix/bsd', 238 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/unix/bsd', 4292 ).
file('getpgrp.c',text,'glibc-1.06/sysdeps/unix/bsd', 1012 ).
file('getprio.S',text,'glibc-1.06/sysdeps/unix/bsd', 882 ).
file('gtty.c',text,'glibc-1.06/sysdeps/unix/bsd', 1072 ).
file('hp',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('i386',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('init-posix.c',text,'glibc-1.06/sysdeps/unix/bsd', 1093 ).
file('killpg.S',text,'glibc-1.06/sysdeps/unix/bsd', 877 ).
file('m68k',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('nice.c',text,'glibc-1.06/sysdeps/unix/bsd', 1402 ).
file('pause.c',text,'glibc-1.06/sysdeps/unix/bsd', 1054 ).
file('posix_opt.h',text,'glibc-1.06/sysdeps/unix/bsd', 161 ).
file('sequent',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('setegid.c',text,'glibc-1.06/sysdeps/unix/bsd', 963 ).
file('seteuid.c',text,'glibc-1.06/sysdeps/unix/bsd', 963 ).
file('setprio.S',text,'glibc-1.06/sysdeps/unix/bsd', 882 ).
file('signum.h',text,'glibc-1.06/sysdeps/unix/bsd', 3047 ).
file('sigstack.S',text,'glibc-1.06/sysdeps/unix/bsd', 879 ).
file('sigsuspend.c',text,'glibc-1.06/sysdeps/unix/bsd', 1335 ).
file('sony',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('statbuf.h',text,'glibc-1.06/sysdeps/unix/bsd', 3048 ).
file('stime.c',text,'glibc-1.06/sysdeps/unix/bsd', 1164 ).
file('stty.c',text,'glibc-1.06/sysdeps/unix/bsd', 1076 ).
file('sun',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('tahoe',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('tcdrain.c',text,'glibc-1.06/sysdeps/unix/bsd', 1318 ).
file('tcflow.c',text,'glibc-1.06/sysdeps/unix/bsd', 1641 ).
file('tcflush.c',text,'glibc-1.06/sysdeps/unix/bsd', 1398 ).
file('tcgetpgrp.c',text,'glibc-1.06/sysdeps/unix/bsd', 1126 ).
file('tcsendbrk.c',text,'glibc-1.06/sysdeps/unix/bsd', 1757 ).
file('tcsetattr.c',text,'glibc-1.06/sysdeps/unix/bsd', 4373 ).
file('tcsetpgrp.c',text,'glibc-1.06/sysdeps/unix/bsd', 1104 ).
file('time.c',text,'glibc-1.06/sysdeps/unix/bsd', 1291 ).
file('ualarm.c',text,'glibc-1.06/sysdeps/unix/bsd', 1552 ).
file('ulimit.c',text,'glibc-1.06/sysdeps/unix/bsd', 2268 ).
file('ultrix4',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('usleep.c',text,'glibc-1.06/sysdeps/unix/bsd', 1231 ).
file('utime.c',text,'glibc-1.06/sysdeps/unix/bsd', 1539 ).
file('vax',dir,'glibc-1.06/sysdeps/unix/bsd', 512 ).
file('waitflags.h',text,'glibc-1.06/sysdeps/unix/bsd', 1100 ).
file('__getdents.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 45 ).
file('__sigblock.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 38 ).
file('__sigstmsk.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 38 ).
file('__sigvec.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 36 ).
file('__tcgetatr.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 1113 ).
file('__wait.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 1201 ).
file('__wait3.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 1447 ).
file('__wait4.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 872 ).
file('__waitpid.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 1761 ).
file('chflags.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 878 ).
file('errnos.h',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 6753 ).
file('fchdir.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 871 ).
file('fchflags.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 879 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 4248 ).
file('ioctls.h',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 13651 ).
file('setegid.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 878 ).
file('seteuid.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 878 ).
file('setlogin.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 879 ).
file('sigaltstack.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 882 ).
file('sstk.S',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 897 ).
file('tcdrain.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 1080 ).
file('tcsetattr.c',text,'glibc-1.06/sysdeps/unix/bsd/bsd4.4', 1534 ).
file('m68k',dir,'glibc-1.06/sysdeps/unix/bsd/hp', 512 ).
file('__brk.S',text,'glibc-1.06/sysdeps/unix/bsd/hp/m68k', 1183 ).
file('__getdents.S',text,'glibc-1.06/sysdeps/unix/bsd/hp/m68k', 45 ).
file('__vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/hp/m68k', 1676 ).
file('__wait3.S',text,'glibc-1.06/sysdeps/unix/bsd/hp/m68k', 1138 ).
file('start.c',text,'glibc-1.06/sysdeps/unix/bsd/hp/m68k', 222 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/hp/m68k', 1857 ).
file('__vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/i386', 1335 ).
file('__wait3.S',text,'glibc-1.06/sysdeps/unix/bsd/i386', 1660 ).
file('__pipe.S',text,'glibc-1.06/sysdeps/unix/bsd/m68k', 924 ).
file('__wait.S',text,'glibc-1.06/sysdeps/unix/bsd/m68k', 934 ).
file('start.c',text,'glibc-1.06/sysdeps/unix/bsd/m68k', 57 ).
file('sysdep.S',text,'glibc-1.06/sysdeps/unix/bsd/m68k', 905 ).
file('i386',dir,'glibc-1.06/sysdeps/unix/bsd/sequent', 512 ).
file('__sigvec.S',text,'glibc-1.06/sysdeps/unix/bsd/sequent/i386', 1708 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/sequent/i386', 2605 ).
file('m68k',dir,'glibc-1.06/sysdeps/unix/bsd/sony', 512 ).
file('newsos4',dir,'glibc-1.06/sysdeps/unix/bsd/sony', 512 ).
file('Implies',text,'glibc-1.06/sysdeps/unix/bsd/sony/m68k', 61 ).
file('m68k',dir,'glibc-1.06/sysdeps/unix/bsd/sony/newsos4', 512 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/sony/newsos4/m68k', 2030 ).
file('__getdents.S',text,'glibc-1.06/sysdeps/unix/bsd/sun', 886 ).
file('__sigret.S',text,'glibc-1.06/sysdeps/unix/bsd/sun', 952 ).
file('__sigvec.S',text,'glibc-1.06/sysdeps/unix/bsd/sun', 933 ).
file('m68k',dir,'glibc-1.06/sysdeps/unix/bsd/sun', 512 ).
file('sethostid.c',text,'glibc-1.06/sysdeps/unix/bsd/sun', 36 ).
file('sparc',dir,'glibc-1.06/sysdeps/unix/bsd/sun', 512 ).
file('sunos3',dir,'glibc-1.06/sysdeps/unix/bsd/sun', 512 ).
file('sunos4',dir,'glibc-1.06/sysdeps/unix/bsd/sun', 512 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 11 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 77 ).
file('__brk.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 1169 ).
file('__vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 1677 ).
file('sethostid.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 1153 ).
file('sigcontext.h',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 998 ).
file('sigtramp.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 3578 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/sun/m68k', 1861 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/bsd/sun/sparc', 11 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/bsd/sun/sparc', 266 ).
file('sethostid.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/sparc', 1253 ).
file('sigcontext.h',text,'glibc-1.06/sysdeps/unix/bsd/sun/sparc', 1292 ).
file('sigtramp.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sparc', 6528 ).
file('vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/sparc', 946 ).
file('m68k',dir,'glibc-1.06/sysdeps/unix/bsd/sun/sunos3', 512 ).
file('__wait.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos3/m68k', 45 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 12 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 78 ).
file('__tcgetatr.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 1105 ).
file('__wait.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 44 ).
file('__wait3.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 45 ).
file('__wait4.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 1579 ).
file('__waitpid.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 47 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 5466 ).
file('speed.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 2729 ).
file('sys_wait4.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 892 ).
file('tcflow.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 1055 ).
file('tcflush.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 1067 ).
file('tcsendbrk.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 1167 ).
file('tcsetattr.c',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 1411 ).
file('termbits.h',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 7364 ).
file('uname.S',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 37 ).
file('utsnamelen.h',text,'glibc-1.06/sysdeps/unix/bsd/sun/sunos4', 26 ).
file('Implies',text,'glibc-1.06/sysdeps/unix/bsd/tahoe', 13 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/tahoe', 152 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 13 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 78 ).
file('__getdents.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 45 ).
file('__sysconf.c',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 1853 ).
file('__wait.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 1039 ).
file('__wait3.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 890 ).
file('__waitpid.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 892 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 4462 ).
file('getsysinfo.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 979 ).
file('mips',dir,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 512 ).
file('posix_opt.h',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 999 ).
file('uname.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 37 ).
file('utsnamelen.h',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4', 27 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 23 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 87 ).
file('__brk.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 1476 ).
file('__fork.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 1004 ).
file('__handler.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 3228 ).
file('__pipe.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 1012 ).
file('__sigret.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 904 ).
file('__sigvec.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 933 ).
file('__vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 1036 ).
file('sigcontext.h',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 1895 ).
file('sigtramp.c',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 2033 ).
file('start.c',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 1807 ).
file('sysdep.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 998 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 2108 ).
file('vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/ultrix4/mips', 960 ).
file('__brk.S',text,'glibc-1.06/sysdeps/unix/bsd/vax', 1135 ).
file('__pipe.S',text,'glibc-1.06/sysdeps/unix/bsd/vax', 933 ).
file('__vfork.S',text,'glibc-1.06/sysdeps/unix/bsd/vax', 1620 ).
file('__wait.S',text,'glibc-1.06/sysdeps/unix/bsd/vax', 918 ).
file('__wait3.S',text,'glibc-1.06/sysdeps/unix/bsd/vax', 1102 ).
file('sysdep.S',text,'glibc-1.06/sysdeps/unix/bsd/vax', 917 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/bsd/vax', 1640 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/common', 41 ).
file('Implies',text,'glibc-1.06/sysdeps/unix/common', 145 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/common', 1582 ).
file('__access.S',text,'glibc-1.06/sysdeps/unix/common', 879 ).
file('__adjtime.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('__dup2.S',text,'glibc-1.06/sysdeps/unix/common', 877 ).
file('__fchmod.S',text,'glibc-1.06/sysdeps/unix/common', 879 ).
file('__fchown.S',text,'glibc-1.06/sysdeps/unix/common', 879 ).
file('__getgrps.c',text,'glibc-1.06/sysdeps/unix/common', 1941 ).
file('__getitmr.S',text,'glibc-1.06/sysdeps/unix/common', 882 ).
file('__getpgrp.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('__getrusag.S',text,'glibc-1.06/sysdeps/unix/common', 882 ).
file('__gettod.S',text,'glibc-1.06/sysdeps/unix/common', 885 ).
file('__lstat.S',text,'glibc-1.06/sysdeps/unix/common', 878 ).
file('__mkdir.S',text,'glibc-1.06/sysdeps/unix/common', 878 ).
file('__readlink.S',text,'glibc-1.06/sysdeps/unix/common', 881 ).
file('__rmdir.S',text,'glibc-1.06/sysdeps/unix/common', 878 ).
file('__select.S',text,'glibc-1.06/sysdeps/unix/common', 879 ).
file('__setitmr.S',text,'glibc-1.06/sysdeps/unix/common', 882 ).
file('__setpgrp.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('__symlink.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('bsd_getgrp.S',text,'glibc-1.06/sysdeps/unix/common', 896 ).
file('configure',text,'glibc-1.06/sysdeps/unix/common', 1148 ).
file('configure.in',text,'glibc-1.06/sysdeps/unix/common', 353 ).
file('ftruncate.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('getrlimit.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('glue-ctype.c',text,'glibc-1.06/sysdeps/unix/common', 1923 ).
file('make_siglist.c',text,'glibc-1.06/sysdeps/unix/common', 1587 ).
file('readv.S',text,'glibc-1.06/sysdeps/unix/common', 876 ).
file('rename.S',text,'glibc-1.06/sysdeps/unix/common', 877 ).
file('setgroups.S',text,'glibc-1.06/sysdeps/unix/common', 874 ).
file('setrlimit.S',text,'glibc-1.06/sysdeps/unix/common', 880 ).
file('swapon.S',text,'glibc-1.06/sysdeps/unix/common', 877 ).
file('truncate.S',text,'glibc-1.06/sysdeps/unix/common', 879 ).
file('vhangup.S',text,'glibc-1.06/sysdeps/unix/common', 878 ).
file('writev.S',text,'glibc-1.06/sysdeps/unix/common', 877 ).
file('__brk.S',text,'glibc-1.06/sysdeps/unix/i386', 1145 ).
file('__fork.S',text,'glibc-1.06/sysdeps/unix/i386', 992 ).
file('__pipe.S',text,'glibc-1.06/sysdeps/unix/i386', 966 ).
file('__sigret.S',text,'glibc-1.06/sysdeps/unix/i386', 1007 ).
file('__wait.S',text,'glibc-1.06/sysdeps/unix/i386', 1069 ).
file('start.c',text,'glibc-1.06/sysdeps/unix/i386', 55 ).
file('sysdep.S',text,'glibc-1.06/sysdeps/unix/i386', 1050 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/i386', 1981 ).
file('Subdirs',text,'glibc-1.06/sysdeps/unix/inet', 5 ).
file('__gethstnm.S',text,'glibc-1.06/sysdeps/unix/inet', 884 ).
file('accept.S',text,'glibc-1.06/sysdeps/unix/inet', 877 ).
file('bind.S',text,'glibc-1.06/sysdeps/unix/inet', 875 ).
file('connect.S',text,'glibc-1.06/sysdeps/unix/inet', 878 ).
file('gethostid.S',text,'glibc-1.06/sysdeps/unix/inet', 880 ).
file('getpeernam.S',text,'glibc-1.06/sysdeps/unix/inet', 882 ).
file('getsocknam.S',text,'glibc-1.06/sysdeps/unix/inet', 882 ).
file('getsockopt.S',text,'glibc-1.06/sysdeps/unix/inet', 881 ).
file('listen.S',text,'glibc-1.06/sysdeps/unix/inet', 877 ).
file('recv.S',text,'glibc-1.06/sysdeps/unix/inet', 875 ).
file('recvfrom.S',text,'glibc-1.06/sysdeps/unix/inet', 879 ).
file('recvmsg.S',text,'glibc-1.06/sysdeps/unix/inet', 878 ).
file('send.S',text,'glibc-1.06/sysdeps/unix/inet', 875 ).
file('sendmsg.S',text,'glibc-1.06/sysdeps/unix/inet', 878 ).
file('sendto.S',text,'glibc-1.06/sysdeps/unix/inet', 877 ).
file('sethostid.S',text,'glibc-1.06/sysdeps/unix/inet', 880 ).
file('sethostnam.S',text,'glibc-1.06/sysdeps/unix/inet', 882 ).
file('setsockopt.S',text,'glibc-1.06/sysdeps/unix/inet', 881 ).
file('shutdown.S',text,'glibc-1.06/sysdeps/unix/inet', 879 ).
file('socket.S',text,'glibc-1.06/sysdeps/unix/inet', 877 ).
file('socketpair.S',text,'glibc-1.06/sysdeps/unix/inet', 881 ).
file('__brk.S',text,'glibc-1.06/sysdeps/unix/sparc', 1468 ).
file('__fork.S',text,'glibc-1.06/sysdeps/unix/sparc', 1005 ).
file('__pipe.S',text,'glibc-1.06/sysdeps/unix/sparc', 1044 ).
file('__vfork.S',text,'glibc-1.06/sysdeps/unix/sparc', 1055 ).
file('start.c',text,'glibc-1.06/sysdeps/unix/sparc', 4866 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/sparc', 1388 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/sysv', 36 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/sysv', 1593 ).
file('__getdents.c',text,'glibc-1.06/sysdeps/unix/sysv', 1216 ).
file('__gethstnm.c',text,'glibc-1.06/sysdeps/unix/sysv', 1265 ).
file('__mkdir.c',text,'glibc-1.06/sysdeps/unix/sysv', 2327 ).
file('__rmdir.c',text,'glibc-1.06/sysdeps/unix/sysv', 1917 ).
file('__settod.c',text,'glibc-1.06/sysdeps/unix/sysv', 1355 ).
file('__sigact.c',text,'glibc-1.06/sysdeps/unix/sysv', 1907 ).
file('__tcgetatr.c',text,'glibc-1.06/sysdeps/unix/sysv', 5451 ).
file('__times.S',text,'glibc-1.06/sysdeps/unix/sysv', 878 ).
file('alarm.S',text,'glibc-1.06/sysdeps/unix/sysv', 876 ).
file('direct.h',text,'glibc-1.06/sysdeps/unix/sysv', 156 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/unix/sysv', 3276 ).
file('i386',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('isc2.2',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('linux',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('local_lim.h',text,'glibc-1.06/sysdeps/unix/sysv', 1254 ).
file('minix',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('nice.S',text,'glibc-1.06/sysdeps/unix/sysv', 869 ).
file('pause.S',text,'glibc-1.06/sysdeps/unix/sysv', 876 ).
file('sco3.2',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('sco3.2.4',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('setrlimit.c',text,'glibc-1.06/sysdeps/unix/sysv', 1715 ).
file('signal.S',text,'glibc-1.06/sysdeps/unix/sysv', 877 ).
file('signum.h',text,'glibc-1.06/sysdeps/unix/sysv', 2352 ).
file('statbuf.h',text,'glibc-1.06/sysdeps/unix/sysv', 2060 ).
file('stime.S',text,'glibc-1.06/sysdeps/unix/sysv', 870 ).
file('sys_getdents.S',text,'glibc-1.06/sysdeps/unix/sysv', 875 ).
file('sysd-stdio.c',text,'glibc-1.06/sysdeps/unix/sysv', 62 ).
file('sysv4',dir,'glibc-1.06/sysdeps/unix/sysv', 512 ).
file('sysv_termio.h',text,'glibc-1.06/sysdeps/unix/sysv', 4187 ).
file('tcdrain.c',text,'glibc-1.06/sysdeps/unix/sysv', 1171 ).
file('tcflow.c',text,'glibc-1.06/sysdeps/unix/sysv', 1386 ).
file('tcflush.c',text,'glibc-1.06/sysdeps/unix/sysv', 1351 ).
file('tcgetpgrp.c',text,'glibc-1.06/sysdeps/unix/sysv', 1173 ).
file('tcsendbrk.c',text,'glibc-1.06/sysdeps/unix/sysv', 1589 ).
file('tcsetattr.c',text,'glibc-1.06/sysdeps/unix/sysv', 5463 ).
file('tcsetpgrp.c',text,'glibc-1.06/sysdeps/unix/sysv', 1131 ).
file('ulimit.S',text,'glibc-1.06/sysdeps/unix/sysv', 877 ).
file('uname.S',text,'glibc-1.06/sysdeps/unix/sysv', 870 ).
file('utime.S',text,'glibc-1.06/sysdeps/unix/sysv', 870 ).
file('utmp.h',text,'glibc-1.06/sysdeps/unix/sysv', 327 ).
file('utsnamelen.h',text,'glibc-1.06/sysdeps/unix/sysv', 26 ).
file('linux',dir,'glibc-1.06/sysdeps/unix/sysv/i386', 512 ).
file('signal.S',text,'glibc-1.06/sysdeps/unix/sysv/i386', 1187 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/sysv/i386', 134 ).
file('time.S',text,'glibc-1.06/sysdeps/unix/sysv/i386', 1037 ).
file('__wait.S',text,'glibc-1.06/sysdeps/unix/sysv/i386/linux', 1159 ).
file('socket.S',text,'glibc-1.06/sysdeps/unix/sysv/i386/linux', 1842 ).
file('sysdep.S',text,'glibc-1.06/sysdeps/unix/sysv/i386/linux', 1100 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/sysv/i386/linux', 2029 ).
file('rename.S',text,'glibc-1.06/sysdeps/unix/sysv/isc2.2', 37 ).
file('Implies',text,'glibc-1.06/sysdeps/unix/sysv/linux', 81 ).
file('__setsid.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 879 ).
file('__wait4.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 45 ).
file('__waitpid.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 880 ).
file('accept.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 42 ).
file('bind.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 40 ).
file('connect.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 43 ).
file('getpeernam.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 47 ).
file('getpgrp.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 878 ).
file('getsocknam.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 47 ).
file('listen.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 42 ).
file('rename.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 37 ).
file('socketpair.S',text,'glibc-1.06/sysdeps/unix/sysv/linux', 46 ).
file('syscall.h',text,'glibc-1.06/sysdeps/unix/sysv/linux', 2858 ).
file('sigaction.h',text,'glibc-1.06/sysdeps/unix/sysv/minix', 1955 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2', 12 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2', 925 ).
file('__fltused.c',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2', 98 ).
file('local_lim.h',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2', 1340 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 11 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 78 ).
file('__getpgrp.c',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 1077 ).
file('__pathconf.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 875 ).
file('__setpgrp.c',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 1223 ).
file('__setsid.c',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 1191 ).
file('__sigact.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 876 ).
file('__sigproc.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 878 ).
file('__sysconf.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 874 ).
file('__waitpid.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 1408 ).
file('confname.h',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 1619 ).
file('direct.h',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 604 ).
file('sco_pgrp.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 883 ).
file('sigpending.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 875 ).
file('sigsuspend.S',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 875 ).
file('syscall.h',text,'glibc-1.06/sysdeps/unix/sysv/sco3.2.4', 2222 ).
file('Dist',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 12 ).
file('Implies',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 97 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 933 ).
file('__getdtsz.c',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 78 ).
file('__getpgsz.c',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 78 ).
file('__sigproc.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 892 ).
file('__sysconf.c',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 3509 ).
file('direct.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 1406 ).
file('fchdir.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 92 ).
file('fcntlbits.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 4355 ).
file('setegid.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 97 ).
file('seteuid.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 97 ).
file('sigaction.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 1977 ).
file('signum.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 2909 ).
file('solaris2',dir,'glibc-1.06/sysdeps/unix/sysv/sysv4', 512 ).
file('sysconfig.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 1105 ).
file('time.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4', 883 ).
file('Makefile',text,'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2', 356 ).
file('__utimes.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2', 85 ).
file('sigaltstack.S',text,'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2', 102 ).
file('start.c',text,'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2', 137 ).
file('sysdep.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2', 135 ).
file('utsnamelen.h',text,'glibc-1.06/sysdeps/unix/sysv/sysv4/solaris2', 28 ).
file('DEFS.h',text,'glibc-1.06/sysdeps/vax', 1654 ).
file('Dist',text,'glibc-1.06/sysdeps/vax', 7 ).
file('Makefile',text,'glibc-1.06/sysdeps/vax', 1142 ).
file('__infnan.c',text,'glibc-1.06/sysdeps/vax', 1668 ).
file('__longjmp.c',text,'glibc-1.06/sysdeps/vax', 3209 ).
file('__memccpy.c',text,'glibc-1.06/sysdeps/vax', 1527 ).
file('bcopy.s',text,'glibc-1.06/sysdeps/vax', 2510 ).
file('bzero.s',text,'glibc-1.06/sysdeps/vax', 2152 ).
file('ffs.s',text,'glibc-1.06/sysdeps/vax', 2058 ).
file('htonl.s',text,'glibc-1.06/sysdeps/vax', 1073 ).
file('htons.s',text,'glibc-1.06/sysdeps/vax', 1067 ).
file('huge_val.h',text,'glibc-1.06/sysdeps/vax', 1036 ).
file('jmp_buf.h',text,'glibc-1.06/sysdeps/vax', 132 ).
file('log10.c',text,'glibc-1.06/sysdeps/vax', 1089 ).
file('memchr.s',text,'glibc-1.06/sysdeps/vax', 2696 ).
file('memcmp.s',text,'glibc-1.06/sysdeps/vax', 2437 ).
file('memmove.s',text,'glibc-1.06/sysdeps/vax', 3203 ).
file('memset.s',text,'glibc-1.06/sysdeps/vax', 2193 ).
file('ntohl.s',text,'glibc-1.06/sysdeps/vax', 1073 ).
file('ntohs.s',text,'glibc-1.06/sysdeps/vax', 1067 ).
file('setjmp.c',text,'glibc-1.06/sysdeps/vax', 1297 ).
file('strcat.s',text,'glibc-1.06/sysdeps/vax', 2451 ).
file('strchr.s',text,'glibc-1.06/sysdeps/vax', 3578 ).
file('strcmp.s',text,'glibc-1.06/sysdeps/vax', 2977 ).
file('strcpy.s',text,'glibc-1.06/sysdeps/vax', 2381 ).
file('strcspn.s',text,'glibc-1.06/sysdeps/vax', 2633 ).
file('strlen.s',text,'glibc-1.06/sysdeps/vax', 2171 ).
file('strncat.s',text,'glibc-1.06/sysdeps/vax', 2939 ).
file('strncmp.s',text,'glibc-1.06/sysdeps/vax', 3026 ).
file('strncpy.s',text,'glibc-1.06/sysdeps/vax', 3027 ).
file('strpbrk.s',text,'glibc-1.06/sysdeps/vax', 2566 ).
file('strrchr.s',text,'glibc-1.06/sysdeps/vax', 3813 ).
file('strsep.s',text,'glibc-1.06/sysdeps/vax', 3098 ).
file('strspn.s',text,'glibc-1.06/sysdeps/vax', 2750 ).
file('strstr.s',text,'glibc-1.06/sysdeps/vax', 3871 ).
file('Makefile',text,'glibc-1.06/termios', 1136 ).
file('TAGS',text,'glibc-1.06/termios', 4327 ).
file('__tcgetatr.dep',text,'glibc-1.06/termios', 306 ).
file('cfmakeraw.c',text,'glibc-1.06/termios', 1212 ).
file('cfmakeraw.dep',text,'glibc-1.06/termios', 182 ).
file('cfsetspeed.c',text,'glibc-1.06/termios', 1159 ).
file('cfsetspeed.dep',text,'glibc-1.06/termios', 224 ).
file('depend-termios',text,'glibc-1.06/termios', 363 ).
file('speed.dep',text,'glibc-1.06/termios', 240 ).
file('sys',dir,'glibc-1.06/termios', 512 ).
file('tcdrain.dep',text,'glibc-1.06/termios', 447 ).
file('tcflow.dep',text,'glibc-1.06/termios', 278 ).
file('tcflush.dep',text,'glibc-1.06/termios', 281 ).
file('tcgetattr.c',text,'glibc-1.06/termios', 1060 ).
file('tcgetattr.dep',text,'glibc-1.06/termios', 201 ).
file('tcgetpgrp.dep',text,'glibc-1.06/termios', 431 ).
file('tcsendbrk.dep',text,'glibc-1.06/termios', 445 ).
file('tcsetattr.dep',text,'glibc-1.06/termios', 303 ).
file('tcsetpgrp.dep',text,'glibc-1.06/termios', 431 ).
file('termios.h',text,'glibc-1.06/termios', 3095 ).
file('termios.h',text,'glibc-1.06/termios/sys', 74 ).
file('ttydefaults.h',text,'glibc-1.06/termios/sys', 3373 ).
file('Makefile',text,'glibc-1.06/time', 3736 ).
file('TAGS',text,'glibc-1.06/time', 4013 ).
file('__adjtime.dep',text,'glibc-1.06/time', 137 ).
file('__getitmr.dep',text,'glibc-1.06/time', 137 ).
file('__gettod.dep',text,'glibc-1.06/time', 134 ).
file('__setitmr.dep',text,'glibc-1.06/time', 137 ).
file('__settod.dep',text,'glibc-1.06/time', 131 ).
file('__tzset.c',text,'glibc-1.06/time', 11728 ).
file('__tzset.dep',text,'glibc-1.06/time', 657 ).
file('adjtime.c',text,'glibc-1.06/time', 1108 ).
file('adjtime.dep',text,'glibc-1.06/time', 187 ).
file('africa',text,'glibc-1.06/time', 1409 ).
file('antarctica',text,'glibc-1.06/time', 90 ).
file('ap',exec,'glibc-1.06/time', 393216 ).
file('ap.c',text,'glibc-1.06/time', 1373 ).
file('ap.dep',text,'glibc-1.06/time', 390 ).
file('asctime.c',text,'glibc-1.06/time', 1666 ).
file('asctime.dep',text,'glibc-1.06/time', 523 ).
file('asia',text,'glibc-1.06/time', 9165 ).
file('australasia',text,'glibc-1.06/time', 16701 ).
file('clock.dep',text,'glibc-1.06/time', 235 ).
file('ctime.c',text,'glibc-1.06/time', 1180 ).
file('ctime.dep',text,'glibc-1.06/time', 153 ).
file('depend-time',text,'glibc-1.06/time', 830 ).
file('difftime.c',text,'glibc-1.06/time', 1043 ).
file('difftime.dep',text,'glibc-1.06/time', 162 ).
file('emkdir.c',text,'glibc-1.06/time', 1530 ).
file('etcetera',text,'glibc-1.06/time', 1282 ).
file('europe',text,'glibc-1.06/time', 12456 ).
file('factory',text,'glibc-1.06/time', 311 ).
file('getitmr.c',text,'glibc-1.06/time', 1107 ).
file('getitmr.dep',text,'glibc-1.06/time', 187 ).
file('gettod.c',text,'glibc-1.06/time', 1084 ).
file('gettod.dep',text,'glibc-1.06/time', 184 ).
file('gmtime.c',text,'glibc-1.06/time', 1099 ).
file('gmtime.dep',text,'glibc-1.06/time', 156 ).
file('ialloc.c',text,'glibc-1.06/time', 1872 ).
file('leapseconds',text,'glibc-1.06/time', 1373 ).
file('localtime.c',text,'glibc-1.06/time', 2239 ).
file('localtime.dep',text,'glibc-1.06/time', 654 ).
file('mktime.c',text,'glibc-1.06/time', 7273 ).
file('mktime.dep',text,'glibc-1.06/time', 178 ).
file('northamerica',text,'glibc-1.06/time', 14740 ).
file('offtime.c',text,'glibc-1.06/time', 2224 ).
file('offtime.dep',text,'glibc-1.06/time', 159 ).
file('pacificnew',text,'glibc-1.06/time', 1098 ).
file('private.h',text,'glibc-1.06/time', 3331 ).
file('scheck.c',text,'glibc-1.06/time', 1194 ).
file('setitmr.c',text,'glibc-1.06/time', 1137 ).
file('setitmr.dep',text,'glibc-1.06/time', 187 ).
file('settod.c',text,'glibc-1.06/time', 1096 ).
file('settod.dep',text,'glibc-1.06/time', 184 ).
file('southamerica',text,'glibc-1.06/time', 4094 ).
file('stime.dep',text,'glibc-1.06/time', 213 ).
file('strftime.c',text,'glibc-1.06/time', 6484 ).
file('strftime.dep',text,'glibc-1.06/time', 651 ).
file('syms-time.c',text,'glibc-1.06/time', 1069 ).
file('syms-time.dep',text,'glibc-1.06/time', 41 ).
file('sys',dir,'glibc-1.06/time', 512 ).
file('systemv',text,'glibc-1.06/time', 1390 ).
file('test_time.args',text,'glibc-1.06/time', 8 ).
file('test_time.c',text,'glibc-1.06/time', 2661 ).
file('test_time.dep',text,'glibc-1.06/time', 376 ).
file('time.dep',text,'glibc-1.06/time', 183 ).
file('time.h',text,'glibc-1.06/time', 5489 ).
file('tzfile.c',text,'glibc-1.06/time', 7646 ).
file('tzfile.dep',text,'glibc-1.06/time', 589 ).
file('tzfile.h',text,'glibc-1.06/time', 4561 ).
file('tzset.c',text,'glibc-1.06/time', 963 ).
file('tzset.dep',text,'glibc-1.06/time', 168 ).
file('yearistype',exec,'glibc-1.06/time', 298 ).
file('zdump',exec,'glibc-1.06/time', 409600 ).
file('zdump.c',text,'glibc-1.06/time', 5961 ).
file('zdump.dep',text,'glibc-1.06/time', 303 ).
file('zic',exec,'glibc-1.06/time', 491520 ).
file('zic.c',text,'glibc-1.06/time', 43437 ).
file('zic.dep',text,'glibc-1.06/time', 371 ).
file('zonenames',text,'glibc-1.06/time', 323 ).
file('zones-africa',text,'glibc-1.06/time', 250 ).
file('zones-antarctica',text,'glibc-1.06/time', 266 ).
file('zones-asia',text,'glibc-1.06/time', 273 ).
file('zones-australasia',text,'glibc-1.06/time', 453 ).
file('zones-etcetera',text,'glibc-1.06/time', 532 ).
file('zones-europe',text,'glibc-1.06/time', 298 ).
file('zones-factory',text,'glibc-1.06/time', 253 ).
file('zones-northamerica',text,'glibc-1.06/time', 645 ).
file('zones-southamerica',text,'glibc-1.06/time', 370 ).
file('zones-systemv',text,'glibc-1.06/time', 433 ).
file('time.h',text,'glibc-1.06/time/sys', 4206 ).
file('0BUGS',text,'gnuplot-3.4', 531 ).
file('0FAQ',text,'gnuplot-3.4', 22844 ).
file('0INSTALL',text,'gnuplot-3.4', 10503 ).
file('0INSTALL.orig',text,'gnuplot-3.4', 10396 ).
file('0README',text,'gnuplot-3.4', 6483 ).
file('Copyright',text,'gnuplot-3.4', 1089 ).
file('History',text,'gnuplot-3.4', 10765 ).
file('Makefile',text,'gnuplot-3.4', 11210 ).
file('Makefile.in',text,'gnuplot-3.4', 11011 ).
file('Makefile.in.orig',text,'gnuplot-3.4', 11109 ).
file('README.3d',text,'gnuplot-3.4', 6205 ).
file('README.3p2',text,'gnuplot-3.4', 2838 ).
file('README.3p3',text,'gnuplot-3.4', 214 ).
file('README.3p4',text,'gnuplot-3.4', 6641 ).
file('README.ami',text,'gnuplot-3.4', 1920 ).
file('README.gnu',text,'gnuplot-3.4', 1168 ).
file('README.mf',text,'gnuplot-3.4', 4322 ).
file('README.nex',text,'gnuplot-3.4', 1428 ).
file('README.os2',text,'gnuplot-3.4', 3851 ).
file('README.pro',text,'gnuplot-3.4', 3142 ).
file('README.win',text,'gnuplot-3.4', 1270 ).
file('README.x11',text,'gnuplot-3.4', 2405 ).
file('README.x11.orig',text,'gnuplot-3.4', 2541 ).
file('bf_test.c',text,'gnuplot-3.4', 3750 ).
file('binary.c',text,'gnuplot-3.4', 13590 ).
file('bitmap.c',text,'gnuplot-3.4', 52385 ).
file('bitmap.h',text,'gnuplot-3.4', 2823 ).
file('buildvms.com',text,'gnuplot-3.4', 1309 ).
file('command.c',text,'gnuplot-3.4', 115092 ).
file('config.status',exec,'gnuplot-3.4', 1865 ).
file('configure',exec,'gnuplot-3.4', 17228 ).
file('configure.in',text,'gnuplot-3.4', 2276 ).
file('configure.in.orig',text,'gnuplot-3.4', 2372 ).
file('configure.orig',text,'gnuplot-3.4', 17229 ).
file('contour.c',text,'gnuplot-3.4', 43010 ).
file('corgraph.asm',text,'gnuplot-3.4', 2272 ).
file('corplot.c',text,'gnuplot-3.4', 2721 ).
file('demo',dir,'gnuplot-3.4', 1024 ).
file('docs',dir,'gnuplot-3.4', 1024 ).
file('epsviewe.h',text,'gnuplot-3.4', 371 ).
file('epsviewe.m',text,'gnuplot-3.4', 2692 ).
file('eval.c',text,'gnuplot-3.4', 4474 ).
file('gnubin.c',text,'gnuplot-3.4', 4611 ).
file('gnuplot',exec,'gnuplot-3.4', 491520 ).
file('gnuplot.def',text,'gnuplot-3.4', 111 ).
file('gnuplot.el',text,'gnuplot-3.4', 5562 ).
file('gnuplot_x11',exec,'gnuplot-3.4', 32768 ).
file('gplt_x11.c',text,'gnuplot-3.4', 30679 ).
file('gplt_x11.c.orig',text,'gnuplot-3.4', 30741 ).
file('graph3d.c',text,'gnuplot-3.4', 82284 ).
file('graph3d.c.orig',text,'gnuplot-3.4', 82276 ).
file('graphics.c',text,'gnuplot-3.4', 55680 ).
file('header.mac',text,'gnuplot-3.4', 918 ).
file('help.c',text,'gnuplot-3.4', 19328 ).
file('help.h',text,'gnuplot-3.4', 2034 ).
file('hrcgraph.asm',text,'gnuplot-3.4', 8192 ).
file('intergra.x11',text,'gnuplot-3.4', 707 ).
file('internal.c',text,'gnuplot-3.4', 16967 ).
file('internal.c.orig',text,'gnuplot-3.4', 16854 ).
file('lasergnu',exec,'gnuplot-3.4', 4511 ).
file('lineproc.mac',text,'gnuplot-3.4', 1980 ).
file('linkopt.amg',text,'gnuplot-3.4', 263 ).
file('linkopt.msc',text,'gnuplot-3.4', 181 ).
file('linkopt.vms',text,'gnuplot-3.4', 26 ).
file('linkopt.ztc',text,'gnuplot-3.4', 179 ).
file('makefile.286',text,'gnuplot-3.4', 4238 ).
file('makefile.amg',text,'gnuplot-3.4', 2889 ).
file('makefile.ami',text,'gnuplot-3.4', 3060 ).
file('makefile.djg',text,'gnuplot-3.4', 2746 ).
file('makefile.emx',text,'gnuplot-3.4', 3535 ).
file('makefile.g',text,'gnuplot-3.4', 2676 ).
file('makefile.msc',text,'gnuplot-3.4', 3897 ).
file('makefile.msw',text,'gnuplot-3.4', 6453 ).
file('makefile.nt',text,'gnuplot-3.4', 5755 ).
file('makefile.os2',text,'gnuplot-3.4', 2363 ).
file('makefile.r',text,'gnuplot-3.4', 17918 ).
file('makefile.st',text,'gnuplot-3.4', 3731 ).
file('makefile.tc',text,'gnuplot-3.4', 5714 ).
file('makefile.unx',text,'gnuplot-3.4', 33064 ).
file('makefile.unx.orig',text,'gnuplot-3.4', 33173 ).
file('makefile.vms',text,'gnuplot-3.4', 2097 ).
file('makefile.win',text,'gnuplot-3.4', 5839 ).
file('makefile.ztc',text,'gnuplot-3.4', 2937 ).
file('misc.c',text,'gnuplot-3.4', 25667 ).
file('os2',dir,'gnuplot-3.4', 512 ).
file('parse.c',text,'gnuplot-3.4', 11696 ).
file('pcgraph.asm',text,'gnuplot-3.4', 3925 ).
file('plot.c',text,'gnuplot-3.4', 13132 ).
file('plot.h',text,'gnuplot-3.4', 15668 ).
file('readline.c',text,'gnuplot-3.4', 22201 ).
file('scanner.c',text,'gnuplot-3.4', 10596 ).
file('setshow.c',text,'gnuplot-3.4', 76060 ).
file('setshow.h',text,'gnuplot-3.4', 5246 ).
file('specfun.c',text,'gnuplot-3.4', 20894 ).
file('standard.c',text,'gnuplot-3.4', 20424 ).
file('term',dir,'gnuplot-3.4', 1536 ).
file('term.c',text,'gnuplot-3.4', 68887 ).
file('term.h',text,'gnuplot-3.4', 5989 ).
file('util.c',text,'gnuplot-3.4', 12982 ).
file('version.c',text,'gnuplot-3.4', 2160 ).
file('win',dir,'gnuplot-3.4', 512 ).
file('1.dat',text,'gnuplot-3.4/demo', 839 ).
file('2.dat',text,'gnuplot-3.4/demo', 839 ).
file('3.dat',text,'gnuplot-3.4/demo', 839 ).
file('airfoil.dem',text,'gnuplot-3.4/demo', 4833 ).
file('all.dem',text,'gnuplot-3.4/demo', 2597 ).
file('animate.dem',text,'gnuplot-3.4/demo', 818 ).
file('antenna.dat',text,'gnuplot-3.4/demo', 20641 ).
file('binary.dem',text,'gnuplot-3.4/demo', 536 ).
file('bivariat.dem',text,'gnuplot-3.4/demo', 3047 ).
file('contours.dem',text,'gnuplot-3.4/demo', 3317 ).
file('controls.dem',text,'gnuplot-3.4/demo', 774 ).
file('discrete.dem',text,'gnuplot-3.4/demo', 739 ).
file('electron.dem',text,'gnuplot-3.4/demo', 2129 ).
file('err.dat',text,'gnuplot-3.4/demo', 478 ).
file('errorbar.dem',text,'gnuplot-3.4/demo', 1211 ).
file('glass.dat',text,'gnuplot-3.4/demo', 8596 ).
file('gnuplot.rot',text,'gnuplot-3.4/demo', 52 ).
file('hemisphr.dat',text,'gnuplot-3.4/demo', 6975 ).
file('hidden.dem',text,'gnuplot-3.4/demo', 1072 ).
file('klein.dat',text,'gnuplot-3.4/demo', 20117 ).
file('multimsh.dem',text,'gnuplot-3.4/demo', 1028 ).
file('param.dem',text,'gnuplot-3.4/demo', 987 ).
file('polar.dat',text,'gnuplot-3.4/demo', 6598 ).
file('polar.dem',text,'gnuplot-3.4/demo', 1063 ).
file('poldat.dem',text,'gnuplot-3.4/demo', 1257 ).
file('prob.dem',text,'gnuplot-3.4/demo', 21935 ).
file('prob2.dem',text,'gnuplot-3.4/demo', 10444 ).
file('random.dem',text,'gnuplot-3.4/demo', 913 ).
file('scatter.dem',text,'gnuplot-3.4/demo', 1373 ).
file('scatter2.dat',text,'gnuplot-3.4/demo', 1845 ).
file('simple.dem',text,'gnuplot-3.4/demo', 1229 ).
file('singulr.dem',text,'gnuplot-3.4/demo', 4183 ).
file('spline.dem',text,'gnuplot-3.4/demo', 2708 ).
file('stat.inc',text,'gnuplot-3.4/demo', 6353 ).
file('steps.dat',text,'gnuplot-3.4/demo', 51 ).
file('steps.dem',text,'gnuplot-3.4/demo', 742 ).
file('surface1.dem',text,'gnuplot-3.4/demo', 3384 ).
file('surface2.dem',text,'gnuplot-3.4/demo', 1755 ).
file('using.dat',text,'gnuplot-3.4/demo', 5630 ).
file('using.dem',text,'gnuplot-3.4/demo', 806 ).
file('whale.dat',text,'gnuplot-3.4/demo', 94483 ).
file('world.cor',text,'gnuplot-3.4/demo', 3485 ).
file('world.dat',text,'gnuplot-3.4/demo', 16782 ).
file('world.dem',text,'gnuplot-3.4/demo', 1414 ).
file('Makefile',text,'gnuplot-3.4/docs', 3617 ).
file('Makefile.in',text,'gnuplot-3.4/docs', 3441 ).
file('README',text,'gnuplot-3.4/docs', 3883 ).
file('checkdoc.c',text,'gnuplot-3.4/docs', 1865 ).
file('doc2gih',exec,'gnuplot-3.4/docs', 16384 ).
file('doc2gih.c',text,'gnuplot-3.4/docs', 2132 ).
file('doc2hlp.c',text,'gnuplot-3.4/docs', 2084 ).
file('doc2hlp.com',text,'gnuplot-3.4/docs', 90 ).
file('doc2info.pl',text,'gnuplot-3.4/docs', 3020 ).
file('doc2ipf.c',text,'gnuplot-3.4/docs', 15084 ).
file('doc2ms.c',text,'gnuplot-3.4/docs', 5822 ).
file('doc2rtf.c',text,'gnuplot-3.4/docs', 8811 ).
file('doc2tex.c',text,'gnuplot-3.4/docs', 5314 ).
file('gnuplot.1',text,'gnuplot-3.4/docs', 7472 ).
file('gnuplot.doc',text,'gnuplot-3.4/docs', 129670 ).
file('gnuplot.gih',text,'gnuplot-3.4/docs', 119818 ).
file('gpcard.tex',text,'gnuplot-3.4/docs', 41221 ).
file('lasergnu.1',text,'gnuplot-3.4/docs', 1594 ).
file('latextut',dir,'gnuplot-3.4/docs', 512 ).
file('makefile.ami',text,'gnuplot-3.4/docs', 2760 ).
file('makefile.org',text,'gnuplot-3.4/docs', 4379 ).
file('titlepag.ms',text,'gnuplot-3.4/docs', 758 ).
file('titlepag.tex',text,'gnuplot-3.4/docs', 1493 ).
file('toc_entr.sty',text,'gnuplot-3.4/docs', 2075 ).
file('Makefile',text,'gnuplot-3.4/docs/latextut', 802 ).
file('Makefile.in',text,'gnuplot-3.4/docs/latextut', 759 ).
file('eg1.plt',text,'gnuplot-3.4/docs/latextut', 65 ).
file('eg2.plt',text,'gnuplot-3.4/docs/latextut', 219 ).
file('eg3.dat',text,'gnuplot-3.4/docs/latextut', 781 ).
file('eg3.plt',text,'gnuplot-3.4/docs/latextut', 199 ).
file('eg4.plt',text,'gnuplot-3.4/docs/latextut', 227 ).
file('eg5.plt',text,'gnuplot-3.4/docs/latextut', 331 ).
file('eg6.plt',text,'gnuplot-3.4/docs/latextut', 457 ).
file('header.tex',text,'gnuplot-3.4/docs/latextut', 1677 ).
file('linepoin.plt',text,'gnuplot-3.4/docs/latextut', 549 ).
file('makefile.org',text,'gnuplot-3.4/docs/latextut', 566 ).
file('tutorial.tex',text,'gnuplot-3.4/docs/latextut', 16602 ).
file('dialogs.c',text,'gnuplot-3.4/os2', 13603 ).
file('dialogs.h',text,'gnuplot-3.4/os2', 2063 ).
file('gclient.c',text,'gnuplot-3.4/os2', 37245 ).
file('gnuicon.uue',text,'gnuplot-3.4/os2', 3557 ).
file('gnupmdrv.c',text,'gnuplot-3.4/os2', 6307 ).
file('gnupmdrv.def',text,'gnuplot-3.4/os2', 178 ).
file('gnupmdrv.h',text,'gnuplot-3.4/os2', 4612 ).
file('gnupmdrv.itl',text,'gnuplot-3.4/os2', 9564 ).
file('gnupmdrv.rc',text,'gnuplot-3.4/os2', 6144 ).
file('makefile',text,'gnuplot-3.4/os2', 1166 ).
file('print.c',text,'gnuplot-3.4/os2', 13301 ).
file('README',text,'gnuplot-3.4/term', 7115 ).
file('aed.trm',text,'gnuplot-3.4/term', 2389 ).
file('ai.trm',text,'gnuplot-3.4/term', 7403 ).
file('amiga.trm',text,'gnuplot-3.4/term', 9618 ).
file('apollo.trm',text,'gnuplot-3.4/term', 17142 ).
file('atari.trm',text,'gnuplot-3.4/term', 9635 ).
file('bigfig.trm',text,'gnuplot-3.4/term', 3690 ).
file('cgi.trm',text,'gnuplot-3.4/term', 4485 ).
file('compact.c',text,'gnuplot-3.4/term', 2493 ).
file('corel.trm',text,'gnuplot-3.4/term', 10947 ).
file('debug.trm',text,'gnuplot-3.4/term', 2749 ).
file('djsvga.trm',text,'gnuplot-3.4/term', 2628 ).
file('dumb.trm',text,'gnuplot-3.4/term', 6318 ).
file('dxf.trm',text,'gnuplot-3.4/term', 9212 ).
file('dxy.trm',text,'gnuplot-3.4/term', 2316 ).
file('eepic.trm',text,'gnuplot-3.4/term', 7751 ).
file('emxvga.trm',text,'gnuplot-3.4/term', 4303 ).
file('epson.trm',text,'gnuplot-3.4/term', 13022 ).
file('excl.trm',text,'gnuplot-3.4/term', 6188 ).
file('fg.trm',text,'gnuplot-3.4/term', 4978 ).
file('fig.trm',text,'gnuplot-3.4/term', 8478 ).
file('gpic.trm',text,'gnuplot-3.4/term', 4834 ).
file('gpr.trm',text,'gnuplot-3.4/term', 11715 ).
file('grass.trm',text,'gnuplot-3.4/term', 10800 ).
file('hp26.trm',text,'gnuplot-3.4/term', 16298 ).
file('hp2648.trm',text,'gnuplot-3.4/term', 2194 ).
file('hp500c.trm',text,'gnuplot-3.4/term', 11014 ).
file('hpgl.trm',text,'gnuplot-3.4/term', 14113 ).
file('hpljii.trm',text,'gnuplot-3.4/term', 8301 ).
file('hppj.trm',text,'gnuplot-3.4/term', 6017 ).
file('imagen.trm',text,'gnuplot-3.4/term', 17390 ).
file('impcodes.h',text,'gnuplot-3.4/term', 1309 ).
file('iris4d.trm',text,'gnuplot-3.4/term', 5417 ).
file('kyo.trm',text,'gnuplot-3.4/term', 2379 ).
file('latex.trm',text,'gnuplot-3.4/term', 20044 ).
file('metafont.trm',text,'gnuplot-3.4/term', 14602 ).
file('mif.trm',text,'gnuplot-3.4/term', 18145 ).
file('next.trm',text,'gnuplot-3.4/term', 11392 ).
file('object.h',text,'gnuplot-3.4/term', 7748 ).
file('pbm.trm',text,'gnuplot-3.4/term', 7408 ).
file('pc.trm',text,'gnuplot-3.4/term', 25354 ).
file('pm.trm',text,'gnuplot-3.4/term', 7141 ).
file('post.trm',text,'gnuplot-3.4/term', 11396 ).
file('pslatex.trm',text,'gnuplot-3.4/term', 5497 ).
file('pstricks.trm',text,'gnuplot-3.4/term', 7739 ).
file('qms.trm',text,'gnuplot-3.4/term', 3863 ).
file('regis.trm',text,'gnuplot-3.4/term', 3748 ).
file('rgip.trm',text,'gnuplot-3.4/term', 8468 ).
file('sun.trm',text,'gnuplot-3.4/term', 6225 ).
file('t410x.trm',text,'gnuplot-3.4/term', 5138 ).
file('tek.trm',text,'gnuplot-3.4/term', 6804 ).
file('texdraw.trm',text,'gnuplot-3.4/term', 7704 ).
file('tgif.h',text,'gnuplot-3.4/term', 3516 ).
file('tgif.trm',text,'gnuplot-3.4/term', 5418 ).
file('tpic.trm',text,'gnuplot-3.4/term', 25800 ).
file('unixpc.trm',text,'gnuplot-3.4/term', 14323 ).
file('unixplot.trm',text,'gnuplot-3.4/term', 2012 ).
file('v384.trm',text,'gnuplot-3.4/term', 2658 ).
file('vws.trm',text,'gnuplot-3.4/term', 8092 ).
file('win.trm',text,'gnuplot-3.4/term', 6130 ).
file('x11.trm',text,'gnuplot-3.4/term', 8645 ).
file('xlib.trm',text,'gnuplot-3.4/term', 1141 ).
file('geticon.c',text,'gnuplot-3.4/win', 2306 ).
file('wcommon.h',text,'gnuplot-3.4/win', 2170 ).
file('wgnuplib.c',text,'gnuplot-3.4/win', 2936 ).
file('wgnuplib.def',text,'gnuplot-3.4/win', 161 ).
file('wgnuplib.h',text,'gnuplot-3.4/win', 8037 ).
file('wgnuplib.rc',text,'gnuplot-3.4/win', 4644 ).
file('wgnuplot.def',text,'gnuplot-3.4/win', 415 ).
file('wgnuplot.hpj',text,'gnuplot-3.4/win', 69 ).
file('wgnuplot.mnu',text,'gnuplot-3.4/win', 11011 ).
file('wgnuplot.rc',text,'gnuplot-3.4/win', 5362 ).
file('wgraph.c',text,'gnuplot-3.4/win', 40235 ).
file('winmain.c',text,'gnuplot-3.4/win', 9035 ).
file('wmenu.c',text,'gnuplot-3.4/win', 18495 ).
file('wpause.c',text,'gnuplot-3.4/win', 7836 ).
file('wprinter.c',text,'gnuplot-3.4/win', 8664 ).
file('wresourc.h',text,'gnuplot-3.4/win', 2228 ).
file('wtext.c',text,'gnuplot-3.4/win', 47292 ).
file('wtext.h',text,'gnuplot-3.4/win', 2840 ).
file('COPYING',text,'gperf-2.1', 12488 ).
file('ChangeLog',text,'gperf-2.1', 19098 ).
file('Makefile',text,'gperf-2.1', 1304 ).
file('README',text,'gperf-2.1', 1174 ).
file('README-FIRST',text,'gperf-2.1', 371 ).
file('gperf.1',text,'gperf-2.1', 722 ).
file('src',dir,'gperf-2.1', 1024 ).
file('tests',dir,'gperf-2.1', 1024 ).
file('texinfo.tex',text,'gperf-2.1', 108872 ).
file('Makefile',text,'gperf-2.1/src', 3533 ).
file('boolarray.c',text,'gperf-2.1/src', 2626 ).
file('boolarray.h',text,'gperf-2.1/src', 1750 ).
file('getopt.c',text,'gperf-2.1/src', 12436 ).
file('gperf-to-do',text,'gperf-2.1/src', 1075 ).
file('hashtable.c',text,'gperf-2.1/src', 3975 ).
file('hashtable.h',text,'gperf-2.1/src', 1330 ).
file('iterator.c',text,'gperf-2.1/src', 3043 ).
file('iterator.h',text,'gperf-2.1/src', 2165 ).
file('keylist.c',text,'gperf-2.1/src', 34522 ).
file('keylist.h',text,'gperf-2.1/src', 2476 ).
file('listnode.c',text,'gperf-2.1/src', 3877 ).
file('listnode.h',text,'gperf-2.1/src', 1771 ).
file('main.c',text,'gperf-2.1/src', 2710 ).
file('options.c',text,'gperf-2.1/src', 18529 ).
file('options.h',text,'gperf-2.1/src', 6400 ).
file('perfect.c',text,'gperf-2.1/src', 10410 ).
file('perfect.h',text,'gperf-2.1/src', 1632 ).
file('prototype.h',text,'gperf-2.1/src', 234 ).
file('readline.c',text,'gperf-2.1/src', 2087 ).
file('readline.h',text,'gperf-2.1/src', 1160 ).
file('stderr.c',text,'gperf-2.1/src', 2881 ).
file('stderr.h',text,'gperf-2.1/src', 1060 ).
file('version.c',text,'gperf-2.1/src', 882 ).
file('xmalloc.c',text,'gperf-2.1/src', 2526 ).
file('Makefile',text,'gperf-2.1/tests', 2696 ).
file('ada.gperf',text,'gperf-2.1/tests', 366 ).
file('adainset.c',text,'gperf-2.1/tests', 3540 ).
file('adapredefined.gperf',text,'gperf-2.1/tests', 473 ).
file('c++.gperf',text,'gperf-2.1/tests', 337 ).
file('c++inset.c',text,'gperf-2.1/tests', 6637 ).
file('c++out',exec,'gperf-2.1/tests', 24576 ).
file('c-parse.gperf',text,'gperf-2.1/tests', 1438 ).
file('c.gperf',text,'gperf-2.1/tests', 198 ).
file('cinset.c',text,'gperf-2.1/tests', 4384 ).
file('gpc.gperf',text,'gperf-2.1/tests', 1122 ).
file('gplus.gperf',text,'gperf-2.1/tests', 2072 ).
file('m3inset.c',text,'gperf-2.1/tests', 4223 ).
file('modula2.gperf',text,'gperf-2.1/tests', 225 ).
file('modula3.gperf',text,'gperf-2.1/tests', 608 ).
file('pascal.gperf',text,'gperf-2.1/tests', 188 ).
file('pinset.c',text,'gperf-2.1/tests', 4139 ).
file('preinset.c',text,'gperf-2.1/tests', 5962 ).
file('test.c',text,'gperf-2.1/tests', 644 ).
file('Makefile',text,'grep-1.5', 828 ).
file('README',text,'grep-1.5', 7593 ).
file('README.cray',text,'grep-1.5', 2730 ).
file('README.sunos4',text,'grep-1.5', 2688 ).
file('alloca.c',text,'grep-1.5', 5090 ).
file('dfa.c',text,'grep-1.5', 59049 ).
file('dfa.h',text,'grep-1.5', 22508 ).
file('getopt.c',text,'grep-1.5', 16593 ).
file('grep.c',text,'grep-1.5', 28346 ).
file('grep.man',text,'grep-1.5', 6850 ).
file('regex.c',text,'grep-1.5', 44772 ).
file('regex.h',text,'grep-1.5', 8085 ).
file('tests',dir,'grep-1.5', 512 ).
file('khadafy.lines',text,'grep-1.5/tests', 728 ).
file('khadafy.regexp',text,'grep-1.5/tests', 66 ).
file('regress.sh',text,'grep-1.5/tests', 455 ).
file('scriptgen.awk',text,'grep-1.5/tests', 274 ).
file('spencer.tests',text,'grep-1.5/tests', 1748 ).
file('BUG-REPORT',text,'groff-1.06', 2112 ).
file('CHANGES',text,'groff-1.06', 20694 ).
file('COPYING',text,'groff-1.06', 17994 ).
file('ChangeLog',text,'groff-1.06', 148006 ).
file('INSTALL',text,'groff-1.06', 5413 ).
file('Makefile',text,'groff-1.06', 12100 ).
file('Makefile.ccpg',text,'groff-1.06', 514 ).
file('Makefile.comm',text,'groff-1.06', 5641 ).
file('Makefile.cpg',text,'groff-1.06', 501 ).
file('Makefile.dev',text,'groff-1.06', 43 ).
file('Makefile.in',text,'groff-1.06', 11812 ).
file('Makefile.init',text,'groff-1.06', 886 ).
file('Makefile.lib',text,'groff-1.06', 319 ).
file('Makefile.man',text,'groff-1.06', 68 ).
file('Makefile.sub',text,'groff-1.06', 152 ).
file('PROBLEMS',text,'groff-1.06', 9774 ).
file('PROJECTS',text,'groff-1.06', 366 ).
file('README',text,'groff-1.06', 1627 ).
file('TODO',text,'groff-1.06', 135 ).
file('VERSION',text,'groff-1.06', 5 ).
file('acgroff.m4',text,'groff-1.06', 8090 ).
file('addftinfo',dir,'groff-1.06', 512 ).
file('afmtodit',dir,'groff-1.06', 512 ).
file('config.status',exec,'groff-1.06', 2058 ).
file('configure',exec,'groff-1.06', 20965 ).
file('configure.in',text,'groff-1.06', 902 ).
file('devascii',dir,'groff-1.06', 512 ).
file('devdvi',dir,'groff-1.06', 512 ).
file('devlatin1',dir,'groff-1.06', 512 ).
file('devps',dir,'groff-1.06', 1024 ).
file('doc',dir,'groff-1.06', 512 ).
file('eqn',dir,'groff-1.06', 1024 ).
file('gcc.patch',text,'groff-1.06', 604 ).
file('gendef.sh',text,'groff-1.06', 238 ).
file('grodvi',dir,'groff-1.06', 512 ).
file('groff',dir,'groff-1.06', 512 ).
file('grog',dir,'groff-1.06', 512 ).
file('grops',dir,'groff-1.06', 512 ).
file('grotty',dir,'groff-1.06', 512 ).
file('include',dir,'groff-1.06', 512 ).
file('indxbib',dir,'groff-1.06', 512 ).
file('libbib',dir,'groff-1.06', 512 ).
file('libdriver',dir,'groff-1.06', 512 ).
file('libgroff',dir,'groff-1.06', 1536 ).
file('lkbib',dir,'groff-1.06', 512 ).
file('lookbib',dir,'groff-1.06', 512 ).
file('man',dir,'groff-1.06', 512 ).
file('mdate.sh',exec,'groff-1.06', 920 ).
file('mm',dir,'groff-1.06', 512 ).
file('nroff',dir,'groff-1.06', 512 ).
file('pfbtops',dir,'groff-1.06', 512 ).
file('pic',dir,'groff-1.06', 1024 ).
file('psbb',dir,'groff-1.06', 512 ).
file('refer',dir,'groff-1.06', 512 ).
file('soelim',dir,'groff-1.06', 512 ).
file('tbl',dir,'groff-1.06', 512 ).
file('tfmtodit',dir,'groff-1.06', 512 ).
file('tmac',dir,'groff-1.06', 1024 ).
file('troff',dir,'groff-1.06', 1024 ).
file('xditview',dir,'groff-1.06', 1024 ).
file('Makefile.dep',text,'groff-1.06/addftinfo', 165 ).
file('Makefile.sub',text,'groff-1.06/addftinfo', 117 ).
file('addftinfo',exec,'groff-1.06/addftinfo', 81920 ).
file('addftinfo.cc',text,'groff-1.06/addftinfo', 5214 ).
file('addftinfo.man',text,'groff-1.06/addftinfo', 2013 ).
file('addftinfo.n',text,'groff-1.06/addftinfo', 1982 ).
file('guess.cc',text,'groff-1.06/addftinfo', 11308 ).
file('guess.h',text,'groff-1.06/addftinfo', 1187 ).
file('Makefile.sub',text,'groff-1.06/afmtodit', 407 ).
file('afmtodit',exec,'groff-1.06/afmtodit', 8418 ).
file('afmtodit.man',text,'groff-1.06/afmtodit', 4819 ).
file('afmtodit.n',text,'groff-1.06/afmtodit', 4877 ).
file('afmtodit.pl',text,'groff-1.06/afmtodit', 8412 ).
file('B',text,'groff-1.06/devascii', 1507 ).
file('BI',text,'groff-1.06/devascii', 1508 ).
file('DESC',text,'groff-1.06/devascii', 88 ).
file('DESC.proto',text,'groff-1.06/devascii', 88 ).
file('I',text,'groff-1.06/devascii', 1507 ).
file('Makefile.sub',text,'groff-1.06/devascii', 848 ).
file('R',text,'groff-1.06/devascii', 1492 ).
file('R.proto',text,'groff-1.06/devascii', 1507 ).
file('B',text,'groff-1.06/devdvi', 5924 ).
file('BI',text,'groff-1.06/devdvi', 7298 ).
file('CW',text,'groff-1.06/devdvi', 3439 ).
file('DESC',text,'groff-1.06/devdvi', 233 ).
file('DESC.in',text,'groff-1.06/devdvi', 220 ).
file('EX',text,'groff-1.06/devdvi', 4849 ).
file('H',text,'groff-1.06/devdvi', 5363 ).
file('HB',text,'groff-1.06/devdvi', 5371 ).
file('HI',text,'groff-1.06/devdvi', 6318 ).
file('I',text,'groff-1.06/devdvi', 7285 ).
file('MI',text,'groff-1.06/devdvi', 4073 ).
file('Makefile.sub',text,'groff-1.06/devdvi', 391 ).
file('R',text,'groff-1.06/devdvi', 6756 ).
file('S',text,'groff-1.06/devdvi', 4055 ).
file('SA',text,'groff-1.06/devdvi', 4813 ).
file('SB',text,'groff-1.06/devdvi', 4264 ).
file('generate',dir,'groff-1.06/devdvi', 512 ).
file('CompileFonts',exec,'groff-1.06/devdvi/generate', 416 ).
file('Makefile',text,'groff-1.06/devdvi/generate', 2728 ).
file('msam.map',text,'groff-1.06/devdvi/generate', 1916 ).
file('msbm.map',text,'groff-1.06/devdvi/generate', 1487 ).
file('texb.map',text,'groff-1.06/devdvi/generate', 823 ).
file('texex.map',text,'groff-1.06/devdvi/generate', 1525 ).
file('texi.map',text,'groff-1.06/devdvi/generate', 832 ).
file('texmi.map',text,'groff-1.06/devdvi/generate', 200 ).
file('texr.map',text,'groff-1.06/devdvi/generate', 868 ).
file('texsy.map',text,'groff-1.06/devdvi/generate', 679 ).
file('textt.map',text,'groff-1.06/devdvi/generate', 788 ).
file('B',text,'groff-1.06/devlatin1', 3683 ).
file('BI',text,'groff-1.06/devlatin1', 3684 ).
file('DESC',text,'groff-1.06/devlatin1', 88 ).
file('DESC.proto',text,'groff-1.06/devlatin1', 88 ).
file('I',text,'groff-1.06/devlatin1', 3683 ).
file('Makefile.sub',text,'groff-1.06/devlatin1', 850 ).
file('R',text,'groff-1.06/devlatin1', 3668 ).
file('R.proto',text,'groff-1.06/devlatin1', 3683 ).
file('AB',text,'groff-1.06/devps', 8817 ).
file('ABI',text,'groff-1.06/devps', 11373 ).
file('AI',text,'groff-1.06/devps', 11369 ).
file('AR',text,'groff-1.06/devps', 8822 ).
file('BMB',text,'groff-1.06/devps', 7794 ).
file('BMBI',text,'groff-1.06/devps', 10299 ).
file('BMI',text,'groff-1.06/devps', 10243 ).
file('BMR',text,'groff-1.06/devps', 7678 ).
file('CB',text,'groff-1.06/devps', 6999 ).
file('CBI',text,'groff-1.06/devps', 9389 ).
file('CI',text,'groff-1.06/devps', 9364 ).
file('CR',text,'groff-1.06/devps', 7007 ).
file('DESC',text,'groff-1.06/devps', 191 ).
file('DESC.in',text,'groff-1.06/devps', 153 ).
file('HB',text,'groff-1.06/devps', 8737 ).
file('HBI',text,'groff-1.06/devps', 11312 ).
file('HI',text,'groff-1.06/devps', 12074 ).
file('HNB',text,'groff-1.06/devps', 8697 ).
file('HNBI',text,'groff-1.06/devps', 11231 ).
file('HNI',text,'groff-1.06/devps', 12010 ).
file('HNR',text,'groff-1.06/devps', 9472 ).
file('HR',text,'groff-1.06/devps', 9502 ).
file('Makefile.sub',text,'groff-1.06/devps', 844 ).
file('NB',text,'groff-1.06/devps', 7959 ).
file('NBI',text,'groff-1.06/devps', 10160 ).
file('NI',text,'groff-1.06/devps', 9980 ).
file('NR',text,'groff-1.06/devps', 7993 ).
file('PB',text,'groff-1.06/devps', 8105 ).
file('PBI',text,'groff-1.06/devps', 10140 ).
file('PI',text,'groff-1.06/devps', 10100 ).
file('PR',text,'groff-1.06/devps', 8146 ).
file('S',text,'groff-1.06/devps', 5900 ).
file('SS',text,'groff-1.06/devps', 7696 ).
file('TB',text,'groff-1.06/devps', 8673 ).
file('TBI',text,'groff-1.06/devps', 10597 ).
file('TI',text,'groff-1.06/devps', 10823 ).
file('TR',text,'groff-1.06/devps', 8564 ).
file('ZCMI',text,'groff-1.06/devps', 10803 ).
file('ZD',text,'groff-1.06/devps', 4777 ).
file('ZDR',text,'groff-1.06/devps', 4787 ).
file('download',text,'groff-1.06/devps', 116 ).
file('generate',dir,'groff-1.06/devps', 512 ).
file('prologue',text,'groff-1.06/devps', 3761 ).
file('symbol.afm',text,'groff-1.06/devps', 9824 ).
file('symbolmap',text,'groff-1.06/devps', 6210 ).
file('symbolsl.ps',text,'groff-1.06/devps', 916 ).
file('text.enc',text,'groff-1.06/devps', 2438 ).
file('zapfdr.afm',text,'groff-1.06/devps', 9475 ).
file('zapfdr.ps',text,'groff-1.06/devps', 4949 ).
file('Makefile',text,'groff-1.06/devps/generate', 5744 ).
file('afmname',exec,'groff-1.06/devps/generate', 1580 ).
file('dingbatsmap',text,'groff-1.06/devps/generate', 14 ).
file('dingbatsrmap',text,'groff-1.06/devps/generate', 7 ).
file('lgreekmap',text,'groff-1.06/devps/generate', 234 ).
file('symbol.sed',text,'groff-1.06/devps/generate', 982 ).
file('symbolchars',text,'groff-1.06/devps/generate', 534 ).
file('symbolsl.afm',text,'groff-1.06/devps/generate', 9407 ).
file('textmap',text,'groff-1.06/devps/generate', 5692 ).
file('Makefile',text,'groff-1.06/doc', 1348 ).
file('meintro.me',text,'groff-1.06/doc', 41284 ).
file('meref.me',text,'groff-1.06/doc', 36639 ).
file('Makefile.dep',text,'groff-1.06/eqn', 1804 ).
file('Makefile.sub',text,'groff-1.06/eqn', 368 ).
file('TODO',text,'groff-1.06/eqn', 947 ).
file('box.cc',text,'groff-1.06/eqn', 12118 ).
file('box.h',text,'groff-1.06/eqn', 6164 ).
file('delim.cc',text,'groff-1.06/eqn', 10623 ).
file('eqn',exec,'groff-1.06/eqn', 385024 ).
file('eqn.cc',text,'groff-1.06/eqn', 39012 ).
file('eqn.h',text,'groff-1.06/eqn', 1518 ).
file('eqn.man',text,'groff-1.06/eqn', 17155 ).
file('eqn.n',text,'groff-1.06/eqn', 17161 ).
file('eqn.tab.h',text,'groff-1.06/eqn', 1171 ).
file('eqn.y',text,'groff-1.06/eqn', 6733 ).
file('lex.cc',text,'groff-1.06/eqn', 23938 ).
file('limit.cc',text,'groff-1.06/eqn', 5338 ).
file('list.cc',text,'groff-1.06/eqn', 6442 ).
file('main.cc',text,'groff-1.06/eqn', 8313 ).
file('mark.cc',text,'groff-1.06/eqn', 2866 ).
file('other.cc',text,'groff-1.06/eqn', 14356 ).
file('over.cc',text,'groff-1.06/eqn', 5294 ).
file('pbox.h',text,'groff-1.06/eqn', 4232 ).
file('pile.cc',text,'groff-1.06/eqn', 7795 ).
file('script.cc',text,'groff-1.06/eqn', 6242 ).
file('special.cc',text,'groff-1.06/eqn', 3161 ).
file('sqrt.cc',text,'groff-1.06/eqn', 5757 ).
file('text.cc',text,'groff-1.06/eqn', 10529 ).
file('y.output',text,'groff-1.06/eqn', 88606 ).
file('Makefile.dep',text,'groff-1.06/grodvi', 134 ).
file('Makefile.sub',text,'groff-1.06/grodvi', 95 ).
file('dvi.cc',text,'groff-1.06/grodvi', 18776 ).
file('grodvi',exec,'groff-1.06/grodvi', 196608 ).
file('grodvi.man',text,'groff-1.06/grodvi', 3623 ).
file('grodvi.n',text,'groff-1.06/grodvi', 3622 ).
file('Makefile.dep',text,'groff-1.06/groff', 248 ).
file('Makefile.sub',text,'groff-1.06/groff', 128 ).
file('groff',exec,'groff-1.06/groff', 180224 ).
file('groff.cc',text,'groff-1.06/groff', 13636 ).
file('groff.man',text,'groff-1.06/groff', 6017 ).
file('groff.n',text,'groff-1.06/groff', 5852 ).
file('pipeline.c',text,'groff-1.06/groff', 5411 ).
file('pipeline.h',text,'groff-1.06/groff', 1041 ).
file('Makefile.sub',text,'groff-1.06/grog', 382 ).
file('grog',exec,'groff-1.06/grog', 2563 ).
file('grog.man',text,'groff-1.06/grog', 1046 ).
file('grog.n',text,'groff-1.06/grog', 973 ).
file('grog.pl',text,'groff-1.06/grog', 2557 ).
file('grog.sh',text,'groff-1.06/grog', 1568 ).
file('Makefile.dep',text,'groff-1.06/grops', 372 ).
file('Makefile.sub',text,'groff-1.06/grops', 116 ).
file('TODO',text,'groff-1.06/grops', 667 ).
file('grops',exec,'groff-1.06/grops', 286720 ).
file('grops.man',text,'groff-1.06/grops', 15058 ).
file('grops.n',text,'groff-1.06/grops', 15118 ).
file('ps.cc',text,'groff-1.06/grops', 37335 ).
file('ps.h',text,'groff-1.06/grops', 4174 ).
file('psrm.cc',text,'groff-1.06/grops', 26054 ).
file('Makefile.dep',text,'groff-1.06/grotty', 134 ).
file('Makefile.sub',text,'groff-1.06/grotty', 95 ).
file('TODO',text,'groff-1.06/grotty', 115 ).
file('grotty',exec,'groff-1.06/grotty', 163840 ).
file('grotty.man',text,'groff-1.06/grotty', 3578 ).
file('grotty.n',text,'groff-1.06/grotty', 3624 ).
file('tty.cc',text,'groff-1.06/grotty', 10159 ).
file('Makefile.sub',text,'groff-1.06/include', 641 ).
file('assert.h',text,'groff-1.06/include', 1188 ).
file('cmap.h',text,'groff-1.06/include', 1377 ).
file('cset.h',text,'groff-1.06/include', 1769 ).
file('defs.h',text,'groff-1.06/include', 439 ).
file('device.h',text,'groff-1.06/include', 833 ).
file('driver.h',text,'groff-1.06/include', 1105 ).
file('errarg.h',text,'groff-1.06/include', 1304 ).
file('error.h',text,'groff-1.06/include', 2070 ).
file('font.h',text,'groff-1.06/include', 3325 ).
file('index.h',text,'groff-1.06/include', 1140 ).
file('lib.h',text,'groff-1.06/include', 2405 ).
file('macropath.h',text,'groff-1.06/include', 837 ).
file('posix.h',text,'groff-1.06/include', 1179 ).
file('printer.h',text,'groff-1.06/include', 1894 ).
file('ptable.h',text,'groff-1.06/include', 5409 ).
file('refid.h',text,'groff-1.06/include', 1291 ).
file('search.h',text,'groff-1.06/include', 2770 ).
file('searchpath.h',text,'groff-1.06/include', 1028 ).
file('stringclass.h',text,'groff-1.06/include', 3932 ).
file('Makefile.dep',text,'groff-1.06/indxbib', 232 ).
file('Makefile.sub',text,'groff-1.06/indxbib', 591 ).
file('dirnamemax.c',text,'groff-1.06/indxbib', 852 ).
file('eign',text,'groff-1.06/indxbib', 620 ).
file('indxbib',exec,'groff-1.06/indxbib', 114688 ).
file('indxbib.cc',text,'groff-1.06/indxbib', 17672 ).
file('indxbib.man',text,'groff-1.06/indxbib', 3636 ).
file('indxbib.n',text,'groff-1.06/indxbib', 3479 ).
file('Makefile.dep',text,'groff-1.06/libbib', 575 ).
file('Makefile.sub',text,'groff-1.06/libbib', 112 ).
file('common.cc',text,'groff-1.06/libbib', 1075 ).
file('index.cc',text,'groff-1.06/libbib', 16075 ).
file('libbib.a',text,'groff-1.06/libbib', 67652 ).
file('linear.cc',text,'groff-1.06/libbib', 11236 ).
file('map.c',text,'groff-1.06/libbib', 1646 ).
file('search.cc',text,'groff-1.06/libbib', 2900 ).
file('Makefile.dep',text,'groff-1.06/libdriver', 308 ).
file('Makefile.sub',text,'groff-1.06/libdriver', 61 ).
file('input.cc',text,'groff-1.06/libdriver', 9728 ).
file('libdriver.a',text,'groff-1.06/libdriver', 32982 ).
file('printer.cc',text,'groff-1.06/libdriver', 3937 ).
file('Makefile.dep',text,'groff-1.06/libgroff', 1537 ).
file('Makefile.sub',text,'groff-1.06/libgroff', 794 ).
file('assert.cc',text,'groff-1.06/libgroff', 1159 ).
file('change_lf.cc',text,'groff-1.06/libgroff', 1119 ).
file('cmap.cc',text,'groff-1.06/libgroff', 1466 ).
file('cset.cc',text,'groff-1.06/libgroff', 2419 ).
file('device.cc',text,'groff-1.06/libgroff', 1052 ).
file('errarg.cc',text,'groff-1.06/libgroff', 2152 ).
file('error.cc',text,'groff-1.06/libgroff', 3453 ).
file('fatal.cc',text,'groff-1.06/libgroff', 904 ).
file('filename.cc',text,'groff-1.06/libgroff', 34 ).
file('fmod.c',text,'groff-1.06/libgroff', 930 ).
file('font.cc',text,'groff-1.06/libgroff', 20076 ).
file('fontfile.cc',text,'groff-1.06/libgroff', 1878 ).
file('getcwd.c',text,'groff-1.06/libgroff', 656 ).
file('iftoa.c',text,'groff-1.06/libgroff', 1828 ).
file('illegal.cc',text,'groff-1.06/libgroff', 892 ).
file('itoa.c',text,'groff-1.06/libgroff', 1265 ).
file('lf.cc',text,'groff-1.06/libgroff', 1601 ).
file('libgroff.a',text,'groff-1.06/libgroff', 142760 ).
file('lineno.cc',text,'groff-1.06/libgroff', 24 ).
file('macropath.cc',text,'groff-1.06/libgroff', 986 ).
file('matherr.c',text,'groff-1.06/libgroff', 1098 ).
file('nametoindex.cc',text,'groff-1.06/libgroff', 2909 ).
file('new.cc',text,'groff-1.06/libgroff', 1501 ).
file('prime.cc',text,'groff-1.06/libgroff', 385 ).
file('progname.cc',text,'groff-1.06/libgroff', 30 ).
file('ptable.cc',text,'groff-1.06/libgroff', 1453 ).
file('searchpath.cc',text,'groff-1.06/libgroff', 2842 ).
file('strerror.c',text,'groff-1.06/libgroff', 1147 ).
file('string.cc',text,'groff-1.06/libgroff', 5946 ).
file('strsave.cc',text,'groff-1.06/libgroff', 954 ).
file('strtol.c',text,'groff-1.06/libgroff', 2554 ).
file('tmpfile.cc',text,'groff-1.06/libgroff', 2719 ).
file('version.cc',text,'groff-1.06/libgroff', 37 ).
file('Makefile.dep',text,'groff-1.06/lkbib', 136 ).
file('Makefile.sub',text,'groff-1.06/lkbib', 94 ).
file('lkbib',exec,'groff-1.06/lkbib', 139264 ).
file('lkbib.cc',text,'groff-1.06/lkbib', 3035 ).
file('lkbib.man',text,'groff-1.06/lkbib', 1728 ).
file('lkbib.n',text,'groff-1.06/lkbib', 1672 ).
file('Makefile.dep',text,'groff-1.06/lookbib', 140 ).
file('Makefile.sub',text,'groff-1.06/lookbib', 118 ).
file('lookbib',exec,'groff-1.06/lookbib', 139264 ).
file('lookbib.cc',text,'groff-1.06/lookbib', 2968 ).
file('lookbib.man',text,'groff-1.06/lookbib', 1272 ).
file('lookbib.n',text,'groff-1.06/lookbib', 1183 ).
file('Makefile.sub',text,'groff-1.06/man', 48 ).
file('groff_char.man',text,'groff-1.06/man', 10809 ).
file('groff_char.n',text,'groff-1.06/man', 10795 ).
file('groff_font.man',text,'groff-1.06/man', 8159 ).
file('groff_font.n',text,'groff-1.06/man', 8184 ).
file('groff_out.man',text,'groff-1.06/man', 6002 ).
file('groff_out.n',text,'groff-1.06/man', 5987 ).
file('ChangeLog',text,'groff-1.06/mm', 5685 ).
file('Makefile.sub',text,'groff-1.06/mm', 768 ).
file('NOTES',text,'groff-1.06/mm', 2525 ).
file('README',text,'groff-1.06/mm', 545 ).
file('groff_mm.man',text,'groff-1.06/mm', 14945 ).
file('groff_mm.n',text,'groff-1.06/mm', 14812 ).
file('groff_mmse.man',text,'groff-1.06/mm', 864 ).
file('groff_mmse.n',text,'groff-1.06/mm', 781 ).
file('mm',dir,'groff-1.06/mm', 512 ).
file('tmac.m',text,'groff-1.06/mm', 52193 ).
file('tmac.mse',text,'groff-1.06/mm', 880 ).
file('0.MT',text,'groff-1.06/mm/mm', 2349 ).
file('4.MT',text,'groff-1.06/mm/mm', 905 ).
file('5.MT',text,'groff-1.06/mm/mm', 559 ).
file('ms.cov',text,'groff-1.06/mm/mm', 1268 ).
file('se_ms.cov',text,'groff-1.06/mm/mm', 39 ).
file('Makefile.sub',text,'groff-1.06/nroff', 158 ).
file('nroff.man',text,'groff-1.06/nroff', 706 ).
file('nroff.n',text,'groff-1.06/nroff', 656 ).
file('nroff.sh',exec,'groff-1.06/nroff', 659 ).
file('Makefile.dep',text,'groff-1.06/pfbtops', 22 ).
file('Makefile.sub',text,'groff-1.06/pfbtops', 77 ).
file('pfbtops',exec,'groff-1.06/pfbtops', 24576 ).
file('pfbtops.c',text,'groff-1.06/pfbtops', 2003 ).
file('pfbtops.man',text,'groff-1.06/pfbtops', 657 ).
file('pfbtops.n',text,'groff-1.06/pfbtops', 658 ).
file('Makefile.dep',text,'groff-1.06/pic', 1368 ).
file('Makefile.sub',text,'groff-1.06/pic', 286 ).
file('TODO',text,'groff-1.06/pic', 1209 ).
file('common.cc',text,'groff-1.06/pic', 15742 ).
file('common.h',text,'groff-1.06/pic', 3227 ).
file('depend',text,'groff-1.06/pic', 1195 ).
file('lex.cc',text,'groff-1.06/pic', 39731 ).
file('main.cc',text,'groff-1.06/pic', 12171 ).
file('make-dos-dist',exec,'groff-1.06/pic', 5927 ).
file('object.cc',text,'groff-1.06/pic', 38142 ).
file('object.h',text,'groff-1.06/pic', 5250 ).
file('output.h',text,'groff-1.06/pic', 2709 ).
file('pic',exec,'groff-1.06/pic', 450560 ).
file('pic.cc',text,'groff-1.06/pic', 113772 ).
file('pic.h',text,'groff-1.06/pic', 2377 ).
file('pic.man',text,'groff-1.06/pic', 15351 ).
file('pic.n',text,'groff-1.06/pic', 15340 ).
file('pic.tab.h',text,'groff-1.06/pic', 2478 ).
file('pic.y',text,'groff-1.06/pic', 34611 ).
file('position.h',text,'groff-1.06/pic', 1562 ).
file('tex.cc',text,'groff-1.06/pic', 10415 ).
file('text.h',text,'groff-1.06/pic', 344 ).
file('troff.cc',text,'groff-1.06/pic', 12979 ).
file('y.output',text,'groff-1.06/pic', 333857 ).
file('Makefile.dep',text,'groff-1.06/psbb', 16 ).
file('Makefile.sub',text,'groff-1.06/psbb', 54 ).
file('psbb',exec,'groff-1.06/psbb', 24576 ).
file('psbb.c',text,'groff-1.06/psbb', 4426 ).
file('psbb.man',text,'groff-1.06/psbb', 589 ).
file('psbb.n',text,'groff-1.06/psbb', 574 ).
file('Makefile.dep',text,'groff-1.06/refer', 1044 ).
file('Makefile.sub',text,'groff-1.06/refer', 234 ).
file('command.cc',text,'groff-1.06/refer', 18120 ).
file('command.h',text,'groff-1.06/refer', 1259 ).
file('label.cc',text,'groff-1.06/refer', 50689 ).
file('label.y',text,'groff-1.06/refer', 28606 ).
file('ref.cc',text,'groff-1.06/refer', 27428 ).
file('ref.h',text,'groff-1.06/refer', 4202 ).
file('refer',exec,'groff-1.06/refer', 409600 ).
file('refer.cc',text,'groff-1.06/refer', 28032 ).
file('refer.h',text,'groff-1.06/refer', 2251 ).
file('refer.man',text,'groff-1.06/refer', 27184 ).
file('refer.n',text,'groff-1.06/refer', 27108 ).
file('token.cc',text,'groff-1.06/refer', 8865 ).
file('token.h',text,'groff-1.06/refer', 2121 ).
file('y.output',text,'groff-1.06/refer', 13359 ).
file('Makefile.dep',text,'groff-1.06/soelim', 106 ).
file('Makefile.sub',text,'groff-1.06/soelim', 91 ).
file('soelim',exec,'groff-1.06/soelim', 81920 ).
file('soelim.cc',text,'groff-1.06/soelim', 5424 ).
file('soelim.man',text,'groff-1.06/soelim', 653 ).
file('soelim.n',text,'groff-1.06/soelim', 628 ).
file('Makefile.dep',text,'groff-1.06/tbl', 302 ).
file('Makefile.sub',text,'groff-1.06/tbl', 111 ).
file('main.cc',text,'groff-1.06/tbl', 30763 ).
file('table.cc',text,'groff-1.06/tbl', 70651 ).
file('table.h',text,'groff-1.06/tbl', 3656 ).
file('tbl',exec,'groff-1.06/tbl', 270336 ).
file('tbl.man',text,'groff-1.06/tbl', 2491 ).
file('tbl.n',text,'groff-1.06/tbl', 2460 ).
file('Makefile.dep',text,'groff-1.06/tfmtodit', 123 ).
file('Makefile.sub',text,'groff-1.06/tfmtodit', 96 ).
file('tfmtodit',exec,'groff-1.06/tfmtodit', 106496 ).
file('tfmtodit.cc',text,'groff-1.06/tfmtodit', 18749 ).
file('tfmtodit.man',text,'groff-1.06/tfmtodit', 3443 ).
file('tfmtodit.n',text,'groff-1.06/tfmtodit', 3460 ).
file('Makefile.sub',text,'groff-1.06/tmac', 1119 ).
file('TODO',text,'groff-1.06/tmac', 970 ).
file('doc-common',text,'groff-1.06/tmac', 10970 ).
file('doc-ditroff',text,'groff-1.06/tmac', 5650 ).
file('doc-nroff',text,'groff-1.06/tmac', 4831 ).
file('doc-syms',text,'groff-1.06/tmac', 6432 ).
file('eqnrc',text,'groff-1.06/tmac', 1153 ).
file('fixmacros.sed',text,'groff-1.06/tmac', 298 ).
file('groff_ms.man',text,'groff-1.06/tmac', 4921 ).
file('groff_ms.n',text,'groff-1.06/tmac', 4861 ).
file('man.local',text,'groff-1.06/tmac', 88 ).
file('man.ultrix',text,'groff-1.06/tmac', 279 ).
file('me.man',text,'groff-1.06/tmac', 6020 ).
file('me.n',text,'groff-1.06/tmac', 6010 ).
file('mm.diff',text,'groff-1.06/tmac', 1290 ).
file('strip.sed',text,'groff-1.06/tmac', 39 ).
file('tmac.X',text,'groff-1.06/tmac', 1450 ).
file('tmac.Xps',text,'groff-1.06/tmac', 1272 ).
file('tmac.an',text,'groff-1.06/tmac', 6281 ).
file('tmac.andoc',text,'groff-1.06/tmac', 159 ).
file('tmac.doc',text,'groff-1.06/tmac', 66969 ).
file('tmac.doc.old',text,'groff-1.06/tmac', 36437 ).
file('tmac.dvi',text,'groff-1.06/tmac', 4038 ).
file('tmac.e',text,'groff-1.06/tmac', 29678 ).
file('tmac.latin1',text,'groff-1.06/tmac', 2645 ).
file('tmac.pic',text,'groff-1.06/tmac', 101 ).
file('tmac.ps',text,'groff-1.06/tmac', 1366 ).
file('tmac.psatk',text,'groff-1.06/tmac', 1061 ).
file('tmac.psfig',text,'groff-1.06/tmac', 1228 ).
file('tmac.psnew',text,'groff-1.06/tmac', 807 ).
file('tmac.psold',text,'groff-1.06/tmac', 1861 ).
file('tmac.pspic',text,'groff-1.06/tmac', 1167 ).
file('tmac.s',text,'groff-1.06/tmac', 32749 ).
file('tmac.tty',text,'groff-1.06/tmac', 907 ).
file('tmac.tty-char',text,'groff-1.06/tmac', 4109 ).
file('troffrc',text,'groff-1.06/tmac', 659 ).
file('Makefile.dep',text,'groff-1.06/troff', 1960 ).
file('Makefile.sub',text,'groff-1.06/troff', 830 ).
file('TODO',text,'groff-1.06/troff', 4936 ).
file('charinfo.h',text,'groff-1.06/troff', 3337 ).
file('column.cc',text,'groff-1.06/troff', 14405 ).
file('dictionary.cc',text,'groff-1.06/troff', 4486 ).
file('dictionary.h',text,'groff-1.06/troff', 2265 ).
file('div.cc',text,'groff-1.06/troff', 27014 ).
file('div.h',text,'groff-1.06/troff', 4510 ).
file('env.cc',text,'groff-1.06/troff', 66917 ).
file('env.h',text,'groff-1.06/troff', 9074 ).
file('hvunits.h',text,'groff-1.06/troff', 6654 ).
file('hyphen',text,'groff-1.06/troff', 27302 ).
file('input.cc',text,'groff-1.06/troff', 116659 ).
file('majorminor.cc',text,'groff-1.06/troff', 67 ).
file('node.cc',text,'groff-1.06/troff', 95196 ).
file('node.h',text,'groff-1.06/troff', 11625 ).
file('number.cc',text,'groff-1.06/troff', 13104 ).
file('reg.cc',text,'groff-1.06/troff', 8536 ).
file('reg.h',text,'groff-1.06/troff', 1907 ).
file('request.h',text,'groff-1.06/troff', 2169 ).
file('symbol.cc',text,'groff-1.06/troff', 3767 ).
file('symbol.h',text,'groff-1.06/troff', 1694 ).
file('token.h',text,'groff-1.06/troff', 4500 ).
file('troff',exec,'groff-1.06/troff', 1040384 ).
file('troff.h',text,'groff-1.06/troff', 2164 ).
file('troff.man',text,'groff-1.06/troff', 41234 ).
file('troff.n',text,'groff-1.06/troff', 41356 ).
file('ChangeLog',text,'groff-1.06/xditview', 7035 ).
file('DESC',text,'groff-1.06/xditview', 116 ).
file('Dvi.c',text,'groff-1.06/xditview', 14501 ).
file('Dvi.h',text,'groff-1.06/xditview', 1228 ).
file('DviChar.c',text,'groff-1.06/xditview', 13605 ).
file('DviChar.h',text,'groff-1.06/xditview', 1055 ).
file('DviP.h',text,'groff-1.06/xditview', 4352 ).
file('FontMap',text,'groff-1.06/xditview', 1024 ).
file('GXditview.ad',text,'groff-1.06/xditview', 1903 ).
file('INSTALL',text,'groff-1.06/xditview', 522 ).
file('Imakefile',text,'groff-1.06/xditview', 1632 ).
file('Makefile',text,'groff-1.06/xditview', 22251 ).
file('Menu.h',text,'groff-1.06/xditview', 1282 ).
file('README',text,'groff-1.06/xditview', 515 ).
file('TODO',text,'groff-1.06/xditview', 451 ).
file('XFontName.c',text,'groff-1.06/xditview', 6434 ).
file('XFontName.h',text,'groff-1.06/xditview', 1205 ).
file('device.c',text,'groff-1.06/xditview', 11033 ).
file('device.h',text,'groff-1.06/xditview', 455 ).
file('draw.c',text,'groff-1.06/xditview', 15091 ).
file('font.c',text,'groff-1.06/xditview', 9009 ).
file('gxditview.man',text,'groff-1.06/xditview', 5057 ).
file('lex.c',text,'groff-1.06/xditview', 1597 ).
file('page.c',text,'groff-1.06/xditview', 1404 ).
file('parse.c',text,'groff-1.06/xditview', 6236 ).
file('xdit.bm',text,'groff-1.06/xditview', 875 ).
file('xdit_mask.bm',text,'groff-1.06/xditview', 890 ).
file('xditview.c',text,'groff-1.06/xditview', 14866 ).
file('xtotroff.c',text,'groff-1.06/xditview', 6609 ).
file('COPYING',text,'gs26', 17982 ).
file('Fontmap',text,'gs26', 11274 ).
file('Fontmap.ATM',text,'gs26', 4538 ).
file('Fontmap.VMS',text,'gs26', 4943 ).
file('NEWS',text,'gs26', 69949 ).
file('README',text,'gs26', 13224 ).
file('alloc.h',text,'gs26', 1629 ).
file('alphabet.ps',text,'gs26', 1439 ).
file('ansi2knr.c',text,'gs26', 8214 ).
file('ansihead.mak',text,'gs26', 4611 ).
file('astate.h',text,'gs26', 4090 ).
file('bc.mak',text,'gs26', 8834 ).
file('bcwin.mak',text,'gs26', 8150 ).
file('bdftops',exec,'gs26', 35 ).
file('bdftops.bat',exec,'gs26', 61 ).
file('bdftops.ps',text,'gs26', 22991 ).
file('bench.ps',text,'gs26', 2429 ).
file('bfont.h',text,'gs26', 1554 ).
file('bnum.h',text,'gs26', 2117 ).
file('bseq.h',text,'gs26', 1501 ).
file('btoken.h',text,'gs26', 1794 ).
file('cc-head.mak',text,'gs26', 4415 ).
file('ccfont.h',text,'gs26', 2886 ).
file('ccgs',exec,'gs26', 362 ).
file('cfonts.mak',text,'gs26', 11038 ).
file('cheq.ps',text,'gs26', 58877 ).
file('chess.ps',text,'gs26', 3047 ).
file('colorcir.ps',text,'gs26', 1815 ).
file('cp.bat',exec,'gs26', 16 ).
file('ctype_.h',text,'gs26', 1140 ).
file('decrypt.ps',text,'gs26', 294 ).
file('devices.doc',text,'gs26', 12322 ).
file('devs.mak',text,'gs26', 27336 ).
file('dict.h',text,'gs26', 4208 ).
file('dodebug.h',text,'gs26', 407 ).
file('dos_.h',text,'gs26', 2918 ).
file('dparam.h',text,'gs26', 1691 ).
file('drivers.doc',text,'gs26', 27998 ).
file('dstack.h',text,'gs26', 2034 ).
file('echogs.c',text,'gs26', 5256 ).
file('ega.c',text,'gs26', 7276 ).
file('ega.tr',text,'gs26', 32 ).
file('errno_.h',text,'gs26', 1314 ).
file('errors.h',text,'gs26', 4423 ).
file('escher.ps',text,'gs26', 10520 ).
file('estack.h',text,'gs26', 4023 ).
file('filedev.h',text,'gs26', 3948 ).
file('files.h',text,'gs26', 3751 ).
file('font.h',text,'gs26', 1691 ).
file('font2c',exec,'gs26', 52 ).
file('font2c.bat',exec,'gs26', 78 ).
file('font2c.ps',text,'gs26', 15038 ).
file('fonts',dir,'gs26', 2048 ).
file('fonts.doc',text,'gs26', 12074 ).
file('fonts.mak',text,'gs26', 14358 ).
file('gcc-head.mak',text,'gs26', 4748 ).
file('gconfig.c',text,'gs26', 2407 ).
file('gdebug.h',text,'gs26', 3343 ).
file('gdev3852.c',text,'gs26', 5910 ).
file('gdev3b1.c',text,'gs26', 21121 ).
file('gdev4081.c',text,'gs26', 2943 ).
file('gdev4693.c',text,'gs26', 4996 ).
file('gdev8510.c',text,'gs26', 4174 ).
file('gdevbgi.c',text,'gs26', 18504 ).
file('gdevbit.c',text,'gs26', 2056 ).
file('gdevbj10.c',text,'gs26', 11719 ).
file('gdevbmp.c',text,'gs26', 7961 ).
file('gdevcdj.c',text,'gs26', 46961 ).
file('gdevdfax.c',text,'gs26', 24978 ).
file('gdevdfg3.h',text,'gs26', 6845 ).
file('gdevdjet.c',text,'gs26', 15782 ).
file('gdevdjtc.c',text,'gs26', 10353 ).
file('gdevegaa.asm',text,'gs26', 6768 ).
file('gdevemap.c',text,'gs26', 3102 ).
file('gdevepsc.c',text,'gs26', 13670 ).
file('gdevepsn.c',text,'gs26', 12853 ).
file('gdevevga.c',text,'gs26', 1598 ).
file('gdevgif.c',text,'gs26', 12429 ).
file('gdevherc.c',text,'gs26', 11534 ).
file('gdevlbp8.c',text,'gs26', 3893 ).
file('gdevln03.c',text,'gs26', 6373 ).
file('gdevmem.h',text,'gs26', 10069 ).
file('gdevmem1.c',text,'gs26', 17540 ).
file('gdevmem2.c',text,'gs26', 15562 ).
file('gdevmem3.c',text,'gs26', 9136 ).
file('gdevmswn.c',text,'gs26', 21424 ).
file('gdevmswn.h',text,'gs26', 3752 ).
file('gdevmsxf.c',text,'gs26', 13326 ).
file('gdevn533.c',text,'gs26', 5444 ).
file('gdevnp6.c',text,'gs26', 7597 ).
file('gdevo182.c',text,'gs26', 8341 ).
file('gdevpbm.c',text,'gs26', 12711 ).
file('gdevpccm.c',text,'gs26', 4253 ).
file('gdevpccm.h',text,'gs26', 1578 ).
file('gdevpcfb.c',text,'gs26', 22299 ).
file('gdevpcfb.h',text,'gs26', 5515 ).
file('gdevpcl.c',text,'gs26', 7753 ).
file('gdevpcl.h',text,'gs26', 1839 ).
file('gdevpcx.c',text,'gs26', 8390 ).
file('gdevpe.c',text,'gs26', 7762 ).
file('gdevpipe.c',text,'gs26', 1725 ).
file('gdevpjet.c',text,'gs26', 8357 ).
file('gdevprn.c',text,'gs26', 12031 ).
file('gdevprn.h',text,'gs26', 8259 ).
file('gdevs3ga.c',text,'gs26', 7378 ).
file('gdevsco.c',text,'gs26', 5712 ).
file('gdevsnfb.c',text,'gs26', 3290 ).
file('gdevsppr.c',text,'gs26', 6198 ).
file('gdevsun.c',text,'gs26', 20803 ).
file('gdevsvga.c',text,'gs26', 20809 ).
file('gdevsvga.h',text,'gs26', 2865 ).
file('gdevtiff.c',text,'gs26', 28056 ).
file('gdevtiff.h',text,'gs26', 12929 ).
file('gdevtknk.c',text,'gs26', 8679 ).
file('gdevtrfx.c',text,'gs26', 2333 ).
file('gdevwddb.c',text,'gs26', 17379 ).
file('gdevwdib.c',text,'gs26', 12345 ).
file('gdevwprn.c',text,'gs26', 18611 ).
file('gdevx.c',text,'gs26', 28933 ).
file('gdevx.h',text,'gs26', 4575 ).
file('gdevxini.c',text,'gs26', 23492 ).
file('gdevxxf.c',text,'gs26', 12896 ).
file('genarch.c',text,'gs26', 4513 ).
file('ghost.h',text,'gs26', 1019 ).
file('golfer.ps',text,'gs26', 25499 ).
file('gp.h',text,'gs26', 6430 ).
file('gp_dosfb.c',text,'gs26', 5566 ).
file('gp_itbc.c',text,'gs26', 6719 ).
file('gp_iwatc.c',text,'gs26', 4346 ).
file('gp_msdos.c',text,'gs26', 7375 ).
file('gp_mswin.c',text,'gs26', 18087 ).
file('gp_mswin.h',text,'gs26', 2107 ).
file('gp_mswtx.c',text,'gs26', 20333 ).
file('gp_mswtx.h',text,'gs26', 1885 ).
file('gp_nofb.c',text,'gs26', 1507 ).
file('gp_sysv.c',text,'gs26', 2005 ).
file('gp_unix.c',text,'gs26', 7301 ).
file('gp_vms.c',text,'gs26', 11503 ).
file('gpcheck.h',text,'gs26', 2080 ).
file('gs.c',text,'gs26', 14199 ).
file('gs.h',text,'gs26', 1071 ).
file('gs.mak',text,'gs26', 29848 ).
file('gs.tr',text,'gs26', 642 ).
file('gs_2asc.ps',text,'gs26', 11875 ).
file('gs_dps1.ps',text,'gs26', 9938 ).
file('gs_fonts.ps',text,'gs26', 16706 ).
file('gs_init.ps',text,'gs26', 27977 ).
file('gs_lev2.ps',text,'gs26', 11171 ).
file('gs_statd.ps',text,'gs26', 6551 ).
file('gs_sym_e.ps',text,'gs26', 3801 ).
file('gs_type0.ps',text,'gs26', 1812 ).
file('gsaddmod',exec,'gs26', 777 ).
file('gsaddmod.bat',exec,'gs26', 1354 ).
file('gsbj',exec,'gs26', 51 ).
file('gsbj.bat',exec,'gs26', 89 ).
file('gsccode.h',text,'gs26', 1737 ).
file('gschar.c',text,'gs26', 23347 ).
file('gschar.h',text,'gs26', 4666 ).
file('gschar0.c',text,'gs26', 6417 ).
file('gscie.c',text,'gs26', 14445 ).
file('gscie.h',text,'gs26', 7361 ).
file('gscolor.c',text,'gs26', 10965 ).
file('gscolor.h',text,'gs26', 2506 ).
file('gscolor2.c',text,'gs26', 7846 ).
file('gscolor2.h',text,'gs26', 2036 ).
file('gsconfig',exec,'gs26', 728 ).
file('gsconfig.bat',exec,'gs26', 1073 ).
file('gsconfig.h',text,'gs26', 2203 ).
file('gscoord.c',text,'gs26', 8823 ).
file('gscoord.h',text,'gs26', 2066 ).
file('gscrypt1.h',text,'gs26', 1725 ).
file('gscspace.h',text,'gs26', 5030 ).
file('gsdevice.c',text,'gs26', 18324 ).
file('gsdj',exec,'gs26', 53 ).
file('gsdj.bat',exec,'gs26', 89 ).
file('gsdj500.bat',exec,'gs26', 95 ).
file('gsdps1.c',text,'gs26', 4011 ).
file('gserrors.h',text,'gs26', 1657 ).
file('gsfile.c',text,'gs26', 4013 ).
file('gsfont.c',text,'gs26', 9076 ).
file('gsfont.h',text,'gs26', 2692 ).
file('gsgraph.icx',text,'gs26', 1580 ).
file('gsht.c',text,'gs26', 9410 ).
file('gsimage.c',text,'gs26', 14090 ).
file('gsimage.h',text,'gs26', 1872 ).
file('gsimage1.c',text,'gs26', 15813 ).
file('gsimage2.c',text,'gs26', 8074 ).
file('gsimpath.c',text,'gs26', 6041 ).
file('gsio.h',text,'gs26', 2105 ).
file('gsline.c',text,'gs26', 4438 ).
file('gslj',exec,'gs26', 54 ).
file('gslj.bat',exec,'gs26', 92 ).
file('gslp',exec,'gs26', 50 ).
file('gslp.bat',exec,'gs26', 89 ).
file('gslp.ps',text,'gs26', 13945 ).
file('gsmain.c',text,'gs26', 8495 ).
file('gsmatrix.c',text,'gs26', 9554 ).
file('gsmatrix.h',text,'gs26', 2974 ).
file('gsmisc.c',text,'gs26', 8926 ).
file('gsnd',exec,'gs26', 18 ).
file('gsnd.bat',exec,'gs26', 43 ).
file('gsndt.bat',exec,'gs26', 47 ).
file('gspaint.c',text,'gs26', 3102 ).
file('gspaint.h',text,'gs26', 1246 ).
file('gspath.c',text,'gs26', 9200 ).
file('gspath.h',text,'gs26', 2563 ).
file('gspath2.c',text,'gs26', 7883 ).
file('gsprops.h',text,'gs26', 4212 ).
file('gssetdev',exec,'gs26', 36 ).
file('gssetdev.bat',exec,'gs26', 80 ).
file('gssetmod',exec,'gs26', 344 ).
file('gssetmod.bat',exec,'gs26', 538 ).
file('gsstate.c',text,'gs26', 13320 ).
file('gsstate.h',text,'gs26', 5170 ).
file('gst.bat',exec,'gs26', 35 ).
file('gstdev.c',text,'gs26', 9672 ).
file('gstext.icx',text,'gs26', 1580 ).
file('gstt.bat',exec,'gs26', 36 ).
file('gstype1.c',text,'gs26', 21911 ).
file('gstype1.h',text,'gs26', 3641 ).
file('gstypes.h',text,'gs26', 2720 ).
file('gsuid.h',text,'gs26', 2175 ).
file('gsutil.c',text,'gs26', 4980 ).
file('gsutil.h',text,'gs26', 1938 ).
file('gsview.bat',exec,'gs26', 87 ).
file('gsw.tr',text,'gs26', 694 ).
file('gswin.def',text,'gs26', 650 ).
file('gswin.rc',text,'gs26', 921 ).
file('gsxfont.h',text,'gs26', 1483 ).
file('gx.h',text,'gs26', 1308 ).
file('gxarith.h',text,'gs26', 2222 ).
file('gxbitmap.h',text,'gs26', 2587 ).
file('gxcache.h',text,'gs26', 4547 ).
file('gxccache.c',text,'gs26', 9065 ).
file('gxccman.c',text,'gs26', 14462 ).
file('gxcdir.h',text,'gs26', 2454 ).
file('gxchar.h',text,'gs26', 4568 ).
file('gxcht.c',text,'gs26', 5811 ).
file('gxcldev.h',text,'gs26', 4785 ).
file('gxclist.c',text,'gs26', 24680 ).
file('gxclist.h',text,'gs26', 3897 ).
file('gxclread.c',text,'gs26', 13296 ).
file('gxcmap.c',text,'gs26', 14995 ).
file('gxcolor.h',text,'gs26', 2281 ).
file('gxcpath.c',text,'gs26', 22624 ).
file('gxcpath.h',text,'gs26', 1680 ).
file('gxdevice.h',text,'gs26', 9215 ).
file('gxdevmem.h',text,'gs26', 4049 ).
file('gxdither.c',text,'gs26', 11200 ).
file('gxdraw.c',text,'gs26', 13871 ).
file('gxfdir.h',text,'gs26', 1385 ).
file('gxfill.c',text,'gs26', 23017 ).
file('gxfixed.h',text,'gs26', 4537 ).
file('gxfmap.h',text,'gs26', 1699 ).
file('gxfont.h',text,'gs26', 5840 ).
file('gxfrac.h',text,'gs26', 1855 ).
file('gxhint1.c',text,'gs26', 7414 ).
file('gxhint2.c',text,'gs26', 11760 ).
file('gxht.c',text,'gs26', 12733 ).
file('gximage.h',text,'gs26', 6189 ).
file('gxlum.h',text,'gs26', 1203 ).
file('gxmatrix.h',text,'gs26', 2636 ).
file('gxop1.h',text,'gs26', 1676 ).
file('gxpath.c',text,'gs26', 13273 ).
file('gxpath.h',text,'gs26', 4385 ).
file('gxpath2.c',text,'gs26', 7103 ).
file('gxpcopy.c',text,'gs26', 17014 ).
file('gxrefct.h',text,'gs26', 2749 ).
file('gxstroke.c',text,'gs26', 25811 ).
file('gxtype1.h',text,'gs26', 6506 ).
file('gxxfont.h',text,'gs26', 4956 ).
file('gzcolor.h',text,'gs26', 3671 ).
file('gzdevice.h',text,'gs26', 1309 ).
file('gzht.h',text,'gs26', 1799 ).
file('gzline.h',text,'gs26', 1605 ).
file('gzpath.h',text,'gs26', 5881 ).
file('gzstate.h',text,'gs26', 3438 ).
file('helpers.doc',text,'gs26', 5913 ).
file('hershey.doc',text,'gs26', 11176 ).
file('history.doc',text,'gs26', 46517 ).
file('humor.doc',text,'gs26', 2558 ).
file('ialloc.c',text,'gs26', 13114 ).
file('ibnum.c',text,'gs26', 6223 ).
file('iccfont.c',text,'gs26', 7851 ).
file('idebug.c',text,'gs26', 6910 ).
file('idict.c',text,'gs26', 21890 ).
file('idparam.c',text,'gs26', 5394 ).
file('iinit.c',text,'gs26', 9708 ).
file('ilevel.h',text,'gs26', 1151 ).
file('impath.ps',text,'gs26', 5731 ).
file('iname.c',text,'gs26', 8653 ).
file('iname.h',text,'gs26', 2948 ).
file('interp.c',text,'gs26', 27028 ).
file('iref.h',text,'gs26', 8237 ).
file('isave.c',text,'gs26', 17611 ).
file('iscan.c',text,'gs26', 28501 ).
file('iscan.h',text,'gs26', 1424 ).
file('iscan2.c',text,'gs26', 11817 ).
file('istack.c',text,'gs26', 2058 ).
file('istack.h',text,'gs26', 2260 ).
file('iutil.c',text,'gs26', 11539 ).
file('iutil.h',text,'gs26', 3085 ).
file('iutilasm.asm',text,'gs26', 8798 ).
file('ivmspace.h',text,'gs26', 2124 ).
file('landscap.ps',text,'gs26', 1455 ).
file('language.doc',text,'gs26', 15888 ).
file('level1.ps',text,'gs26', 56 ).
file('lib.doc',text,'gs26', 3772 ).
file('lines.ps',text,'gs26', 2648 ).
file('lprsetup.sh',exec,'gs26', 4602 ).
file('main.h',text,'gs26', 2196 ).
file('maint.mak',text,'gs26', 8126 ).
file('make.doc',text,'gs26', 25705 ).
file('malloc_.h',text,'gs26', 1707 ).
file('markpath.ps',text,'gs26', 2096 ).
file('math_.h',text,'gs26', 1830 ).
file('memory_.h',text,'gs26', 2971 ).
file('mergeini.ps',text,'gs26', 2350 ).
file('msc.mak',text,'gs26', 7344 ).
file('mv.bat',exec,'gs26', 15 ).
file('n.h',text,'gs26', 85 ).
file('nalloc.h',text,'gs26', 7036 ).
file('nmman.h',text,'gs26', 5796 ).
file('npacked.h',text,'gs26', 4510 ).
file('nsave.h',text,'gs26', 1894 ).
file('ntest.mak',text,'gs26', 1226 ).
file('opdef.h',text,'gs26', 3788 ).
file('oper.h',text,'gs26', 4957 ).
file('ostack.h',text,'gs26', 1576 ).
file('overlay.h',text,'gs26', 1820 ).
file('packed.h',text,'gs26', 4239 ).
file('pcharstr.ps',text,'gs26', 2118 ).
file('pj-gs.sh',exec,'gs26', 6672 ).
file('ppath.ps',text,'gs26', 1850 ).
file('prfont.ps',text,'gs26', 2368 ).
file('ps2ascii',exec,'gs26', 560 ).
file('ps2ascii.bat',exec,'gs26', 325 ).
file('ps2epsi',exec,'gs26', 807 ).
file('ps2epsi.bat',exec,'gs26', 656 ).
file('ps2epsi.doc',text,'gs26', 2517 ).
file('ps2epsi.ps',text,'gs26', 6124 ).
file('ps2image.ps',text,'gs26', 6187 ).
file('psfiles.doc',text,'gs26', 5269 ).
file('pstoppm.ps',text,'gs26', 7343 ).
file('pv.sh',exec,'gs26', 455 ).
file('quit.ps',text,'gs26', 5 ).
file('readme.doc',text,'gs26', 4292 ).
file('rm.bat',exec,'gs26', 104 ).
file('save.h',text,'gs26', 1940 ).
file('save.ps',text,'gs26', 9 ).
file('sbits.c',text,'gs26', 2459 ).
file('sbits.h',text,'gs26', 3920 ).
file('scanchar.h',text,'gs26', 1958 ).
file('scf.h',text,'gs26', 5181 ).
file('scfd.c',text,'gs26', 17635 ).
file('scfdgen.c',text,'gs26', 7889 ).
file('scfdtab.c',text,'gs26', 11322 ).
file('scfe.c',text,'gs26', 9261 ).
file('scftab.c',text,'gs26', 6876 ).
file('screen.ps',text,'gs26', 726 ).
file('sdct.h',text,'gs26', 1288 ).
file('sdctd.c',text,'gs26', 1420 ).
file('sdcte.c',text,'gs26', 1503 ).
file('sfilter.c',text,'gs26', 9816 ).
file('sfilter2.c',text,'gs26', 7436 ).
file('shc.h',text,'gs26', 2043 ).
file('showchar.ps',text,'gs26', 2767 ).
file('showpage.ps',text,'gs26', 9 ).
file('slzwd.c',text,'gs26', 6789 ).
file('slzwe.c',text,'gs26', 7613 ).
file('snowflak.ps',text,'gs26', 2071 ).
file('stat_.h',text,'gs26', 1460 ).
file('state.h',text,'gs26', 3924 ).
file('std.h',text,'gs26', 15246 ).
file('stdio_.h',text,'gs26', 1325 ).
file('store.h',text,'gs26', 6971 ).
file('stream.c',text,'gs26', 13205 ).
file('stream.h',text,'gs26', 12466 ).
file('string_.h',text,'gs26', 1465 ).
file('sysvlp.sh',exec,'gs26', 861 ).
file('tar_cat',exec,'gs26', 218 ).
file('tc.mak',text,'gs26', 6468 ).
file('tccommon.mak',text,'gs26', 3312 ).
file('tiger.ps',text,'gs26', 78519 ).
file('time_.h',text,'gs26', 1827 ).
file('traceop.ps',text,'gs26', 2524 ).
file('turboc.cfg',text,'gs26', 178 ).
file('type1ops.ps',text,'gs26', 6879 ).
file('uglyr.gsf',text,'gs26', 22643 ).
file('uniq.c',text,'gs26', 1850 ).
file('unix-ansi.mak',text,'gs26', 67818 ).
file('unix-cc.mak',text,'gs26', 67622 ).
file('unix-gcc.mak',text,'gs26', 67955 ).
file('unix-lpr.doc',text,'gs26', 5698 ).
file('unix-lpr.sh',exec,'gs26', 3926 ).
file('unixhead.mak',text,'gs26', 1721 ).
file('unixtail.mak',text,'gs26', 4302 ).
file('unprot.ps',text,'gs26', 1862 ).
file('use.doc',text,'gs26', 21572 ).
file('vms-cc.mak',text,'gs26', 6456 ).
file('vms-gcc.mak',text,'gs26', 6598 ).
file('vmsmath.h',text,'gs26', 1559 ).
file('watc.mak',text,'gs26', 5940 ).
file('watcwin.mak',text,'gs26', 5964 ).
file('waterfal.ps',text,'gs26', 2489 ).
file('wccommon.mak',text,'gs26', 3309 ).
file('windows_.h',text,'gs26', 1337 ).
file('winmaps.ps',text,'gs26', 3982 ).
file('wrfont.ps',text,'gs26', 9518 ).
file('x_.h',text,'gs26', 5553 ).
file('xfonts.doc',text,'gs26', 6554 ).
file('zarith.c',text,'gs26', 8257 ).
file('zarray.c',text,'gs26', 2866 ).
file('zbseq.c',text,'gs26', 8141 ).
file('zchar.c',text,'gs26', 13336 ).
file('zchar2.c',text,'gs26', 6755 ).
file('zcie.c',text,'gs26', 20961 ).
file('zcolor.c',text,'gs26', 8726 ).
file('zcolor2.c',text,'gs26', 6198 ).
file('zcontext.c',text,'gs26', 13389 ).
file('zcontrol.c',text,'gs26', 10446 ).
file('zcspace2.c',text,'gs26', 8713 ).
file('zdevice.c',text,'gs26', 5494 ).
file('zdict.c',text,'gs26', 6625 ).
file('zdosio.c',text,'gs26', 2629 ).
file('zdps1.c',text,'gs26', 9214 ).
file('zfile.c',text,'gs26', 21766 ).
file('zfiledev.c',text,'gs26', 8846 ).
file('zfileio.c',text,'gs26', 11221 ).
file('zfilter.c',text,'gs26', 5788 ).
file('zfilter2.c',text,'gs26', 10745 ).
file('zfont.c',text,'gs26', 9043 ).
file('zfont0.c',text,'gs26', 7282 ).
file('zfont1.c',text,'gs26', 7712 ).
file('zfont2.c',text,'gs26', 11490 ).
file('zgeneric.c',text,'gs26', 12489 ).
file('zgstate.c',text,'gs26', 7209 ).
file('zht.c',text,'gs26', 4414 ).
file('zht2.c',text,'gs26', 9238 ).
file('zimage2.c',text,'gs26', 5692 ).
file('zmath.c',text,'gs26', 5121 ).
file('zmatrix.c',text,'gs26', 7118 ).
file('zmisc.c',text,'gs26', 7623 ).
file('zmisc2.c',text,'gs26', 11737 ).
file('zpacked.c',text,'gs26', 5669 ).
file('zpaint.c',text,'gs26', 9756 ).
file('zpath.c',text,'gs26', 4921 ).
file('zpath2.c',text,'gs26', 4932 ).
file('zprops.c',text,'gs26', 7693 ).
file('zrelbit.c',text,'gs26', 6203 ).
file('zstack.c',text,'gs26', 5015 ).
file('zstring.c',text,'gs26', 4646 ).
file('ztype.c',text,'gs26', 8787 ).
file('zupath.c',text,'gs26', 12138 ).
file('zvmem.c',text,'gs26', 6219 ).
file('zvmem2.c',text,'gs26', 1731 ).
file('bchb.afm',text,'gs26/fonts', 21626 ).
file('bchb.gsf',text,'gs26/fonts', 52032 ).
file('bchbi.afm',text,'gs26/fonts', 22099 ).
file('bchbi.gsf',text,'gs26/fonts', 54791 ).
file('bchr.afm',text,'gs26/fonts', 19764 ).
file('bchr.gsf',text,'gs26/fonts', 53462 ).
file('bchri.afm',text,'gs26/fonts', 19927 ).
file('bchri.gsf',text,'gs26/fonts', 54602 ).
file('cyr.gsf',text,'gs26/fonts', 35905 ).
file('cyri.gsf',text,'gs26/fonts', 44432 ).
file('hrge_r.gsf',text,'gs26/fonts', 12621 ).
file('hrge_rb.gsf',text,'gs26/fonts', 109 ).
file('hrge_ro.gsf',text,'gs26/fonts', 114 ).
file('hrgk_c.gsf',text,'gs26/fonts', 8889 ).
file('hrgk_s.gsf',text,'gs26/fonts', 6961 ).
file('hrgr_r.gsf',text,'gs26/fonts', 13145 ).
file('hrgr_rb.gsf',text,'gs26/fonts', 107 ).
file('hrgr_ro.gsf',text,'gs26/fonts', 112 ).
file('hrit_r.gsf',text,'gs26/fonts', 11167 ).
file('hrit_rb.gsf',text,'gs26/fonts', 109 ).
file('hrit_ro.gsf',text,'gs26/fonts', 114 ).
file('hrpl_r.gsf',text,'gs26/fonts', 8964 ).
file('hrpl_rb.gsf',text,'gs26/fonts', 91 ).
file('hrpl_ro.gsf',text,'gs26/fonts', 96 ).
file('hrpl_s.gsf',text,'gs26/fonts', 6675 ).
file('hrpl_sb.gsf',text,'gs26/fonts', 107 ).
file('hrpl_sbo.gsf',text,'gs26/fonts', 117 ).
file('hrpl_so.gsf',text,'gs26/fonts', 112 ).
file('hrpl_t.gsf',text,'gs26/fonts', 11852 ).
file('hrpl_tb.gsf',text,'gs26/fonts', 107 ).
file('hrpl_tbi.gsf',text,'gs26/fonts', 116 ).
file('hrpl_ti.gsf',text,'gs26/fonts', 11852 ).
file('hrsc_c.gsf',text,'gs26/fonts', 9597 ).
file('hrsc_cb.gsf',text,'gs26/fonts', 109 ).
file('hrsc_co.gsf',text,'gs26/fonts', 114 ).
file('hrsc_s.gsf',text,'gs26/fonts', 8097 ).
file('hrsc_sb.gsf',text,'gs26/fonts', 109 ).
file('hrsc_so.gsf',text,'gs26/fonts', 114 ).
file('hrsy_r.gsf',text,'gs26/fonts', 13602 ).
file('n019003l.afm',text,'gs26/fonts', 13603 ).
file('n019003l.gsf',text,'gs26/fonts', 58388 ).
file('n021003l.afm',text,'gs26/fonts', 13738 ).
file('n021003l.gsf',text,'gs26/fonts', 77752 ).
file('ncrb.gsf',text,'gs26/fonts', 97891 ).
file('ncrbi.gsf',text,'gs26/fonts', 96987 ).
file('ncrr.gsf',text,'gs26/fonts', 97281 ).
file('ncrri.gsf',text,'gs26/fonts', 94824 ).
file('pagd.gsf',text,'gs26/fonts', 33020 ).
file('pagdo.gsf',text,'gs26/fonts', 35624 ).
file('pagk.gsf',text,'gs26/fonts', 55105 ).
file('pagko.gsf',text,'gs26/fonts', 36778 ).
file('pbkd.gsf',text,'gs26/fonts', 36435 ).
file('pbkdi.gsf',text,'gs26/fonts', 36365 ).
file('pbkl.gsf',text,'gs26/fonts', 36048 ).
file('pbkli.gsf',text,'gs26/fonts', 38419 ).
file('phvb.gsf',text,'gs26/fonts', 70304 ).
file('phvbo.gsf',text,'gs26/fonts', 80025 ).
file('phvr.gsf',text,'gs26/fonts', 71002 ).
file('phvro.gsf',text,'gs26/fonts', 80885 ).
file('phvrrn.gsf',text,'gs26/fonts', 42625 ).
file('pncb.gsf',text,'gs26/fonts', 72960 ).
file('pncbi.gsf',text,'gs26/fonts', 82520 ).
file('pncr.gsf',text,'gs26/fonts', 83588 ).
file('pncri.gsf',text,'gs26/fonts', 93295 ).
file('pplb.gsf',text,'gs26/fonts', 34877 ).
file('pplbi.gsf',text,'gs26/fonts', 37031 ).
file('pplr.gsf',text,'gs26/fonts', 56242 ).
file('pplri.gsf',text,'gs26/fonts', 37727 ).
file('psyr.gsf',text,'gs26/fonts', 61593 ).
file('ptmb.gsf',text,'gs26/fonts', 77812 ).
file('ptmbi.gsf',text,'gs26/fonts', 85909 ).
file('ptmr.gsf',text,'gs26/fonts', 78972 ).
file('ptmri.gsf',text,'gs26/fonts', 87136 ).
file('putb.gsf',text,'gs26/fonts', 60932 ).
file('putbi.gsf',text,'gs26/fonts', 63677 ).
file('putr.gsf',text,'gs26/fonts', 60720 ).
file('putri.gsf',text,'gs26/fonts', 63057 ).
file('pzdr.afm',text,'gs26/fonts', 9317 ).
file('pzdr.gsf',text,'gs26/fonts', 84801 ).
file('u003043t.afm',text,'gs26/fonts', 19656 ).
file('u003043t.gsf',text,'gs26/fonts', 77158 ).
file('u004006t.afm',text,'gs26/fonts', 19506 ).
file('u004006t.gsf',text,'gs26/fonts', 59411 ).
file('zcb.gsf',text,'gs26/fonts', 71922 ).
file('zcr.gsf',text,'gs26/fonts', 67154 ).
file('zcro.gsf',text,'gs26/fonts', 82792 ).
file('COPYING',text,'gzip-1.2.3', 17982 ).
file('ChangeLog',text,'gzip-1.2.3', 23977 ).
file('INSTALL',text,'gzip-1.2.3', 9556 ).
file('Makefile',text,'gzip-1.2.3', 11211 ).
file('Makefile.in',text,'gzip-1.2.3', 11167 ).
file('NEWS',text,'gzip-1.2.3', 9062 ).
file('README',text,'gzip-1.2.3', 6895 ).
file('THANKS',text,'gzip-1.2.3', 11538 ).
file('TODO',text,'gzip-1.2.3', 2412 ).
file('algorithm.doc',text,'gzip-1.2.3', 7724 ).
file('amiga',dir,'gzip-1.2.3', 512 ).
file('atari',dir,'gzip-1.2.3', 512 ).
file('bits.c',text,'gzip-1.2.3', 5878 ).
file('config.status',exec,'gzip-1.2.3', 1744 ).
file('configure',exec,'gzip-1.2.3', 18930 ).
file('configure.in',text,'gzip-1.2.3', 2209 ).
file('crypt.c',text,'gzip-1.2.3', 189 ).
file('crypt.h',text,'gzip-1.2.3', 256 ).
file('deflate.c',text,'gzip-1.2.3', 29153 ).
file('getopt.c',text,'gzip-1.2.3', 21423 ).
file('getopt.h',text,'gzip-1.2.3', 4376 ).
file('gpl.texinfo',text,'gzip-1.2.3', 18317 ).
file('gunzip',exec,'gzip-1.2.3', 81920 ).
file('gzexe',exec,'gzip-1.2.3', 3858 ).
file('gzexe.1',text,'gzip-1.2.3', 1296 ).
file('gzexe.in',exec,'gzip-1.2.3', 3836 ).
file('gzip',exec,'gzip-1.2.3', 81920 ).
file('gzip.1',text,'gzip-1.2.3', 12410 ).
file('gzip.c',text,'gzip-1.2.3', 50274 ).
file('gzip.doc',text,'gzip-1.2.3', 13961 ).
file('gzip.h',text,'gzip-1.2.3', 10619 ).
file('gzip.info',text,'gzip-1.2.3', 34094 ).
file('gzip.texi',text,'gzip-1.2.3', 15843 ).
file('inflate.c',text,'gzip-1.2.3', 31533 ).
file('lzw.c',text,'gzip-1.2.3', 574 ).
file('lzw.h',text,'gzip-1.2.3', 1481 ).
file('match.S',text,'gzip-1.2.3', 11764 ).
file('msdos',dir,'gzip-1.2.3', 512 ).
file('os2',dir,'gzip-1.2.3', 512 ).
file('primos',dir,'gzip-1.2.3', 512 ).
file('revision.h',text,'gzip-1.2.3', 477 ).
file('sample',dir,'gzip-1.2.3', 512 ).
file('tailor.h',text,'gzip-1.2.3', 6682 ).
file('texinfo.tex',text,'gzip-1.2.3', 108872 ).
file('trees.c',text,'gzip-1.2.3', 40691 ).
file('unlzh.c',text,'gzip-1.2.3', 9119 ).
file('unlzw.c',text,'gzip-1.2.3', 8729 ).
file('unpack.c',text,'gzip-1.2.3', 8169 ).
file('unzip.c',text,'gzip-1.2.3', 6019 ).
file('util.c',text,'gzip-1.2.3', 14111 ).
file('vms',dir,'gzip-1.2.3', 512 ).
file('zcat',exec,'gzip-1.2.3', 81920 ).
file('zdiff',exec,'gzip-1.2.3', 1996 ).
file('zdiff.1',text,'gzip-1.2.3', 802 ).
file('zdiff.in',exec,'gzip-1.2.3', 1990 ).
file('zforce',exec,'gzip-1.2.3', 1010 ).
file('zforce.1',text,'gzip-1.2.3', 591 ).
file('zforce.in',exec,'gzip-1.2.3', 1004 ).
file('zgrep',exec,'gzip-1.2.3', 1339 ).
file('zgrep.1',text,'gzip-1.2.3', 958 ).
file('zgrep.in',exec,'gzip-1.2.3', 1333 ).
file('zip.c',text,'gzip-1.2.3', 3170 ).
file('zmore',exec,'gzip-1.2.3', 1007 ).
file('zmore.1',text,'gzip-1.2.3', 4226 ).
file('zmore.in',exec,'gzip-1.2.3', 1001 ).
file('znew',exec,'gzip-1.2.3', 3508 ).
file('znew.1',text,'gzip-1.2.3', 885 ).
file('znew.in',exec,'gzip-1.2.3', 3502 ).
file('Makefile.gcc',text,'gzip-1.2.3/amiga', 1844 ).
file('Makefile.sasc',text,'gzip-1.2.3/amiga', 2050 ).
file('match.a',text,'gzip-1.2.3/amiga', 3597 ).
file('tailor.c',text,'gzip-1.2.3/amiga', 4439 ).
file('utime.h',text,'gzip-1.2.3/amiga', 197 ).
file('Makefile.st',text,'gzip-1.2.3/atari', 1336 ).
file('Makefile.bor',text,'gzip-1.2.3/msdos', 2812 ).
file('Makefile.djg',text,'gzip-1.2.3/msdos', 5723 ).
file('Makefile.msc',text,'gzip-1.2.3/msdos', 2813 ).
file('doturboc.bat',text,'gzip-1.2.3/msdos', 830 ).
file('gzip.prj',text,'gzip-1.2.3/msdos', 552 ).
file('match.asm',text,'gzip-1.2.3/msdos', 8248 ).
file('tailor.c',text,'gzip-1.2.3/msdos', 1638 ).
file('Makefile.os2',text,'gzip-1.2.3/os2', 2061 ).
file('gzip.def',text,'gzip-1.2.3/os2', 98 ).
file('gzip16.def',text,'gzip-1.2.3/os2', 80 ).
file('build.cpl',text,'gzip-1.2.3/primos', 1647 ).
file('ci.opts',text,'gzip-1.2.3/primos', 74 ).
file('include',dir,'gzip-1.2.3/primos', 512 ).
file('primos.c',text,'gzip-1.2.3/primos', 2027 ).
file('readme',text,'gzip-1.2.3/primos', 948 ).
file('errno.h',text,'gzip-1.2.3/primos/include', 218 ).
file('fcntl.h',text,'gzip-1.2.3/primos/include', 329 ).
file('stdlib.h',text,'gzip-1.2.3/primos/include', 227 ).
file('sysStat.h',text,'gzip-1.2.3/primos/include', 2027 ).
file('sysTypes.h',text,'gzip-1.2.3/primos/include', 388 ).
file('add.c',text,'gzip-1.2.3/sample', 1444 ).
file('makecrc.c',text,'gzip-1.2.3/sample', 2569 ).
file('sub.c',text,'gzip-1.2.3/sample', 2721 ).
file('zfile',exec,'gzip-1.2.3/sample', 584 ).
file('zread.c',text,'gzip-1.2.3/sample', 1314 ).
file('ztouch',exec,'gzip-1.2.3/sample', 384 ).
file('Makefile.gcc',text,'gzip-1.2.3/vms', 4490 ).
file('Makefile.mms',text,'gzip-1.2.3/vms', 4378 ).
file('Makefile.vms',text,'gzip-1.2.3/vms', 4522 ).
file('Readme.vms',text,'gzip-1.2.3/vms', 3252 ).
file('gzip.hlp',text,'gzip-1.2.3/vms', 12357 ).
file('makegzip.com',text,'gzip-1.2.3/vms', 2428 ).
file('vms.c',text,'gzip-1.2.3/vms', 2233 ).
file('COPYING',text,'libg++-2.4', 17982 ).
file('COPYING.LIB',text,'libg++-2.4', 25265 ).
file('Makefile',text,'libg++-2.4', 30899 ).
file('Makefile.in',text,'libg++-2.4', 30153 ).
file('README',text,'libg++-2.4', 1190 ).
file('config',dir,'libg++-2.4', 512 ).
file('config.guess',exec,'libg++-2.4', 5537 ).
file('config.status',exec,'libg++-2.4', 195 ).
file('config.sub',exec,'libg++-2.4', 17075 ).
file('configure',exec,'libg++-2.4', 38075 ).
file('configure.in',text,'libg++-2.4', 8051 ).
file('etc',dir,'libg++-2.4', 512 ).
file('include',dir,'libg++-2.4', 512 ).
file('install.sh',exec,'libg++-2.4', 3462 ).
file('libg++',dir,'libg++-2.4', 1024 ).
file('libiberty',dir,'libg++-2.4', 2048 ).
file('move-if-change',exec,'libg++-2.4', 129 ).
file('texinfo',dir,'libg++-2.4', 512 ).
file('xiberty',dir,'libg++-2.4', 512 ).
file('ChangeLog',text,'libg++-2.4/config', 1735 ).
file('mh-a68bsd',text,'libg++-2.4/config', 90 ).
file('mh-aix',text,'libg++-2.4/config', 172 ).
file('mh-aix386',text,'libg++-2.4/config', 12 ).
file('mh-apollo68',text,'libg++-2.4/config', 91 ).
file('mh-decstation',text,'libg++-2.4/config', 185 ).
file('mh-delta88',text,'libg++-2.4/config', 17 ).
file('mh-dgux',text,'libg++-2.4/config', 76 ).
file('mh-hpux',text,'libg++-2.4/config', 180 ).
file('mh-irix4',text,'libg++-2.4/config', 273 ).
file('mh-linux',text,'libg++-2.4/config', 118 ).
file('mh-ncr3000',text,'libg++-2.4/config', 710 ).
file('mh-sco',text,'libg++-2.4/config', 215 ).
file('mh-solaris',text,'libg++-2.4/config', 472 ).
file('mh-sun',text,'libg++-2.4/config', 116 ).
file('mh-sun3',text,'libg++-2.4/config', 268 ).
file('mh-sysv',text,'libg++-2.4/config', 98 ).
file('mh-sysv4',text,'libg++-2.4/config', 188 ).
file('mh-vaxult2',text,'libg++-2.4/config', 73 ).
file('Makefile',text,'libg++-2.4/etc', 2221 ).
file('Makefile.in',text,'libg++-2.4/etc', 1879 ).
file('cfg-paper.texi',text,'libg++-2.4/etc', 29217 ).
file('config.status',exec,'libg++-2.4/etc', 198 ).
file('configure.in',text,'libg++-2.4/etc', 378 ).
file('configure.man',text,'libg++-2.4/etc', 3188 ).
file('configure.texi',text,'libg++-2.4/etc', 73534 ).
file('make-stds.texi',text,'libg++-2.4/etc', 18223 ).
file('standards.texi',text,'libg++-2.4/etc', 47797 ).
file('ChangeLog',text,'libg++-2.4/include', 19115 ).
file('ansidecl.h',text,'libg++-2.4/include', 4150 ).
file('demangle.h',text,'libg++-2.4/include', 2775 ).
file('getopt.h',text,'libg++-2.4/include', 4333 ).
file('ieee-float.h',text,'libg++-2.4/include', 2347 ).
file('obstack.h',text,'libg++-2.4/include', 18564 ).
file('ChangeLog',text,'libg++-2.4/libg++', 62480 ).
file('Makefile',text,'libg++-2.4/libg++', 12768 ).
file('Makefile.in',text,'libg++-2.4/libg++', 6629 ).
file('NEWS',text,'libg++-2.4/libg++', 21650 ).
file('README',text,'libg++-2.4/libg++', 9308 ).
file('README.386bsd',text,'libg++-2.4/libg++', 18130 ).
file('TODO',text,'libg++-2.4/libg++', 338 ).
file('_G_config.h',text,'libg++-2.4/libg++', 1450 ).
file('config',dir,'libg++-2.4/libg++', 512 ).
file('config.shared',exec,'libg++-2.4/libg++', 9769 ).
file('config.status',exec,'libg++-2.4/libg++', 198 ).
file('configure.in',text,'libg++-2.4/libg++', 5577 ).
file('etc',dir,'libg++-2.4/libg++', 512 ).
file('g++-include',dir,'libg++-2.4/libg++', 512 ).
file('genclass',dir,'libg++-2.4/libg++', 512 ).
file('gperf',dir,'libg++-2.4/libg++', 512 ).
file('iostream',dir,'libg++-2.4/libg++', 2048 ).
file('lgpl.texinfo',text,'libg++-2.4/libg++', 25632 ).
file('libg++.a',text,'libg++-2.4/libg++', 1539256 ).
file('libg++.info',text,'libg++-2.4/libg++', 2191 ).
file('libg++.info-1',text,'libg++-2.4/libg++', 40874 ).
file('libg++.info-2',text,'libg++-2.4/libg++', 47613 ).
file('libg++.info-3',text,'libg++-2.4/libg++', 48275 ).
file('libg++.info-4',text,'libg++-2.4/libg++', 51145 ).
file('libg++.info-5',text,'libg++-2.4/libg++', 13589 ).
file('libg++.texi',text,'libg++-2.4/libg++', 172971 ).
file('no-stream',dir,'libg++-2.4/libg++', 512 ).
file('old-stream',dir,'libg++-2.4/libg++', 1024 ).
file('proto-kit',dir,'libg++-2.4/libg++', 512 ).
file('src',dir,'libg++-2.4/libg++', 3584 ).
file('target-mkfrag',text,'libg++-2.4/libg++', 436 ).
file('test-install',dir,'libg++-2.4/libg++', 512 ).
file('tests',dir,'libg++-2.4/libg++', 2048 ).
file('utils',dir,'libg++-2.4/libg++', 512 ).
file('vms',dir,'libg++-2.4/libg++', 512 ).
file('aix.mt',text,'libg++-2.4/libg++/config', 54 ).
file('aix3-1.mt',text,'libg++-2.4/libg++/config', 335 ).
file('hpux.mt',text,'libg++-2.4/libg++/config', 481 ).
file('isc.mt',text,'libg++-2.4/libg++/config', 201 ).
file('linux.mt',text,'libg++-2.4/libg++/config', 494 ).
file('ms-1.40',text,'libg++-2.4/libg++/config', 296 ).
file('sco4.mt',text,'libg++-2.4/libg++/config', 463 ).
file('solaris2.mt',text,'libg++-2.4/libg++/config', 492 ).
file('ADT-examples',dir,'libg++-2.4/libg++/etc', 512 ).
file('ChangeLog',text,'libg++-2.4/libg++/etc', 4873 ).
file('HINTS',text,'libg++-2.4/libg++/etc', 23757 ).
file('Makefile',text,'libg++-2.4/libg++/etc', 6457 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc', 1023 ).
file('PlotFile3D',dir,'libg++-2.4/libg++/etc', 512 ).
file('benchmarks',dir,'libg++-2.4/libg++/etc', 512 ).
file('config.status',exec,'libg++-2.4/libg++/etc', 226 ).
file('configure.in',text,'libg++-2.4/libg++/etc', 602 ).
file('fib',dir,'libg++-2.4/libg++/etc', 512 ).
file('graph',dir,'libg++-2.4/libg++/etc', 512 ).
file('lf',dir,'libg++-2.4/libg++/etc', 512 ).
file('trie-gen',dir,'libg++-2.4/libg++/etc', 512 ).
file('Makefile',text,'libg++-2.4/libg++/etc/ADT-examples', 6147 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/ADT-examples', 995 ).
file('Patmain.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 1090 ).
file('Patricia.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 6643 ).
file('Patricia.h',text,'libg++-2.4/libg++/etc/ADT-examples', 2200 ).
file('config.status',exec,'libg++-2.4/libg++/etc/ADT-examples', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/ADT-examples', 616 ).
file('depend',text,'libg++-2.4/libg++/etc/ADT-examples', 1244 ).
file('genPatkey.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 634 ).
file('generic-q.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 3701 ).
file('keyhash.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 4800 ).
file('kmp.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 11162 ).
file('search.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 6052 ).
file('tsort.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 8350 ).
file('tsortinp.cc',text,'libg++-2.4/libg++/etc/ADT-examples', 335 ).
file('Makefile',text,'libg++-2.4/libg++/etc/PlotFile3D', 4876 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/PlotFile3D', 374 ).
file('PlotFile3D.cc',text,'libg++-2.4/libg++/etc/PlotFile3D', 14674 ).
file('PlotFile3D.h',text,'libg++-2.4/libg++/etc/PlotFile3D', 6896 ).
file('README',text,'libg++-2.4/libg++/etc/PlotFile3D', 4592 ).
file('Vec3D.h',text,'libg++-2.4/libg++/etc/PlotFile3D', 5181 ).
file('config.status',exec,'libg++-2.4/libg++/etc/PlotFile3D', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/PlotFile3D', 616 ).
file('depend',text,'libg++-2.4/libg++/etc/PlotFile3D', 594 ).
file('tPlotFile3D.cc',text,'libg++-2.4/libg++/etc/PlotFile3D', 389 ).
file('ChangeLog',text,'libg++-2.4/libg++/etc/benchmarks', 649 ).
file('Char.h',text,'libg++-2.4/libg++/etc/benchmarks', 14862 ).
file('Int.h',text,'libg++-2.4/libg++/etc/benchmarks', 14226 ).
file('Makefile',text,'libg++-2.4/libg++/etc/benchmarks', 5845 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/benchmarks', 1951 ).
file('config.status',exec,'libg++-2.4/libg++/etc/benchmarks', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/benchmarks', 610 ).
file('dhrystone.cc',text,'libg++-2.4/libg++/etc/benchmarks', 22373 ).
file('Makefile',text,'libg++-2.4/libg++/etc/fib', 4967 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/fib', 1095 ).
file('config.status',exec,'libg++-2.4/libg++/etc/fib', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/fib', 585 ).
file('fib.cc',text,'libg++-2.4/libg++/etc/fib', 13775 ).
file('ChangeLog',text,'libg++-2.4/libg++/etc/graph', 5193 ).
file('Makefile',text,'libg++-2.4/libg++/etc/graph', 6945 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/graph', 1899 ).
file('config.status',exec,'libg++-2.4/libg++/etc/graph', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/graph', 660 ).
file('depend',text,'libg++-2.4/libg++/etc/graph', 1085 ).
file('eGetOpt.cc',text,'libg++-2.4/libg++/etc/graph', 562 ).
file('eGetOpt.h',text,'libg++-2.4/libg++/etc/graph', 1134 ).
file('ePlotFile.cc',text,'libg++-2.4/libg++/etc/graph', 248 ).
file('ePlotFile.h',text,'libg++-2.4/libg++/etc/graph', 1214 ).
file('graph.cc',text,'libg++-2.4/libg++/etc/graph', 23775 ).
file('graph.tex',text,'libg++-2.4/libg++/etc/graph', 13774 ).
file('pdefs.h',text,'libg++-2.4/libg++/etc/graph', 219 ).
file('read_data.cc',text,'libg++-2.4/libg++/etc/graph', 3003 ).
file('read_data.h',text,'libg++-2.4/libg++/etc/graph', 732 ).
file('test.dat',text,'libg++-2.4/libg++/etc/graph', 1782 ).
file('test2.dat',text,'libg++-2.4/libg++/etc/graph', 48 ).
file('tick_intrvl.cc',text,'libg++-2.4/libg++/etc/graph', 1072 ).
file('tick_intrvl.h',text,'libg++-2.4/libg++/etc/graph', 468 ).
file('ChangeLog',text,'libg++-2.4/libg++/etc/lf', 1856 ).
file('Dirent.cc',text,'libg++-2.4/libg++/etc/lf', 1942 ).
file('Dirent.h',text,'libg++-2.4/libg++/etc/lf', 1751 ).
file('Makefile',text,'libg++-2.4/libg++/etc/lf', 5051 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/lf', 700 ).
file('config.status',exec,'libg++-2.4/libg++/etc/lf', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/lf', 590 ).
file('depend',text,'libg++-2.4/libg++/etc/lf', 477 ).
file('directory.cc',text,'libg++-2.4/libg++/etc/lf', 2695 ).
file('directory.h',text,'libg++-2.4/libg++/etc/lf', 1120 ).
file('entry.cc',text,'libg++-2.4/libg++/etc/lf', 2648 ).
file('entry.h',text,'libg++-2.4/libg++/etc/lf', 1974 ).
file('lf.cc',text,'libg++-2.4/libg++/etc/lf', 544 ).
file('option.cc',text,'libg++-2.4/libg++/etc/lf', 1403 ).
file('option.h',text,'libg++-2.4/libg++/etc/lf', 1070 ).
file('screen.cc',text,'libg++-2.4/libg++/etc/lf', 1540 ).
file('screen.h',text,'libg++-2.4/libg++/etc/lf', 1245 ).
file('sort.cc',text,'libg++-2.4/libg++/etc/lf', 5663 ).
file('ChangeLog',text,'libg++-2.4/libg++/etc/trie-gen', 4411 ).
file('Makefile',text,'libg++-2.4/libg++/etc/trie-gen', 6348 ).
file('Makefile.in',text,'libg++-2.4/libg++/etc/trie-gen', 1924 ).
file('compact.cc',text,'libg++-2.4/libg++/etc/trie-gen', 12563 ).
file('compact.h',text,'libg++-2.4/libg++/etc/trie-gen', 3325 ).
file('config.status',exec,'libg++-2.4/libg++/etc/trie-gen', 232 ).
file('configure.in',text,'libg++-2.4/libg++/etc/trie-gen', 610 ).
file('depend',text,'libg++-2.4/libg++/etc/trie-gen', 512 ).
file('main.cc',text,'libg++-2.4/libg++/etc/trie-gen', 1585 ).
file('options.cc',text,'libg++-2.4/libg++/etc/trie-gen', 5319 ).
file('options.h',text,'libg++-2.4/libg++/etc/trie-gen', 2905 ).
file('test.cc',text,'libg++-2.4/libg++/etc/trie-gen', 601 ).
file('trie-gen.1',text,'libg++-2.4/libg++/etc/trie-gen', 3607 ).
file('trie.cc',text,'libg++-2.4/libg++/etc/trie-gen', 6531 ).
file('trie.h',text,'libg++-2.4/libg++/etc/trie-gen', 1944 ).
file('version.cc',text,'libg++-2.4/libg++/etc/trie-gen', 878 ).
file('ChangeLog',text,'libg++-2.4/libg++/g++-include', 14810 ).
file('assert.h',text,'libg++-2.4/libg++/g++-include', 256 ).
file('bstring.h',text,'libg++-2.4/libg++/g++-include', 20 ).
file('ctype.h',text,'libg++-2.4/libg++/g++-include', 178 ).
file('curses.h',text,'libg++-2.4/libg++/g++-include', 2715 ).
file('dir.h',text,'libg++-2.4/libg++/g++-include', 21 ).
file('dirent.h',text,'libg++-2.4/libg++/g++-include', 832 ).
file('errno.h',text,'libg++-2.4/libg++/g++-include', 351 ).
file('fcntl.h',text,'libg++-2.4/libg++/g++-include', 458 ).
file('grp.h',text,'libg++-2.4/libg++/g++-include', 718 ).
file('math.h',text,'libg++-2.4/libg++/g++-include', 5164 ).
file('memory.h',text,'libg++-2.4/libg++/g++-include', 1047 ).
file('netdb.h',text,'libg++-2.4/libg++/g++-include', 39 ).
file('pwd.h',text,'libg++-2.4/libg++/g++-include', 651 ).
file('setjmp.h',text,'libg++-2.4/libg++/g++-include', 414 ).
file('signal.h',text,'libg++-2.4/libg++/g++-include', 2436 ).
file('stdarg.h',text,'libg++-2.4/libg++/g++-include', 40 ).
file('stddef.h',text,'libg++-2.4/libg++/g++-include', 169 ).
file('stdio.h',text,'libg++-2.4/libg++/g++-include', 4485 ).
file('stdlib.h',text,'libg++-2.4/libg++/g++-include', 1875 ).
file('string.h',text,'libg++-2.4/libg++/g++-include', 1248 ).
file('strings.h',text,'libg++-2.4/libg++/g++-include', 20 ).
file('sys',dir,'libg++-2.4/libg++/g++-include', 512 ).
file('time.h',text,'libg++-2.4/libg++/g++-include', 2400 ).
file('unistd.h',text,'libg++-2.4/libg++/g++-include', 4893 ).
file('values.h',text,'libg++-2.4/libg++/g++-include', 5849 ).
file('ChangeLog',text,'libg++-2.4/libg++/g++-include/sys', 218 ).
file('dir.h',text,'libg++-2.4/libg++/g++-include/sys', 980 ).
file('fcntl.h',text,'libg++-2.4/libg++/g++-include/sys', 217 ).
file('file.h',text,'libg++-2.4/libg++/g++-include/sys', 394 ).
file('mman.h',text,'libg++-2.4/libg++/g++-include/sys', 211 ).
file('param.h',text,'libg++-2.4/libg++/g++-include/sys', 336 ).
file('resource.h',text,'libg++-2.4/libg++/g++-include/sys', 600 ).
file('select.h',text,'libg++-2.4/libg++/g++-include/sys', 149 ).
file('signal.h',text,'libg++-2.4/libg++/g++-include/sys', 1315 ).
file('socket.h',text,'libg++-2.4/libg++/g++-include/sys', 1546 ).
file('stat.h',text,'libg++-2.4/libg++/g++-include/sys', 1082 ).
file('time.h',text,'libg++-2.4/libg++/g++-include/sys', 714 ).
file('times.h',text,'libg++-2.4/libg++/g++-include/sys', 291 ).
file('types.h',text,'libg++-2.4/libg++/g++-include/sys', 305 ).
file('wait.h',text,'libg++-2.4/libg++/g++-include/sys', 951 ).
file('ChangeLog',text,'libg++-2.4/libg++/genclass', 865 ).
file('Makefile',text,'libg++-2.4/libg++/genclass', 4506 ).
file('Makefile.in',text,'libg++-2.4/libg++/genclass', 689 ).
file('config.status',exec,'libg++-2.4/libg++/genclass', 226 ).
file('configure.in',text,'libg++-2.4/libg++/genclass', 584 ).
file('expected.out',text,'libg++-2.4/libg++/genclass', 25967 ).
file('genclass',exec,'libg++-2.4/libg++/genclass', 10259 ).
file('genclass.sh',exec,'libg++-2.4/libg++/genclass', 10275 ).
file('gentest.sh',exec,'libg++-2.4/libg++/genclass', 4086 ).
file('COPYING',text,'libg++-2.4/libg++/gperf', 12488 ).
file('ChangeLog',text,'libg++-2.4/libg++/gperf', 54037 ).
file('Makefile',text,'libg++-2.4/libg++/gperf', 7446 ).
file('Makefile.in',text,'libg++-2.4/libg++/gperf', 1433 ).
file('README',text,'libg++-2.4/libg++/gperf', 956 ).
file('config.status',exec,'libg++-2.4/libg++/gperf', 226 ).
file('configure.in',text,'libg++-2.4/libg++/gperf', 679 ).
file('gperf.1',text,'libg++-2.4/libg++/gperf', 722 ).
file('gperf.info',text,'libg++-2.4/libg++/gperf', 51808 ).
file('gperf.texi',text,'libg++-2.4/libg++/gperf', 50791 ).
file('src',dir,'libg++-2.4/libg++/gperf', 1024 ).
file('tests',dir,'libg++-2.4/libg++/gperf', 1024 ).
file('Makefile',text,'libg++-2.4/libg++/gperf/src', 6550 ).
file('Makefile.in',text,'libg++-2.4/libg++/gperf/src', 1287 ).
file('bool-array.cc',text,'libg++-2.4/libg++/gperf/src', 2772 ).
file('bool-array.h',text,'libg++-2.4/libg++/gperf/src', 3080 ).
file('config.status',exec,'libg++-2.4/libg++/gperf/src', 232 ).
file('configure.in',text,'libg++-2.4/libg++/gperf/src', 566 ).
file('depend',text,'libg++-2.4/libg++/gperf/src', 1335 ).
file('gen-perf.cc',text,'libg++-2.4/libg++/gperf/src', 11310 ).
file('gen-perf.h',text,'libg++-2.4/libg++/gperf/src', 1790 ).
file('gperf',exec,'libg++-2.4/libg++/gperf/src', 180224 ).
file('hash-table.cc',text,'libg++-2.4/libg++/gperf/src', 3211 ).
file('hash-table.h',text,'libg++-2.4/libg++/gperf/src', 1425 ).
file('iterator.cc',text,'libg++-2.4/libg++/gperf/src', 2617 ).
file('iterator.h',text,'libg++-2.4/libg++/gperf/src', 2173 ).
file('key-list.cc',text,'libg++-2.4/libg++/gperf/src', 41752 ).
file('key-list.h',text,'libg++-2.4/libg++/gperf/src', 4018 ).
file('list-node.cc',text,'libg++-2.4/libg++/gperf/src', 3889 ).
file('list-node.h',text,'libg++-2.4/libg++/gperf/src', 1896 ).
file('main.cc',text,'libg++-2.4/libg++/gperf/src', 2385 ).
file('new.cc',text,'libg++-2.4/libg++/gperf/src', 2745 ).
file('options.cc',text,'libg++-2.4/libg++/gperf/src', 25007 ).
file('options.h',text,'libg++-2.4/libg++/gperf/src', 9546 ).
file('read-line.cc',text,'libg++-2.4/libg++/gperf/src', 2298 ).
file('read-line.h',text,'libg++-2.4/libg++/gperf/src', 1837 ).
file('std-err.cc',text,'libg++-2.4/libg++/gperf/src', 2916 ).
file('std-err.h',text,'libg++-2.4/libg++/gperf/src', 1185 ).
file('trace.h',text,'libg++-2.4/libg++/gperf/src', 337 ).
file('vectors.h',text,'libg++-2.4/libg++/gperf/src', 1252 ).
file('version.cc',text,'libg++-2.4/libg++/gperf/src', 903 ).
file('Makefile',text,'libg++-2.4/libg++/gperf/tests', 7377 ).
file('Makefile.in',text,'libg++-2.4/libg++/gperf/tests', 3405 ).
file('ada-pred.exp',text,'libg++-2.4/libg++/gperf/tests', 1121 ).
file('ada-res.exp',text,'libg++-2.4/libg++/gperf/tests', 1122 ).
file('ada.gperf',text,'libg++-2.4/libg++/gperf/tests', 366 ).
file('adadefs.gperf',text,'libg++-2.4/libg++/gperf/tests', 473 ).
file('c++.gperf',text,'libg++-2.4/libg++/gperf/tests', 304 ).
file('c-parse.gperf',text,'libg++-2.4/libg++/gperf/tests', 1444 ).
file('c.exp',text,'libg++-2.4/libg++/gperf/tests', 582 ).
file('c.gperf',text,'libg++-2.4/libg++/gperf/tests', 198 ).
file('config.status',exec,'libg++-2.4/libg++/gperf/tests', 232 ).
file('configure.in',text,'libg++-2.4/libg++/gperf/tests', 664 ).
file('gpc.gperf',text,'libg++-2.4/libg++/gperf/tests', 1122 ).
file('gplus.gperf',text,'libg++-2.4/libg++/gperf/tests', 2072 ).
file('irc.gperf',text,'libg++-2.4/libg++/gperf/tests', 1858 ).
file('makeinfo.gperf',text,'libg++-2.4/libg++/gperf/tests', 3009 ).
file('modula.exp',text,'libg++-2.4/libg++/gperf/tests', 1880 ).
file('modula2.gperf',text,'libg++-2.4/libg++/gperf/tests', 225 ).
file('modula3.gperf',text,'libg++-2.4/libg++/gperf/tests', 608 ).
file('pascal.exp',text,'libg++-2.4/libg++/gperf/tests', 620 ).
file('pascal.gperf',text,'libg++-2.4/libg++/gperf/tests', 188 ).
file('test-1.exp',text,'libg++-2.4/libg++/gperf/tests', 4172 ).
file('test-2.exp',text,'libg++-2.4/libg++/gperf/tests', 5058 ).
file('test-3.exp',text,'libg++-2.4/libg++/gperf/tests', 5543 ).
file('test-4.exp',text,'libg++-2.4/libg++/gperf/tests', 4781 ).
file('test-5.exp',text,'libg++-2.4/libg++/gperf/tests', 3259 ).
file('test-6.exp',text,'libg++-2.4/libg++/gperf/tests', 4708 ).
file('test-7.exp',text,'libg++-2.4/libg++/gperf/tests', 678 ).
file('test.c',text,'libg++-2.4/libg++/gperf/tests', 540 ).
file('ChangeLog',text,'libg++-2.4/libg++/iostream', 40706 ).
file('LICENSE',text,'libg++-2.4/libg++/iostream', 25265 ).
file('Makefile',text,'libg++-2.4/libg++/iostream', 10454 ).
file('Makefile.in',text,'libg++-2.4/libg++/iostream', 2509 ).
file('PlotFile.C',text,'libg++-2.4/libg++/iostream', 2967 ).
file('PlotFile.h',text,'libg++-2.4/libg++/iostream', 2914 ).
file('QUESTIONS',text,'libg++-2.4/libg++/iostream', 1383 ).
file('README',text,'libg++-2.4/libg++/iostream', 4121 ).
file('SFile.C',text,'libg++-2.4/libg++/iostream', 1449 ).
file('SFile.h',text,'libg++-2.4/libg++/iostream', 1513 ).
file('TODO',text,'libg++-2.4/libg++/iostream', 986 ).
file('config.status',exec,'libg++-2.4/libg++/iostream', 226 ).
file('configure.in',text,'libg++-2.4/libg++/iostream', 732 ).
file('depend',text,'libg++-2.4/libg++/iostream', 1952 ).
file('editbuf.C',text,'libg++-2.4/libg++/iostream', 18566 ).
file('editbuf.doc',text,'libg++-2.4/libg++/iostream', 1238 ).
file('editbuf.h',text,'libg++-2.4/libg++/iostream', 6424 ).
file('filebuf.C',text,'libg++-2.4/libg++/iostream', 15522 ).
file('floatconv.C',text,'libg++-2.4/libg++/iostream', 75906 ).
file('floatio.h',text,'libg++-2.4/libg++/iostream', 1101 ).
file('fstream.C',text,'libg++-2.4/libg++/iostream', 1621 ).
file('fstream.h',text,'libg++-2.4/libg++/iostream', 2596 ).
file('igetline.C',text,'libg++-2.4/libg++/iostream', 3992 ).
file('igetsb.C',text,'libg++-2.4/libg++/iostream', 1566 ).
file('indstream.C',text,'libg++-2.4/libg++/iostream', 3002 ).
file('indstream.h',text,'libg++-2.4/libg++/iostream', 2480 ).
file('iomanip.C',text,'libg++-2.4/libg++/iostream', 1883 ).
file('iomanip.h',text,'libg++-2.4/libg++/iostream', 4632 ).
file('ioprivate.h',text,'libg++-2.4/libg++/iostream', 2213 ).
file('iostream.C',text,'libg++-2.4/libg++/iostream', 16846 ).
file('iostream.h',text,'libg++-2.4/libg++/iostream', 8097 ).
file('iostream.info',text,'libg++-2.4/libg++/iostream', 17086 ).
file('iostream.texi',text,'libg++-2.4/libg++/iostream', 15058 ).
file('istream.h',text,'libg++-2.4/libg++/iostream', 20 ).
file('libio.a',text,'libg++-2.4/libg++/iostream', 484878 ).
file('makebuf.C',text,'libg++-2.4/libg++/iostream', 2084 ).
file('ostream.h',text,'libg++-2.4/libg++/iostream', 20 ).
file('outfloat.C',text,'libg++-2.4/libg++/iostream', 5710 ).
file('parsestream.C',text,'libg++-2.4/libg++/iostream', 7595 ).
file('parsestream.h',text,'libg++-2.4/libg++/iostream', 5052 ).
file('procbuf.C',text,'libg++-2.4/libg++/iostream', 3226 ).
file('procbuf.h',text,'libg++-2.4/libg++/iostream', 1190 ).
file('sbufvform.C',text,'libg++-2.4/libg++/iostream', 20478 ).
file('sbufvscan.C',text,'libg++-2.4/libg++/iostream', 17366 ).
file('sgetline.C',text,'libg++-2.4/libg++/iostream', 1979 ).
file('stdio',dir,'libg++-2.4/libg++/iostream', 2560 ).
file('stdiostream.C',text,'libg++-2.4/libg++/iostream', 3099 ).
file('stdiostream.h',text,'libg++-2.4/libg++/iostream', 1489 ).
file('stdstrbufs.C',text,'libg++-2.4/libg++/iostream', 3833 ).
file('stdstreams.C',text,'libg++-2.4/libg++/iostream', 4781 ).
file('stream.C',text,'libg++-2.4/libg++/iostream', 2714 ).
file('stream.h',text,'libg++-2.4/libg++/iostream', 770 ).
file('streambuf.C',text,'libg++-2.4/libg++/iostream', 14863 ).
file('streambuf.h',text,'libg++-2.4/libg++/iostream', 17035 ).
file('strstream.C',text,'libg++-2.4/libg++/iostream', 5859 ).
file('strstream.h',text,'libg++-2.4/libg++/iostream', 4125 ).
file('test',dir,'libg++-2.4/libg++/iostream', 1024 ).
file('ChangeLog',text,'libg++-2.4/libg++/iostream/stdio', 5007 ).
file('Makefile',text,'libg++-2.4/libg++/iostream/stdio', 10935 ).
file('Makefile.in',text,'libg++-2.4/libg++/iostream/stdio', 1397 ).
file('clearerr.C',text,'libg++-2.4/libg++/iostream/stdio', 145 ).
file('config.status',exec,'libg++-2.4/libg++/iostream/stdio', 232 ).
file('configure.in',text,'libg++-2.4/libg++/iostream/stdio', 670 ).
file('depend',text,'libg++-2.4/libg++/iostream/stdio', 5613 ).
file('emulate.C',text,'libg++-2.4/libg++/iostream/stdio', 2617 ).
file('fclose.C',text,'libg++-2.4/libg++/iostream/stdio', 431 ).
file('fdopen.C',text,'libg++-2.4/libg++/iostream/stdio', 338 ).
file('feof.C',text,'libg++-2.4/libg++/iostream/stdio', 142 ).
file('ferror.C',text,'libg++-2.4/libg++/iostream/stdio', 144 ).
file('fflush.C',text,'libg++-2.4/libg++/iostream/stdio', 291 ).
file('fgetc.C',text,'libg++-2.4/libg++/iostream/stdio', 83 ).
file('fgetpos.C',text,'libg++-2.4/libg++/iostream/stdio', 439 ).
file('fgets.C',text,'libg++-2.4/libg++/iostream/stdio', 234 ).
file('fileno.C',text,'libg++-2.4/libg++/iostream/stdio', 185 ).
file('fopen.C',text,'libg++-2.4/libg++/iostream/stdio', 230 ).
file('fprintf.C',text,'libg++-2.4/libg++/iostream/stdio', 597 ).
file('fputc.C',text,'libg++-2.4/libg++/iostream/stdio', 93 ).
file('fputs.C',text,'libg++-2.4/libg++/iostream/stdio', 191 ).
file('fread.C',text,'libg++-2.4/libg++/iostream/stdio', 346 ).
file('freopen.C',text,'libg++-2.4/libg++/iostream/stdio', 275 ).
file('fscanf.C',text,'libg++-2.4/libg++/iostream/stdio', 265 ).
file('fseek.C',text,'libg++-2.4/libg++/iostream/stdio', 248 ).
file('fsetpos.C',text,'libg++-2.4/libg++/iostream/stdio', 407 ).
file('ftell.C',text,'libg++-2.4/libg++/iostream/stdio', 423 ).
file('fwrite.C',text,'libg++-2.4/libg++/iostream/stdio', 346 ).
file('gets.C',text,'libg++-2.4/libg++/iostream/stdio', 173 ).
file('libstdio++.a',text,'libg++-2.4/libg++/iostream/stdio', 178474 ).
file('perror.C',text,'libg++-2.4/libg++/iostream/stdio', 367 ).
file('popen.C',text,'libg++-2.4/libg++/iostream/stdio', 494 ).
file('printf.C',text,'libg++-2.4/libg++/iostream/stdio', 367 ).
file('puts.C',text,'libg++-2.4/libg++/iostream/stdio', 229 ).
file('remove.C',text,'libg++-2.4/libg++/iostream/stdio', 122 ).
file('rename.C',text,'libg++-2.4/libg++/iostream/stdio', 179 ).
file('rewind.C',text,'libg++-2.4/libg++/iostream/stdio', 147 ).
file('scanf.C',text,'libg++-2.4/libg++/iostream/stdio', 277 ).
file('setbuf.C',text,'libg++-2.4/libg++/iostream/stdio', 256 ).
file('setbuffer.C',text,'libg++-2.4/libg++/iostream/stdio', 246 ).
file('setlinebuf.C',text,'libg++-2.4/libg++/iostream/stdio', 271 ).
file('setvbuf.C',text,'libg++-2.4/libg++/iostream/stdio', 475 ).
file('sprintf.C',text,'libg++-2.4/libg++/iostream/stdio', 231 ).
file('sscanf.C',text,'libg++-2.4/libg++/iostream/stdio', 281 ).
file('stdio.C',text,'libg++-2.4/libg++/iostream/stdio', 99 ).
file('stdio.h',text,'libg++-2.4/libg++/iostream/stdio', 5823 ).
file('stdioprivate.h',text,'libg++-2.4/libg++/iostream/stdio', 489 ).
file('tmpfile.C',text,'libg++-2.4/libg++/iostream/stdio', 393 ).
file('tmpnam.C',text,'libg++-2.4/libg++/iostream/stdio', 3455 ).
file('ungetc.C',text,'libg++-2.4/libg++/iostream/stdio', 181 ).
file('vfprintf.C',text,'libg++-2.4/libg++/iostream/stdio', 196 ).
file('vfscanf.C',text,'libg++-2.4/libg++/iostream/stdio', 195 ).
file('vprintf.C',text,'libg++-2.4/libg++/iostream/stdio', 209 ).
file('vscanf.C',text,'libg++-2.4/libg++/iostream/stdio', 197 ).
file('vsprintf.C',text,'libg++-2.4/libg++/iostream/stdio', 242 ).
file('vsscanf.C',text,'libg++-2.4/libg++/iostream/stdio', 230 ).
file('ChangeLog',text,'libg++-2.4/libg++/iostream/test', 3784 ).
file('Makefile',text,'libg++-2.4/libg++/iostream/test', 9405 ).
file('Makefile.in',text,'libg++-2.4/libg++/iostream/test', 4327 ).
file('config.status',exec,'libg++-2.4/libg++/iostream/test', 232 ).
file('configure.in',text,'libg++-2.4/libg++/iostream/test', 836 ).
file('depend',text,'libg++-2.4/libg++/iostream/test', 937 ).
file('hounddog.C',text,'libg++-2.4/libg++/iostream/test', 1684 ).
file('hounddog.exp',text,'libg++-2.4/libg++/iostream/test', 199 ).
file('hounddog.in',text,'libg++-2.4/libg++/iostream/test', 45 ).
file('putbackdog.C',text,'libg++-2.4/libg++/iostream/test', 1896 ).
file('tFile.C',text,'libg++-2.4/libg++/iostream/test', 10972 ).
file('tFile.exp',text,'libg++-2.4/libg++/iostream/test', 1725 ).
file('tFile.inp',text,'libg++-2.4/libg++/iostream/test', 60 ).
file('tformat.C',text,'libg++-2.4/libg++/iostream/test', 226277 ).
file('tfseek-cur.exp',text,'libg++-2.4/libg++/iostream/test', 203 ).
file('tfseek-set.exp',text,'libg++-2.4/libg++/iostream/test', 204 ).
file('tfseek.c',text,'libg++-2.4/libg++/iostream/test', 3486 ).
file('tgetl.C',text,'libg++-2.4/libg++/iostream/test', 289 ).
file('tiformat.C',text,'libg++-2.4/libg++/iostream/test', 202695 ).
file('tiomanip.C',text,'libg++-2.4/libg++/iostream/test', 596 ).
file('tiomanip.exp',text,'libg++-2.4/libg++/iostream/test', 35 ).
file('tpopen.c',text,'libg++-2.4/libg++/iostream/test', 527 ).
file('tpopen.exp',text,'libg++-2.4/libg++/iostream/test', 36 ).
file('trdseek.c',text,'libg++-2.4/libg++/iostream/test', 1077 ).
file('tst.C',text,'libg++-2.4/libg++/iostream/test', 462 ).
file('twrseek.c',text,'libg++-2.4/libg++/iostream/test', 797 ).
file('twrseek.exp',text,'libg++-2.4/libg++/iostream/test', 89 ).
file('Makefile',text,'libg++-2.4/libg++/no-stream', 4503 ).
file('Makefile.in',text,'libg++-2.4/libg++/no-stream', 682 ).
file('config.status',exec,'libg++-2.4/libg++/no-stream', 226 ).
file('configure.in',text,'libg++-2.4/libg++/no-stream', 589 ).
file('ChangeLog',text,'libg++-2.4/libg++/old-stream', 704 ).
file('File.cc',text,'libg++-2.4/libg++/old-stream', 11201 ).
file('File.h',text,'libg++-2.4/libg++/old-stream', 7254 ).
file('Filebuf.cc',text,'libg++-2.4/libg++/old-stream', 6812 ).
file('Filebuf.h',text,'libg++-2.4/libg++/old-stream', 1904 ).
file('Fmodes.h',text,'libg++-2.4/libg++/old-stream', 924 ).
file('Makefile',text,'libg++-2.4/libg++/old-stream', 5617 ).
file('Makefile.in',text,'libg++-2.4/libg++/old-stream', 973 ).
file('PlotFile.cc',text,'libg++-2.4/libg++/old-stream', 3920 ).
file('PlotFile.h',text,'libg++-2.4/libg++/old-stream', 4100 ).
file('SFile.cc',text,'libg++-2.4/libg++/old-stream', 1288 ).
file('SFile.h',text,'libg++-2.4/libg++/old-stream', 1972 ).
file('config.status',exec,'libg++-2.4/libg++/old-stream', 226 ).
file('configure.in',text,'libg++-2.4/libg++/old-stream', 558 ).
file('depend',text,'libg++-2.4/libg++/old-stream', 811 ).
file('filebuf.cc',text,'libg++-2.4/libg++/old-stream', 2988 ).
file('filebuf.h',text,'libg++-2.4/libg++/old-stream', 1708 ).
file('form.cc',text,'libg++-2.4/libg++/old-stream', 1487 ).
file('istream.cc',text,'libg++-2.4/libg++/old-stream', 9765 ).
file('istream.h',text,'libg++-2.4/libg++/old-stream', 6034 ).
file('itoa.cc',text,'libg++-2.4/libg++/old-stream', 5259 ).
file('ostream.cc',text,'libg++-2.4/libg++/old-stream', 4244 ).
file('ostream.h',text,'libg++-2.4/libg++/old-stream', 5658 ).
file('stream.h',text,'libg++-2.4/libg++/old-stream', 187 ).
file('streambuf.cc',text,'libg++-2.4/libg++/old-stream', 2634 ).
file('streambuf.h',text,'libg++-2.4/libg++/old-stream', 4124 ).
file('Makefile',text,'libg++-2.4/libg++/proto-kit', 12685 ).
file('Makefile.new',text,'libg++-2.4/libg++/proto-kit', 13322 ).
file('file_types.hierarchy',text,'libg++-2.4/libg++/proto-kit', 216 ).
file('file_types.includes.h',text,'libg++-2.4/libg++/proto-kit', 561 ).
file('file_types.operation.h',text,'libg++-2.4/libg++/proto-kit', 591 ).
file('file_types.type.h',text,'libg++-2.4/libg++/proto-kit', 1145 ).
file('genclass.extnsn',text,'libg++-2.4/libg++/proto-kit', 2758 ).
file('hierarchy',text,'libg++-2.4/libg++/proto-kit', 3513 ).
file('make-defs-file',exec,'libg++-2.4/libg++/proto-kit', 459 ).
file('make-source-dependencies',exec,'libg++-2.4/libg++/proto-kit', 1475 ).
file('make-types',exec,'libg++-2.4/libg++/proto-kit', 1716 ).
file('prepend',exec,'libg++-2.4/libg++/proto-kit', 360 ).
file('prepend-header',exec,'libg++-2.4/libg++/proto-kit', 365 ).
file('prototype',exec,'libg++-2.4/libg++/proto-kit', 3983 ).
file('prototype.dependencies',text,'libg++-2.4/libg++/proto-kit', 3942 ).
file('ACG.cc',text,'libg++-2.4/libg++/src', 9328 ).
file('ACG.h',text,'libg++-2.4/libg++/src', 2178 ).
file('AllocRing.cc',text,'libg++-2.4/libg++/src', 2327 ).
file('AllocRing.h',text,'libg++-2.4/libg++/src', 1679 ).
file('Binomial.cc',text,'libg++-2.4/libg++/src', 1099 ).
file('Binomial.h',text,'libg++-2.4/libg++/src', 1546 ).
file('BitSet.cc',text,'libg++-2.4/libg++/src', 19617 ).
file('BitSet.h',text,'libg++-2.4/libg++/src', 8860 ).
file('BitString.cc',text,'libg++-2.4/libg++/src', 47181 ).
file('BitString.h',text,'libg++-2.4/libg++/src', 20233 ).
file('ChangeLog',text,'libg++-2.4/libg++/src', 21531 ).
file('Complex.cc',text,'libg++-2.4/libg++/src', 6120 ).
file('Complex.h',text,'libg++-2.4/libg++/src', 6776 ).
file('CursesW.cc',text,'libg++-2.4/libg++/src', 5538 ).
file('CursesW.h',text,'libg++-2.4/libg++/src', 13822 ).
file('DLList.cc',text,'libg++-2.4/libg++/src', 5526 ).
file('DLList.h',text,'libg++-2.4/libg++/src', 4072 ).
file('DiscUnif.cc',text,'libg++-2.4/libg++/src', 1072 ).
file('DiscUnif.h',text,'libg++-2.4/libg++/src', 1860 ).
file('Erlang.cc',text,'libg++-2.4/libg++/src', 1089 ).
file('Erlang.h',text,'libg++-2.4/libg++/src', 1891 ).
file('Fix.cc',text,'libg++-2.4/libg++/src', 12842 ).
file('Fix.h',text,'libg++-2.4/libg++/src', 10021 ).
file('Fix16.cc',text,'libg++-2.4/libg++/src', 5982 ).
file('Fix16.h',text,'libg++-2.4/libg++/src', 13861 ).
file('Fix24.cc',text,'libg++-2.4/libg++/src', 8346 ).
file('Fix24.h',text,'libg++-2.4/libg++/src', 13339 ).
file('Geom.cc',text,'libg++-2.4/libg++/src', 1081 ).
file('Geom.h',text,'libg++-2.4/libg++/src', 1419 ).
file('GetOpt.cc',text,'libg++-2.4/libg++/src', 8540 ).
file('GetOpt.h',text,'libg++-2.4/libg++/src', 4872 ).
file('HypGeom.cc',text,'libg++-2.4/libg++/src', 1109 ).
file('HypGeom.h',text,'libg++-2.4/libg++/src', 1970 ).
file('Incremental.h',text,'libg++-2.4/libg++/src', 372 ).
file('Integer.cc',text,'libg++-2.4/libg++/src', 49954 ).
file('Integer.h',text,'libg++-2.4/libg++/src', 27255 ).
file('LogNorm.cc',text,'libg++-2.4/libg++/src', 1118 ).
file('LogNorm.h',text,'libg++-2.4/libg++/src', 2098 ).
file('MLCG.cc',text,'libg++-2.4/libg++/src', 2792 ).
file('MLCG.h',text,'libg++-2.4/libg++/src', 1831 ).
file('Makefile',text,'libg++-2.4/libg++/src', 11260 ).
file('Makefile.in',text,'libg++-2.4/libg++/src', 2035 ).
file('NegExp.cc',text,'libg++-2.4/libg++/src', 1025 ).
file('NegExp.h',text,'libg++-2.4/libg++/src', 1508 ).
file('Normal.cc',text,'libg++-2.4/libg++/src', 1697 ).
file('Normal.h',text,'libg++-2.4/libg++/src', 1838 ).
file('Obstack.cc',text,'libg++-2.4/libg++/src', 3068 ).
file('Obstack.h',text,'libg++-2.4/libg++/src', 4398 ).
file('Pix.h',text,'libg++-2.4/libg++/src', 59 ).
file('Poisson.cc',text,'libg++-2.4/libg++/src', 1159 ).
file('Poisson.h',text,'libg++-2.4/libg++/src', 1400 ).
file('RNG.cc',text,'libg++-2.4/libg++/src', 3788 ).
file('RNG.h',text,'libg++-2.4/libg++/src', 1742 ).
file('Random.cc',text,'libg++-2.4/libg++/src', 66 ).
file('Random.h',text,'libg++-2.4/libg++/src', 1365 ).
file('Rational.cc',text,'libg++-2.4/libg++/src', 7504 ).
file('Rational.h',text,'libg++-2.4/libg++/src', 7925 ).
file('Regex.cc',text,'libg++-2.4/libg++/src', 3432 ).
file('Regex.h',text,'libg++-2.4/libg++/src', 2615 ).
file('RndInt.cc',text,'libg++-2.4/libg++/src', 66 ).
file('RndInt.h',text,'libg++-2.4/libg++/src', 4484 ).
file('SLList.cc',text,'libg++-2.4/libg++/src', 4524 ).
file('SLList.h',text,'libg++-2.4/libg++/src', 3922 ).
file('Sample.cc',text,'libg++-2.4/libg++/src', 5291 ).
file('SmplHist.cc',text,'libg++-2.4/libg++/src', 2696 ).
file('SmplHist.h',text,'libg++-2.4/libg++/src', 1967 ).
file('SmplStat.cc',text,'libg++-2.4/libg++/src', 3678 ).
file('SmplStat.h',text,'libg++-2.4/libg++/src', 2014 ).
file('String.cc',text,'libg++-2.4/libg++/src', 28310 ).
file('String.h',text,'libg++-2.4/libg++/src', 36196 ).
file('Uniform.cc',text,'libg++-2.4/libg++/src', 1021 ).
file('Uniform.h',text,'libg++-2.4/libg++/src', 1802 ).
file('Weibull.cc',text,'libg++-2.4/libg++/src', 1152 ).
file('Weibull.h',text,'libg++-2.4/libg++/src', 1813 ).
file('bool.h',text,'libg++-2.4/libg++/src', 285 ).
file('builtin.cc',text,'libg++-2.4/libg++/src', 67 ).
file('builtin.h',text,'libg++-2.4/libg++/src', 3387 ).
file('chr.cc',text,'libg++-2.4/libg++/src', 1218 ).
file('compare.cc',text,'libg++-2.4/libg++/src', 67 ).
file('compare.h',text,'libg++-2.4/libg++/src', 2199 ).
file('complex.h',text,'libg++-2.4/libg++/src', 121 ).
file('config.status',exec,'libg++-2.4/libg++/src', 226 ).
file('configure.in',text,'libg++-2.4/libg++/src', 567 ).
file('depend',text,'libg++-2.4/libg++/src', 5405 ).
file('drt0.c',text,'libg++-2.4/libg++/src', 682 ).
file('dtoa.cc',text,'libg++-2.4/libg++/src', 7268 ).
file('dynamic_lib.c',text,'libg++-2.4/libg++/src', 4405 ).
file('error.cc',text,'libg++-2.4/libg++/src', 1512 ).
file('fmtq.cc',text,'libg++-2.4/libg++/src', 1106 ).
file('gcd.cc',text,'libg++-2.4/libg++/src', 1248 ).
file('gen',dir,'libg++-2.4/libg++/src', 2560 ).
file('generic.h',text,'libg++-2.4/libg++/src', 1786 ).
file('getpagesize.h',text,'libg++-2.4/libg++/src', 471 ).
file('hash.cc',text,'libg++-2.4/libg++/src', 1460 ).
file('init.c',text,'libg++-2.4/libg++/src', 1720 ).
file('init_main.c',text,'libg++-2.4/libg++/src', 1548 ).
file('ioob.cc',text,'libg++-2.4/libg++/src', 1070 ).
file('lg.cc',text,'libg++-2.4/libg++/src', 991 ).
file('libc.h',text,'libg++-2.4/libg++/src', 20 ).
file('malloc.c',text,'libg++-2.4/libg++/src', 34015 ).
file('math-68881.h',text,'libg++-2.4/libg++/src', 9209 ).
file('math.cc',text,'libg++-2.4/libg++/src', 64 ).
file('minmax.cc',text,'libg++-2.4/libg++/src', 65 ).
file('minmax.h',text,'libg++-2.4/libg++/src', 2477 ).
file('new.cc',text,'libg++-2.4/libg++/src', 1034 ).
file('new.h',text,'libg++-2.4/libg++/src', 798 ).
file('osfcn.h',text,'libg++-2.4/libg++/src', 215 ).
file('pow.cc',text,'libg++-2.4/libg++/src', 1490 ).
file('regex.cc',text,'libg++-2.4/libg++/src', 83688 ).
file('regex.h',text,'libg++-2.4/libg++/src', 9985 ).
file('sqrt.cc',text,'libg++-2.4/libg++/src', 1200 ).
file('std.h',text,'libg++-2.4/libg++/src', 1176 ).
file('str.cc',text,'libg++-2.4/libg++/src', 1255 ).
file('strclass.h',text,'libg++-2.4/libg++/src', 96 ).
file('swap.h',text,'libg++-2.4/libg++/src', 133 ).
file('timer.cc',text,'libg++-2.4/libg++/src', 3398 ).
file('typemacros.h',text,'libg++-2.4/libg++/src', 268 ).
file('xyzzy.cc',text,'libg++-2.4/libg++/src', 213 ).
file('AVLMap.ccP',text,'libg++-2.4/libg++/src/gen', 12955 ).
file('AVLMap.hP',text,'libg++-2.4/libg++/src/gen', 3297 ).
file('AVLSet.ccP',text,'libg++-2.4/libg++/src/gen', 17496 ).
file('AVLSet.hP',text,'libg++-2.4/libg++/src/gen', 3357 ).
file('AVec.ccP',text,'libg++-2.4/libg++/src/gen', 8174 ).
file('AVec.hP',text,'libg++-2.4/libg++/src/gen', 3584 ).
file('BSTSet.ccP',text,'libg++-2.4/libg++/src/gen', 7255 ).
file('BSTSet.hP',text,'libg++-2.4/libg++/src/gen', 3333 ).
file('Bag.ccP',text,'libg++-2.4/libg++/src/gen', 1678 ).
file('Bag.hP',text,'libg++-2.4/libg++/src/gen', 2359 ).
file('CHBag.ccP',text,'libg++-2.4/libg++/src/gen', 4353 ).
file('CHBag.hP',text,'libg++-2.4/libg++/src/gen', 1853 ).
file('CHMap.ccP',text,'libg++-2.4/libg++/src/gen', 3744 ).
file('CHMap.hP',text,'libg++-2.4/libg++/src/gen', 2467 ).
file('CHNode.ccP',text,'libg++-2.4/libg++/src/gen', 902 ).
file('CHNode.hP',text,'libg++-2.4/libg++/src/gen', 1370 ).
file('CHSet.ccP',text,'libg++-2.4/libg++/src/gen', 6101 ).
file('CHSet.hP',text,'libg++-2.4/libg++/src/gen', 2110 ).
file('DLDeque.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('DLDeque.hP',text,'libg++-2.4/libg++/src/gen', 2532 ).
file('DLList.ccP',text,'libg++-2.4/libg++/src/gen', 5758 ).
file('DLList.hP',text,'libg++-2.4/libg++/src/gen', 3807 ).
file('Deque.ccP',text,'libg++-2.4/libg++/src/gen', 176 ).
file('Deque.hP',text,'libg++-2.4/libg++/src/gen', 1739 ).
file('FPQueue.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('FPQueue.hP',text,'libg++-2.4/libg++/src/gen', 2368 ).
file('FPStack.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('FPStack.hP',text,'libg++-2.4/libg++/src/gen', 2377 ).
file('FPlex.ccP',text,'libg++-2.4/libg++/src/gen', 3980 ).
file('FPlex.hP',text,'libg++-2.4/libg++/src/gen', 6101 ).
file('List.ccP',text,'libg++-2.4/libg++/src/gen', 17493 ).
file('List.hP',text,'libg++-2.4/libg++/src/gen', 6052 ).
file('MPlex.ccP',text,'libg++-2.4/libg++/src/gen', 16890 ).
file('MPlex.hP',text,'libg++-2.4/libg++/src/gen', 10589 ).
file('Map.ccP',text,'libg++-2.4/libg++/src/gen', 1475 ).
file('Map.hP',text,'libg++-2.4/libg++/src/gen', 2464 ).
file('OSLBag.ccP',text,'libg++-2.4/libg++/src/gen', 3535 ).
file('OSLBag.hP',text,'libg++-2.4/libg++/src/gen', 2071 ).
file('OSLSet.ccP',text,'libg++-2.4/libg++/src/gen', 5589 ).
file('OSLSet.hP',text,'libg++-2.4/libg++/src/gen', 2336 ).
file('OXPBag.ccP',text,'libg++-2.4/libg++/src/gen', 4743 ).
file('OXPBag.hP',text,'libg++-2.4/libg++/src/gen', 1273 ).
file('OXPSet.ccP',text,'libg++-2.4/libg++/src/gen', 5662 ).
file('OXPSet.hP',text,'libg++-2.4/libg++/src/gen', 2404 ).
file('PHPQ.ccP',text,'libg++-2.4/libg++/src/gen', 7471 ).
file('PHPQ.hP',text,'libg++-2.4/libg++/src/gen', 2468 ).
file('PQ.ccP',text,'libg++-2.4/libg++/src/gen', 1478 ).
file('PQ.hP',text,'libg++-2.4/libg++/src/gen', 2373 ).
file('PSList.hP',text,'libg++-2.4/libg++/src/gen', 1110 ).
file('PVec.hP',text,'libg++-2.4/libg++/src/gen', 2573 ).
file('Plex.ccP',text,'libg++-2.4/libg++/src/gen', 4207 ).
file('Plex.hP',text,'libg++-2.4/libg++/src/gen', 13020 ).
file('Queue.ccP',text,'libg++-2.4/libg++/src/gen', 196 ).
file('Queue.hP',text,'libg++-2.4/libg++/src/gen', 1506 ).
file('RAVLMap.ccP',text,'libg++-2.4/libg++/src/gen', 14389 ).
file('RAVLMap.hP',text,'libg++-2.4/libg++/src/gen', 3509 ).
file('RPlex.ccP',text,'libg++-2.4/libg++/src/gen', 10187 ).
file('RPlex.hP',text,'libg++-2.4/libg++/src/gen', 6532 ).
file('SLBag.ccP',text,'libg++-2.4/libg++/src/gen', 2165 ).
file('SLBag.hP',text,'libg++-2.4/libg++/src/gen', 2130 ).
file('SLList.ccP',text,'libg++-2.4/libg++/src/gen', 5240 ).
file('SLList.hP',text,'libg++-2.4/libg++/src/gen', 3279 ).
file('SLQueue.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('SLQueue.hP',text,'libg++-2.4/libg++/src/gen', 2204 ).
file('SLSet.ccP',text,'libg++-2.4/libg++/src/gen', 1685 ).
file('SLSet.hP',text,'libg++-2.4/libg++/src/gen', 1973 ).
file('SLStack.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('SLStack.hP',text,'libg++-2.4/libg++/src/gen', 2212 ).
file('Set.ccP',text,'libg++-2.4/libg++/src/gen', 2463 ).
file('Set.hP',text,'libg++-2.4/libg++/src/gen', 2468 ).
file('SkipBag.ccP',text,'libg++-2.4/libg++/src/gen', 6571 ).
file('SkipBag.hP',text,'libg++-2.4/libg++/src/gen', 3831 ).
file('SkipMap.ccP',text,'libg++-2.4/libg++/src/gen', 6608 ).
file('SkipMap.hP',text,'libg++-2.4/libg++/src/gen', 4115 ).
file('SkipSet.ccP',text,'libg++-2.4/libg++/src/gen', 7853 ).
file('SkipSet.hP',text,'libg++-2.4/libg++/src/gen', 4180 ).
file('SplayBag.ccP',text,'libg++-2.4/libg++/src/gen', 8379 ).
file('SplayBag.hP',text,'libg++-2.4/libg++/src/gen', 3658 ).
file('SplayMap.ccP',text,'libg++-2.4/libg++/src/gen', 7948 ).
file('SplayMap.hP',text,'libg++-2.4/libg++/src/gen', 3588 ).
file('SplayNode.ccP',text,'libg++-2.4/libg++/src/gen', 905 ).
file('SplayNode.hP',text,'libg++-2.4/libg++/src/gen', 1424 ).
file('SplayPQ.ccP',text,'libg++-2.4/libg++/src/gen', 9926 ).
file('SplayPQ.hP',text,'libg++-2.4/libg++/src/gen', 2663 ).
file('SplaySet.ccP',text,'libg++-2.4/libg++/src/gen', 9320 ).
file('SplaySet.hP',text,'libg++-2.4/libg++/src/gen', 2971 ).
file('Stack.ccP',text,'libg++-2.4/libg++/src/gen', 176 ).
file('Stack.hP',text,'libg++-2.4/libg++/src/gen', 1516 ).
file('VHBag.ccP',text,'libg++-2.4/libg++/src/gen', 6269 ).
file('VHBag.hP',text,'libg++-2.4/libg++/src/gen', 1984 ).
file('VHMap.ccP',text,'libg++-2.4/libg++/src/gen', 5073 ).
file('VHMap.hP',text,'libg++-2.4/libg++/src/gen', 2082 ).
file('VHSet.ccP',text,'libg++-2.4/libg++/src/gen', 6001 ).
file('VHSet.hP',text,'libg++-2.4/libg++/src/gen', 2245 ).
file('VOHSet.ccP',text,'libg++-2.4/libg++/src/gen', 7302 ).
file('VOHSet.hP',text,'libg++-2.4/libg++/src/gen', 2260 ).
file('VQueue.ccP',text,'libg++-2.4/libg++/src/gen', 2144 ).
file('VQueue.hP',text,'libg++-2.4/libg++/src/gen', 2746 ).
file('VStack.ccP',text,'libg++-2.4/libg++/src/gen', 1804 ).
file('VStack.hP',text,'libg++-2.4/libg++/src/gen', 2523 ).
file('Vec.ccP',text,'libg++-2.4/libg++/src/gen', 12305 ).
file('Vec.hP',text,'libg++-2.4/libg++/src/gen', 3433 ).
file('XPBag.ccP',text,'libg++-2.4/libg++/src/gen', 1772 ).
file('XPBag.hP',text,'libg++-2.4/libg++/src/gen', 2237 ).
file('XPDeque.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('XPDeque.hP',text,'libg++-2.4/libg++/src/gen', 2707 ).
file('XPPQ.ccP',text,'libg++-2.4/libg++/src/gen', 2962 ).
file('XPPQ.hP',text,'libg++-2.4/libg++/src/gen', 2293 ).
file('XPQueue.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('XPQueue.hP',text,'libg++-2.4/libg++/src/gen', 2396 ).
file('XPSet.ccP',text,'libg++-2.4/libg++/src/gen', 1572 ).
file('XPSet.hP',text,'libg++-2.4/libg++/src/gen', 2049 ).
file('XPStack.ccP',text,'libg++-2.4/libg++/src/gen', 71 ).
file('XPStack.hP',text,'libg++-2.4/libg++/src/gen', 2394 ).
file('XPlex.ccP',text,'libg++-2.4/libg++/src/gen', 8142 ).
file('XPlex.hP',text,'libg++-2.4/libg++/src/gen', 6031 ).
file('defs.hP',text,'libg++-2.4/libg++/src/gen', 1736 ).
file('intSList.hP',text,'libg++-2.4/libg++/src/gen', 1095 ).
file('intVec.hP',text,'libg++-2.4/libg++/src/gen', 2522 ).
file('ChangeLog',text,'libg++-2.4/libg++/test-install', 2555 ).
file('Foo.cc',text,'libg++-2.4/libg++/test-install', 1957 ).
file('Foo.h',text,'libg++-2.4/libg++/test-install', 399 ).
file('Makefile',text,'libg++-2.4/libg++/test-install', 8061 ).
file('Makefile.in',text,'libg++-2.4/libg++/test-install', 4209 ).
file('a.cc',text,'libg++-2.4/libg++/test-install', 119 ).
file('bf.cc',text,'libg++-2.4/libg++/test-install', 312 ).
file('bm.cc',text,'libg++-2.4/libg++/test-install', 310 ).
file('config.status',exec,'libg++-2.4/libg++/test-install', 226 ).
file('configure.in',text,'libg++-2.4/libg++/test-install', 590 ).
file('ex_bar.cc',text,'libg++-2.4/libg++/test-install', 2135 ).
file('expected.out',text,'libg++-2.4/libg++/test-install', 655 ).
file('foo_func.cc',text,'libg++-2.4/libg++/test-install', 243 ).
file('foo_main.cc',text,'libg++-2.4/libg++/test-install', 1081 ).
file('ChangeLog',text,'libg++-2.4/libg++/tests', 7161 ).
file('Makefile',text,'libg++-2.4/libg++/tests', 22962 ).
file('Makefile.in',text,'libg++-2.4/libg++/tests', 631 ).
file('Makefile.sh',exec,'libg++-2.4/libg++/tests', 4918 ).
file('config.status',exec,'libg++-2.4/libg++/tests', 226 ).
file('configure.in',text,'libg++-2.4/libg++/tests', 830 ).
file('depend',text,'libg++-2.4/libg++/tests', 4321 ).
file('tBag.cc',text,'libg++-2.4/libg++/tests', 14357 ).
file('tBag.exp',text,'libg++-2.4/libg++/tests', 1815 ).
file('tBitSet.cc',text,'libg++-2.4/libg++/tests', 4348 ).
file('tBitSet.exp',text,'libg++-2.4/libg++/tests', 1448 ).
file('tBitString.cc',text,'libg++-2.4/libg++/tests', 7582 ).
file('tBitString.exp',text,'libg++-2.4/libg++/tests', 2432 ).
file('tComplex.cc',text,'libg++-2.4/libg++/tests', 3608 ).
file('tComplex.exp',text,'libg++-2.4/libg++/tests', 805 ).
file('tComplex.inp',text,'libg++-2.4/libg++/tests', 11 ).
file('tCurses.cc',text,'libg++-2.4/libg++/tests', 1594 ).
file('tCurses.inp',text,'libg++-2.4/libg++/tests', 6 ).
file('tDeque.cc',text,'libg++-2.4/libg++/tests', 2943 ).
file('tDeque.exp',text,'libg++-2.4/libg++/tests', 173 ).
file('tFile.cc',text,'libg++-2.4/libg++/tests', 8231 ).
file('tFile.exp',text,'libg++-2.4/libg++/tests', 1412 ).
file('tFile.inp',text,'libg++-2.4/libg++/tests', 43 ).
file('tFix.cc',text,'libg++-2.4/libg++/tests', 2572 ).
file('tFix.exp',text,'libg++-2.4/libg++/tests', 1797 ).
file('tFix16.cc',text,'libg++-2.4/libg++/tests', 2633 ).
file('tFix16.exp',text,'libg++-2.4/libg++/tests', 1073 ).
file('tFix24.cc',text,'libg++-2.4/libg++/tests', 2687 ).
file('tFix24.exp',text,'libg++-2.4/libg++/tests', 952 ).
file('tGetOpt.cc',text,'libg++-2.4/libg++/tests', 1314 ).
file('tGetOpt.exp',text,'libg++-2.4/libg++/tests', 208 ).
file('tInteger.cc',text,'libg++-2.4/libg++/tests', 10420 ).
file('tInteger.exp',text,'libg++-2.4/libg++/tests', 1559 ).
file('tInteger.inp',text,'libg++-2.4/libg++/tests', 21 ).
file('tLList.cc',text,'libg++-2.4/libg++/tests', 5264 ).
file('tLList.exp',text,'libg++-2.4/libg++/tests', 1244 ).
file('tList.cc',text,'libg++-2.4/libg++/tests', 2912 ).
file('tList.exp',text,'libg++-2.4/libg++/tests', 1105 ).
file('tMap.cc',text,'libg++-2.4/libg++/tests', 7235 ).
file('tMap.exp',text,'libg++-2.4/libg++/tests', 3691 ).
file('tObstack.cc',text,'libg++-2.4/libg++/tests', 1610 ).
file('tObstack.exp',text,'libg++-2.4/libg++/tests', 257 ).
file('tObstack.inp',text,'libg++-2.4/libg++/tests', 257 ).
file('tPQ.cc',text,'libg++-2.4/libg++/tests', 5575 ).
file('tPQ.exp',text,'libg++-2.4/libg++/tests', 755 ).
file('tPlex.cc',text,'libg++-2.4/libg++/tests', 13121 ).
file('tPlex.exp',text,'libg++-2.4/libg++/tests', 366 ).
file('tQueue.cc',text,'libg++-2.4/libg++/tests', 3578 ).
file('tQueue.exp',text,'libg++-2.4/libg++/tests', 225 ).
file('tRandom.cc',text,'libg++-2.4/libg++/tests', 2449 ).
file('tRandom.exp',text,'libg++-2.4/libg++/tests', 2667 ).
file('tRational.cc',text,'libg++-2.4/libg++/tests', 4566 ).
file('tRational.exp',text,'libg++-2.4/libg++/tests', 7450 ).
file('tRational.inp',text,'libg++-2.4/libg++/tests', 19 ).
file('tSet.cc',text,'libg++-2.4/libg++/tests', 18241 ).
file('tSet.exp',text,'libg++-2.4/libg++/tests', 2687 ).
file('tStack.cc',text,'libg++-2.4/libg++/tests', 3603 ).
file('tStack.exp',text,'libg++-2.4/libg++/tests', 225 ).
file('tString.cc',text,'libg++-2.4/libg++/tests', 8789 ).
file('tString.exp',text,'libg++-2.4/libg++/tests', 1348 ).
file('tString.inp',text,'libg++-2.4/libg++/tests', 27 ).
file('tVec.cc',text,'libg++-2.4/libg++/tests', 2119 ).
file('tVec.exp',text,'libg++-2.4/libg++/tests', 1681 ).
file('test_h.cc',text,'libg++-2.4/libg++/tests', 2123 ).
file('tiLList.cc',text,'libg++-2.4/libg++/tests', 5447 ).
file('Makefile',text,'libg++-2.4/libg++/utils', 5077 ).
file('Makefile.in',text,'libg++-2.4/libg++/utils', 1264 ).
file('c++-mode.el',text,'libg++-2.4/libg++/utils', 88002 ).
file('config.status',exec,'libg++-2.4/libg++/utils', 226 ).
file('configure.in',text,'libg++-2.4/libg++/utils', 565 ).
file('etags',exec,'libg++-2.4/libg++/utils', 90112 ).
file('etags.c',text,'libg++-2.4/libg++/utils', 68191 ).
file('g++dep',exec,'libg++-2.4/libg++/utils', 1489 ).
file('g++dep.sh',exec,'libg++-2.4/libg++/utils', 1489 ).
file('gen-params',exec,'libg++-2.4/libg++/utils', 15516 ).
file('gendepend',exec,'libg++-2.4/libg++/utils', 1606 ).
file('AAAREADME.TXT',text,'libg++-2.4/libg++/vms', 13642 ).
file('CXLINK.COM',text,'libg++-2.4/libg++/vms', 266 ).
file('CXSHARE.COM',text,'libg++-2.4/libg++/vms', 343 ).
file('EXPECTED.LIST',text,'libg++-2.4/libg++/vms', 176 ).
file('EXPECTED.VMS',text,'libg++-2.4/libg++/vms', 37428 ).
file('FNDVEC.C',text,'libg++-2.4/libg++/vms', 6309 ).
file('GENCLASS.COM',text,'libg++-2.4/libg++/vms', 1615 ).
file('GENCLASS.TPU',text,'libg++-2.4/libg++/vms', 1629 ).
file('GXX_MAIN_SHR.MAR',text,'libg++-2.4/libg++/vms', 311 ).
file('OPTIONS.OPT',text,'libg++-2.4/libg++/vms', 195 ).
file('VMS-BUILD-LIBGXX.COM',text,'libg++-2.4/libg++/vms', 12096 ).
file('VMS-CURSES.C',text,'libg++-2.4/libg++/vms', 632 ).
file('VMS-GCCLIB.MAR',text,'libg++-2.4/libg++/vms', 773 ).
file('VMS-INSTALL-LIBGXX.COM',text,'libg++-2.4/libg++/vms', 10526 ).
file('VMS-TEST-LIBGXX.COM',text,'libg++-2.4/libg++/vms', 2882 ).
file('VMS_FIXINCLUDES.TPU',text,'libg++-2.4/libg++/vms', 1619 ).
file('_G_config.h',text,'libg++-2.4/libg++/vms', 1285 ).
file('COPYING.LIB',text,'libg++-2.4/libiberty', 25265 ).
file('ChangeLog',text,'libg++-2.4/libiberty', 26931 ).
file('Makefile',text,'libg++-2.4/libiberty', 7696 ).
file('Makefile.in',text,'libg++-2.4/libiberty', 7347 ).
file('README',text,'libg++-2.4/libiberty', 5400 ).
file('alloca-botch.h',text,'libg++-2.4/libiberty', 188 ).
file('alloca-norm.h',text,'libg++-2.4/libiberty', 217 ).
file('alloca.c',text,'libg++-2.4/libiberty', 5483 ).
file('argv.c',text,'libg++-2.4/libiberty', 7387 ).
file('basename.c',text,'libg++-2.4/libiberty', 1367 ).
file('bcmp.c',text,'libg++-2.4/libiberty', 1567 ).
file('bcopy.c',text,'libg++-2.4/libiberty', 1522 ).
file('bzero.c',text,'libg++-2.4/libiberty', 1286 ).
file('concat.c',text,'libg++-2.4/libiberty', 3087 ).
file('config',dir,'libg++-2.4/libiberty', 512 ).
file('config.h',text,'libg++-2.4/libiberty', 118 ).
file('config.status',exec,'libg++-2.4/libiberty', 198 ).
file('config.table',text,'libg++-2.4/libiberty', 649 ).
file('configure.bat',text,'libg++-2.4/libiberty', 292 ).
file('configure.in',text,'libg++-2.4/libiberty', 420 ).
file('copysign.c',text,'libg++-2.4/libiberty', 2490 ).
file('cplus-dem.c',text,'libg++-2.4/libiberty', 57523 ).
file('dummy',exec,'libg++-2.4/libiberty', 24576 ).
file('dummy.c',text,'libg++-2.4/libiberty', 1368 ).
file('errors',text,'libg++-2.4/libiberty', 74 ).
file('fdmatch.c',text,'libg++-2.4/libiberty', 2041 ).
file('functions.def',text,'libg++-2.4/libiberty', 2656 ).
file('getcwd.c',text,'libg++-2.4/libiberty', 1705 ).
file('getopt.c',text,'libg++-2.4/libiberty', 19056 ).
file('getopt1.c',text,'libg++-2.4/libiberty', 3598 ).
file('getpagesize.c',text,'libg++-2.4/libiberty', 2161 ).
file('ieee-float.c',exec,'libg++-2.4/libiberty', 4887 ).
file('index.c',text,'libg++-2.4/libiberty', 143 ).
file('insque.c',text,'libg++-2.4/libiberty', 1703 ).
file('lconfig.h',text,'libg++-2.4/libiberty', 118 ).
file('libiberty.a',text,'libg++-2.4/libiberty', 110456 ).
file('lneeded-list',text,'libg++-2.4/libiberty', 30 ).
file('memchr.c',text,'libg++-2.4/libiberty', 1185 ).
file('memcmp.c',text,'libg++-2.4/libiberty', 1548 ).
file('memcpy.c',text,'libg++-2.4/libiberty', 1280 ).
file('memmove.c',text,'libg++-2.4/libiberty', 1355 ).
file('memset.c',text,'libg++-2.4/libiberty', 1124 ).
file('msdos.c',text,'libg++-2.4/libiberty', 206 ).
file('needed-list',text,'libg++-2.4/libiberty', 30 ).
file('needed.awk',text,'libg++-2.4/libiberty', 1045 ).
file('needed2.awk',text,'libg++-2.4/libiberty', 473 ).
file('obstack.c',text,'libg++-2.4/libiberty', 13365 ).
file('random.c',text,'libg++-2.4/libiberty', 12800 ).
file('rename.c',text,'libg++-2.4/libiberty', 1110 ).
file('rindex.c',text,'libg++-2.4/libiberty', 147 ).
file('sigsetmask.c',text,'libg++-2.4/libiberty', 1415 ).
file('spaces.c',text,'libg++-2.4/libiberty', 1545 ).
file('strchr.c',text,'libg++-2.4/libiberty', 1343 ).
file('strdup.c',text,'libg++-2.4/libiberty', 172 ).
file('strerror.c',text,'libg++-2.4/libiberty', 20370 ).
file('strrchr.c',text,'libg++-2.4/libiberty', 1360 ).
file('strsignal.c',text,'libg++-2.4/libiberty', 14678 ).
file('strstr.c',text,'libg++-2.4/libiberty', 1717 ).
file('strtod.c',text,'libg++-2.4/libiberty', 2605 ).
file('strtol.c',text,'libg++-2.4/libiberty', 1196 ).
file('strtoul.c',text,'libg++-2.4/libiberty', 1778 ).
file('tmpnam.c',text,'libg++-2.4/libiberty', 619 ).
file('vfork.c',text,'libg++-2.4/libiberty', 916 ).
file('vfprintf.c',text,'libg++-2.4/libiberty', 184 ).
file('vprintf.c',text,'libg++-2.4/libiberty', 160 ).
file('vsprintf.c',text,'libg++-2.4/libiberty', 1501 ).
file('mh-a68bsd',text,'libg++-2.4/libiberty/config', 70 ).
file('mh-aix',text,'libg++-2.4/libiberty/config', 442 ).
file('mh-apollo68',text,'libg++-2.4/libiberty/config', 68 ).
file('mh-hpux',text,'libg++-2.4/libiberty/config', 24 ).
file('mh-irix4',text,'libg++-2.4/libiberty/config', 65 ).
file('mh-ncr3000',text,'libg++-2.4/libiberty/config', 741 ).
file('mh-sysv',text,'libg++-2.4/libiberty/config', 12 ).
file('mh-sysv4',text,'libg++-2.4/libiberty/config', 51 ).
file('gpl.texinfo',text,'libg++-2.4/texinfo', 18545 ).
file('tex3patch',exec,'libg++-2.4/texinfo', 1762 ).
file('texinfo.tex',text,'libg++-2.4/texinfo', 131666 ).
file('configure.in',text,'libg++-2.4/xiberty', 2158 ).
file('COPYING',text,'make-3.68', 17982 ).
file('ChangeLog',text,'make-3.68', 266533 ).
file('INSTALL',text,'make-3.68', 5776 ).
file('Makefile',text,'make-3.68', 9564 ).
file('Makefile.in',text,'make-3.68', 9550 ).
file('NEWS',text,'make-3.68', 16496 ).
file('README',text,'make-3.68', 1101 ).
file('TAGS',text,'make-3.68', 11471 ).
file('alloca.c',text,'make-3.68', 13780 ).
file('ar.c',text,'make-3.68', 7304 ).
file('arscan.c',text,'make-3.68', 13101 ).
file('build.sh',text,'make-3.68', 2341 ).
file('build.sh.in',text,'make-3.68', 2313 ).
file('commands.c',text,'make-3.68', 11410 ).
file('commands.h',text,'make-3.68', 1673 ).
file('config.h',text,'make-3.68', 4860 ).
file('config.h.in',text,'make-3.68', 4542 ).
file('config.status',exec,'make-3.68', 9849 ).
file('configure',exec,'make-3.68', 52699 ).
file('configure.in',text,'make-3.68', 2304 ).
file('default.c',text,'make-3.68', 9564 ).
file('dep.h',text,'make-3.68', 2046 ).
file('dir.c',text,'make-3.68', 14824 ).
file('expand.c',text,'make-3.68', 12073 ).
file('file.c',text,'make-3.68', 12588 ).
file('file.h',text,'make-3.68', 3778 ).
file('function.c',text,'make-3.68', 30592 ).
file('getloadavg.c',text,'make-3.68', 20130 ).
file('getopt.c',text,'make-3.68', 21362 ).
file('getopt.h',text,'make-3.68', 4412 ).
file('getopt1.c',text,'make-3.68', 4142 ).
file('glob',dir,'make-3.68', 512 ).
file('implicit.c',text,'make-3.68', 18475 ).
file('job.c',text,'make-3.68', 36188 ).
file('job.h',text,'make-3.68', 1975 ).
file('main.c',text,'make-3.68', 46546 ).
file('make-stds.texi',text,'make-3.68', 20304 ).
file('make.aux',text,'make-3.68', 8704 ).
file('make.cp',text,'make-3.68', 39162 ).
file('make.cps',text,'make-3.68', 23131 ).
file('make.fn',text,'make-3.68', 6620 ).
file('make.fns',text,'make-3.68', 4869 ).
file('make.h',text,'make-3.68', 7567 ).
file('make.info',text,'make-3.68', 4564 ).
file('make.info-1',text,'make-3.68', 50836 ).
file('make.info-2',text,'make-3.68', 49297 ).
file('make.info-3',text,'make-3.68', 50658 ).
file('make.info-4',text,'make-3.68', 47527 ).
file('make.info-5',text,'make-3.68', 49056 ).
file('make.info-6',text,'make-3.68', 47230 ).
file('make.info-7',text,'make-3.68', 17209 ).
file('make.info-8',text,'make-3.68', 47987 ).
file('make.man',text,'make-3.68', 7671 ).
file('make.texinfo',text,'make-3.68', 322809 ).
file('make.toc',text,'make-3.68', 6187 ).
file('misc.c',text,'make-3.68', 15007 ).
file('mkinstalldirs',exec,'make-3.68', 619 ).
file('read.c',text,'make-3.68', 51567 ).
file('remake.c',text,'make-3.68', 25704 ).
file('remote-cstms.c',text,'make-3.68', 7755 ).
file('remote-stub.c',text,'make-3.68', 2629 ).
file('rule.c',text,'make-3.68', 15515 ).
file('rule.h',text,'make-3.68', 1717 ).
file('signame.c',text,'make-3.68', 7014 ).
file('signame.h',text,'make-3.68', 1816 ).
file('tags',text,'make-3.68', 13787 ).
file('texinfo.tex',text,'make-3.68', 131902 ).
file('variable.c',text,'make-3.68', 21707 ).
file('variable.h',text,'make-3.68', 3514 ).
file('version.c',text,'make-3.68', 91 ).
file('vpath.c',text,'make-3.68', 13100 ).
file('COPYING.LIB',text,'make-3.68/glob', 25265 ).
file('Makefile',text,'make-3.68/glob', 1843 ).
file('Makefile.in',text,'make-3.68/glob', 1786 ).
file('fnmatch.c',text,'make-3.68/glob', 4878 ).
file('fnmatch.h',text,'make-3.68/glob', 2158 ).
file('glob.c',text,'make-3.68/glob', 16168 ).
file('glob.h',text,'make-3.68/glob', 3645 ).
file('COPYING',text,'patch-2.1', 17982 ).
file('ChangeLog',text,'patch-2.1', 12118 ).
file('EXTERN.h',text,'patch-2.1', 299 ).
file('INSTALL',text,'patch-2.1', 5776 ).
file('INTERN.h',text,'patch-2.1', 276 ).
file('Makefile',text,'patch-2.1', 2178 ).
file('Makefile.in',text,'patch-2.1', 2157 ).
file('NEWS',text,'patch-2.1', 708 ).
file('README',text,'patch-2.1', 2047 ).
file('alloca.c',text,'patch-2.1', 13704 ).
file('backupfile.c',text,'patch-2.1', 9790 ).
file('backupfile.h',text,'patch-2.1', 1467 ).
file('common.h',text,'patch-2.1', 4642 ).
file('config.h',text,'patch-2.1', 2460 ).
file('config.h.in',text,'patch-2.1', 2332 ).
file('config.status',exec,'patch-2.1', 5240 ).
file('configure',exec,'patch-2.1', 30889 ).
file('configure.in',text,'patch-2.1', 450 ).
file('getopt.c',text,'patch-2.1', 20876 ).
file('getopt.h',text,'patch-2.1', 4412 ).
file('getopt1.c',text,'patch-2.1', 4142 ).
file('inp.c',text,'patch-2.1', 9902 ).
file('inp.h',text,'patch-2.1', 473 ).
file('patch.c',text,'patch-2.1', 22788 ).
file('patch.man',text,'patch-2.1', 17676 ).
file('patchlevel.h',text,'patch-2.1', 28 ).
file('pch.c',text,'patch-2.1', 33105 ).
file('pch.h',text,'patch-2.1', 780 ).
file('rename.c',text,'patch-2.1', 1323 ).
file('util.c',text,'patch-2.1', 9361 ).
file('util.h',text,'patch-2.1', 2476 ).
file('version.c',text,'patch-2.1', 448 ).
file('version.h',text,'patch-2.1', 185 ).
file('CW.ms',text,'portability', 207 ).
file('KR2.errata',text,'portability', 4699 ).
file('Makefile',text,'portability', 1727 ).
file('README',text,'portability', 1279 ).
file('ansi-c.summary',text,'portability', 32690 ).
file('bornat.errata',text,'portability', 2760 ).
file('c-news.doc',text,'portability', 48122 ).
file('c-news.ps',text,'portability', 88718 ).
file('c-news.tbl.ms',text,'portability', 35864 ).
file('c.quiz',text,'portability', 3533 ).
file('canthappen.PS',text,'portability', 104937 ).
file('defensive',text,'portability', 3062 ).
file('dichter.inspection',text,'portability', 2000 ).
file('ihstyle.doc',text,'portability', 49504 ).
file('ihstyle.ps',text,'portability', 99861 ).
file('ihstyle.tbl.ms',text,'portability', 39118 ).
file('mistakes.tips',text,'portability', 1795 ).
file('oracle.five-fold-path',text,'portability', 3315 ).
file('pikestyle.doc',text,'portability', 17882 ).
file('pikestyle.ms',text,'portability', 15947 ).
file('pikestyle.ps',text,'portability', 43968 ).
file('portability',text,'portability', 1868 ).
file('setuid.doc',text,'portability', 12365 ).
file('setuid.man',text,'portability', 9462 ).
file('setuid.ps',text,'portability', 32504 ).
file('steal.doc',text,'portability', 39805 ).
file('steal.ms',text,'portability', 31959 ).
file('steal.ps',text,'portability', 78518 ).
file('ten-commandments',text,'portability', 2521 ).
file('COPYING',text,'rcs', 17982 ).
file('Makefile',text,'rcs', 300 ).
file('README',text,'rcs', 17679 ).
file('man',dir,'rcs', 512 ).
file('rcs.ms',text,'rcs', 56906 ).
file('rcs.ps',text,'rcs', 141433 ).
file('rcs_func.ms',text,'rcs', 3683 ).
file('src',dir,'rcs', 1024 ).
file('Makefile',text,'rcs/man', 603 ).
file('ci.1',text,'rcs/man', 17938 ).
file('co.1',text,'rcs/man', 13648 ).
file('ident.1',text,'rcs/man', 1414 ).
file('merge.1',text,'rcs/man', 1787 ).
file('rcs.1',text,'rcs/man', 8642 ).
file('rcsclean.1',text,'rcs/man', 3703 ).
file('rcsdiff.1',text,'rcs/man', 2809 ).
file('rcsfile.5',text,'rcs/man', 6267 ).
file('rcsfreeze.1',text,'rcs/man', 1723 ).
file('rcsintro.1',text,'rcs/man', 9299 ).
file('rcsmerge.1',text,'rcs/man', 3305 ).
file('rlog.1',text,'rcs/man', 5697 ).
file('Makefile',text,'rcs/src', 6792 ).
file('Makefile.orig',text,'rcs/src', 6792 ).
file('README',text,'rcs/src', 9190 ).
file('ci.c',text,'rcs/src', 33764 ).
file('co.c',text,'rcs/src', 21119 ).
file('conf.heg',text,'rcs/src', 14899 ).
file('conf.sh',text,'rcs/src', 35856 ).
file('ident.c',text,'rcs/src', 5571 ).
file('maketime.c',text,'rcs/src', 9481 ).
file('merge.c',text,'rcs/src', 2143 ).
file('merger.c',text,'rcs/src', 3036 ).
file('partime.c',text,'rcs/src', 18891 ).
file('rcs.c',text,'rcs/src', 43795 ).
file('rcsbase.h',text,'rcs/src', 22152 ).
file('rcsclean.c',text,'rcs/src', 6396 ).
file('rcsdiff.c',text,'rcs/src', 11285 ).
file('rcsedit.c',text,'rcs/src', 39683 ).
file('rcsfcmp.c',text,'rcs/src', 8406 ).
file('rcsfnms.c',text,'rcs/src', 27744 ).
file('rcsfreeze.sh',text,'rcs/src', 3267 ).
file('rcsgen.c',text,'rcs/src', 11606 ).
file('rcskeep.c',text,'rcs/src', 10028 ).
file('rcskeys.c',text,'rcs/src', 2683 ).
file('rcslex.c',text,'rcs/src', 27526 ).
file('rcsmap.c',text,'rcs/src', 3048 ).
file('rcsmerge.c',text,'rcs/src', 6990 ).
file('rcsrev.c',text,'rcs/src', 21002 ).
file('rcssyn.c',text,'rcs/src', 20108 ).
file('rcstest',exec,'rcs/src', 9809 ).
file('rcsutil.c',text,'rcs/src', 20392 ).
file('rlog.c',text,'rcs/src', 32185 ).
file('ChangeLog',text,'rdist-6.0.0', 16279 ).
file('Makefile',text,'rdist-6.0.0', 4002 ).
file('README',text,'rdist-6.0.0', 6814 ).
file('child.c',text,'rdist-6.0.0', 12647 ).
file('client.c',text,'rdist-6.0.0', 20719 ).
file('common.c',text,'rdist-6.0.0', 15942 ).
file('config-data.h',text,'rdist-6.0.0', 2406 ).
file('config-info.h',text,'rdist-6.0.0', 2546 ).
file('config-os.h',text,'rdist-6.0.0', 2455 ).
file('config.h',text,'rdist-6.0.0', 7998 ).
file('defs.h',text,'rdist-6.0.0', 12510 ).
file('distopt.c',text,'rdist-6.0.0', 4492 ).
file('docmd.c',text,'rdist-6.0.0', 21888 ).
file('expand.c',text,'rdist-6.0.0', 11474 ).
file('filesys-os.c',text,'rdist-6.0.0', 7520 ).
file('filesys.c',text,'rdist-6.0.0', 10512 ).
file('filesys.h',text,'rdist-6.0.0', 3811 ).
file('gram.y',text,'rdist-6.0.0', 12166 ).
file('isexec.c',text,'rdist-6.0.0', 4016 ).
file('lookup.c',text,'rdist-6.0.0', 4314 ).
file('message.c',text,'rdist-6.0.0', 15936 ).
file('pathnames.h',text,'rdist-6.0.0', 2169 ).
file('paths.h',text,'rdist-6.0.0', 2183 ).
file('rdist.1',text,'rdist-6.0.0', 19268 ).
file('rdist.c',text,'rdist-6.0.0', 9437 ).
file('rdistcfilter.pl',text,'rdist-6.0.0', 6600 ).
file('rdistd.8',text,'rdist-6.0.0', 2604 ).
file('rdistd.c',text,'rdist-6.0.0', 3049 ).
file('rdistvfilter.pl',text,'rdist-6.0.0', 1812 ).
file('regex.c',text,'rdist-6.0.0', 9973 ).
file('server.c',text,'rdist-6.0.0', 30435 ).
file('setargs.c',text,'rdist-6.0.0', 3758 ).
file('strcasecmp.c',text,'rdist-6.0.0', 3242 ).
file('strdup.c',text,'rdist-6.0.0', 622 ).
file('strerror.c',text,'rdist-6.0.0', 621 ).
file('version.h',text,'rdist-6.0.0', 2225 ).
file('AUTHORS',text,'regex-0.12', 408 ).
file('COPYING',text,'regex-0.12', 17982 ).
file('ChangeLog',text,'regex-0.12', 139627 ).
file('INSTALL',text,'regex-0.12', 5733 ).
file('Makefile',text,'regex-0.12', 2963 ).
file('Makefile.in',text,'regex-0.12', 2874 ).
file('NEWS',text,'regex-0.12', 2024 ).
file('README',text,'regex-0.12', 2271 ).
file('config.status',exec,'regex-0.12', 1614 ).
file('configure',exec,'regex-0.12', 12190 ).
file('configure.in',text,'regex-0.12', 439 ).
file('doc',dir,'regex-0.12', 512 ).
file('regex.c',text,'regex-0.12', 161730 ).
file('regex.h',text,'regex-0.12', 18702 ).
file('test',dir,'regex-0.12', 1024 ).
file('Makefile',text,'regex-0.12/doc', 2502 ).
file('Makefile.in',text,'regex-0.12/doc', 2447 ).
file('include.awk',text,'regex-0.12/doc', 645 ).
file('regex.aux',text,'regex-0.12/doc', 6096 ).
file('regex.cps',text,'regex-0.12/doc', 5783 ).
file('regex.info',text,'regex-0.12/doc', 119268 ).
file('regex.texi',text,'regex-0.12/doc', 119680 ).
file('texinfo.tex',text,'regex-0.12/doc', 128926 ).
file('xregex.texi',text,'regex-0.12/doc', 114026 ).
file('ChangeLog',text,'regex-0.12/test', 2481 ).
file('Makefile',text,'regex-0.12/test', 4776 ).
file('Makefile.in',text,'regex-0.12/test', 4708 ).
file('TAGS',text,'regex-0.12/test', 9586 ).
file('alloca.c',text,'regex-0.12/test', 5327 ).
file('bsd-interf.c',text,'regex-0.12/test', 824 ).
file('debugmalloc.c',text,'regex-0.12/test', 5863 ).
file('emacsmalloc.c',text,'regex-0.12/test', 21379 ).
file('fileregex.c',text,'regex-0.12/test', 1662 ).
file('g++malloc.c',text,'regex-0.12/test', 34715 ).
file('getpagesize.h',text,'regex-0.12/test', 416 ).
file('iregex.c',text,'regex-0.12/test', 3164 ).
file('main.c',text,'regex-0.12/test', 819 ).
file('malloc-test.c',text,'regex-0.12/test', 929 ).
file('other.c',text,'regex-0.12/test', 19577 ).
file('printchar.c',text,'regex-0.12/test', 229 ).
file('psx-basic.c',text,'regex-0.12/test', 9063 ).
file('psx-extend.c',text,'regex-0.12/test', 42094 ).
file('psx-generic.c',text,'regex-0.12/test', 11301 ).
file('psx-group.c',text,'regex-0.12/test', 19112 ).
file('psx-interf.c',text,'regex-0.12/test', 18939 ).
file('psx-interv.c',text,'regex-0.12/test', 6310 ).
file('regexcpp.sed',text,'regex-0.12/test', 69 ).
file('syntax.skel',text,'regex-0.12/test', 1585 ).
file('test.c',text,'regex-0.12/test', 22359 ).
file('test.h',text,'regex-0.12/test', 4210 ).
file('tregress.c',text,'regex-0.12/test', 18247 ).
file('upcase.c',text,'regex-0.12/test', 1712 ).
file('xmalloc.c',text,'regex-0.12/test', 303 ).
file('README',text,'regex-doc', 1957 ).
file('TR92gas01.dvi',text,'regex-doc', 395788 ).
file('TR92gas01.ps',text,'regex-doc', 778262 ).
file('TR92gas01.tex',text,'regex-doc', 283118 ).
file('TR92gas01.us.dvi',text,'regex-doc', 400484 ).
file('TR92gas01.us.ps',text,'regex-doc', 793812 ).
file('a4wide.sty',text,'regex-doc', 1407 ).
file('amssymbols.sty',text,'regex-doc', 9663 ).
file('COPYING',text,'sed-1.17', 17982 ).
file('COPYING.LIB',text,'sed-1.17', 25265 ).
file('ChangeLog',text,'sed-1.17', 11565 ).
file('INSTALL',text,'sed-1.17', 4119 ).
file('Makefile',text,'sed-1.17', 3012 ).
file('Makefile.in',text,'sed-1.17', 2907 ).
file('README',text,'sed-1.17', 1781 ).
file('alloca.c',text,'sed-1.17', 5356 ).
file('config.status',exec,'sed-1.17', 1434 ).
file('configure',exec,'sed-1.17', 11990 ).
file('configure.in',text,'sed-1.17', 263 ).
file('getopt.c',text,'sed-1.17', 20876 ).
file('getopt.h',text,'sed-1.17', 4412 ).
file('getopt1.c',text,'sed-1.17', 4142 ).
file('regex.c',text,'sed-1.17', 161729 ).
file('regex.h',text,'sed-1.17', 18702 ).
file('sed.c',text,'sed-1.17', 41502 ).
file('utils.c',text,'sed-1.17', 5293 ).
file('COPYING',text,'shellutils-1.8', 17982 ).
file('COPYING.LIB',text,'shellutils-1.8', 25265 ).
file('ChangeLog',text,'shellutils-1.8', 23587 ).
file('INSTALL',text,'shellutils-1.8', 5458 ).
file('Makefile',text,'shellutils-1.8', 4815 ).
file('Makefile.in',text,'shellutils-1.8', 4585 ).
file('NEWS',text,'shellutils-1.8', 532 ).
file('README',text,'shellutils-1.8', 847 ).
file('config.status',exec,'shellutils-1.8', 1670 ).
file('configure',exec,'shellutils-1.8', 18743 ).
file('configure.in',text,'shellutils-1.8', 2488 ).
file('lib',dir,'shellutils-1.8', 1024 ).
file('man',dir,'shellutils-1.8', 512 ).
file('src',dir,'shellutils-1.8', 1536 ).
file('Makefile',text,'shellutils-1.8/lib', 2412 ).
file('Makefile.in',text,'shellutils-1.8/lib', 2368 ).
file('alloca.c',text,'shellutils-1.8/lib', 5327 ).
file('basename.c',text,'shellutils-1.8/lib', 1085 ).
file('error.c',text,'shellutils-1.8/lib', 2927 ).
file('getdate.c',text,'shellutils-1.8/lib', 47280 ).
file('getdate.y',text,'shellutils-1.8/lib', 22778 ).
file('gethostname.c',text,'shellutils-1.8/lib', 1471 ).
file('getopt.c',text,'shellutils-1.8/lib', 19292 ).
file('getopt.h',text,'shellutils-1.8/lib', 4333 ).
file('getopt1.c',text,'shellutils-1.8/lib', 3427 ).
file('getugroups.c',text,'shellutils-1.8/lib', 2451 ).
file('getusershell.c',text,'shellutils-1.8/lib', 4176 ).
file('libshu.a',text,'shellutils-1.8/lib', 146836 ).
file('mktime.c',text,'shellutils-1.8/lib', 5710 ).
file('posixtm.c',text,'shellutils-1.8/lib', 22677 ).
file('posixtm.y',text,'shellutils-1.8/lib', 3439 ).
file('putenv.c',text,'shellutils-1.8/lib', 2554 ).
file('regex.c',text,'shellutils-1.8/lib', 159196 ).
file('regex.h',text,'shellutils-1.8/lib', 18484 ).
file('stime.c',text,'shellutils-1.8/lib', 1087 ).
file('strcspn.c',text,'shellutils-1.8/lib', 1170 ).
file('strftime.c',text,'shellutils-1.8/lib', 10846 ).
file('stripslash.c',text,'shellutils-1.8/lib', 1370 ).
file('strstr.c',text,'shellutils-1.8/lib', 1388 ).
file('strtod.c',text,'shellutils-1.8/lib', 4449 ).
file('strtol.c',text,'shellutils-1.8/lib', 4031 ).
file('strtoul.c',text,'shellutils-1.8/lib', 865 ).
file('xmalloc.c',text,'shellutils-1.8/lib', 1615 ).
file('xstrdup.c',text,'shellutils-1.8/lib', 1063 ).
file('xwrite.c',text,'shellutils-1.8/lib', 1138 ).
file('Makefile',text,'shellutils-1.8/man', 2023 ).
file('Makefile.in',text,'shellutils-1.8/man', 1955 ).
file('basename.1',text,'shellutils-1.8/man', 443 ).
file('date.1',text,'shellutils-1.8/man', 2832 ).
file('dirname.1',text,'shellutils-1.8/man', 359 ).
file('echo.1',text,'shellutils-1.8/man', 780 ).
file('env.1',text,'shellutils-1.8/man', 1411 ).
file('expr.1',text,'shellutils-1.8/man', 3408 ).
file('false.1',text,'shellutils-1.8/man', 382 ).
file('groups.1',text,'shellutils-1.8/man', 499 ).
file('id.1',text,'shellutils-1.8/man', 1377 ).
file('logname.1',text,'shellutils-1.8/man', 403 ).
file('nice.1',text,'shellutils-1.8/man', 1031 ).
file('nohup.1',text,'shellutils-1.8/man', 1016 ).
file('pathchk.1',text,'shellutils-1.8/man', 1179 ).
file('printenv.1',text,'shellutils-1.8/man', 482 ).
file('printf.1',text,'shellutils-1.8/man', 857 ).
file('sleep.1',text,'shellutils-1.8/man', 449 ).
file('stty.1',text,'shellutils-1.8/man', 8604 ).
file('su.1',text,'shellutils-1.8/man', 3377 ).
file('tee.1',text,'shellutils-1.8/man', 939 ).
file('test.1',text,'shellutils-1.8/man', 3099 ).
file('true.1',text,'shellutils-1.8/man', 448 ).
file('tty.1',text,'shellutils-1.8/man', 791 ).
file('uname.1',text,'shellutils-1.8/man', 1162 ).
file('who.1',text,'shellutils-1.8/man', 1904 ).
file('whoami.1',text,'shellutils-1.8/man', 292 ).
file('yes.1',text,'shellutils-1.8/man', 391 ).
file('Makefile',text,'shellutils-1.8/src', 4778 ).
file('Makefile.in',text,'shellutils-1.8/src', 4720 ).
file('basename',exec,'shellutils-1.8/src', 32768 ).
file('basename.c',text,'shellutils-1.8/src', 1919 ).
file('date',exec,'shellutils-1.8/src', 81920 ).
file('date.c',text,'shellutils-1.8/src', 4743 ).
file('dirname',exec,'shellutils-1.8/src', 32768 ).
file('dirname.c',text,'shellutils-1.8/src', 1448 ).
file('echo',exec,'shellutils-1.8/src', 24576 ).
file('echo.c',text,'shellutils-1.8/src', 4157 ).
file('env',exec,'shellutils-1.8/src', 40960 ).
file('env.c',text,'shellutils-1.8/src', 4550 ).
file('expr',exec,'shellutils-1.8/src', 98304 ).
file('expr.c',text,'shellutils-1.8/src', 11255 ).
file('false',exec,'shellutils-1.8/src', 17 ).
file('false.sh',exec,'shellutils-1.8/src', 17 ).
file('groups',exec,'shellutils-1.8/src', 1060 ).
file('groups.sh',exec,'shellutils-1.8/src', 1054 ).
file('id',exec,'shellutils-1.8/src', 49152 ).
file('id.c',text,'shellutils-1.8/src', 7492 ).
file('logname',exec,'shellutils-1.8/src', 24576 ).
file('logname.c',text,'shellutils-1.8/src', 1158 ).
file('nice',exec,'shellutils-1.8/src', 40960 ).
file('nice.c',text,'shellutils-1.8/src', 3300 ).
file('nohup',exec,'shellutils-1.8/src', 1630 ).
file('nohup.sh',exec,'shellutils-1.8/src', 1624 ).
file('pathchk',exec,'shellutils-1.8/src', 49152 ).
file('pathchk.c',text,'shellutils-1.8/src', 8643 ).
file('printenv',exec,'shellutils-1.8/src', 24576 ).
file('printenv.c',text,'shellutils-1.8/src', 1726 ).
file('printf',exec,'shellutils-1.8/src', 49152 ).
file('printf.c',text,'shellutils-1.8/src', 9527 ).
file('sleep',exec,'shellutils-1.8/src', 32768 ).
file('sleep.c',text,'shellutils-1.8/src', 1777 ).
file('stty',exec,'shellutils-1.8/src', 65536 ).
file('stty.c',text,'shellutils-1.8/src', 28146 ).
file('su',exec,'shellutils-1.8/src', 57344 ).
file('su.c',text,'shellutils-1.8/src', 13001 ).
file('system.h',text,'shellutils-1.8/src', 2780 ).
file('tee',exec,'shellutils-1.8/src', 40960 ).
file('tee.c',text,'shellutils-1.8/src', 3560 ).
file('test',exec,'shellutils-1.8/src', 49152 ).
file('test.c',text,'shellutils-1.8/src', 23709 ).
file('true',exec,'shellutils-1.8/src', 17 ).
file('true.sh',exec,'shellutils-1.8/src', 17 ).
file('tty',exec,'shellutils-1.8/src', 32768 ).
file('tty.c',text,'shellutils-1.8/src', 1955 ).
file('uname',exec,'shellutils-1.8/src', 40960 ).
file('uname.c',text,'shellutils-1.8/src', 3559 ).
file('version.c',text,'shellutils-1.8/src', 127 ).
file('who',exec,'shellutils-1.8/src', 49152 ).
file('who.c',text,'shellutils-1.8/src', 8993 ).
file('whoami',exec,'shellutils-1.8/src', 24576 ).
file('whoami.c',text,'shellutils-1.8/src', 1326 ).
file('yes',exec,'shellutils-1.8/src', 24576 ).
file('yes.c',text,'shellutils-1.8/src', 1090 ).
file('COPYING',text,'tar-1.11.2', 17982 ).
file('ChangeLog',text,'tar-1.11.2', 61978 ).
file('INSTALL',text,'tar-1.11.2', 5733 ).
file('Makefile.in',text,'tar-1.11.2', 6306 ).
file('NEWS',text,'tar-1.11.2', 3859 ).
file('README',text,'tar-1.11.2', 1839 ).
file('alloca.c',text,'tar-1.11.2', 5356 ).
file('backup-specs',text,'tar-1.11.2', 2661 ).
file('buffer.c',text,'tar-1.11.2', 35194 ).
file('configure',exec,'tar-1.11.2', 19657 ).
file('configure.in',text,'tar-1.11.2', 1551 ).
file('create.c',text,'tar-1.11.2', 33888 ).
file('diffarch.c',text,'tar-1.11.2', 16692 ).
file('dump-remind',exec,'tar-1.11.2', 2969 ).
file('extract.c',text,'tar-1.11.2', 24051 ).
file('fnmatch.c',text,'tar-1.11.2', 3960 ).
file('fnmatch.h',text,'tar-1.11.2', 1955 ).
file('getdate.c',text,'tar-1.11.2', 46681 ).
file('getdate.y',text,'tar-1.11.2', 22885 ).
file('getoldopt.c',text,'tar-1.11.2', 2310 ).
file('getopt.c',text,'tar-1.11.2', 20137 ).
file('getopt.h',text,'tar-1.11.2', 4333 ).
file('getopt1.c',text,'tar-1.11.2', 3486 ).
file('getpagesize.h',text,'tar-1.11.2', 610 ).
file('gnu.c',text,'tar-1.11.2', 13964 ).
file('level-0',exec,'tar-1.11.2', 6470 ).
file('level-1',exec,'tar-1.11.2', 6590 ).
file('list.c',text,'tar-1.11.2', 20047 ).
file('makefile.pc',text,'tar-1.11.2', 1700 ).
file('malloc.c',text,'tar-1.11.2', 30710 ).
file('mangle.c',text,'tar-1.11.2', 6635 ).
file('msd_dir.c',text,'tar-1.11.2', 4500 ).
file('msd_dir.h',text,'tar-1.11.2', 1060 ).
file('names.c',text,'tar-1.11.2', 3302 ).
file('open3.h',text,'tar-1.11.2', 2639 ).
file('pathmax.h',text,'tar-1.11.2', 1691 ).
file('port.c',text,'tar-1.11.2', 25899 ).
file('port.h',text,'tar-1.11.2', 5312 ).
file('regex.c',text,'tar-1.11.2', 161195 ).
file('regex.h',text,'tar-1.11.2', 18688 ).
file('rmt.c',text,'tar-1.11.2', 5987 ).
file('rmt.h',text,'tar-1.11.2', 3321 ).
file('rtapelib.c',text,'tar-1.11.2', 14007 ).
file('tar.c',text,'tar-1.11.2', 36191 ).
file('tar.h',text,'tar-1.11.2', 9318 ).
file('tcexparg.c',text,'tar-1.11.2', 6079 ).
file('testpad.c',text,'tar-1.11.2', 1541 ).
file('update.c',text,'tar-1.11.2', 13900 ).
file('version.c',text,'tar-1.11.2', 50 ).
file('COPYING',text,'termcap-1.1', 17982 ).
file('ChangeLog',text,'termcap-1.1', 316 ).
file('INSTALL',text,'termcap-1.1', 5458 ).
file('Makefile',text,'termcap-1.1', 3409 ).
file('Makefile.in',text,'termcap-1.1', 3326 ).
file('NEWS',text,'termcap-1.1', 119 ).
file('README',text,'termcap-1.1', 685 ).
file('config.status',exec,'termcap-1.1', 1247 ).
file('configure',exec,'termcap-1.1', 7850 ).
file('configure.in',text,'termcap-1.1', 199 ).
file('libtermcap.a',text,'termcap-1.1', 18340 ).
file('termcap.c',text,'termcap-1.1', 14579 ).
file('termcap.h',text,'termcap-1.1', 1585 ).
file('termcap.info',text,'termcap-1.1', 2095 ).
file('termcap.info-1',text,'termcap-1.1', 48597 ).
file('termcap.info-2',text,'termcap-1.1', 43788 ).
file('termcap.info-3',text,'termcap-1.1', 49048 ).
file('termcap.info-4',text,'termcap-1.1', 10345 ).
file('termcap.texi',text,'termcap-1.1', 142108 ).
file('texinfo.tex',text,'termcap-1.1', 119530 ).
file('tparam.c',text,'termcap-1.1', 7065 ).
file('version.c',text,'termcap-1.1', 130 ).
file('Makefile',text,'texi2roff', 537 ).
file('Readme',text,'texi2roff', 3773 ).
file('copyright',text,'texi2roff', 1632 ).
file('table.c',text,'texi2roff', 2621 ).
file('tableme.h',text,'texi2roff', 6905 ).
file('tablemm.h',text,'texi2roff', 6840 ).
file('tablems.h',text,'texi2roff', 6840 ).
file('texi2roff',exec,'texi2roff', 49152 ).
file('texi2roff.1',text,'texi2roff', 3669 ).
file('texi2roff.c',text,'texi2roff', 3086 ).
file('texi2roff.h',text,'texi2roff', 2278 ).
file('translate.c',text,'texi2roff', 12350 ).
file('COPYING',text,'texinfo-3.1', 17982 ).
file('ChangeLog',text,'texinfo-3.1', 44555 ).
file('INSTALL',text,'texinfo-3.1', 5458 ).
file('INTRODUCTION',text,'texinfo-3.1', 4493 ).
file('Makefile',text,'texinfo-3.1', 4963 ).
file('Makefile.in',text,'texinfo-3.1', 4681 ).
file('NEWS',text,'texinfo-3.1', 9630 ).
file('README',text,'texinfo-3.1', 6784 ).
file('config.status',exec,'texinfo-3.1', 2082 ).
file('configure',exec,'texinfo-3.1', 20121 ).
file('configure.in',text,'texinfo-3.1', 625 ).
file('emacs',dir,'texinfo-3.1', 512 ).
file('info',dir,'texinfo-3.1', 2048 ).
file('libtxi',dir,'texinfo-3.1', 512 ).
file('makeinfo',dir,'texinfo-3.1', 512 ).
file('texi.info',text,'texinfo-3.1', 8393 ).
file('texi.info-1',text,'texinfo-3.1', 50585 ).
file('texi.info-10',text,'texinfo-3.1', 43599 ).
file('texi.info-11',text,'texinfo-3.1', 24680 ).
file('texi.info-2',text,'texinfo-3.1', 50855 ).
file('texi.info-3',text,'texinfo-3.1', 49835 ).
file('texi.info-4',text,'texinfo-3.1', 49510 ).
file('texi.info-5',text,'texinfo-3.1', 50874 ).
file('texi.info-6',text,'texinfo-3.1', 50311 ).
file('texi.info-7',text,'texinfo-3.1', 50629 ).
file('texi.info-8',text,'texinfo-3.1', 42671 ).
file('texi.info-9',text,'texinfo-3.1', 47096 ).
file('texi.texi',text,'texinfo-3.1', 544952 ).
file('texinfo.tex',text,'texinfo-3.1', 131253 ).
file('util',dir,'texinfo-3.1', 512 ).
file('detexinfo.el',text,'texinfo-3.1/emacs', 9618 ).
file('info.el',text,'texinfo-3.1/emacs', 24088 ).
file('informat.el',text,'texinfo-3.1/emacs', 13609 ).
file('makeinfo.el',text,'texinfo-3.1/emacs', 11872 ).
file('texinfmt.el',text,'texinfo-3.1/emacs', 113666 ).
file('texinfo.el',text,'texinfo-3.1/emacs', 28194 ).
file('texnfo-tex.el',text,'texinfo-3.1/emacs', 13911 ).
file('texnfo-upd.el',text,'texinfo-3.1/emacs', 78422 ).
file('Makefile',text,'texinfo-3.1/info', 6394 ).
file('Makefile.in',text,'texinfo-3.1/info', 6104 ).
file('NEWS',text,'texinfo-3.1/info', 7114 ).
file('README',text,'texinfo-3.1/info', 2169 ).
file('dir.c',text,'texinfo-3.1/info', 6704 ).
file('display.c',text,'texinfo-3.1/info', 16682 ).
file('display.h',text,'texinfo-3.1/info', 2889 ).
file('doc.h',text,'texinfo-3.1/info', 1757 ).
file('dribble.c',text,'texinfo-3.1/info', 2128 ).
file('dribble.h',text,'texinfo-3.1/info', 1517 ).
file('echo_area.c',text,'texinfo-3.1/info', 39921 ).
file('echo_area.h',text,'texinfo-3.1/info', 2646 ).
file('filesys.c',text,'texinfo-3.1/info', 15507 ).
file('filesys.h',text,'texinfo-3.1/info', 3473 ).
file('footnotes.c',text,'texinfo-3.1/info', 7454 ).
file('footnotes.h',text,'texinfo-3.1/info', 1705 ).
file('gc.c',text,'texinfo-3.1/info', 3034 ).
file('gc.h',text,'texinfo-3.1/info', 1405 ).
file('general.h',text,'texinfo-3.1/info', 2707 ).
file('indices.c',text,'texinfo-3.1/info', 17660 ).
file('indices.h',text,'texinfo-3.1/info', 1449 ).
file('info-stnd.info',text,'texinfo-3.1/info', 53542 ).
file('info-stnd.texi',text,'texinfo-3.1/info', 49720 ).
file('info-utils.c',text,'texinfo-3.1/info', 16260 ).
file('info-utils.h',text,'texinfo-3.1/info', 5525 ).
file('info.1',text,'texinfo-3.1/info', 4650 ).
file('info.c',text,'texinfo-3.1/info', 15002 ).
file('info.h',text,'texinfo-3.1/info', 3807 ).
file('info.info',text,'texinfo-3.1/info', 31400 ).
file('info.texi',text,'texinfo-3.1/info', 35868 ).
file('infodoc.c',text,'texinfo-3.1/info', 17776 ).
file('infomap.c',text,'texinfo-3.1/info', 8554 ).
file('infomap.h',text,'texinfo-3.1/info', 2556 ).
file('m-x.c',text,'texinfo-3.1/info', 4830 ).
file('makedoc.c',text,'texinfo-3.1/info', 11769 ).
file('nodemenu.c',text,'texinfo-3.1/info', 8211 ).
file('nodes.c',text,'texinfo-3.1/info', 35154 ).
file('nodes.h',text,'texinfo-3.1/info', 7370 ).
file('search.c',text,'texinfo-3.1/info', 14501 ).
file('search.h',text,'texinfo-3.1/info', 2924 ).
file('session.c',text,'texinfo-3.1/info', 107405 ).
file('session.h',text,'texinfo-3.1/info', 5906 ).
file('signals.c',text,'texinfo-3.1/info', 4928 ).
file('signals.h',text,'texinfo-3.1/info', 2585 ).
file('termdep.h',text,'texinfo-3.1/info', 1757 ).
file('terminal.c',text,'texinfo-3.1/info', 18942 ).
file('terminal.h',text,'texinfo-3.1/info', 4769 ).
file('tilde.c',text,'texinfo-3.1/info', 9575 ).
file('tilde.h',text,'texinfo-3.1/info', 2345 ).
file('userdoc.texi',text,'texinfo-3.1/info', 45961 ).
file('variables.c',text,'texinfo-3.1/info', 7414 ).
file('variables.h',text,'texinfo-3.1/info', 2424 ).
file('window.c',text,'texinfo-3.1/info', 39821 ).
file('window.h',text,'texinfo-3.1/info', 9975 ).
file('xmalloc.c',text,'texinfo-3.1/info', 2194 ).
file('Makefile',text,'texinfo-3.1/libtxi', 2498 ).
file('Makefile.in',text,'texinfo-3.1/libtxi', 2210 ).
file('alloca.c',text,'texinfo-3.1/libtxi', 13198 ).
file('getopt.c',text,'texinfo-3.1/libtxi', 20876 ).
file('getopt.h',text,'texinfo-3.1/libtxi', 4412 ).
file('getopt1.c',text,'texinfo-3.1/libtxi', 4142 ).
file('Makefile',text,'texinfo-3.1/makeinfo', 2855 ).
file('Makefile.in',text,'texinfo-3.1/makeinfo', 2565 ).
file('makeinfo.c',text,'texinfo-3.1/makeinfo', 183587 ).
file('makeinfo.info',text,'texinfo-3.1/makeinfo', 8247 ).
file('makeinfo.texi',text,'texinfo-3.1/makeinfo', 9978 ).
file('Makefile',text,'texinfo-3.1/util', 2551 ).
file('Makefile.in',text,'texinfo-3.1/util', 2261 ).
file('deref.c',text,'texinfo-3.1/util', 4591 ).
file('fixfonts',exec,'texinfo-3.1/util', 2488 ).
file('mkinstalldirs',exec,'texinfo-3.1/util', 619 ).
file('tex3patch',exec,'texinfo-3.1/util', 1845 ).
file('texi2dvi',exec,'texinfo-3.1/util', 9344 ).
file('texindex.c',text,'texinfo-3.1/util', 39717 ).
file('COPYING',text,'time-1.4', 17982 ).
file('ChangeLog',text,'time-1.4', 4664 ).
file('INSTALL',text,'time-1.4', 5458 ).
file('Makefile',text,'time-1.4', 3452 ).
file('Makefile.in',text,'time-1.4', 3276 ).
file('NEWS',text,'time-1.4', 153 ).
file('README',text,'time-1.4', 338 ).
file('config.status',exec,'time-1.4', 1403 ).
file('configure',exec,'time-1.4', 12251 ).
file('configure.in',text,'time-1.4', 587 ).
file('error.c',text,'time-1.4', 2927 ).
file('getopt.c',text,'time-1.4', 19292 ).
file('getopt.h',text,'time-1.4', 4333 ).
file('getopt1.c',text,'time-1.4', 3427 ).
file('getpagesize.h',text,'time-1.4', 565 ).
file('rusage.h',text,'time-1.4', 349 ).
file('texinfo.tex',text,'time-1.4', 119529 ).
file('time',exec,'time-1.4', 49152 ).
file('time.c',text,'time-1.4', 19932 ).
file('time.info',text,'time-1.4', 10748 ).
file('time.texinfo',text,'time-1.4', 12600 ).
file('timeval.h',text,'time-1.4', 282 ).
file('wait3.c',text,'time-1.4', 1117 ).

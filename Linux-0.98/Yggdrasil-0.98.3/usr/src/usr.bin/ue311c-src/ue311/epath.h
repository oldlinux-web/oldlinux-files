/*	EPATH:	This file contains certain info needed to locate the
		MicroEMACS files on a system dependant basis.

									*/

/*	possible names and paths of help files under different OSs	*/

NOSHARE CONST char *pathname[] =

#if	AMIGA
{
	".emacsrc",
	"emacs.hlp",
	"",
	"c:",
	":t/",
	":s/"
};
#endif

#if	TOS
{
	"emacs.rc",
	"emacs.hlp",
	"\\",
	"\\bin\\",
	"\\util\\",
	""
};
#endif
 
#if	FINDER
{
	"emacs.rc",
	"emacs.hlp",
	"/bin",
	"/sys/public",
	""
};
#endif

#if	MSDOS
{
	"emacs.rc",
	"emacs.hlp",
	"\\sys\\public\\",
	"\\usr\\bin\\",
	"\\bin\\",
	"\\",
	""
};
#endif

#if OS2
{
        "emacs.rc",
        "emacs.hlp",
        "C:\\OS2\\SYSTEM\\",
        "C:\\OS2\\DLL\\",
        "C:\\OS2\\BIN\\",
        "C:\\OS2\\",
        "\\",
        ""
};
#endif

#if	V7 | BSD | USG | SMOS | HPUX | XENIX | SUN | AVIION
{
	".emacsrc",
	"emacs.hlp",
	"/usr/local/",
	"/usr/lib/",
	""
};
#endif

#if	VMS
{
	"emacs.rc",
	"emacs.hlp",
	"MICROEMACS$LIB:",
	"SYS$LOGIN:",
	"",
	"sys$sysdevice:[vmstools]"
};
#endif

#if WMCS
{
	"emacs.rc",
	"emacs.hlp",
	"",
	"sys$disk/syslib.users/"
};
#endif

#if AOSVS
/*
    NOTE: you must use the Unix style pathnames here!
*/
{
    "emacs.rc",
    "emacs.hlp",
    "",
    "/macros/",
    "/help/"
};
#endif

#if MPE
{
	"emacsrc",
	"emacshlp",
	".pub",
	".pub.sys",
	""
};
#endif /* MPE */

#define	NPNAMES	(sizeof(pathname)/sizeof(char *))

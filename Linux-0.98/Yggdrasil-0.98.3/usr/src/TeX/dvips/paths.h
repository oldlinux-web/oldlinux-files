/*
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files.  It is (C) Copyright 1987 by Tomas Rokicki.
 *   You may modify and use this program to your heart's content,
 *   so long as you send modifications to Tomas Rokicki.  It can
 *   be included in any distribution, commercial or otherwise, so
 *   long as the banner string defined in structures.h is not
 *   modified (except for the version number) and this banner is
 *   printed on program invocation, or can be printed on program
 *   invocation with the -? option.
 */
/*
 *   This is the paths file for dvisw.  (C) 1986 Radical Eye Software.
 *
 *   The following definition tells whether a character separates paths.
 *   Unix uses a colon.  DIRECTSPEC indicates whether a character does
 *   not require another slash, as in the case of a slash.
 */
#define BETWEENPATHS(c) (c==':')
#define DIRECTSPEC(c) (c=='/')
/*
 *   PKPATH is where to search for packed files, directories separated by
 *   commas.  A period means the current directory (don't prepend anything.)
 *   TFMPATH is the same for TFM files.
 */
/* #define PKPATH ".:/usr/local/lib/tex82/fonts:/ug/lib/tex82/fonts"
   #define TFMPATH ".:/usr/local/lib/tex82/fonts:/ug/lib/tex82/fonts" */
#define PKPATH ".:/usr/TeX/lib/tex/fonts"
#define TFMPATH ".:/usr/TeX/lib/tex/fonts"
/*
 *   OUTPATH is where to send the output.  If you want a bit file to be
 *   created by default, set this to "".
 */
#define OUTPATH ""
/*
 *   CONFIGPATH is where to search for the configuration file.
 */
/* #define CONFIGPATH ".:/usr/local/lib/ps:/usr/local/lib/tex82:/ug/lib/tex82" */
#define CONFIGPATH ".:/usr/TeX/lib/ps"
#define CONFIGFILE "config.ps"
/* #define HEADERPATH ".:/usr/local/lib/ps:/usr/local/lib/tex82:/ug/lib/tex82" */
#define HEADERPATH ".:/usr/TeX/lib/ps"
#define HEADERFILE "tex.pro"

#ifdef	ScriptPrinter
#define PSFONTHEADER "texps-scriptprinter.pro"
#endif

#if	!defined(ScriptPrinter)
#define PSFONTHEADER "texps.pro"
#endif

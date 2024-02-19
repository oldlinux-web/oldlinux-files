/* Header file for termcap library.  */

#ifdef __STDC__

extern int tgetent (char *buffer, char *termtype);

extern int tgetnum (char *name);
extern int tgetflag (char *name);
extern char *tgetstr (char *name, char **area);

extern char PC;
extern short ospeed;
extern int tputs (char *string, int nlines, int (*outfun) ());

extern char *tparam (char *ctlstring, char *buffer, int size, ...);

extern char *UP;
extern char *BC;

extern char *tgoto (char *cstring, int hpos, int vpos);

#else /* not ANSI C */

extern int tgetent ();

extern int tgetnum ();
extern int tgetflag ();
extern char *tgetstr ();

extern char PC;
extern short ospeed;

extern int tputs ();

extern char *tparam ();

extern char *UP;
extern char *BC;

extern char *tgoto ();

#endif /* not ANSI C */

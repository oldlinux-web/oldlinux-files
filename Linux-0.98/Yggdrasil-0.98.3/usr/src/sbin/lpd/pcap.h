#ifndef _TERMCAP_H
#define _TERMCAP_H

extern int pgetent(char *bp, char *name);
extern int pgetflag(char *id);
extern int pgetnum(char *id);
extern char *pgetstr(char *id, char **area);

#endif /* _TERMCAP_H */

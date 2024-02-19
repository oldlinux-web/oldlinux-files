/*
	getttyent, getttynam, setttyent, endttyent for linux
	by qpliu@phoenix.princeton.edu 1992
	$Id$
	simple test appears to be duplicate the sun library version
 */
/*
	my computer is a puny 2meg 386sx, and can't run gcc.
	so I made cc1 on my account on a sun 4/490 (soon to be
	4/690MP, which is good because the load avg is usually 10-20),
	and compile to .s on the sun.  however, it dies with
	SIGIOT when trying to optimize some stuff, thus this is 
	a bit more contorted to make it compile
*/
#define GCC_O_BUSTED

#include <stdio.h>
#include <string.h>
#include <ttyent.h>

#define	ENTRY_SIZE	120	/* what's the standard way to do this? */

#define isspace(c) (c==' '||c=='\t'||c=='\n')

static struct ttyent *entry = (struct ttyent *)NULL;
static FILE *tabfile = (FILE *)NULL;
static char *tabbuf = (char *)NULL;	/* buffer to read line of ttytab */

static void __read_ttyent (void);
static void __free_bufs (void);

static void read_flags (char *, struct ttyent *);

struct ttyent *
getttyent ()
{
	if (!tabfile)
		if (!(tabfile = fopen (_PATH_TTYTAB, "r"))) {
			__free_bufs ();
			return (struct ttyent *)NULL;
		}

	if (!entry)
		if (!(entry = (struct ttyent *)malloc(sizeof(struct ttyent)))) {
			fclose (tabfile);
			tabfile = (FILE *)NULL;
			return (struct ttyent *)NULL;
		}

	if (!tabbuf)
		if (!(tabbuf = (char *)malloc (ENTRY_SIZE))) {
			fclose (tabfile);
			tabfile = (FILE *)NULL;
			__free_bufs ();
			return (struct ttyent *)NULL;
		}

	__read_ttyent ();

	return entry;
}

int
setttyent ()
{
	if (tabfile)
		rewind (tabfile);
}

int
endttyent ()
{
	if (tabfile) {
		fclose (tabfile);
		tabfile = (FILE *)NULL;
	}

	__free_bufs ();
}

static void
__free_bufs ()
{
	if (tabbuf) {
		free (tabbuf);
		tabbuf = (char *)NULL;
	}

	if (entry) {
		free (entry);
		entry = (struct ttyent *)NULL;
	}
}

static void
__read_ttyent ()
{
	char *c, dump[ENTRY_SIZE];
	/* one label for gotos, used when discarding bad lines */
restart:
	*tabbuf = '#';
	c = tabbuf;
	while (*c == '#') {	/* skip comments */
		if (!fgets (tabbuf, ENTRY_SIZE, tabfile)) {	/* EOF */
			__free_bufs ();
			return;
		}
		c = tabbuf;
		while (isspace (*c) && *c) ++c;
		if (!*c) goto restart;
	}

	/* discard the end of lines longer than ENTRY_SIZE */
	if (tabbuf [strlen (tabbuf) - 1] != '\n')
		do {
			if (!fgets (dump, ENTRY_SIZE, tabfile)) break;
		} while (dump [strlen (dump) - 1] != '\n');

	entry->ty_name = c;	/* get name */
	while (*c && !isspace (*c)) ++c;
	if (!*c) goto restart;
	*(c++) = '\0';
	while (*c && isspace (*c)) ++c;
	if (!*c) goto restart;
	if (*c == '"') {	/* getty is quoted */
		entry->ty_getty = ++c;
		while (*c && *c != '"') ++c;
		if (!*c) goto restart;
		*(c++) = '\0';
	} else {	/* getty in not quoted */
		entry->ty_getty = c;
		while (*c && !isspace (*c)) ++c;
		if (!*c) goto restart;
		*(c++) = '\0';
	}
	while (*c && isspace (*c)) ++c;
	if (!*c) goto restart;
	entry->ty_type = c;	/* term type */
	while (*c && !isspace (*c)) ++c;
	if (*c) *(c++) = '\0';

	/* defaults for optional parts */
	entry->ty_status = 0;
	entry->ty_window = (char *)NULL;
	entry->ty_comment = (char *)NULL;

	/* check for remaining flags, don't truly enforce format */
	read_flags (c, entry);
}

void
read_flags (char *c, struct ttyent *entry)
{
	int ts_off = strlen (_TTYS_OFF),
		ts_on = strlen (_TTYS_ON),
		ts_local = strlen (_TTYS_LOCAL),
		ts_secure = strlen (_TTYS_SECURE),
		ts_window = strlen (_TTYS_WINDOW);

	while (*c) {
		if (!strncmp (c, _TTYS_OFF, ts_off)) {
			entry->ty_status &= ~TTY_ON;
			c += ts_off;
		} else if (!strncmp (c, _TTYS_ON, ts_on)) {
			entry->ty_status |= TTY_ON;
			c += ts_on;
		} else if (!strncmp (c, _TTYS_LOCAL, ts_local)) {
			entry->ty_status |= TTY_LOCAL;
			c += ts_local;
		} else if (!strncmp (c, _TTYS_SECURE, ts_secure)) {
			entry->ty_status |= TTY_SECURE;
			c += ts_secure;
		} else if (!strncmp (c, _TTYS_WINDOW, ts_window)) {
			c += ts_window;
			/* window="pathname of window thing" */
			if (*c) break;
			if (*++c) break;	/* skip the = */
			if (*++c) break;	/* skip the quote */
			entry->ty_window = c;
			while (*c && *c != '"') ++c;
			if (*c) *(c++) = '\0';
		} else if (*c == '#') {
			/* keep the trailing newline? */
			++c;
			while (isspace (*c)) ++c;
			entry->ty_comment = c;
			break;
		}
		while (isspace (*c)) ++c;
	}
}

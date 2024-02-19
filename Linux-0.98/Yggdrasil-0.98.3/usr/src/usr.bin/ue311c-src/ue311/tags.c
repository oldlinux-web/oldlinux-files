/*
 * The routines in this file provides support for  vi-like tagging
 * of defined identifiers.  We presuppose the "tags" file in the
 * current directory (constructed by 'ctags' or 'etags'), with each
 * entry on the following format:
 *
 *	 identifier<tab>file<tab>vi-search-pattern
 *
 * Code will be generated if both  MAGIC and CTAGS  is wanted.
 *							880826mhs
 * 890622mhs
 * Changed code so as to take advantage of the new FAST search routine
 * i.e. we do not use the MAGIC search.
 * Also implemented indexing of the "tags" file to reduce the time used
 * when tagging is performed more than once.  Moreover, we now support
 * tagging of file(s) other than those in the current directory.  We
 * automatically locates the correct "tags" and records its vital info
 * (only the first time it's referenced!!).
 * 890627mhs
 * Added possibility to be prompted for word to tag.  You will be prompted
 * if you execute the tag-word function whenever dot is not within a word.
 */

#include <stdio.h>
#include "estruct.h"

#if CTAGS
#include "eproto.h"
#include "edef.h"
#include "elang.h"

static char SVER[] = "@(#) %M% %I% %H%";

#define min(x, y)	((x) <= (y) ? (x) : (y))

#define INDEX(c)	(islower(c) ? c-'a'+27 : \
			 (isletter(c) ? c-'A'+1: ((c == '_') ? 0 : -1)))
#define NINDEXES	26+26+1
#define TAGWIDTH	30

typedef struct	TAG {
	struct	TAG *t_tagp;		/* Link to the next 'tags' file	*/
	char	t_path[NFILEN];		/* Path of 'tags' file		*/
	FILE	*t_fp;			/* File pointer to 'tags' file	*/
	char	t_wd[TAGWIDTH+1];	/* Word last tagged (this file)	*/
	char	t_fname[NFILEN];	/* Holds name of file from where*/
					/* we tagged.			*/
	int	t_indexed;		/* Flag:  1=file is indexed	*/
	long	t_dotos[NINDEXES];	/* Offsets of first chars (used	*/
					/* for speed-up purposes only).	*/
}	TAG;

static TAG *theadp = NULL;		/* Pointer to the head of the	*/
					/* 'tags'-chain.		*/
static TAG *curtp  = NULL;		/* Currently in-use 'tags'.	*/


/*
 * Look-up a 'tags' file in the directory given by
 * 'path'.  If such a file exists and we are allowed
 * to read it, we'll read it and construct an index
 * of file positions of the first occurence of each
 * starting character ('_', 'a'-'z', 'A'-'Z').  We
 * return with TRUE only if we are succesfull.
 */

newtags(path)
char path[NFILEN];
{
	register TAG	*tnewp;
	register int	i = NINDEXES;

	if ((tnewp = (TAG *)malloc(sizeof(TAG))) == NULL) {
		mlwrite("[OUT OF MEMORY]");
		return(FALSE);
	}
	strcpy(tnewp->t_path, path);
	strcat(path, "tags");
	if ((tnewp->t_fp = fopen(path, "r")) == NULL) {
		free((char *)tnewp);
		return(FALSE);
	}

	tnewp->t_tagp  = theadp;
	curtp = theadp = tnewp;
	strcpy(tnewp->t_fname, curbp->b_fname);
	strcpy(tnewp->t_wd, "");

	/* Initialize index...	*/
	tnewp->t_indexed = FALSE;
	while (i > 0)
		tnewp->t_dotos[--i] = -1L;

	return(TRUE);
}


/*
 * Look-up 'tags' file; first in our list and if it isn't there
 * try it the hard way.  If we find the file we return TRUE.
 */

lookup()
{
	TAG		*tmp = curtp;	/* Remember current 'tags'	*/
	char		cpath[NFILEN];	/* Path of current file		*/
	register char	*cp;		/* Auxiliary pointer		*/
	register int	nope = TRUE;	/* True if 'tags' is unknown	*/

	cp = curbp->b_fname + strlen(curbp->b_fname) - 1;
#if	MSDOS
	while (cp >= curbp->b_fname  &&  *cp != DIRSEPCHAR  &&  *cp != ':')
#else
	while (cp >= curbp->b_fname  &&  *cp != DIRSEPCHAR)
#endif
		cp--;

	memset(cpath, '\0', NFILEN);
	if (cp >= curbp->b_fname)
		strncpy(cpath, curbp->b_fname, (int)(cp-curbp->b_fname));
	else
		strcpy(cpath, ".");
	/* Append a DIRSEPCHAR character to the path...	*/
	if (strlen(cpath) < NFILEN - 1)
		cpath[strlen(cpath)] = DIRSEPCHAR;

	while (curtp != NULL  &&  (nope = strcmp(curtp->t_path, cpath)) != 0)
		curtp = curtp->t_tagp;

	if (nope == 0)			/* We already knew the tags path*/
		return(TRUE);

	/* We'll have to look it up...	*/
	if (newtags(cpath))
		return(TRUE);
	else
		curtp = tmp;
	return(FALSE);
}


/*
 * Create an index of offsets into the 'tags' file at
 * curtp->t_path.  We use the first character of the
 * tagged words as our index index.
 */

fix_index()
{
	register int	i  = -1;
	register long	lastpos = 0L;
	char		line[NLINE];

	if (curtp->t_indexed == TRUE)
		return;

	mlwrite("[Indexing 'tags' file, please wait...]");

	while (fgets(line, NLINE, curtp->t_fp) != NULL) {
		if (i != INDEX(line[0])  &&  (i=INDEX(line[0])) != -1)
			curtp->t_dotos[i] = lastpos;
		lastpos = (long)ftell(curtp->t_fp);
	}
	curtp->t_indexed = TRUE;
}


/*
 * Put the rest of the characters of the current word at '.' in
 * str (but maximum lmax characters).  '.' is preserved.
 */
restword(str, lmax)
char *str;
int  lmax;
{
	register int i;
	register int go_on  = TRUE;
	register LINE *dotp = curwp->w_dotp;	/* Preserve '.' info	*/
	register int   doto = curwp->w_doto;	/* Preserve '.' info	*/

	for (i=0 ; go_on  &&  i < lmax-1  &&  inword() ; i++) {
		str[i] = lgetc(curwp->w_dotp, curwp->w_doto);
		go_on  = forwchar(FALSE, 1);
	}

	str[i] = 0;			/* Terminate word		*/
	curwp->w_dotp = dotp;		/* Restore '.' 			*/
	curwp->w_doto = doto;

	return(TRUE);
}


/*
 * Move '.' backwards until start of current word.
 * NOTE, we rely on inword(), which normally don't
 * consider '_' part of a word.  You might want to
 * change inword() in order to obtain satisfactory
 * results from this code (I did).
 */

backupword(f, n)

int f, n;

{
	while (inword())
		if (backchar(FALSE, 1) == FALSE)
			break;
	if (!inword())		/* Adjust for not beginning of file	*/
		forwchar(FALSE, 1);

	return(TRUE);
}


/*
 * Alter the vi-search-pattern in pattern inorder to use it
 * as a search pattern for uEMACS' search routines.
 * The vi-pattern contains \-escaped characters...we have
 * to get rid of the \'es.
 * Moreover, as we want to make use of the new FAST search
 * routine, we have to remove the pattern anchoring (^ and $)
 * and search direction characters (? or /)
 */

alterpattern(pattern)
register char pattern[];
{
	register int	i   = 0;		/* EMACS pattern index	*/
	register int	j   = 1;		/* VI pattern -skip /or?*/
	int		len = strlen(pattern)-1;/* pattern length - 1	*/
						/* i.e. drop '/' or '?'	*/

	if (pattern[len-1] == '$')  len--;

	while (++j < len)
		if (pattern[j] != '\\')
			pattern[i++] = pattern[j];
		else if (pattern[j+1] == '\\')
			pattern[i++] = pattern[++j];

	pattern[min(i, NPAT/2)] = '\0';	/* Terminate pattern string	*/
	return(TRUE);
}


/*
 * Some locally shared variables
 */

static int thisfile  = FALSE;		/* TRUE if curtp->t_fname equals*/
					/* curbp->fname when tagging	*/
static int tagvalid  = FALSE;		/* TRUE if last tag was a succes*/


/*
 * Tag current word if '.' is within a word, otherwise tag next word.
 * '.' is preserved, and return information (= current filename) is saved.
 */
extern int PASCAL NEAR tagword(f, n)

int f, n;

{
	LINE	*pretagdotp = curwp->w_dotp;	/* Preserve '.' info	*/
	int	pretagdoto  = curwp->w_doto;
	int	i;

	if (restflag == TRUE)	/* Don't allow when in restricted mode	*/
		return(resterr());

	if (lookup() == FALSE) { /* Is 'tags' avaliable for this file?	*/
		mlwrite("[Sorry, can't find any 'tags']");
		return(FALSE);
	}

	/* Get word to tag	*/
	if (inword()) {
		backupword(FALSE, 1);
		restword(curtp->t_wd, TAGWIDTH);	/* Grab word to tag	*/
	}
	else if (mlreply("Word to tag: ", curtp->t_wd, TAGWIDTH) != TRUE)
		return(FALSE);

	fix_index();

	curwp->w_dotp = pretagdotp;	/* Restore '.'	*/
	curwp->w_doto = pretagdoto;

	/* Ok, set file offset according to  curtp->t_wd (if any)	*/
	if ((i=INDEX(*curtp->t_wd)) == -1  ||  curtp->t_dotos[i] == -1L) {
		mlwrite("[No tag entry for '%s' found]", curtp->t_wd);
		return(FALSE);
	}
	fseek(curtp->t_fp, curtp->t_dotos[i], 0);

	strcpy(curtp->t_fname, curbp->b_fname); /* Save name of current file */
	return(tagvalid = tagger("[No tag entry for '%s' found]", FALSE));
}


/*
 * Does the actual work.  Presumes that  tagw  containes the correct
 * word to tag.  Note that we do not rewind the file pointer as to
 * allow you to do a re-tag.
 * If the tagged word is defined in the current file we mark '.', 
 * goes to line 1 of file and searches for the pattern.  We do this
 * so as to prevent loosing the return information.
 */

tagger(errmsg, retag)
char *errmsg;
int  retag;
{
	char	tagf[NFILEN];			/* File of tagged word	*/
	char	pretagpat[NPAT];		/* Search pattern prior	*/
						/* to our tagging.	*/
	char	line[NLINE];			/* Auxilliary string	*/
	int	ok = 1;				/* Tag search flag	*/
	int	result   = FALSE;		/* Default return value */
	int	oldbmode;			/* For preserving bmode	*/
	int	taglen   = strlen(curtp->t_wd);
	int	file_ok;			/* TRUE if file found	*/

	/* Tell user what we are doing		*/
	mlwrite("[Tagging '%s'...]", curtp->t_wd);

	/* Search for  curtp->t_wd  in the 'tags' file	*/
	while (ok > 0  &&  fgets(line, NLINE, curtp->t_fp) != NULL) {
		if ((ok = strncmp(curtp->t_wd, line, taglen)) < 0)
			break;
		else if (ok == 0  &&  line[taglen] != '\t')
			ok = -1;
	}

	if (ok < 0) {				/* We couldn't find it..*/
		mlwrite(errmsg, curtp->t_wd);
		return(FALSE);
	}

	strcpy(pretagpat, pat);		/* Preserve old search pattern	*/

	/* Scan line for file and pattern	*/
	sscanf(line, "%s %s %[^\n]", curtp->t_wd, tagf, pat);
	/* Alter pattern... we cannot use vi's	*/
	alterpattern(pat);

	/* Add path to tagf if necessary...	*/
	add_path(tagf);
	/* Should we search the current file?	*/
	thisfile = strcmp(tagf, curbp->b_fname) == 0;
	file_ok  = thisfile ? TRUE : getfile(tagf, TRUE);
	oldbmode = curbp->b_mode;	/* Preserve buffer mode		*/
	if (file_ok) {			/* Ok, we got the file. Search!	*/
		if (thisfile  &&  retag == FALSE)
			/* It's the same file so just set mark	*/
			setmark(FALSE, FALSE);
		gotoline(TRUE, 1);

		/* Set-up for searching... use exact mode, not magic	*/
		curbp->b_mode |= MDEXACT;
		curbp->b_mode &= ~MDMAGIC;
		setjtable();
		mcclear();
		rmcclear();

		if (scanner(FORWARD, PTBEG, 1) == FALSE) {
			/* Sorry, we couldn't find pattern so return...	*/
			if (thisfile  &&  retag == FALSE)
				/* It's the same file so simply swapmark*/
				swapmark(FALSE, FALSE);
			else	/* Get old file	*/
				getfile(curtp->t_fname, TRUE);
			/* Tell user about our misfortune	*/
			mlwrite("[Failed to tag '%s']", curtp->t_wd);
		}
		else {	/* We found the pattern.  Now point at word!	*/
			strcpy(pat, curtp->t_wd);
			setjtable();
			result = scanner(FORWARD, PTBEG, 1);
		}
	}

	curbp->b_mode = oldbmode;	/* Restore buffer mode		*/

	strcpy(pat, pretagpat);		/* Restore search pattern	*/
	setjtable();
	if (curbp->b_mode & MDMAGIC)
		mcstr();
	else {
		mcclear();
		rmcclear();
	}

	return(result);
}


/*
 * Prefix filename with path in curtp->t_path (if any)
 * if filename doesn't include a full path.  This routine
 * allways succeeds.
 */

add_path(filename)
char *filename;
{
	char temp[NFILEN];
	char *tp;

	if (curtp->t_path[0] == '.'  &&  curtp->t_path[1] == DIRSEPCHAR  &&
						    curtp->t_path[2]  == '\0')
		return;
		
	for (tp=filename ; *tp  &&  *tp != DIRSEPCHAR ; tp++);

	if (*tp == DIRSEPCHAR)
		return;

	strcpy(temp, curtp->t_path);
	strcat(temp, filename);
	strcpy(filename, temp);
}



/*
 * Sometimes the 'tags' file will contain multiple entries for the
 * same identifier.  This occures whenever an identifier is multiple
 * defined.  retagword  asks  tagger  to tag for the same  tagw  again
 * but after the position where the last tag entry for tagw was found.
 * Note, retagword  do not mess up the return information (tagf).
 */
 
extern int PASCAL NEAR retagword(f, n)

int f, n;

{
	if (restflag == TRUE)	/* Don't allow when in restricted mode	*/
		return(resterr());

	if (*curtp->t_fname == '\0'  ||  !tagvalid)
		return(FALSE);

	return(tagger("[No additional tag entry for '%s' found]", TRUE));
}


/*
 * Return to the file from where we tagged the  tagw  identifier.
 * You can only return once for each tag.  If it's the same file
 * we just swap mark with '.' .
 */

extern int PASCAL NEAR backtagword(f, n)

int f, n;

{
	if (restflag == TRUE)	/* Don't allow when in restricted mode	*/
		return(resterr());

	if (*curtp->t_fname != '\0') {
		if (thisfile)
			swapmark(FALSE, FALSE);
		else
			getfile(curtp->t_fname, TRUE);
		*curtp->t_fname = '\0';
	}

	return(TRUE);
}
#else
tagshello()
{
}	
#endif

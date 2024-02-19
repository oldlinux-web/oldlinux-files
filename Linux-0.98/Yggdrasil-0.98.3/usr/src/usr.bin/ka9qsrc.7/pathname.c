#include "global.h"

/* Given a working directory and an arbitrary pathname, resolve them into
 * an absolute pathname. Memory is allocated for the result, which
 * the caller must free
 */
char *
pathname(cd,path)
char *cd;	/* Current working directory */
char *path;	/* Pathname argument */
{
	register char *buf;
#if	(defined(MSDOS) || defined(ATARI_ST))
	register char *cp;
	char *cdtmp,*pathtmp;
#endif

	if(cd == NULLCHAR || path == NULLCHAR)
		return NULLCHAR;
#if	(defined(MSDOS) || defined(ATARI_ST))
	/* Make temporary copies of cd and path
	 * with all \'s translated to /'s
	*/
	pathtmp = malloc((unsigned)strlen(path)+1);
	strcpy(pathtmp,path);
	path = pathtmp;
	if((cp = path) != NULLCHAR){
		while((cp = index(cp,'\\')) != NULLCHAR)
			*cp = '/';
	}
	cdtmp = malloc((unsigned)strlen(cd)+1);
	strcpy(cdtmp,cd);
	cd = cdtmp;
	if((cp = cd) != NULLCHAR){
		while((cp = index(cp,'\\')) != NULLCHAR)
			*cp = '/';
	}
#endif
	/* Strip any leading white space on args */
	while(*cd == ' ' || *cd == '\t')
		cd++;
	while(*path == ' ' || *path == '\t')
		path++;

	/* Allocate and initialize output buffer; user must free */
	buf = malloc((unsigned)strlen(cd) + strlen(path) + 10);	/* fudge factor */
	buf[0] = '\0';

	/* Interpret path relative to cd only if it doesn't begin with "/" */
	if(path[0] != '/')
		crunch(buf,cd);

	crunch(buf,path);

	/* Special case: null final path means the root directory */
	if(buf[0] == '\0'){
		buf[0] = '/';
		buf[1] = '\0';
	}

#if	(defined(MSDOS) || defined(ATARI_ST))
	/* Translate all /'s back to \'s and free temp copies of args */
	if((cp = buf) != NULLCHAR){
		while((cp = index(cp,'/')) != NULLCHAR)
			*cp = '\\';
	}
	free(cdtmp);
	free(pathtmp);
#endif
	return buf;
}

/* Process a path name string, starting with and adding to
 * the existing buffer
 */
static
crunch(buf,path)
char *buf;
register char *path;
{
	register char *cp;
	

	cp = buf + strlen(buf);	/* Start write at end of current buffer */
	
	/* Now start crunching the pathname argument */
	for(;;){
		/* Strip leading /'s; one will be written later */
		while(*path == '/')
			path++;
		if(*path == '\0')
			break;		/* no more, all done */
		/* Look for parent directory references, either at the end
		 * of the path or imbedded in it
		 */
		if(strcmp(path,"..") == 0 || strncmp(path,"../",3) == 0){
			/* Hop up a level */
			if((cp = rindex(buf,'/')) == NULLCHAR)
				cp = buf;	/* Don't back up beyond root */
			*cp = '\0';		/* In case there's another .. */
			path += 2;		/* Skip ".." */
			while(*path == '/')	/* Skip one or more slashes */
				path++;
		/* Look for current directory references, either at the end
		 * of the path or imbedded in it
		 */
		} else if(strcmp(path,".") == 0 || strncmp(path,"./",2) == 0){
			/* "no op" */
			path++;			/* Skip "." */
			while(*path == '/')	/* Skip one or more slashes */
				path++;
		} else {
			/* Ordinary name, copy up to next '/' or end of path */
			*cp++ = '/';
			while(*path != '/' && *path != '\0')
				*cp++ = *path++;
		}
	}
	*cp++ = '\0';
}

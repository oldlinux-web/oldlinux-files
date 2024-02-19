/* Given a filename, dld_find_executable searches the directories listed in the
   environment variable PATH for a file with that filename.
   A new copy of the complete path name of that file is returned.  This new
   string may be disposed by free() later on.
*/

#include <sys/file.h>
#include <sys/param.h>
#include <strings.h>

#define DEFAULT_PATH ".:~/bin::/usr/local/bin:/usr/new:/usr/ucb:/usr/bin:/bin:/usr/hosts"

static char *
copy_of (s)
register char *s;
{
    register char *p = (char *) malloc (strlen(s)+1);

    if (!p) return 0;

    *p = 0;
    strcpy (p, s);
    return p;
}


char *
dld_find_executable (file)
char *file;
{
    char *search;
    register char *p;
    
    if (*file == '/')
	return copy_of (file);
    
    if ((search = (char *) getenv("PATH")) == 0)
	search = DEFAULT_PATH;
	
    p = search;
    
    while (*p) {
	char  name[MAXPATHLEN];
	register char *next;

	next = name;
	
	/* copy directory name into [name] */
	while (*p && *p != ':') *next++ = *p++;
	*next = 0;
	if (*p) p++;

	if (name[0] == '.' && name[1] == 0)
	    getwd (name);
	
	strcat (name, "/");
	strcat (name, file);
	      
	if (access (name, X_OK) == 0)
	    return copy_of (name);
    }

    return 0;
}

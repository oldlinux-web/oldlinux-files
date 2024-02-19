/* $Id: popen.c 3.2 92/03/14 11:58:07 cthuang Exp $
 *
 * Imitate a UNIX pipe in MS-DOS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>
#include "cproto.h"

static char pipe_name[FILENAME_MAX];	/* name of the temporary file */

/* Open a pipe for reading.
 */
FILE *
popen (cmd, type)
char *cmd, *type;
{
    char *tmpdir, *argv[30], **arg, *cmdline, *s, opt[FILENAME_MAX];
    int ostdout, status;

    /* Set temporary file name. */
    if ((tmpdir = getenv("TMP")) == NULL) {
	pipe_name[0] = '\0';
    } else {
	strcpy(pipe_name, tmpdir);
	trim_path_sep(pipe_name);
	strcat(pipe_name, "/");
    }
    strcat(pipe_name, tmpnam(NULL));

    /* Split the command into an argument array. */
    cmdline = xstrdup(cmd);
    arg = argv;
    s = strtok(cmdline, " ");
    *arg++ = s;
#ifdef M_I86
    sprintf(opt, "-o%s.", pipe_name);
#else
    sprintf(opt, "-o%s", pipe_name);
#endif
    *arg++ = opt;
    while ((s = strtok(NULL, " ")) != NULL) {
	*arg++ = s;
    }
    *arg = NULL;
 
    /* Redirect the program's stdout to /dev/null. */
    ostdout = dup(fileno(stdout));
    freopen("nul", "w", stdout);
 
    /* Run the program. */
    status = spawnvp(P_WAIT, argv[0], argv);
 
    /* Restore stdout. */
    dup2(ostdout, fileno(stdout));
    free(cmdline);

    if (status != 0)
	return NULL;
 
    /* Open the intermediate file and return the stream. */
    return fopen(pipe_name, type) ;
}
 
/* Close the pipe.
 */
int
pclose (f)
FILE *f;
{
    int status;
 
    status = fclose(f);
    unlink(pipe_name);
    return status;
}


#include <stdio.h>
#include <sys/file.h>
#include <errno.h>

extern char *sys_errlist[];

main(int argc, char *argv[])
{

    if (argc != 3) {
	fprintf(stderr, "usage: doshell <ttyname> <shellname> &\n");
	exit(1);
    }

    /* close down fd's */
    close(0);
    close(1);
    close(2);

    /* detach from parent process's group */
    setsid();

    /* open new tty */
    if (open(argv[1], O_RDWR, 0) == -1)
	exit(2);
    dup(0);
    dup(0);
    execlp(argv[2], "-", 0);
    /* should appear on new tty...: */
    fprintf(stderr, "can't exec shell: %s\n", sys_errlist[errno]);
    exit(3);
}


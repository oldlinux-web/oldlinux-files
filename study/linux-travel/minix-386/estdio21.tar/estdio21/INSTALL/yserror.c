#include <errno.h>

extern char *sys_errlist[];
extern int sys_nerr;
extern int __errlist;

int perror() {}

int main() { char *s; s = sys_errlist[0]; __errlist = sys_nerr; }

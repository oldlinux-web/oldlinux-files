/*
    tty.c
    By John Harvey AKA johnbob AKA qk
    copy this all you want
*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

void syntax(int rv)
{
    fprintf(stderr,"syntax: tty [-s]\n");
    exit(rv);
}

main(int argc, char **argv)
{
    int silent = 0;
    char *n;

    if( argc == 2 )
    {
	if( strcmp(argv[1],"-s") )
	    syntax(1);
	silent = 1;
    }
    else if ( argc != 1 )
	syntax(1);
    if( n=ttyname(0) )
    {
	if ( ! silent )
	    printf("%s\n", n);
	exit(0);
    }
    else
    {
	if ( ! silent )
	    printf("%s\n", "Not a tty");
	exit(1);
    }
}

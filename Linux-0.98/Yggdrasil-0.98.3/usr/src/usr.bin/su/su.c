/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
$Header: /usr/src/su/su.c,v 1.1 1992/01/08 21:45:23 james_r_wiegand Exp james_r_wiegand $
/usr/src/su.c
hacked from a Berkeley su by james wiegand
*/

#include <limits.h>
#include <getopt.h>
#include <sys/time.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <unistd.h>

#define	ARGSTR	"l"
#define _PATH_BSHELL "/bin/sh" 
#define rindex strchr
#define _PATH_SEARCHPATH "PATH=:/usr/gnu/bin:/usr/local/bin:/bin:/usr/bin:/etc"
#define _MINIX_TERM "TERM=console"

char *getpass( char * );
int putenv( char * );
#define FAILURE -1

int main( int argc, char **argv )
{
extern char **environ;
extern int errno, optind;
register struct passwd *pwd;
register char *p, **g;
struct group *gr;
uid_t ruid;
int  ch, login = 0;
char *user, *shell, username[ 512 ], *cleanenv[4], *nargv[4], **np;

	np = &nargv[3];
	*np-- = NULL;

	while (( ch = getopt( argc, argv, ARGSTR )) != EOF )
		switch((char)ch)
			{
			case 'l':
					login = 1;
					break;
			case '?':
			default:
				(void)fprintf(stderr, "usage: su [%s] [login]\n",
			                ARGSTR);
				exit(1);
			}

	argv += optind;

	errno = 0;

	/* get current login name and shell */
	ruid = getuid(); 
	pwd = getpwuid( ruid );
	if( pwd == NULL )
		{
		fprintf(stderr, "su: invalid login\n");
		exit(1);
		}

	strcpy( username, pwd->pw_name ); 
	if( strcmp( pwd->pw_shell, _PATH_BSHELL ))
		printf( "su warning: using sh as shell, not %s\n\r", pwd->pw_shell );
		
	shell = _PATH_BSHELL;

	/* get target login information, default to root */
	user = *argv ? *argv : "root";
	
	pwd = getpwnam( user );
	if( pwd == NULL )
		{
		fprintf(stderr, "su: unknown login %s\n", user);
		exit(1);
		}
  
	/* only allow those in the same group to su. */
	/* this is required by setgid */
	gr = getgrgid( (gid_t) 0 );
	if( gr )
		{
		g = gr->gr_mem;
		while( *g )
			{
			if( !strcmp( username, *g ))
				break;
			g++;
			}
/*
		if( !*g )
			{
			fprintf(stderr, "su: you are not in the same group as %s.\n", user);
			exit(1);
			} */
		}
		
	if( ruid )
		{
		/* if target requires a password, verify it */
		if( *pwd->pw_passwd )
			{
			p = getpass( "Password:" );
			if( strcmp( pwd->pw_passwd, crypt( p, pwd->pw_passwd )))
				{
				fprintf(stderr, "Sorry\n");
				exit(1);
				}
			}
		}

	/* set permissions */
	if( setgid( pwd->pw_gid ) == FAILURE )
		{
		perror("su: setgid");
		exit(1);
		}

	if( setuid( pwd->pw_uid ) == FAILURE )
		{
		perror("su: setuid");
		exit(1);
		}

	if( login )
		{
		cleanenv[0] = _PATH_SEARCHPATH;
		cleanenv[1] = _MINIX_TERM;
		cleanenv[2] = (char *) NULL;
		/* get a malloc'd environment to pass on */
		environ = cleanenv;
		putenv( p );

		if( chdir( pwd->pw_dir ) == FAILURE )
			{
			fprintf(stderr, "su: no login directory\n");
			exit(1);
			}
		}
	

	*np = login ? "-su" : "su";
		
 	execv(shell, np );
	(void)fprintf(stderr, "su: %s not found.\n", shell);
	exit(1);
}
/*
char *getpass(char *foo)
{
char buf[ 512 ];

	printf("\n\rpassword: ");
	gets( buf );

return buf;
}	
*/


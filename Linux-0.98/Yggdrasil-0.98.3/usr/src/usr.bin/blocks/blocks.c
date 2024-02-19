/* Copyright 1992 Yggdrasil Computing, Incorporated 
   This file may be copied under the terms of the
   GNU General Public License, version 2, which should
   be in the file COPYING in the same directory as this file.
   Author: Adam J. Richter (adam@yygdrasil.com)
 */


#include <sys/types.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include <stdio.h>

int
valid_offset( int fd, int offset )
{
    char ch;

    if( lseek( fd, offset, 0 ) < 0 ) return 0;
    if( read( fd, &ch, 1 ) < 1 ) return 0;
    return 1;
}
main( int argc, char **argv )
{
    unsigned long high, low;
    int fd;

    if( argc != 2 ) {
	fprintf (stderr, "Usage: %s file\n", argv[0] );
	exit ( -1 );
    }
    if( (fd = open( argv[1], O_RDONLY )) < 0 ) {
	perror (argv[1]);
	exit (1);
    }
    low = 0;
    for( high = 1; valid_offset( fd, high ); high *= 2 ) {
	low = high;
    }
    while( low < high - 1 ) {
	const int mid = (low + high) / 2;
	if ( valid_offset( fd, mid)) {
	    low = mid;
	}
	else {
	    high = mid;
	}
    }
    printf ("%d\n", (low+1) / 1024 );
}

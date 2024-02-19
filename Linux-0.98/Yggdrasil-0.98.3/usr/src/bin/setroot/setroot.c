#include <sys/types.h>
#include <linux/fcntl.h>
#include <sys/file.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAJOR 509
#define MINOR 508

main ( int argc, char **argv ) {
   int fd;
   char buf[512];
   unsigned int major, minor;
   char *target;

   switch(argc ) {
   case 3:
       {
	   struct stat statbuf;
	   if( stat( argv[1], &statbuf) < 0 ) {
	       perror( argv[1] );
	       return 1;
	   }
	   major = statbuf.st_rdev >> 8;
	   minor = statbuf.st_rdev & 0xff;
	   target = argv[2];
	   break;
       }
   case 4:
       major = atoi( argv[1] );
       minor = atoi( argv[2] );
       target = argv[3];
       break;
   default:
       fprintf (stderr, "Usage: %s major minor file\n"
	        "\t%s device file\n", argv[0], argv[0] );
       return -1;
   }

   if( (fd = open( target, O_RDWR )) < 0 ) {
      perror( target );
      return 1;
   }

   if( read( fd, buf, sizeof( buf )) != sizeof( buf )) {
      perror ("read");
      return 2;
   }

   printf ("Changing root device of %s from <%d,%d> to ",
	   target, buf[MAJOR], buf[MINOR] );
   buf[MAJOR] = major;
   buf[MINOR] = minor;
   printf ("<%d,%d>.\n", buf[MAJOR], buf[MINOR] );

   if( lseek( fd, 0, L_SET ) < 0 ) {
      perror ("lseek");
      return 3;
   }

   if( write( fd, buf, sizeof( buf )) != sizeof( buf )) {
      perror( "write" );
   }
}


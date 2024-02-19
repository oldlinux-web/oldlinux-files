/* This program is under the GNU copyright
 * Author Damiano Bolla (Italy)
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFSIZE	8192	/* Is 2^13 .... any meaning ?		*/
#define NAMESIZE	255	/* This should be enought....		*/
#define NUMSIZE		30	/* This also should be enought...	*/
#define FLOPPYSIZE	1030000	/* How many bytes the floppy stores ?	*/
#define TMPFILE		"/tmp/backtmp"
#define DUMMYLEN	10	/* USed to read the ENTER		*/

#define MORE		1
#define NOMORE		0

main ( int argc, char *argv[] )
   {
   char archname[NAMESIZE];	/* The name of the archive in floppy	*/
   char *disk;		/* Name of the floppy device to use		*/
   char *buff;		/* The I/O buffer				*/
   int  outf;		/* The output file    Temporary file		*/
   int  More;		/* Do we have more data ? 1=yes 0=no		*/
   int  Leng;		/* The lenght of the current fdata		*/
   int  DiskNum;	/* The current disk number			*/
   int  count;		
   
   
   if ( argc < 2 )
      {
      fprintf (stderr,"You must give me the name of the archive \n");
      exit (2);
      }
   strncpy ( archname, argv[1], NAMESIZE );     
  
   disk = (char *)getenv ( "FLOPPYDISK" );
   if ( (disk == NULL) || (strlen(disk) == 0) )
      {
      fprintf (stderr,"You need to set the variable FLOPPYDISK \n");
      exit (2);
      }

   buff = (char *)malloc (BUFFSIZE);
   if ( buff == NULL )
      {
      fprintf (stderr,"Sorry can't allocate buffer \n");
      exit (2);
      }

   /* The sequence of operations is :
    * Copy part of the source into the temporary file keeping the length
    * Then copy the temporary file with the length informations to floppy
    * Repeat until stdin is exausted.
    */
          
   outf = open (TMPFILE,O_WRONLY|O_CREAT,0777);
   if ( outf < 0 )
      {
      fprintf (stderr,"Sorry can't create %s \n",TMPFILE);
      exit (2);
      }

   /* Ok, now we have to copy from stdin to the tmpfile until size	*/
   Leng    = 0;
   DiskNum = 0;		/* The first disk is 0,1,2,3,4......		*/
   while ( (count=read(0,buff,BUFFSIZE)) >= BUFFSIZE )
      {
      Leng += count;
      write (outf,buff, count);		/* Write what you read		*/
      fprintf (stderr,"."); fflush (stderr);
      if ( Leng >= FLOPPYSIZE )
         {
         close ( outf );
         sync(); sync();	/* Anyway it doen't harm....		*/
         CopyToFloppy ( disk, TMPFILE, archname, DiskNum, MORE, Leng );
         unlink (TMPFILE); sync ();
         Leng = 0;	/* Reset the length				*/         
         DiskNum++;	/* Set for the next floppy			*/
         outf = open (TMPFILE,O_WRONLY|O_CREAT,0777);	/* Should work.	*/
         } 
      }          
      
   /* we reached the end of stdin........				*/
   if ( count > 0 )
      {
      Leng += count;
      write (outf,buff, count);		/* Write what you read		*/
      close ( outf );	
      sync(); sync();
      CopyToFloppy ( disk, TMPFILE, archname, DiskNum, NOMORE , Leng );
      unlink (TMPFILE);	
      sync ();
      }
   else
      fprintf (stderr,"Archive corrupted at last block \n");
      
   exit (0);
   }
   
/* -----------------------------------------------------------------
 * This function take care of writting a file to floppy
 */
int CopyToFloppy ( char *Device,	/* The destination device	*/
                   char *FromFile, 	/* The source file		*/
                   char *ArchName, 	/* The archive name		*/
                   int   DiskNum,	/* The current disk number	*/
                   int   More, 		/* FLAG to indicate nore data	*/
                   int   Leng )  	/* The lenght to write		*/
   {
   char *buff;		/* To read/write from files			*/
   char  dummy[DUMMYLEN];	/* For the ENTER			*/
   int   outf;		/* We write the file here			*/
   int   inf;		/* We read from this file			*/
   char  disk[NUMSIZE];		/* The current disk number		*/
   char  archend[NUMSIZE];	/* 1=End of archive 0=More		*/
   char  curleng[NUMSIZE];	/* The current length of data in floppy	*/
   int   written;	/* How many chars did we write ....		*/
   int   count;		/* The usual temp counter			*/
 
   fprintf (stderr,"\n");
   fprintf (stderr,"Insert a floppy and press ENTER \n");
   read (2,dummy,2);

   fprintf (stderr,"\n");
   fprintf (stderr,"Writting archive %s \n",ArchName);
   fprintf (stderr,"Writting disknum %d \n",DiskNum);
   fprintf (stderr,"Writting more    %d \n",More);
   fprintf (stderr,"Writting Leng    %d \n",Leng);
                        
   buff = (char *)malloc (BUFFSIZE);
   if ( buff == NULL )
      {
      fprintf (stderr,"Sorry can't allocate buffer \n");
      return (-1);
      }
      
   outf = open (Device,O_WRONLY);
   if ( outf < 0 )
      {
      fprintf (stderr,"Sorry can't open device %s \n",Device);
      return (-1);
      }

   inf = open (FromFile,O_RDONLY);
   if ( inf < 0 )
      {
      fprintf (stderr,"Sorry can't open file %s \n",FromFile);
      return (-1);
      }

   /* Now I need to fill the header data to be stored in disk		*/
   sprintf (disk,"%d",DiskNum);
   sprintf (archend,"%d",More );
   sprintf (curleng,"%d",Leng );

   /* Ok, time to start writting archive info to disk			*/
   write (outf,ArchName, NAMESIZE);
   write (outf,disk ,NUMSIZE );
   write (outf,archend ,NUMSIZE );
   write (outf,curleng ,NUMSIZE );

   /* and then it is time for the data from the tmpfile to floppy	*/
   written = 0;
   while ( (count=read(inf,buff, BUFFSIZE)) > 0 )
      {
      /* We have some data from file to store in floppy			*/
      fprintf(stderr,"#"); fflush (stderr);
      written += count;
      write ( outf, buff, count);	/* We write what we read	*/
      }

   fprintf (stderr,"\n");
   if ( written != Leng )
      {
      fprintf (stderr,"ERROR, written %d != Leng %d \n ",written,Leng);
      return (-1);
      }

   /* Ok the data should be in the floppy now, close and sync	      	*/
   close ( inf );
   close ( outf );
   sync (); sync (); sync ();

   return (0);
   }
   
       
       

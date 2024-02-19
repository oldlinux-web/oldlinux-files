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
#define FLOPPYSIZE	2794*512	/* How many bytes the floppy	*/
#define TMPFILE		"/tmp/backtmp"
#define DUMMYLEN	10	/* USed to read the ENTER		*/

#define MORE		1
#define NOMORE		0

#define USEMEM		1
#define USEFILE		2

main ( int argc, char *argv[] )
   {
   char archname[NAMESIZE];	/* The name of the archive in floppy	*/
   char *disk;		/* Name of the floppy device to use		*/
   char *buff;		/* The I/O buffer				*/
   char *tbuff;		/* The temp buffer				*/
   int  outf;		/* The output file    Temporary file		*/
   int  More;		/* Do we have more data ? 1=yes 0=no		*/
   int  Leng;		/* The lenght of the current fdata		*/
   int  DiskNum;	/* The current disk number			*/
   int  count;		
   int  tempstore;	/* Flag to know where to store....		*/
   
   /* By default we use memory as a temporary....			*/
   tempstore = USEMEM;
   
   if ( argc < 2 )
      {
      fprintf (stderr,"You must give me the name of the archive \n");
      exit (2);
      }
      
   /* If we have three params the second is the option...		*/
   if ( argc == 3 )
      { 
      if ( strcmp( argv[1],"-f" ) == 0 )
         tempstore = USEFILE;
      strncpy ( archname, argv[2], NAMESIZE );     
      }   
   else
      strncpy ( archname, argv[1], NAMESIZE );     
         
   disk = (char *)getenv ( "FLOPPYDISK" );
   if ( (disk == NULL) || (strlen(disk) == 0) )
      {
      fprintf (stderr,"You need to set the variable FLOPPYDISK \n");
      exit (2);
      }

   if ( tempstore == USEMEM )
      { 
      int c;
      tbuff = (char *)malloc (FLOPPYSIZE);
      if ( tbuff == NULL )
         {
         fprintf (stderr,"Sorry can't allocate temp buffer \n");
         exit (2);
         }
      /* We now get all the reqired mem. Linux is a bit "strange"	*/
      printf ("Testing memory \n");
      for (c=0; c<FLOPPYSIZE; c++ ) tbuff[c]=0;   
      printf ("Memory OK \n");
      }

   /* This is the I/O buffer.....					*/   
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

   if ( tempstore == USEFILE )       
      {
      outf = open (TMPFILE,O_WRONLY|O_CREAT,0777);
      if ( outf < 0 )
         {
         fprintf (stderr,"Sorry can't create %s \n",TMPFILE);
         exit (2);
         }

      /* Ok, now we have to copy from stdin to the tmpfile until size	*/
      Leng    = 0;
      DiskNum = 0;		/* The first disk is 0,1,2,3,4......		*/
   
      while ( (count=Rread(0,buff,BUFFSIZE)) >= BUFFSIZE )
         {
         Leng += count;
         write (outf,buff,count);		/* Write what you read		*/
         fprintf (stderr,"."); fflush (stderr);
         if ( Leng >= FLOPPYSIZE )
            {
            close ( outf );
            sync(); sync();	/* Anyway it doen't harm....		*/
            CopyFilToFloppy ( disk, TMPFILE, archname, DiskNum, MORE, Leng );
            unlink (TMPFILE); sync ();
            Leng = 0;	/* Reset the length				*/         
            DiskNum++;	/* Set for the next floppy			*/
            outf = open (TMPFILE,O_WRONLY|O_CREAT,0777);/* Should work.	*/
            } 
         }          
         
      /* we reached the end of stdin........				*/
      if ( count > 0 )
         {
         Leng += count;
         write (outf,buff, count);		/* Write what you read		*/
         close ( outf );	
         sync(); sync();
            CopyFilToFloppy ( disk, TMPFILE, archname, DiskNum, NOMORE , Leng );
         unlink (TMPFILE);	
         sync ();
         }
      else
         {
         /* We don't have data.... BUT did we had an error ???		*/
         if ( count < 0 ) fprintf (stderr,"Archive corrupted at last block \n");
         }
      }
   else
      {
      /* WE use memory as temporary here.....				*/   
      char *tptr = tbuff;
      
      /* Ok, now we have to copy from stdin to the tbuff until size	*/
      Leng    = 0;
      DiskNum = 0;		/* The first disk is 0,1,2,3,4......	*/
   
      while ( (count=Rread(0,tptr,BUFFSIZE)) >= BUFFSIZE )
         {
         Leng += count; tptr += count;
         fprintf (stderr,"."); fflush (stderr);
         
         if ( Leng >= FLOPPYSIZE )
            {
            CopyMemToFloppy ( disk, tbuff, archname, DiskNum, MORE, Leng );
            Leng = 0;		/* Reset the length			*/         
            tptr = tbuff;	/* Restore the initial pointer		*/
            DiskNum++;		/* Set for the next floppy		*/
            } 
         }          
         
      /* we reached the end of stdin........				*/
      if ( count > 0 )
         {
         Leng += count;
         CopyMemToFloppy ( disk, tbuff, archname, DiskNum, NOMORE , Leng );
         sync ();
         }
      else
         {
         /* We don't have data.... BUT did we had an error ???		*/
         if ( count < 0 ) fprintf (stderr,"Archive corrupted at last block \n");
         } 
      }
   exit (0);
   }
   
/* -----------------------------------------------------------------
 * This function take care of writting a file to floppy
 */
int CopyFilToFloppy ( char *Device,	/* The destination device	*/
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
   fprintf (stderr,"Writing archive %s \n",ArchName);
   fprintf (stderr,"Writing disknum %d \n",DiskNum);
   fprintf (stderr,"Writing more    %d \n",More);
   fprintf (stderr,"Writing Leng    %d \n",Leng);
                        
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
   write (outf,disk     ,NUMSIZE );
   write (outf,archend  ,NUMSIZE );
   write (outf,curleng  ,NUMSIZE );

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
   
/* -------------------------------------------------------------- 
 * This function will behave in a consistemt way if used over a file 
 * or a pipe. Thanks to Linux for pointing out the reasons of the
 * pipe behaviour.
 */
       
int Rread(int chan, char *buff, int size )
   {
   int err;
   int requested;
   requested = size;
    
   while ( size > 0 )
      {
      err = read ( chan, buff, size );
      if ( err > 0 ) { buff += err; size -= err; }
      else break;
      }

   if ( requested-size > 0 )
      return ( requested - size );
   else
      return (err);
   }   
      
   
/* -----------------------------------------------------------------
 * This function take care of writting a block of mem to floppy
 */
int CopyMemToFloppy ( char *Device,	/* The destination device	*/
                   char *FromBuff, 	/* The source buffer		*/
                   char *ArchName, 	/* The archive name		*/
                   int   DiskNum,	/* The current disk number	*/
                   int   More, 		/* FLAG to indicate nore data	*/
                   int   Leng )  	/* The lenght to write		*/
   {
   char  *tptr;
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
   fprintf (stderr,"Writing archive %s \n",ArchName);
   fprintf (stderr,"Writing disknum %d \n",DiskNum);
   fprintf (stderr,"Writing more    %d \n",More);
   fprintf (stderr,"Writing Leng    %d \n",Leng);
                        
   outf = open (Device,O_WRONLY);
   if ( outf < 0 )
      {
      fprintf (stderr,"Sorry can't open device %s \n",Device);
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
   tptr = FromBuff;
   written = 0;
   while ( written < Leng )
      {
      /* We have some data from file to store in floppy			*/
      fprintf(stderr,"#"); fflush (stderr);
      if ( Leng - written > BUFFSIZE )
         count = BUFFSIZE;
      else
         count = Leng - written;
      write ( outf, tptr, count);	
      written += count;
      tptr += count;
      }

   fprintf (stderr,"\n");
   close ( outf );
   sync (); sync (); sync ();

   return (0);
   }
   

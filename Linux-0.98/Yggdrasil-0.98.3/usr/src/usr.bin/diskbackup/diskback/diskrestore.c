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
#define DUMMYLEN	10
#define MORE		1
#define NOMORE		0

int  More = MORE;	/* We try to start with more data		*/
int  DiskNum = 0;	/* The current disk number			*/
char backname[NAMESIZE] ="";	/* The name of the archive in floppy	*/

main ( int argc, char *argv[] )
   {
   char tmpname[NAMESIZE];	/* The name of the archive in floppy	*/
   char tmpnum[NUMSIZE];
   char dummy[DUMMYLEN];
   int  tmpdisk;	/* Temporary for the disk number		*/
   char *disk;		/* Name of the floppy device to use		*/
   char *buff;		/* IO buffer for the data			*/
   int  Leng;		/* The lenght of the current fdata		*/
   int  count;		
   int  TmpLeng;
   int  inf;
   
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
    * Get the informations from the disk read data, write data
    */
          
   while ( More == MORE )
      {
      fprintf (stderr,"Insert floppy, then press ENTER \n");
      read (2,dummy,2);
      
      inf = open (disk,O_RDONLY);
      if ( inf < 0 )
         {
         fprintf (stderr,"Sorry can't open device %s \n",disk);
         return (-1);
         }
   
      /* Get the archive name from the floppy and check if OK		*/
      read (inf, tmpname, NAMESIZE);
      if ( strlen ( backname ) <= 0 )
         strcpy ( backname, tmpname );
      else
         {
         if ( strcmp ( backname, tmpname ) != 0 )
            {
            fprintf (stderr,"Wrong floppy, I want %s \n",backname );
            close (inf);
            continue;
            }
         }      

     /* Then I would like to know ehat floppy it is in the archive	*/
     read (inf, tmpnum, NUMSIZE);
     tmpdisk = atoi ( tmpnum );
     if (DiskNum != tmpdisk )
        {
        fprintf (stderr,"Wrong floppy, I want num %d \n",DiskNum );
        close (inf);
        continue; 
        }

      read (inf, tmpnum, NUMSIZE);
      More = atoi ( tmpnum );
        
      read (inf, tmpnum, NUMSIZE);
      Leng = atoi ( tmpnum );
      
      fprintf (stderr,"Archive name : %s \n",backname);
      fprintf (stderr,"Current disk : %d \n",DiskNum);
      fprintf (stderr,"More disks   : %d \n",More);
      fprintf (stderr,"Cur Leng     : %d \n",Leng);
      
      TmpLeng = 0;
      while ( (count=read(inf,buff,BUFFSIZE)) > 0 )
         {
         fprintf (stderr,"#"); fflush (stderr);

         /* The floppy will read MORE than needed, then TURUNCATE	*/
         if ( (TmpLeng+count) > Leng )
            {
            write (1,buff, (Leng-TmpLeng));
            fprintf (stderr,"\n");
            fprintf (stderr,"Short write %d \n",(Leng-TmpLeng));
            TmpLeng += count; 
            }
         else
            {
            write (1,buff, count);	
            TmpLeng += count;
            }

         if ( TmpLeng >= Leng )
            {
            close (inf);
            sync (); sync ();
            DiskNum ++;		/* Want the next disk			*/
            break;		/* Exit from this while			*/
            }
         }      
      fprintf (stderr,"\n");
      /* This while exit when more == 0 ( No more disks in archive	*/
      }          

   close (1);	/* Possibly you loose chars if you exit without close	*/
   sync (); sync (); sync ();
   exit (0);
   }
   

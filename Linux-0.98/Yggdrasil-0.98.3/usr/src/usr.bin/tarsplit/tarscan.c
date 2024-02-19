/*
**  TARSCAN
**  News archive tape scanner/splitter.
**
**  Reads a tar news archive, optionally splitting it into smaller
**  archives or listing the contained files and their Subject lines
**  on stdout.  Useful for creating a multi-floppy archive when
**  you might want to retrieve just one file from the 27th floppy
**  of an archive.  Each volume is a complete stand-alone tar archive.
**
**  Arguments:
**
**     One or more of:
**
**     -v              Write file names and Subject lines on stdout.
**                     (Mainly for news articles or mail.)  If the
**                     -s switch is present, tarscan will announce
**                     volume changes on stdout.
**
**     -f filename     Copy the input tarfile to the named file.
**
**     -s size         Only used with -f.  Break the tarfile into
**                     individual tarfiles that don't exceed 'size'
**                     tape blocks.  If 'filename' is a device,
**                     the user will be prompted to change media
**                     when appropriate.  If 'filename' is not a
**                     device, then the input tarfile is copied
**                     into a series of files with 'filename' as
**                     a base and sequential numbers as a suffix.
**                     (Beware of 14-character filenames!)
**
*/
#include <stdio.h>
#include <sys/file.h>
#include <strings.h>

#define TBLOCK         512
#define NAMSIZ         100
#define CLUMP          1

extern char    *strcpy();

extern char    *optarg;
extern int      optind;

typedef union {
    char        dummy[TBLOCK];
    struct {
       char     name[NAMSIZ];
       char     mode[8];
       char     uid[8];
       char     gid[8];
       char     size[12];
       char     mtime[12];
       char     chksum[8];
       char     linkflag;
       char     linkname[NAMSIZ];
    } D;
} HEADER;

#define OCTAL(x) (((x) >= '0') && ((x) <= '7'))

void
usage ()
{
    fprintf (stderr, "Usage: tarscan [-v] [-f filename [-s size] ] <tarfile\n");
    exit (1);
}

static HEADER *
myread(mt)
    int                         mt;
{
    static HEADER       buff[CLUMP];
    static int          pos = CLUMP;
    int                         len;

    if (pos == CLUMP) {
       if ((len=read(mt, (char *)buff, sizeof buff)) == 0) {
           return ((HEADER *) 0);
       } else if (len == -1) {
           perror ("read");
           exit (1);
       }
       pos = 0;
    }
    return(&buff[pos++]);
}


main(ac, av)
    int                         ac;
    char               *av[];
{
    register HEADER    *H;
    register char      *p;
    register char      *q;
    register char      *r;
    register char      *t;
    char                subject[256];
    register int        Nblocks;
    register int        first;
    register int        size;
    char                Name[NAMSIZ];
    int                         outblocks;
    int                         outvol = 0;
    int                         outfile = -1;
    int                         volume_size;
    char                zbuff [TBLOCK];
    char                outname [100];
    char                t_outname [100];
    int                         verbose = 0;
    int                         make_copy = 0;
    int                         limit_size = 0;
    int                         start;
    int                         c;
    int                         device;

    for (p=zbuff,q=zbuff+TBLOCK; p<q; (*p++)=0) ;

    while ((c=getopt(ac, av, "vf:s:")) != -1)
       switch (c) {
       case 'v':
           verbose = 1;
           fprintf (stderr, "verbose flag\n");
           break;
       case 'f':
           make_copy = 1;
           strcpy (outname, optarg);
           device = strncmp ("/dev/", outname, 5) == 0;
           fprintf (stderr, "make copy on %s\n", outname);
           break;
       case 's':
           limit_size = 1;
           volume_size = atoi(optarg);
           fprintf (stderr, "limit output volumes to %d blocks\n", volume_size);
           break;
       default:
           usage ();
           exit (1);
       }

    if ((verbose + make_copy + limit_size) == 0) {
       usage ();
       exit (1);
    }
    if (limit_size && !make_copy) {
       usage ();
       exit (1);
    }

    if (!limit_size) {
       if (device) {
           outfile = open (outname, O_WRONLY);
       } else {
           outfile = creat (outname, 0777);
       }
       if (outfile == -1) {
           perror ("open output file");
           exit (1);
       }
    } else {
       outblocks = volume_size;
    }

    for (first = 0, Nblocks = 0; H = myread(0); ) {
       if (Nblocks) {
           Nblocks--;
           if (first && verbose) {
               for (p=H->D.name, q=p+TBLOCK, start=1; p<q; p++) {
                   if (start && (strncmp(p, "Subject", 7)==0)) {
                       for (t=subject, r=p+8; r<q && *r != '\n'; r++) *(t++) = *r;
                       *t = 0;
                       printf ("       %s", subject);
                       break;
                   }
                   start = *p == '\n';
               }
               printf ("\n");
           }
           first = 0;
       }
       else if (H->D.name[0]) {
           if (H->D.name[NAMSIZ - 1]) {
               H->D.name[NAMSIZ - 1] = 0;
               fprintf(stderr,
                       "Name \"%s\" is more than %d bytes long, I give up!\n",
                       H->D.name,
                       NAMSIZ);
               exit(1);
           }
           size = 0;
           if (H->D.linkflag != '1')
               for (p = H->D.size,q=H->D.size+12; p<q ; p++) {
                   if (OCTAL(*p)) size = (size << 3) + *p - '0';
               }
           Nblocks = size / TBLOCK + ((size % TBLOCK) > 0);
           first = Nblocks > 0;
           if (make_copy && ((outblocks+Nblocks+3) > volume_size)) {
               if (outfile > 0) {
                   write (outfile, zbuff, TBLOCK);
                   write (outfile, zbuff, TBLOCK);
                   close (outfile);
               }
               if (device) {
                   outfile = open (outname, O_RDONLY);
               } else {
                   sprintf (t_outname, "%s%03d", outname , outvol);
                   outfile=creat(t_outname, 0777);
               }
               if (outfile == -1) {
                   perror ("open output file");
                   exit(1);
               } 
               if (verbose && limit_size) 
                   if (device) 
                       printf ("\n\nVolume %d on %s\n\n", outvol, outname);
                   else
                       printf ("\n\nTarfile: %s\n\n", t_outname);
               outvol++;
               outblocks = 0;
           }
           printf ("%s", H->D.name);
       } 
       if (make_copy) {
           if (write(outfile, (char *) H, TBLOCK) != TBLOCK) { 
               perror ("write tape block");
               exit (1);
           }
           outblocks++;
       }
    }    

    exit(0);
}

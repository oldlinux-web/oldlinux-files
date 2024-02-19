/* This file contains system-specific functions for MS-DOS.
 * The program pfdisk.c calls these routines.
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dos.h>

#define extern
#include "sysdep.h"
#undef extern

int usage(prog)	/* print a usage message */
char	*prog;	/* program name */
{
  fprintf(stderr,"Usage: %s <disk>\n", prog);
  fprintf(stderr,"\twhere <disk> is a digit [0-9]\n");
}

void getGeometry(name, c, h, s)
char	*name;		/* device name */
unsigned *c,*h,*s;	/* cyls, heads, sectors */
{
  int dev;	/* hard disk number */
  union 	REGS	regs;
  struct	SREGS	sregs;
  
  if (name[0] < '0' ||
      name[0] > '9' ||
      name[1] != 0  )
    {
      fprintf(stderr,"%s: device name must be a digit\n", name);
      return;
    }
  dev = (name[0] - '0');
  
  regs.h.ah = 8;		/* get param.	*/
  regs.h.dl = dev | 0x80;
  
  int86x(0x13,&regs,&regs,&sregs);
  
  /* Are that many drives responding? */
  if (regs.h.dl <= dev ) {
    fprintf(stderr,"%s: drive not found\n", name);
    return;
  }
  if (regs.x.cflag) {
    fprintf(stderr,"%s: can't get disk parameters\n", name);
    return;
  }
  *c = ((((int) regs.h.cl << 2) & 0x300) | regs.h.ch) + 1;
  *h = regs.h.dh + 1;
  *s = regs.h.cl & 0x3F;
}

int getFile(name, buf, len)	/* read file into buffer */
char	*name, *buf;
int	len;
{	/* (open, read, close) */
  int devfd, retval;
  
  devfd = open(name, O_RDONLY|O_BINARY, 0);
  if (devfd < 0) {
    fprintf(stderr,"%s: can't open for reading\n", name);
    return(devfd);
  }
  retval = read(devfd, buf, len);
  if (retval < 0)
    fprintf(stderr,"%s: read failed\n", name);
  close(devfd);
  return(retval);
}

int putFile(name, buf, len)	/* write buffer to file */
char	*name, *buf;
int	len;
{	/* (open, write, close) */
  int devfd, retval;
  
  devfd = open(name,
	       O_WRONLY|O_CREAT|O_BINARY,
	       S_IREAD|S_IWRITE ); /* stupid DOS... */
  if (devfd < 0) {
    fprintf(stderr,"%s: can't open for writing\n", name);
    return(devfd);
  }
  retval = write(devfd, buf, len);
  if (retval < 0)
    fprintf(stderr,"%s: write failed\n", name);
  close(devfd);
  return(retval);
}

int getBBlk(name, buf)	/* read boot block into buffer */
char	*name, *buf;
{	/* BIOS absolute disk read */
  int dev;
  union 	REGS	regs;
  struct	SREGS	sregs;
  
  if (name[0] < '0' ||
      name[0] > '9' ||
      name[1] != 0  )
    {
      fprintf(stderr,"%s: device name must be a digit\n",name);
      return(-1);
    }
  dev = (name[0] - '0');
  
  segread(&sregs);	/* get ds */
  sregs.es = sregs.ds;	/* buffer address */
  regs.x.bx = (int) buf;
  
  regs.h.ah = 2;		/* read		*/
  regs.h.al = 1;		/* sector count	*/
  regs.h.ch = 0;		/* track	*/
  regs.h.cl = 1;		/* start sector	*/
  regs.h.dh = 0;		/* head		*/
  regs.h.dl = dev|0x80;	/* drive	*/
  
  int86x(0x13,&regs,&regs,&sregs);
  if (regs.x.cflag) {
    fprintf(stderr,"%s: read failed\n", name);
    return(-1);
  }
  return(SECSIZE);
}

int putBBlk(name, buf)	/* write buffer to boot block */
char	*name, *buf;
{	/* BIOS absolute disk write */
  int dev;
  union 	REGS	regs;
  struct	SREGS	sregs;
  
  if (name[0] < '0' ||
      name[0] > '9' ||
      name[1] != 0  )
    {
      fprintf(stderr,"%s: device name must be a digit\n", name);
      return(-1);
    }
  dev = (name[0] - '0');
  
  segread(&sregs);	/* get ds */
  sregs.es = sregs.ds;	/* buffer address */
  regs.x.bx = (int) buf;
  
  regs.h.ah = 3;		/* write	*/
  regs.h.al = 1;		/* sector count	*/
  regs.h.ch = 0;		/* track	*/
  regs.h.cl = 1;		/* start sector	*/
  regs.h.dh = 0;		/* head		*/
  regs.h.dl = dev|0x80;	/* drive	*/
  
  int86x(0x13,&regs,&regs,&sregs);
  if (regs.x.cflag) {
    fprintf(stderr,"%s: write failed\n",name);
    return(-1);
  }
  return(SECSIZE);
}

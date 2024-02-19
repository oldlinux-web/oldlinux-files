#include <amoeba.h>
#include <minix/callnr.h>
#include <errno.h>
#include "header.h"

header hdr;				/* header for incoming messages */
char buffer[BUF_SIZE+NAME_SIZE];	/* buffer for incoming messages */
char *server_name = "filsrv";
int repsize;
extern int errno;

main()
{
/* This is a primitive file server.  The client runs with a special set of
 * routines for read(), write(), etc. that call this server.  The server is
 * stateless.
 */

  int s;
  int count;
  unshort opcode, size;

  strncpy( (char *) &hdr.h_port, server_name, PORTSIZE);  /* init port */

  while (1) {
	/* Wait for a request to arrive. */
	count = (short) getreq(&hdr, buffer, MAX_TRANS);
	if (count < 0) {
	    printf("Server's getreq failed. Error = %d.   ", count);
	    printf("Hit F1 to see if AMTASK running.\n");
	    exit(1);
	}
	
	/* Dispatch on opcode. */
	opcode = hdr.h_command;
	repsize = 0;
	errno = 0;
	switch(opcode) {
		case CREAT:	s = do_creat();		break;
		case READ:	s = do_read();		break;
		case WRITE:	s = do_write();		break;
		default:	s = EINVAL;		break;
	}

	/* Work done.  Send a reply. */
	hdr.h_status = (unshort) s;
 	hdr.h_extra = (unshort) errno;
	putrep(&hdr, buffer, repsize);
  }
}

int do_read()
{
  /* Stateless read. */

  int fd, n;
  long offset;
  unshort count;

  offset = hdr.h_offset;
  count = hdr.h_size;
  if (count > MAX_TRANS) count = MAX_TRANS;

  fd = open(buffer, 0);		/* open the file for reading */
  if (fd < 0) return(errno);
  lseek(fd, offset, 0);
  n = read(fd, buffer, count);
  close(fd);
  repsize = n;
  return(n);
}

int do_write()
{
  /* Stateless write. */

  int fd, n;
  long offset;
  unshort count;

  offset = hdr.h_offset;
  count = hdr.h_size;
  if (count > MAX_TRANS) count = MAX_TRANS;

  fd = open(&buffer[BUF_SIZE], 2);		/* open the file for writing */
  if (fd < 0) return(errno);
  lseek(fd, offset, 0);
  n = write(fd, buffer, count);
  close(fd);
  return(n);
}

int do_creat()
{
  /* Stateless creat. */

  int fd, n, mode;
  

  mode = hdr.h_size;
  fd = creat(buffer, mode);		/* creat the file  */ 
  close(fd);
  return(fd);
}

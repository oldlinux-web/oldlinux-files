/* This file shows how one could build a remote file server for MINIX.
 * In this file there are several library routines for the basic system
 * calls.  Unlike the "real" ones, these call a remote file server instead
 * of the local kernel.  On client machines, one would replace the library
 * routines with these routines, and then recompile programs.  In this way,
 * clients will then call the remote file server.  It should be obvious that
 * this file is just an example, and that a productio version would have to
 * be much more complete.
 *
 * The file server3.c contains the start of a stateless file server.  Because
 * MINIX is not stateless, the conversion must be done in this library.  For
 * example, when an open() is done, the library records the name, but no
 * operation is performed on the file server.
 *
 * An alternative approach to making a remote file systems is to replace FS, 
 * the local file server, with one that makes the calls to the remote file 
 * server itself.  This approach is less efficient, because a call then 
 * consists of a local FS call plus a remote one, but it is more transparent 
 * because no programs need to recompiled.
 */

#include <amoeba.h>
#include <errno.h>
#include <minix/callnr.h>
#include "header.h"

#define MAX_FD 20
#define LOCAL 100
#define HEAPSIZE 512			/* space for file names */
#define WRITING 2
#define ER -1
#define FS 1
#define NIL_PTR (char*) 0

/* The local array is indexed by file descriptor.  Those entries containing
 * LOCAL are local (e.g., stdin), those containing REMOTE are remote, and
 * those containing 0 are unassigned.
 */

char where[MAX_FD] = {LOCAL, LOCAL, LOCAL};
long pos[MAX_FD];			/* current offset */
char *server_name = "filsrv";
char *file_name[MAX_FD];

char heap[HEAPSIZE];
char *heap_ptr = heap;
header hdr1, hdr2;
char buffer[BUF_SIZE+NAME_SIZE];

extern int errno;


/*============================= Remote Library ==============================*/
int open(name, how)
char *name;
int how;
{
/* Open is entirely local. */

  int i, len;

  if (how < 0 || how > 2) { errno = EINVAL; return(ER);}

  /* Find a free file descriptor. */
  for (i = 0; i < MAX_FD; i++) {
	if (where[i] == 0) {
		len = strlen(name);
		file_name[i] = heap_ptr;
		bcopy(name, heap_ptr, len);
		heap_ptr += len;
		*heap_ptr++ = 0;
		where[i] = how+1;
		return(i);
	}
  }
  errno = EMFILE;
  return(ER);
}
  
int creat(name, mode)
char *name;
int mode;
{
/* Create a file. */

  int i, len, n;

  /* Find a free file descriptor. */
  for (i = 0; i < MAX_FD; i++) {
	if (where[i] == 0) {
		len = strlen(name);
		file_name[i] = heap_ptr;
		bcopy(name, heap_ptr, len);
		heap_ptr += len;
		*heap_ptr++ = 0;
		where[i] = WRITING;

		strncpy(&hdr1.h_port, server_name, PORTSIZE);
		hdr1.h_command = CREAT;
		hdr1.h_size = mode;
		n = trans(&hdr1, file_name[i], len+1, &hdr2, buffer, 0);
		if (n < 0) {errno = EIO; return(ER);}
		return(hdr2.h_status);
	}
  }
  errno = EMFILE;
  return(ER);
}
  

/* int close(fd)
int fd;
{

  if (where[fd] == LOCAL) return(Xclose(fd));
  if (where[fd] == 0) {errno = EBADF; return(ER);}
  where[fd] = 0;
  return(OK);
}
*/

int read(fd, buf, bytes)
int fd, bytes;
char buf[];
{
/* Primitive read() routine for reads up to 1K. */

  int n;

  if (where[fd] == LOCAL) return (Xread(fd, buf, bytes));
  if ((where[fd]&1) == 0) {errno = EBADF; return(ER);}
  if (bytes > BUF_SIZE) return(EINVAL);	/* in a real version, fix this */
  strncpy(&hdr1.h_port, server_name, PORTSIZE);

  hdr1.h_command = READ;
  hdr1.h_size = bytes;
  hdr1.h_offset = pos[fd];
  n = trans(&hdr1, file_name[fd], strlen(file_name[fd])+1, &hdr2, buf, bytes);
  if (n < 0) {errno = EIO; return(ER);}
  if (hdr2.h_extra != 0) errno = hdr2.h_extra;
  pos[fd] += hdr2.h_status;		/* advance file position */
  return(hdr2.h_status);
}


int write(fd, buf, bytes)
int fd, bytes;
char buf[];
{
/* Primitive write() routine for writes up to 1K.  This is a very simple
 * routine.  Because the server is stateless, for a write we must send both
 * the data and the file name.  In this example, the first 1K of the buffer
 * is reserved for the data, with the file name starting at position 1024.
 */

  int n, len;

  if (where[fd] == LOCAL) return (Xwrite(fd, buf, bytes));
  if ((where[fd]&02) == 0) {errno = EBADF; return(ER);}
  if (bytes > BUF_SIZE) return(EINVAL);	/* in a real version, fix this */
  strncpy(&hdr1.h_port, server_name, PORTSIZE);

  len = strlen(file_name[fd]);
  hdr1.h_command = WRITE;
  hdr1.h_size = bytes;
  hdr1.h_offset = pos[fd];
  bcopy(buf, buffer, bytes);		/* copy data to message */
  bcopy(file_name[fd], &buffer[BUF_SIZE], len+1);
  n = trans(&hdr1, buffer, BUF_SIZE+len+1, &hdr2, buf, 0);
  if(n < 0) {errno = EIO; return(ER);}
  if (hdr2.h_extra != 0) errno = hdr2.h_extra;
  pos[fd] += hdr2.h_status;
  return(hdr2.h_status);
}




/* Below are the real calls, which are sometimes needed. */

int Xread(fd, buffer, nbytes)
int fd;
char *buffer;
int nbytes;
{
  int n;
  n = callm1(FS, READ, fd, nbytes, 0, buffer, NIL_PTR, NIL_PTR);
  return(n);
}

int Xwrite(fd, buffer, nbytes)
char *buffer;
int nbytes;
{
  return callm1(FS, WRITE, fd, nbytes, 0, buffer, NIL_PTR, NIL_PTR);
}


int Xclose(fd)
int fd;
{
  return callm1(FS, CLOSE, fd, 0, 0, NIL_PTR, NIL_PTR, NIL_PTR);

}


/* ========================= test program =============================*/
main(argc, argv)
int argc;
char *argv[];
{
  int fd1, n;
  char b[1024];

  if (argc != 2) {
	printf("Usage: client3 file\n");
	exit(1);
  }

  fd1 = open(argv[1], 0);
  if (fd1 < 0) {
	printf("Open of %s failed\n", argv[1]);
	exit(1);
  }

  do {
	if ((n=read(fd1, b, 1024) < 0)) {
		printf("Cannot read %s\n", argv[1]);
		exit(1);
	}
	if (write(1, b, n) < 0) {
		printf("Cannot write stdout\n");
		exit(1);
	}
  } while (n > 0);
}

/* ar.c - Archive modify and extract.
   Copyright (C) 1988 Free Software Foundation, Inc.

		       NO WARRANTY

  BECAUSE THIS PROGRAM IS LICENSED FREE OF CHARGE, WE PROVIDE ABSOLUTELY
NO WARRANTY, TO THE EXTENT PERMITTED BY APPLICABLE STATE LAW.  EXCEPT
WHEN OTHERWISE STATED IN WRITING, FREE SOFTWARE FOUNDATION, INC,
RICHARD M. STALLMAN AND/OR OTHER PARTIES PROVIDE THIS PROGRAM "AS IS"
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY
AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
CORRECTION.

 IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW WILL RICHARD M.
STALLMAN, THE FREE SOFTWARE FOUNDATION, INC., AND/OR ANY OTHER PARTY
WHO MAY MODIFY AND REDISTRIBUTE THIS PROGRAM AS PERMITTED BELOW, BE
LIABLE TO YOU FOR DAMAGES, INCLUDING ANY LOST PROFITS, LOST MONIES, OR
OTHER SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
USE OR INABILITY TO USE (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR
DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR
A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) THIS
PROGRAM, EVEN IF YOU HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES, OR FOR ANY CLAIM BY ANY OTHER PARTY.

		GENERAL PUBLIC LICENSE TO COPY

  1. You may copy and distribute verbatim copies of this source file
as you receive it, in any medium, provided that you conspicuously
and appropriately publish on each copy a valid copyright notice
"Copyright (C) 1987 Free Software Foundation, Inc.", and include
following the copyright notice a verbatim copy of the above disclaimer
of warranty and of this License.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be licensed at no charge to all
    third parties on terms identical to those contained in this
    License Agreement (except that you may choose to grant more
    extensive warranty protection to third parties, at your option).

    c) You may charge a distribution fee for the physical act of
    transferring a copy, and you may at your option offer warranty
    protection in exchange for a fee.

  3. You may copy and distribute this program or any portion of it in
compiled, executable or object code form under the terms of Paragraphs
1 and 2 above provided that you do the following:

    a) cause each such copy to be accompanied by the
    corresponding machine-readable source code, which must
    be distributed under the terms of Paragraphs 1 and 2 above; or,

    b) cause each such copy to be accompanied by a
    written offer, with no time limit, to give any third party
    free (except for a nominal shipping charge) a machine readable
    copy of the corresponding source code, to be distributed
    under the terms of Paragraphs 1 and 2 above; or,

    c) in the case of a recipient of this program in compiled, executable
    or object code form (without the corresponding source code) you
    shall cause copies you distribute to be accompanied by a copy
    of the written offer of source code which you received along
    with the copy you received.

  4. You may not copy, sublicense, distribute or transfer this program
except as expressly provided under this License Agreement.  Any attempt
otherwise to copy, sublicense, distribute or transfer this program is void and
your rights to use the program under this License agreement shall be
automatically terminated.  However, parties who have received computer
software programs from you with this License Agreement will not have
their licenses terminated so long as such parties remain in full compliance.

  5. If you wish to incorporate parts of this program into other free
programs whose distribution conditions are different, write to the Free
Software Foundation at 675 Mass Ave, Cambridge, MA 02139.  We have not yet
worked out a simple rule that can be stated here, but we will often permit
this.  We will be guided by the two goals of preserving the free status of
all derivatives of our free software and of promoting the sharing and reuse of
software.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

#include <stdio.h>
#include <ar.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef COFF_ENCAPSULATE
#include "a.out.encap.h"
#else
#include <a.out.h>
#endif

#ifdef USG
#include <time.h>
#include <fcntl.h>
#else
#include <sys/file.h>
#include <sys/time.h>
#endif

  /* Locking is normally disabled because fcntl hangs on the Sun
     and it isn't supported properly across NFS anyhow.  */

#ifdef LOCKS
#include <sys/fcntl.h>
#endif

/* Define a name for the length of member name an archive can store.  */

struct ar_hdr foo;

#define ARNAMESIZE sizeof(foo.ar_name)

/* This structure is used internally to represent the info
 on a member of an archive.  This is to make it easier to change format.  */

struct member_desc
  {
    char *name;		/* Name of member */
/* The following fields are stored in the member header
   as decimal or octal numerals,
   but in this structure they are stored as machine numbers.  */
    int mode;		/* Protection mode from member header */
    int date;		/* Last modify date as stored in member header */
    int size;		/* size of member's data in bytes, from member header */
    int uid, gid;	/* uid and gid fields copied from member header */
    int offset;		/* Offset in archive of the header of this member */
    int data_offset;	/* Offset of first data byte of the member */
/* The next field does not describe where the member was in the old archive,
 but rather where it will be in the modified archive.
 It is set up by write_archive.  */
    int new_offset;	/* Offset of this member in new archive */
    struct symdef *symdefs;	/* Symdef data for member.
				   Used only for files being inserted.  */
    int nsymdefs;	/* Number of entries of symdef data */
    int string_size;	/* Size of strings needed by symdef data */
  };

/* Each symbol is recorded by something like this.  */

struct symdef
  {
    union
      {
	long stringoffset;
	char *name;
      } s;
    long offset;
  };

/* Nonzero means it's the name of an existing member;
   position new or moved files with respect to this one.  */

char *posname;


/* How to use `posname':
   POS_BEFORE means position before that member.
   POS_AFTER means position after that member.
   POS_DEFAULT if position by default; then `posname' should also be zero. */

enum { POS_DEFAULT, POS_BEFORE, POS_AFTER } postype;

/* Nonzero means describe each action performed.  */

int verbose;

/* Nonzero means don't warn about creating the archive file if necessary.  */

int silent_create;

/* Nonzero means don't replace existing members
 whose dates are more recent than the corresponding files.  */

int newer_only;

/* Nonzero means preserve dates of members when extracting them.  */

int preserve_dates;

/* Operation to be performed.  */

#define DELETE 1
#define REPLACE 2
#define PRINT_TABLE 3
#define PRINT_FILES 4
#define EXTRACT 5
#define MOVE 6
#define QUICK_APPEND 7

int operation;

/* Name of archive file.  */

char *archive;

/* Descriptor on which we have locked the original archive file,
   or -1 if this has not been done.  */

int lock_indesc;

/* Pointer to tail of `argv', at first subfile name argument,
 or zero if no such were specified.  */

char **files;

/* Nonzero means write a __.SYMDEF member into the modified archive.  */

int symdef_flag;

/* Nonzero means __.SYMDEF member exists in old archive.  */

int symdef_exists;

/* Total number of symdef entries we will have. */

long nsymdefs;

/* Symdef data from old archive (set up only if we need it) */

struct symdef *old_symdefs;

/* Number of symdefs in remaining in old_symdefs.  */

int num_old_symdefs;

/* Number of symdefs old_symdefs had when it was read in.  */

long original_num_symdefs;

/* String table from old __.SYMDEF member.  */

char *old_strings;

/* Size of old_strings */

long old_strings_size;

/* String table to be written into __.SYMDEF member.  */

char *new_strings;

/* Size of new_strings */

long new_strings_size;

/* An archive map is a chain of these structures.
  Each structure describes one member of the archive.
  The chain is in the same order as the members are.  */

struct mapelt
  {
    struct member_desc info;
    struct mapelt *next;
  };

struct mapelt *maplast;

/* If nonzero, this is the map-element for the __.SYMDEF member
   and we should update the time of that member just before finishing.  */

struct mapelt *symdef_mapelt;

/* Header that we wrote for the __.SYMDEF member.  */

struct ar_hdr symdef_header;

void add_to_map ();
void print_descr ();
char *concat ();
void scan ();
char *requestedp ();
void extract_member ();
void print_contents ();
void write_symdef_member ();
void read_old_symdefs ();

main (argc, argv)
     int argc;
     char **argv;
{
  int i;

  operation = 0;
  verbose = 0;
  newer_only = 0;
  silent_create = 0;
  posname = 0;
  postype = POS_DEFAULT;
  preserve_dates = 0;
  symdef_flag = 0;
  symdef_exists = 0;
  symdef_mapelt = 0;
  files = 0;
  lock_indesc = -1;

  if (argc < 2)
    fatal ("too few command arguments", 0);
  {
    char *key = argv[1];
    char *p = key;
    char c;

    while (c = *p++)
      {
	switch (c)
	  {
	  case 'a':
	    postype = POS_AFTER;
	    break;

	  case 'b':
	    postype = POS_BEFORE;
	    break;

	  case 'c':
	    silent_create = 1;
	    break;

	  case 'd':
	    if (operation)
	      two_operations ();

	    operation = DELETE;
	    break;

	  case 'i':
	    postype = POS_BEFORE;
	    break;

	  case 'l':
	    break;

	  case 'm':
	    if (operation) two_operations ();
	    operation = MOVE;
	    break;

	  case 'o':
	    preserve_dates = 1;
	    break;

	  case 'p':
	    if (operation) two_operations ();
	    operation = PRINT_FILES;
	    break;

	  case 'q':
	    if (operation) two_operations ();
	    operation = QUICK_APPEND;
	    break;

	  case 'r':
	    if (operation) two_operations ();;
	    operation = REPLACE;
	    break;

	  case 's':
	    symdef_flag = 1;
	    break;

	  case 't':
	    if (operation) two_operations ();
	    operation = PRINT_TABLE;
	    break;

	  case 'u':
	    newer_only = 1;
	    break;

	  case 'v':
	    verbose = 1;
	    break;

	  case 'x':
	    if (operation) two_operations ();
	    operation = EXTRACT;
	    break;
	  }
      }
  
  }

  if (!operation && symdef_flag)
    operation = REPLACE;

  if (!operation)
    fatal ("no operation specified", 0);

  i = 2;

  if (postype != POS_DEFAULT)
    posname = argv[i++];

  archive = argv[i++];

  if (i < argc)
    files = &argv[i];

  switch (operation)
    {
    case EXTRACT:
	extract_members (extract_member);
	break;

    case PRINT_TABLE:
	extract_members (print_descr);
	break;

    case PRINT_FILES:
	extract_members (print_contents);
	break;

    case DELETE:
	if (!files) break;
	delete_members ();
	break;

    case MOVE:
	if (!files) break;
	move_members ();
	break;

    case REPLACE:
	if (!files && !symdef_flag) break;
	replace_members ();
	break;

    case QUICK_APPEND:
	if (!files) break;
	quick_append ();
	break;

    default:
	fatal ("operation not implemented yet", 0);
    }
}

two_operations ()
{
  fatal ("two different operation switches specified", 0);
}

void
scan (function, crflag)
     void (*function) ();
     int crflag;
{
  int desc = open (archive, 0, 0);

  if (desc < 0 && crflag)
    /* Creation-warning, if desired, will happen later.  */
    return;

  if (desc < 0)
    {
      perror_with_name (archive);
      exit (1);
    }
  {
    char buf[SARMAG];
    int nread = read (desc, buf, SARMAG);
    if (nread != SARMAG || bcmp (buf, ARMAG, SARMAG))
      fatal ("file %s not a valid archive", archive);
  }

  /* Now find the members one by one.  */
  {
    int member_offset = SARMAG;
    while (1)
      {
	int nread;
	struct ar_hdr member_header;
	struct member_desc member_desc;
	char name [1 + sizeof member_header.ar_name];

	if (lseek (desc, member_offset, 0) < 0)
	  perror_with_name (archive);

	nread = read (desc, &member_header, sizeof (struct ar_hdr));
	if (!nread) break;	/* No data left means end of file; that is ok */

	if (nread != sizeof (member_header)
	    || bcmp (member_header.ar_fmag, ARFMAG, 2))
	  fatal ("file %s not a valid archive", archive);
	bcopy (member_header.ar_name, name, sizeof member_header.ar_name);
	{
	  char *p = name + sizeof member_header.ar_name;
	  while (p > name && *--p == ' ') *p = 0;
	}
	member_desc.name = name;
	member_desc.date = atoi (member_header.ar_date);
	member_desc.size = atoi (member_header.ar_size);
	sscanf (member_header.ar_mode, "%o", &member_desc.mode);
	member_desc.uid = atoi (member_header.ar_uid);
	member_desc.gid = atoi (member_header.ar_gid);
	member_desc.offset = member_offset;
	member_desc.data_offset = member_offset + sizeof (member_header);

	if (!strcmp (name, "__.SYMDEF"))
	  symdef_exists = 1;

	function (member_desc, desc);

	member_offset += sizeof (member_header) + member_desc.size;
	if (member_offset & 1) member_offset++;
      }
  }

  close (desc);
}

/* If the string `name' matches one of the member-name arguments
   from the command line, return nonzero.  Otherwise return 0.
   Only the first `len' characters of `name' need to match.  */
/* In fact, the value is the command argument that `name' matched.  */

char *
requestedp (name, len)
     char *name;
     int len;
{
  char **fp = files;
  char *fn;

  if (fp == 0)
    return 0;

  while (fn = *fp++)
    if (!strncmp (name, fn, len))
      return fn;

  return 0;
}

void
print_descr (member)
     struct member_desc member;
{
  char *timestring;
  if (!verbose)
    {
      printf ("%s\n", member.name);
      return;
    }
  print_modes (member.mode);
  timestring = ctime (&member.date);
  printf (" %2d/%2d %6d %12.12s %4.4s %s\n",
	  member.uid, member.gid,
	  member.size, timestring + 4, timestring + 20,
	  member.name);
}

print_modes (modes)
     int modes;
{
  putchar (modes & 0400 ? 'r' : '-');
  putchar (modes & 0200 ? 'w' : '-');
  putchar (modes & 0100 ? 'x' : '-');
  putchar (modes & 040 ? 'r' : '-');
  putchar (modes & 020 ? 'w' : '-');
  putchar (modes & 010 ? 'x' : '-');
  putchar (modes & 04 ? 'r' : '-');
  putchar (modes & 02 ? 'w' : '-');
  putchar (modes & 01 ? 'x' : '-');
}

#define BUFSIZE 1024

void
extract_member (member, arcdesc)
     struct member_desc member;
     int arcdesc;
{
  int ncopied = 0;
  FILE *istream, *ostream;

  lseek (arcdesc, member.data_offset, 0);
  istream = fdopen (arcdesc, "r");
  ostream = fopen (member.name, "w");
  if (!ostream)
    {
      perror_with_name (member.name);
      return;
    }

  if (verbose)
    printf ("extracting %s\n", member.name);

  while (ncopied < member.size)
    {
      char buf [BUFSIZE];
      int tocopy = member.size - ncopied;
      int nread;
      if (tocopy > BUFSIZE) tocopy = BUFSIZE;
      nread = fread (buf, 1, tocopy, istream);
      if (nread != tocopy)
	fatal ("file %s not a valid archive", archive);
      fwrite (buf, 1, nread, ostream);
      ncopied += tocopy;
    }

#ifdef USG
  chmod (member.name, member.mode);
#else
  fchmod (fileno (ostream), member.mode);
#endif
  fclose (ostream);
  if (preserve_dates)
    {
#ifdef USG
      long tv[2];
      tv[0] = member.date;
      tv[1] = member.date;
      utime (member.name, tv);
#else
      struct timeval tv[2];
      tv[0].tv_sec = member.date;
      tv[0].tv_usec = 0;
      tv[1].tv_sec = member.date;
      tv[1].tv_usec = 0;
      utimes (member.name, tv);
#endif
    }
}

void
print_contents (member, arcdesc)
     struct member_desc member;
     int arcdesc;
{
  int ncopied = 0;
  FILE *istream;

  lseek (arcdesc, member.data_offset, 0);
  istream = fdopen (arcdesc, "r");

  if (verbose)
  printf ("\n<member %s>\n\n", member.name);

  while (ncopied < member.size)
    {
      char buf [BUFSIZE];
      int tocopy = member.size - ncopied;
      int nread;
      if (tocopy > BUFSIZE) tocopy = BUFSIZE;
      nread = fread (buf, 1, tocopy, istream);
      if (nread != tocopy)
	fatal ("file %s not a valid archive", archive);
      fwrite (buf, 1, nread, stdout);
      ncopied += tocopy;
    }
}

/* Make a map of the existing members of the archive: their names,
 positions and sizes.  */

/* If `nonexistent_ok' is nonzero,
 just return 0 for an archive that does not exist.
 This will cause the ordinary supersede procedure to
 create a new archive.  */

struct mapelt *
make_map (nonexistent_ok)
     int nonexistent_ok;
{
  struct mapelt mapstart;
  mapstart.next = 0;
  maplast = &mapstart;
  scan (add_to_map, nonexistent_ok);
  return mapstart.next;
}

void
add_to_map (member)
     struct member_desc member;
{
  struct mapelt *mapelt = (struct mapelt *) xmalloc (sizeof (struct mapelt));
  mapelt->info = member;
  mapelt->info.name = concat (mapelt->info.name, "", "");
  maplast->next = mapelt;
  mapelt->next = 0;
  maplast = mapelt;
}

/* Return the last element of the specified map.  */

struct mapelt *
last_mapelt (map)
     struct mapelt *map;
{
  struct mapelt *tail = map;
  while (tail->next) tail = tail->next;
  return tail;
}

/* Return the element of the specified map which precedes elt.  */

struct mapelt *
prev_mapelt (map, elt)
     struct mapelt *map, *elt;
{
  struct mapelt *tail = map;
  while (tail->next && tail->next != elt)
    tail = tail->next;
  if (tail->next) return tail;
  return 0;
}

/* Return the element of the specified map which has the specified name.  */

struct mapelt *
find_mapelt (map, name)
     struct mapelt *map;
     char *name;
{
  struct mapelt *tail = map;
  for (;tail; tail = tail->next)
    if (tail->info.name && !strcmp (tail->info.name, name))
      return tail;
  error ("no member named %s", name);
  return 0;
}

struct mapelt *
find_mapelt_noerror (map, name)
     struct mapelt *map;
     char *name;
{
  struct mapelt *tail = map;
  for (;tail; tail = tail->next)
    if (tail->info.name && !strcmp (tail->info.name, name))
      return tail;
  return 0;
}

/* Before looking at the archive, if we are going to update it
   based on looking at its current contents, make an exclusive lock on it.
   The lock is released when `write_archive' is called.  */

void
lock_for_update ()
{
  /* Open the existing archive file; if that fails, create an empty one.  */

  lock_indesc = open (archive, O_RDWR, 0);

  if (lock_indesc < 0)
    {
      int outdesc;

      if (!silent_create)
	printf ("Creating archive file `%s'\n", archive);
      outdesc = open (archive, O_WRONLY | O_APPEND | O_CREAT, 0666);
      if (outdesc < 0)
	pfatal_with_name (archive);
      write (outdesc, ARMAG, SARMAG);
      close (outdesc);

      /* Now we had better be able to open for update!  */

      lock_indesc = open (archive, O_RDWR, 0);
      if (lock_indesc < 0)
	pfatal_with_name (archive);
    }

#ifdef LOCKS
  /* Lock the old file so that it won't be updated by two programs at once.
     This uses the fcntl locking facility found on Sun systems
     which is also in POSIX.  (Perhaps it comes from sysV.)

     Note that merely reading an archive does not require a lock,
     because we use `rename' to update the whole file atomically.  */

  {
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    while (1)
      {
	int value = fcntl (lock_indesc, F_SETLKW, &lock);
	if (value >= 0)
	  break;
	else if (errno == EINTR)
	  continue;
	else
	  pfatal_with_name ("locking archive");
      }
  }
#endif
}

/* Write a new archive file from a given map.  */
/* When a map is used as the pattern for a new archive,
 each element represents one member to put in it, and
 the order of elements controls the order of writing.

 Ordinarily, the element describes a member of the old
 archive, to be copied into the new one.

 If the `offset' field of the element's info is 0,
 then the element describes a file to be copied into the
 new archive.  The `name' field is the file's name.

 If the `name' field of an element is 0, the element is ignored.
 This makes it easy to specify deletion of archive members.

 Every operation that will eventually call `write_archive'
 should call `lock_for_update' before beginning
 to do any I/O on the archive file.
*/

char *make_tempname ();
void copy_out_member ();

write_archive (map, appendflag)
     struct mapelt *map;
     int appendflag;
{
  char *tempname = make_tempname (archive);
  int indesc = lock_indesc;
  int outdesc;
  struct mapelt *tail;

  /* Now open the output.  */

  if (!appendflag)
    {
      /* Updating an existing archive normally.
	 Write output as TEMPNAME and rename at the end.
	 There can never be two invocations trying to do this at once,
	 because of the lock made on the old archive file.  */

      outdesc = open (tempname, O_WRONLY | O_CREAT, 0666);
      if (outdesc < 0)
	pfatal_with_name (tempname);
      write (outdesc, ARMAG, SARMAG);
    }
  else
    {
      /* Fast-append to existing archive.  */

      outdesc = open (archive, O_WRONLY | O_APPEND, 0);
    }

  /* If archive has or should have a __.SYMDEF member,
     compute the contents for it.  */

  if (symdef_flag || symdef_exists)
    {
      if (symdef_exists)
	{
#if 0
	  /* This is turned off because there seems to be a bug
	     in deleting the symdefs for members that are deleted.
	     The easiest way to fix it
	     is to regenerate the symdefs from scratch each time,
	     which happens if this is not done.  */
	  read_old_symdefs (map, indesc);
#endif
	}
      else
	{
	  struct mapelt *this = (struct mapelt *) xmalloc (sizeof (struct mapelt));

	  this->info.name = "__.SYMDEF";
	  this->info.offset = 0;
	  this->info.data_offset = 0;
	  this->info.date = 0;
	  this->info.size = 0;
	  this->info.uid = 0;
	  this->info.gid = 0;
	  this->info.mode = 0666;
	  this->next = map;
	  map = this;
	}

      original_num_symdefs = 0;
      old_strings_size = 0;

      update_symdefs (map, indesc);
    }

  /* Copy the members into the output, either from the old archive
     or from specified files.  */

  for (tail = map; tail; tail = tail->next)
    {
      if (!strcmp (tail->info.name, "__.SYMDEF") && tail->info.date==0)
	write_symdef_member (tail, map, outdesc);
      else
	copy_out_member (tail, indesc, outdesc);
    }

  /* Mark the __.SYMDEF member as up to date.  */

  if (symdef_mapelt)
    touch_symdef_member (outdesc);

  /* Install the new output under the intended name.  */

#ifndef USG
  fsync (outdesc);
#endif
  close (outdesc);

  if (!appendflag)
    if (rename (tempname, archive))
      pfatal_with_name (tempname);

#ifdef LOCKS
  {
    struct flock lock;

    /* Unlock the old archive.  */

    lock.l_type = F_UNLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    fcntl (lock_indesc, F_SETLK, &lock);
  }
#endif

  /* Close the archive.  If we renamed a new one, the old one disappears.  */
  close (lock_indesc);
}

void
copy_out_member (mapelt, archive_indesc, outdesc)
     struct mapelt *mapelt;
     int archive_indesc;
     int outdesc;
{
  struct ar_hdr header;
  int indesc;

  if (!mapelt->info.name)
    return;	/* This element was cancelled.  */

  /* Zero the header, then store in the data as text.  */
  bzero (&header, sizeof (header));

  strncpy (header.ar_name, mapelt->info.name, sizeof (header.ar_name));

  sprintf (header.ar_date, "%d", mapelt->info.date);
  sprintf (header.ar_size, "%d", mapelt->info.size);
  sprintf (header.ar_uid, "%d", mapelt->info.uid);
  sprintf (header.ar_gid, "%d", mapelt->info.gid);
  sprintf (header.ar_mode, "%o", mapelt->info.mode);
  strncpy (header.ar_fmag, ARFMAG, sizeof (header.ar_fmag));

  /* Change all remaining nulls in the header into spaces.  */

  {
    char *tem = (char *) &header;
    char *end = (char *) &header + sizeof (header);
    while (tem < end)
      {
	if (*tem == 0)
	  *tem = ' ';
	tem++;
      }
  }

  if (mapelt->info.data_offset)
    {
      indesc = archive_indesc;
      lseek (indesc, mapelt->info.data_offset, 0);
    }
  else
    {
      indesc = open (mapelt->info.name, 0, 0);
      if (indesc < 0)
	{
	  perror_with_name (mapelt->info.name);
	  return;
	}
    }

  write (outdesc, &header, sizeof (header));

  {
    char buf[BUFSIZE];
    int tocopy = mapelt->info.size;
    while (tocopy > 0)
      {
	int thistime = tocopy;
	if (thistime > BUFSIZE) thistime = BUFSIZE;
        read (indesc, buf, thistime);
	write (outdesc, buf, thistime);
	tocopy -= thistime;
      }
  }

  if (indesc != archive_indesc)
    close (indesc);

  if (mapelt->info.size & 1)
    write (outdesc, "\n", 1);

  if (verbose)
    printf ("member %s copied to new archive\n", mapelt->info.name);
}

/* Update the time of the __.SYMDEF member; done when we updated
   that member, just before we close the new archive file.
   It is open on OUTDESC.  */

touch_symdef_member (outdesc)
     int outdesc;
{
  struct stat statbuf;
  int i;

  /* See what mtime the archive file has as a result of our writing it.  */
  fstat (outdesc, &statbuf);

  /* Advance member's time to that time */
  bzero (symdef_header.ar_date, sizeof symdef_header.ar_date);
  sprintf (symdef_header.ar_date, "%d", statbuf.st_mtime);
  for (i = 0; i < sizeof symdef_header.ar_date; i++)
    if (symdef_header.ar_date[i] == 0)
      symdef_header.ar_date[i] = ' ';

  /* Write back this member's header with the new time.  */
  if (lseek (outdesc, symdef_mapelt->info.new_offset, 0) >= 0)
    write (outdesc, &symdef_header, sizeof symdef_header);
}

char *
make_tempname (name)
     char *name;
{
  return concat (name, "", "_supersede");
}

delete_members ()
{
  struct mapelt *map = make_map (0);
  struct mapelt *tail;
  struct mapelt mapstart;
  char **p;

  mapstart.info.name = 0;
  mapstart.next = map;
  map = &mapstart;

  lock_for_update ();

  if (files)
    for (p = files; *p; p++)
      {
	/* If user says to delete the __.SYMDEF member,
	   don't make a new one to replace it.  */
	if (!strcmp (*p, "__.SYMDEF"))
	  symdef_exists = 0;
	delete_from_map (*p, map);
      }

  write_archive (map->next, 0);
}

delete_from_map (name, map)
     char *name;
     struct mapelt *map;
{
  struct mapelt *this = find_mapelt (map, name);
  struct mapelt *prev;

  if (!this) return;
  prev = prev_mapelt (map, this);
  prev->next = this->next;
  if (verbose)
    printf ("deleting member %s\n", name);
}

move_members ()
{
  struct mapelt *map = make_map (0);
  char **p;
  struct mapelt *after_mapelt;
  struct mapelt mapstart;
  struct mapelt *change_map;

  mapstart.info.name = 0;
  mapstart.next = map;
  change_map = &mapstart;

  lock_for_update ();

  switch (postype)
    {
    case POS_DEFAULT:
      after_mapelt = last_mapelt (change_map);
      break;

    case POS_AFTER:
      after_mapelt = find_mapelt (map, posname);
      break;

    case POS_BEFORE:
      after_mapelt = prev_mapelt (change_map, find_mapelt (map, posname));
    }

  /* Failure to find specified "before" or "after" member
     is a fatal error; message has already been printed.  */

  if (!after_mapelt) exit (1);

  if (files)
    for (p = files; *p; p++)
      {
	if (move_in_map (*p, change_map, after_mapelt))
	  after_mapelt = after_mapelt->next;
      }

  write_archive (map, 0);
}

int
move_in_map (name, map, after)
     char *name;
     struct mapelt *map, *after;
{
  struct mapelt *this = find_mapelt (map, name);
  struct mapelt *prev;

  if (!this) return 0;
  prev = prev_mapelt (map, this);
  prev->next = this->next;
  this->next = after->next;
  after->next = this;
  if (verbose)
    printf ("moving member %s\n", name);
  return 1;
}

/* Insert files into the archive.  */

replace_members ()
{
  struct mapelt *map = make_map (1);
  struct mapelt mapstart;
  struct mapelt *after_mapelt;
  struct mapelt *change_map;
  char **p;

  mapstart.info.name = 0;
  mapstart.next = map;
  change_map = &mapstart;

  lock_for_update ();

  switch (postype)
    {
    case POS_DEFAULT:
      after_mapelt = last_mapelt (change_map);
      break;

    case POS_AFTER:
      after_mapelt = find_mapelt (map, posname);
      break;

    case POS_BEFORE:
      after_mapelt = prev_mapelt (change_map, find_mapelt (map, posname));
    }

  /* Failure to find specified "before" or "after" member
     is a fatal error; message has already been printed.  */
  if (!after_mapelt) exit (1);

  if (files)
    for (p = files; *p; p++)
      {
	if (insert_in_map (*p, change_map, after_mapelt))
	  after_mapelt = after_mapelt->next;
      }

  write_archive (change_map->next, 0);
}

/* Handle the "quick insert" operation.  */

quick_append ()
{
  struct mapelt *map;
  struct mapelt *after;
  struct mapelt mapstart;
  char **p;

  mapstart.info.name = 0;
  mapstart.next = 0;
  map = &mapstart;
  after = map;

  lock_for_update ();

  /* Insert the specified files into the "map",
     but is a map of the inserted files only,
     and starts out empty.  */
  if (files)
    for (p = files; *p; p++)
      {
	if (insert_in_map (*p, map, after))
	  after = after->next;
      }

  /* Append these files to the end of the existing archive file.  */

  write_archive (map->next, 1);
}

/* Insert an entry for name NAME into the map MAP after the map entry AFTER.
   Delete an old entry for NAME.
   MAP is assumed to start with a dummy entry, which facilitates
   insertion at the beginning of the list.
   Return 1 if successful, 0 if did nothing because file NAME doesn't
   exist or (optionally) is older.  */

insert_in_map (name, map, after)
     char *name;
     struct mapelt *map, *after;
{
  struct mapelt *old = find_mapelt_noerror (map, name);
  struct mapelt *this;
  struct stat status;

  if (stat (name, &status))
    {
      perror_with_name (name);
      return 0;
    }
  if (old && newer_only && status.st_mtime <= old->info.date)
    return 0;
  if (old && verbose)
    printf ("replacing old member %s\n", old->info.name);
  if (old) old->info.name = 0;	/* Delete the old one.  */
  this = (struct mapelt *) xmalloc (sizeof (struct mapelt));
  this->info.name = name;
  this->info.offset = 0;
  this->info.data_offset = 0;
  this->info.date = status.st_mtime;
  this->info.size = status.st_size;
  this->info.uid = status.st_uid;
  this->info.gid = status.st_gid;
  this->info.mode = status.st_mode;
  this->next = after->next;
  after->next = this;
  if (verbose)
    printf ("inserting file %s\n", name);
  return 1;
}

/* Apply a function to each of the specified members.
*/

extract_members (function)
     void (*function) ();
{
  struct mapelt *map;
  int arcdesc;
  char **p;

  if (!files)
    {
      /* Handle case where we want to operate on every member.
	 No need to make a map and search it for this.  */
      scan (function, 0);
      return;
    }

  arcdesc = open (archive, 0, 0);
  if (!arcdesc)
    fatal ("failure opening archive %s for the second time", archive);
  map = make_map (0);

  for (p = files; *p; p++)
    {
      struct mapelt *this = find_mapelt (map, *p);
      if (!this) continue;
      function (this->info, arcdesc);
    }

  close (arcdesc);
}

/* Write the __.SYMDEF member from data in core.  */

void
write_symdef_member (mapelt, map, outdesc)
     struct mapelt *mapelt;
     struct mapelt *map;
     int outdesc;
{
  struct ar_hdr header;
  int indesc;
  struct mapelt *mapptr;
  int symdefs_size;

  if (!mapelt->info.name)
    return;	/* This element was cancelled.  */

  /* Clear the header, then store in the data as text.  */

  bzero (&header, sizeof header);

  strncpy (header.ar_name, mapelt->info.name, sizeof (header.ar_name));

  sprintf (header.ar_date, "%d", mapelt->info.date);
  sprintf (header.ar_size, "%d", mapelt->info.size);
  sprintf (header.ar_uid, "%d", mapelt->info.uid);
  sprintf (header.ar_gid, "%d", mapelt->info.gid);
  sprintf (header.ar_mode, "%o", mapelt->info.mode);
  strncpy (header.ar_fmag, ARFMAG, sizeof (header.ar_fmag));

  /* Change all remaining nulls in the header into spaces.  */

  {
    char *tem = (char *) &header;
    char *end = (char *) &header + sizeof (header);
    while (tem < end)
      {
	if (*tem == 0)
	  *tem = ' ';
	tem++;
      }
  }

  bcopy (&header, &symdef_header, sizeof header);

  write (outdesc, &header, sizeof (header));

  /* Write long containing number of symdefs.  */

  symdefs_size = nsymdefs * sizeof (struct symdef);
  write (outdesc, &symdefs_size, sizeof symdefs_size);

  /* Write symdefs surviving from old archive.  */

  write (outdesc, old_symdefs, num_old_symdefs * sizeof (struct symdef));

  /* Write symdefs for new members.  */

  for (mapptr = map; mapptr; mapptr = mapptr->next)
    {
      if (mapptr->info.nsymdefs)
	{
	  write (outdesc, mapptr->info.symdefs,
		 mapptr->info.nsymdefs * sizeof (struct symdef));
	}
    }

  /* Write long containing string table size.  */

  write (outdesc, &new_strings_size, sizeof new_strings_size);

  /* Write string table  */

  write (outdesc, new_strings, new_strings_size);

  if (mapelt->info.size & 1)
    write (outdesc, "", 1);

  if (verbose)
    printf ("member %s copied to new archive\n", mapelt->info.name);
}

void
read_old_symdefs (map, archive_indesc)
     int archive_indesc;
{
  struct mapelt *mapelt;
  char *data;
  int val;
  int symdefs_size;

  mapelt = find_mapelt_noerror (map, "__.SYMDEF");
  if (!mapelt)
    abort ();			/* Only call here if an old one exists */

  data  = (char *) xmalloc (mapelt->info.size);
  lseek (archive_indesc, mapelt->info.data_offset, 0);
  val = read (archive_indesc, data, mapelt->info.size);

  symdefs_size = * (long *) data;
  original_num_symdefs = symdefs_size / sizeof (struct symdef);
  old_symdefs = (struct symdef *) (data + sizeof (long));
  old_strings = (char *) (old_symdefs + original_num_symdefs) + sizeof (long);
  old_strings_size = * (long *) (old_strings - sizeof (long));
}

/* Create the info.symdefs for a new member
   by reading the file it is coming from.
   This code was taken from the GNU nm.c.  */

void
make_new_symdefs (mapelt, archive_indesc)
     struct mapelt *mapelt;
     int archive_indesc;
{
  int indesc;
  int len;
  char *name = mapelt->info.name;
  struct exec header;   /* file header read in here */
  int string_size;
  struct nlist *symbols_and_strings;
  int symcount;
  int totalsize;
  char *strings;
  int i;
  int offset;

  if (mapelt->info.data_offset)
    {
      indesc = archive_indesc;
      lseek (indesc, mapelt->info.data_offset, 0);
      offset = mapelt->info.data_offset;
    }
  else
    {
      indesc = open (mapelt->info.name, 0, 0);
      if (indesc < 0)
	{
	  perror_with_name (mapelt->info.name);
	  return;
	}
      offset = 0;
    }
#ifdef HEADER_SEEK_FD
  HEADER_SEEK_FD (indesc);
#endif
  len = read (indesc, &header, sizeof header);
  if (len != sizeof header)
    error_with_file ("failure reading header of ", mapelt);
  else if (N_BADMAG(header))
    error_with_file ("bad format (not an object file) in ", mapelt);

  /* read the string-table-length out of the file */

  lseek (indesc, N_STROFF(header) + offset, 0);
  if (sizeof string_size != read (indesc, &string_size, sizeof string_size))
    {
      error_with_file ("bad string table in ", mapelt);
      if (mapelt->info.data_offset)
	close (indesc);	/* We just opened it.  Give up */
      return;
    }

  /* number of symbol entries in the file */
  symcount = header.a_syms / sizeof (struct nlist);

  totalsize = string_size + header.a_syms;
  /* allocate space for symbol entries and string table */
  symbols_and_strings = (struct nlist *) xmalloc (totalsize);
  strings = (char *) symbols_and_strings + header.a_syms;

  /* read them both in all at once */
  lseek (indesc, N_SYMOFF(header) + offset, 0);
  if (totalsize != read (indesc, symbols_and_strings, totalsize))
    {
      error_with_file ("premature end of file in symbols/strings of ");
      if (mapelt->info.data_offset)
	close (indesc);	/* Give up! */
      return;
    }

  if (indesc != archive_indesc)
    close (indesc);

  /* discard the symbols we don't want to mention; compact the rest down */

  symcount = filter_symbols (symbols_and_strings, symcount, strings);

  /* We have a vector of struct nlist; we want a vector of struct symdef.
     Convert it in place, reusing the space.
     This works since a struct nlist is longer than a struct symdef.

     Also make each symdef point directly at the symbol name string.  */

  mapelt->info.symdefs = (struct symdef *) symbols_and_strings;
  mapelt->info.nsymdefs = symcount;
  mapelt->info.string_size = 0;

  for (i = 0; i < symcount; i++)
    {
      mapelt->info.symdefs[i].s.name = strings + symbols_and_strings[i].n_un.n_strx;
      mapelt->info.string_size += strlen (mapelt->info.symdefs[i].s.name) + 1;
    }
}

/* Choose which symbol entries to mention in __.SYMDEF;
  compact them downward to get rid of the rest.
  Return the number of symbols left.  */

int
filter_symbols (syms, symcount, strings)
     struct nlist *syms;
     int symcount;
     char *strings;
{
  struct nlist *from = syms, *to = syms;
  struct nlist *end = syms + symcount;

  while (from < end)
    {
      if ((from->n_type & N_EXT)
	  && (from->n_type != N_EXT || from->n_value))
	*to++ = *from;
      from++;
    }

  return to - syms;
}


/* Update the __.SYMDEF data before writing a new archive.  */

update_symdefs (map, archive_indesc)
     struct mapelt *map;
     int archive_indesc;
{
  struct mapelt *tail;
  int pos;
  int i,j;
  int len;
  struct symdef *s;

  nsymdefs = original_num_symdefs;
  num_old_symdefs = original_num_symdefs;
  new_strings_size = old_strings_size;

  if (nsymdefs != 0)
    {
      /* We already had a __.SYMDEF member, so just update it.  */

      /* Mark as canceled any old symdefs for members being deleted.  */

      for (tail = map; tail; tail = tail->next)
	{
	  if (tail->info.name == 0)
	    /* Old member being deleted.  Delete its symdef entries too.  */
	    {
	      for (i = 0; i < nsymdefs; i++)
		if (old_symdefs[i].offset == tail->info.offset)
		  {
		    old_symdefs[i].offset = 0;
		    nsymdefs--;
		    num_old_symdefs--;
		    new_strings_size
		      -= strlen (old_strings + old_symdefs[i].s.stringoffset);
		  }
	    }
	}

      /* Now compactify old_symdefs */

      for (i = 0, j = 0; i < num_old_symdefs; i++)
	if (old_symdefs[i].offset)
	  old_symdefs[j++] = old_symdefs[i];

      /* Create symdef data for any new members.  */

      for (tail = map; tail; tail = tail->next)
	{
	  if (tail->info.offset || !strcmp (tail->info.name, "__.SYMDEF"))
	    continue;
	  make_new_symdefs (tail, archive_indesc);
	  nsymdefs += tail->info.nsymdefs;
	  new_strings_size += tail->info.string_size;
	}
    }
  else
    {
      /* Create symdef data for all existing members.  */

      for (tail = map; tail; tail = tail->next)
	{
	  if (!strcmp (tail->info.name, "__.SYMDEF"))
	    continue;
	  make_new_symdefs (tail, archive_indesc);
	  nsymdefs += tail->info.nsymdefs;
	  new_strings_size += tail->info.string_size;
	}
    }

  /* Now we know the size of __.SYMDEF,
     so assign the positions of all the members. */

  tail = find_mapelt_noerror (map, "__.SYMDEF");
  tail->info.size = nsymdefs * sizeof (struct symdef) + sizeof (long) + sizeof (long) + new_strings_size;
  symdef_mapelt = tail;

  pos = SARMAG;
  for (tail = map; tail; tail = tail->next)
    {
      if (!tail->info.name) continue;	/* Ignore deleted members */
      tail->info.new_offset = pos;
      pos += tail->info.size + sizeof (struct ar_hdr);
      if (tail->info.size & 1) pos++;
    }

  /* Now update the offsets in the symdef data
     to be the new offsets rather than the old ones.  */

  for (tail = map; tail; tail = tail->next)
    {
      if (!tail->info.name) continue;
      if (!tail->info.symdefs)
	/* Member without new symdef data.
	   Check the old symdef data; it may be included there. */
	for (i = 0; i < num_old_symdefs; i++)
	  {
	    if (old_symdefs[i].offset == tail->info.offset)
	      old_symdefs[i].offset = tail->info.new_offset;
	  }
      else
	for (i = 0; i < tail->info.nsymdefs; i++)
	  tail->info.symdefs[i].offset = tail->info.new_offset;
    }

  /* Generate new, combined string table
     and put each string's offset into the symdef that refers to it.
     Note that old symdefs ref their strings by offsets into old_strings
     but new symdefs contain addresses of strings.  */

  new_strings = (char *) xmalloc (new_strings_size);
  pos = 0;

  for (i = 0; i < num_old_symdefs; i++)
    {
      if (old_symdefs[i].offset == 0) continue;
      strcpy (new_strings + pos, old_strings + old_symdefs[i].s.stringoffset);
      old_symdefs[i].s.stringoffset = pos;
      pos += strlen (new_strings + pos) + 1;
    }
  for (tail = map; tail; tail = tail->next)
    {
      len = tail->info.nsymdefs;
      s = tail->info.symdefs;

      for (i = 0; i < len; i++)
	{
	  strcpy (new_strings + pos, s[i].s.name);
	  s[i].s.stringoffset = pos;
	  pos += strlen (new_strings + pos) + 1;
	}
    }
  if (pos != new_strings_size)
    fatal ("inconsistency in new_strings_size", 0);
}

/* Print error message and exit.  */

fatal (s1, s2)
     char *s1, *s2;
{
  error (s1, s2);
  exit (1);
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

error (s1, s2)
     char *s1, *s2;
{
  fprintf (stderr, "ar: ");
  fprintf (stderr, s1, s2);
  fprintf (stderr, "\n");
}

error_with_file (string, mapelt)
     char *string;
     struct mapelt *mapelt;
{
  fprintf (stderr, "ar: ");
  fprintf (stderr, string);
  if (mapelt->info.offset)
    fprintf (stderr, "%s(%s)", archive, mapelt->info.name);
  else
    fprintf (stderr, "%s", mapelt->info.name);
  fprintf (stderr, "\n");
}

perror_with_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";
  error (s, name);
}

pfatal_with_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";
  fatal (s, name);
}

/* Return a newly-allocated string whose contents concatenate those of s1, s2, s3.  */

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

/* Like malloc but get fatal error if memory is exhausted.  */

int
xmalloc (size)
     int size;
{
  int result = malloc (size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}


int
xrealloc (ptr, size)
     char *ptr;
     int size;
{
  int result = realloc (ptr, size);
  if (!result)
    fatal ("virtual memory exhausted");
  return result;
}

#ifdef USG

bcmp (a, b, n)
     char *a, *b;
{
  return (memcmp (a, b, n));
}

bcopy (from, to, n)
     char *from, *to;
{
  memcpy (to, from, n);
}

bzero (p, n)
     char *p;
{
  memset (p, 0, n);
}

rename (from, to)
     char *from, *to;
{
  if (unlink (to) < 0
      || link (from, to) < 0
      || unlink (from) < 0)
    return -1;
  else
    return 0;
}

#endif

/* Unexec for MIPS (including IRIS4D).
   Note that the GNU project considers support for MIPS operation
   a peripheral activity which should not be allowed to divert effort
   from development of the GNU system.  Changes in this code will be
   installed when users send them in, but aside from that
   we don't plan to think about it, or about whether other Emacs
   maintenance might break it.

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
as you receive it, in any medium, provided that you conspicuously and
appropriately publish on each copy a valid copyright notice "Copyright
(C) 1987 Free Software Foundation, Inc."; and include following the
copyright notice a verbatim copy of the above disclaimer of warranty
and of this License.  You may charge a distribution fee for the
physical act of transferring a copy.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be licensed at no charge to all
    third parties on terms identical to those contained in this
    License Agreement (except that you may choose to grant more extensive
    warranty protection to some or all third parties, at your option).

    c) You may charge a distribution fee for the physical act of
    transferring a copy, and you may at your option offer warranty
    protection in exchange for a fee.

Mere aggregation of another unrelated program with this program (or its
derivative) on a volume of a storage or distribution medium does not bring
the other program under the scope of these terms.

  3. You may copy and distribute this program (or a portion or derivative
of it, under Paragraph 2) in object code or executable form under the terms
of Paragraphs 1 and 2 above provided that you also do one of the following:

    a) accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    b) accompany it with a written offer, valid for at least three
    years, to give any third party free (except for a nominal
    shipping charge) a complete machine-readable copy of the
    corresponding source code, to be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    c) accompany it with the information you received as to where the
    corresponding source code may be obtained.  (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form alone.)

For an executable file, complete source code means all the source code for
all modules it contains; but, as a special exception, it need not include
source code for modules which are standard libraries that accompany the
operating system on which the executable file runs.

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


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
THIS IS A MODIFIED VERSION.

Modified by: Jan Wielemaker (jan@swi.psy.uva.nl)
Reason:	     Changed error message interface to confirm with SWI-Prolog
Changes:     Various, but only messages
Date:	     10/10/90
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef EMACS
#include "config.h"
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#ifndef va_dcl
#include <varargs.h>
#endif
#include <filehdr.h>
#include <aouthdr.h>
#include <scnhdr.h>
#include <sym.h>

#ifdef IRIS_4D
#include "getpagesize.h"
#include <fcntl.h>
#endif

static void fatal_unexec ();

#define READ(_fd, _buffer, _size, _error_message, _error_arg) \
	errno = EEOF; \
	if (read(_fd, _buffer, _size) != _size) \
	  fatal_unexec(_error_message, _error_arg);

#define WRITE(_fd, _buffer, _size, _error_message, _error_arg) \
	if (write(_fd, _buffer, _size) != _size) \
	  fatal_unexec(_error_message, _error_arg);

#define SEEK(_fd, _position, _error_message, _error_arg) \
	errno = EEOF; \
	if (lseek(_fd, _position, L_SET) != _position) \
	  fatal_unexec(_error_message, _error_arg);

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];
#define EEOF -1

static struct scnhdr *text_section;
static struct scnhdr *init_section;
static struct scnhdr *finit_section;
static struct scnhdr *rdata_section;
static struct scnhdr *data_section;
static struct scnhdr *lit8_section;
static struct scnhdr *lit4_section;
static struct scnhdr *sdata_section;
static struct scnhdr *sbss_section;
static struct scnhdr *bss_section;

struct headers {
    struct filehdr fhdr;
    struct aouthdr aout;
    struct scnhdr section[10];
};

/* Define name of label for entry point for the dumped executable.  */

#ifndef DEFAULT_ENTRY_ADDRESS
#define DEFAULT_ENTRY_ADDRESS __start
#endif

unexec (new_name, a_name, data_start, bss_start, entry_address)
     char *new_name, *a_name;
     unsigned data_start, bss_start, entry_address;
{
  int new, old;
  int pagesize, brk;
  int newsyms, symrel;
  int nread;
  struct headers hdr;
  int i;
  int vaddr, scnptr;
#define BUFSIZE 8192
  char buffer[BUFSIZE];

  old = open (a_name, O_RDONLY, 0);
  if (old < 0) fatal_unexec ("opening %s", a_name);

  new = creat (new_name, 0666);
  if (new < 0) fatal_unexec ("creating %s", new_name);

  hdr = *((struct headers *)TEXT_START);
  if (hdr.fhdr.f_magic != MIPSELMAGIC
      && hdr.fhdr.f_magic != MIPSEBMAGIC)
    {
      fatalError("input file magic number is %x, not %x or %x.\n",
		 hdr.fhdr.f_magic, MIPSELMAGIC, MIPSEBMAGIC);
    }
  if (hdr.fhdr.f_opthdr != sizeof(hdr.aout))
    {
      fatalError("unexec: input a.out header is %d bytes, not %d.\n",
		 hdr.fhdr.f_opthdr, sizeof(hdr.aout));
    }
  if (hdr.aout.magic != ZMAGIC)
    {
      fatalError("unexec: input file a.out magic number is %o, not %o.\n",
		 hdr.aout.magic, ZMAGIC);
      exit(1);
    }

#define CHECK_SCNHDR(ptr, name, flags) \
  if (strcmp(hdr.section[i].s_name, name) == 0) { \
    if (hdr.section[i].s_flags != flags) { \
      warning("%x flags where %x expected in %s section.\n", \
	      hdr.section[i].s_flags, flags, name); \
    } \
    ptr = hdr.section + i; \
    i += 1; \
  } \
  else { \
    ptr = NULL; \
    }

  i = 0;
  CHECK_SCNHDR(text_section,  _TEXT,  STYP_TEXT);
  CHECK_SCNHDR(init_section,  _INIT,  STYP_INIT);
  CHECK_SCNHDR(rdata_section, _RDATA, STYP_RDATA);
  CHECK_SCNHDR(data_section,  _DATA,  STYP_DATA);
#ifdef _LIT8
  CHECK_SCNHDR(lit8_section,  _LIT8,  STYP_LIT8);
  CHECK_SCNHDR(lit4_section,  _LIT4,  STYP_LIT4);
#endif /* _LIT8 */
  CHECK_SCNHDR(sdata_section, _SDATA, STYP_SDATA);
  CHECK_SCNHDR(sbss_section,  _SBSS,  STYP_SBSS);
  CHECK_SCNHDR(bss_section,   _BSS,   STYP_BSS);
  if (i != hdr.fhdr.f_nscns)
    warning("%d sections found instead of %d.\n",
	    i, hdr.fhdr.f_nscns);

  pagesize = getpagesize();
  brk = (sbrk(0) + pagesize - 1) & (-pagesize);
  hdr.aout.dsize = brk - DATA_START;
  hdr.aout.bsize = 0;
  if (entry_address == 0)
    {
      extern DEFAULT_ENTRY_ADDRESS();
      hdr.aout.entry = (unsigned)DEFAULT_ENTRY_ADDRESS;
    }
  else
    hdr.aout.entry = entry_address;

  hdr.aout.bss_start = hdr.aout.data_start + hdr.aout.dsize;
  rdata_section->s_size = data_start - DATA_START;
  data_section->s_vaddr = data_start;
  data_section->s_paddr = data_start;
  data_section->s_size = brk - DATA_START;
  data_section->s_scnptr = rdata_section->s_scnptr + rdata_section->s_size;
  vaddr = data_section->s_vaddr + data_section->s_size;
  scnptr = data_section->s_scnptr + data_section->s_size;
  if (lit8_section != NULL)
    {
      lit8_section->s_vaddr = vaddr;
      lit8_section->s_paddr = vaddr;
      lit8_section->s_size = 0;
      lit8_section->s_scnptr = scnptr;
    }
  if (lit4_section != NULL)
    {
      lit4_section->s_vaddr = vaddr;
      lit4_section->s_paddr = vaddr;
      lit4_section->s_size = 0;
      lit4_section->s_scnptr = scnptr;
    }
  if (sdata_section != NULL)
    {
      sdata_section->s_vaddr = vaddr;
      sdata_section->s_paddr = vaddr;
      sdata_section->s_size = 0;
      sdata_section->s_scnptr = scnptr;
    }
  if (sbss_section != NULL)
    {
      sbss_section->s_vaddr = vaddr;
      sbss_section->s_paddr = vaddr;
      sbss_section->s_size = 0;
      sbss_section->s_scnptr = scnptr;
    }
  if (bss_section != NULL)
    {
      bss_section->s_vaddr = vaddr;
      bss_section->s_paddr = vaddr;
      bss_section->s_size = 0;
      bss_section->s_scnptr = scnptr;
    }

  Putf("text: %ld ... ", hdr.aout.tsize); pl_flush();
  WRITE(new, TEXT_START, hdr.aout.tsize,
	"writing text section to %s", new_name);
  Putf("data: %ld ... ", hdr.aout.dsize); pl_flush();
  WRITE(new, DATA_START, hdr.aout.dsize,
	"writing text section to %s", new_name);

  SEEK(old, hdr.fhdr.f_symptr, "seeking to start of symbols in %s", a_name);
  errno = EEOF;
  nread = read(old, buffer, BUFSIZE);
  if (nread < sizeof(HDRR)) fatal_unexec("reading symbols from %s", a_name);
#define symhdr ((pHDRR)buffer)
  newsyms = hdr.aout.tsize + hdr.aout.dsize;
  symrel = newsyms - hdr.fhdr.f_symptr;
  hdr.fhdr.f_symptr = newsyms;
  symhdr->cbLineOffset += symrel;
  symhdr->cbDnOffset += symrel;
  symhdr->cbPdOffset += symrel;
  symhdr->cbSymOffset += symrel;
  symhdr->cbOptOffset += symrel;
  symhdr->cbAuxOffset += symrel;
  symhdr->cbSsOffset += symrel;
  symhdr->cbSsExtOffset += symrel;
  symhdr->cbFdOffset += symrel;
  symhdr->cbRfdOffset += symrel;
  symhdr->cbExtOffset += symrel;
#undef symhdr
  Putf("symbols ... "); pl_flush();
  do
    {
      if (write(new, buffer, nread) != nread)
	fatal_unexec("writing symbols to %s", new_name);
      nread = read(old, buffer, BUFSIZE);
      if (nread < 0) fatal_unexec("reading symbols from %s", a_name);
#undef BUFSIZE
    } while (nread != 0);

  SEEK(new, 0, "seeking to start of header in %s", new_name);
  WRITE(new, &hdr, sizeof(hdr),
	"writing header of %s", new_name);

  Putf("ok\n");
  close(old);
  close(new);
  mark_x(new_name);
}

/*
 * mark_x
 *
 * After succesfully building the new a.out, mark it executable
 */

static
mark_x (name)
     char *name;
{
  struct stat sbuf;
  int um = umask (777);
  umask (um);
  if (stat (name, &sbuf) < 0)
    fatal_unexec ("getting protection on %s", name);
  sbuf.st_mode |= 0111 & ~um;
  if (chmod (name, sbuf.st_mode) < 0)
    fatal_unexec ("setting protection on %s", name);
}

static void
fatal_unexec (s, va_alist)
     va_dcl
{ char msg[1000];
  va_list ap;

  va_start(ap);
  vsprintf(msg, s, ap);
  va_end(ap);

  fatalError("save_program/2: failed to create a saved state: %s (%s)",
	     OsError(), msg);
}

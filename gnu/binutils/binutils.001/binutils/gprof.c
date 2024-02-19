/* `gprof', analyze gmon.out and print a profile.
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

/* GNU gprof was written mainly by Jay Fenlason.  */

#ifdef __STDC__
#include "stddef.h"
#include "stdarg.h"
#else
#ifdef sparc
#include "alloca.h"
#endif
#include "varargs.h"
#endif

#undef NULL
#include <stdio.h>
#include <a.out.h>
#include <assert.h>
#include "gmon.h"
/* #include <nlist.h> */

/* Names or default names for various files.  */
#define OBJ_NAM "a.out"
#define MON_NAM "gmon.out"
#define SUM_NAM "gmon.sum"

/* Debugging stuff */

#define DEBUG

/* A mask of flags defined below.  */
long debug=0;

/* Print obnoxious msgs about the a.out file, and the amusing values therein */
#define DB_AFILE	(1<<0)	/* a.out file */

/* Describe in detail the rending and tearing of the gmon.out file */
#define DB_GFILE	(1<<1)	/* gmon.out file */

/* Describe in detail the writing out of the gmon.sum file */
#define DB_SUM		(1<<2)	/* gmon.sum file */

/* Print neeto messages as we deal with -e -E -f and -F options */
#define DB_OPT		(1<<3)	/* Options */

/* Print msgs whenever the ring buffer is used */
/* This probably doesn't work since the ring buffer code was
   moved into the utilities file */
#define DB_RB		(1<<4)	/* Ring buffer */

/* Print msgs about cycles */
#define DB_CYC		(1<<5)

/* Print msgs about assigning the histogram entries to functions */
#define DB_HISTO		(1<<6)

/* Print obnoxious msgs here and there as we do our stuff */
#define DB_MISC		(1<<31)	/* misc stuff */

#ifdef DEBUG
#define PRINT_OBNOXIOUS_DEBUG_MESSAGE(x,msg) if(debug&x) dbgprintf msg
#else
#define PRINT_OBNOXIOUS_DEBUG_MESSAGE(x,msg)
#endif

/* LessThan EQual GreaterThan */
/* These get returned by the various comparison functions */
#define LT (-1)
#define EQ (0)
#define GT (1)

/* Note since *ALL* non-zero values are true, do *NOT* say
      if(x==TRUE)
   These values are here only for saying
      return TRUE;
   and things like that.   */

#define TRUE (1)
#define FALSE (0)

/* The floating point datatype to use for storing
   propagated info.  Float should be big enough */
#define FLOAT	double

/* Used when creating a gmon.sum file */
/* #define FUDGE_FACTOR (sizeof(CHUNK)) */
#define FUDGE_FACTOR	0


/* Description of one symbol in gprof's internal symbol table.
   There is one of these for each function and one for each cycle.

   NAME is the name of the function or cycle.
   VALUE is the address of the start of the function.

   CALLS and CALLED are chains of edges for calls into and out of
   this function.  These constitute the call graph.

   NCALLS is the total number
   of times this function called other functions,
   NCALLED is the total number of times this function was called.
   Note that if NCALLED is zero, but NCALLS is nozero, this function
   must have been started magically.  It happens.  (Signals, etc.)

   RECURSIVE is the total number of times this function was called
   recursively.

   HISTO is the total number of histogram counts for this function.

   SUB_HISTO is the total number of histogram counts for this function
   plus time propagated from the children.

   CYCNUM is the number of the cycle this function is in,
   or the number of this cycle is this entry is for a cycle.
   -1 means the function isn't in any cycles.  0 means the
   cycle-ness of the function hasn't been determined yet.

   NUMINDEX is the index number assigned to this function
   for the call graph.

   If FLAG is non-zero, this function is in the process of being
   saved from the oblivion caused by the -f or -F options.  */

struct mesym {
	char *name;
	unsigned long value;

	struct symlist *calls;
	unsigned ncalls;
	struct symlist *called;
	unsigned ncalled;

	unsigned recursive;

	unsigned histo;
	FLOAT sub_histo;
	int	cycnum;
	int	numindex;
	int	flag;
};

/* Vector of `struct mesym' for all functions,
   sorted in ascending order by VALUE field for binary search.  */
struct mesym *syms;
int nsym=0;			/* Length of vector */

/* Vector of `struct mesym' for all cycles so far identified.  */
struct mesym **cycles;
int number_of_cycles = 0;		/* Length of vector */

/* Structure for an edges of the call graph.

   Each edge--each pair of functions A and B such that A called B--
   is represented by one of these structures.  It appears in
   A's `calls' chain and in B's `called' chain.

   Meanwhile, the structure describes its meaning with the
   SYM_FROM and SYM_TO fields, which point to the symbol entries
   for A and B.

   Basically, if you got here from a mesym's calls
   pointer, SYM_FROM points back to that symbol, NEXT_FROM is irrevelant,
   SYM_TO points to the symbol it called, and NEXT_TO points to the
   next one in the list.

   If you got here from a mesym's called pointer, SYM_FROM points to
   the symbol that called it, NEXT_FROM points to the next one in the
   list, SYM_TO points back to the symbol, and NEXT_TO is irrelevant.

   NCALLS is the number of times SYM_FROM called SYM_TO,
   PROP_TIME is the amount of time in SYM_TO itself propagated to SYM_FROM,
   CHILD_TIME is the amount of time
     propagated from SYM_TO's children to SYM_FROM.  */
   
struct symlist {
	struct mesym *sym_from;
	struct symlist *next_from;
	struct mesym *sym_to;
	struct symlist *next_to;

	unsigned ncalls;
	FLOAT	prop_time;
	FLOAT	child_time;
};

/* argv[0], here for the sake of error messages.  */
char *myname = 0;

/* Header of the executable file.  */
struct exec exec_header;

/* Name of the executable file.  */
char *exec_file_name;

/* The string table of the executable file.
   Each symbol entry in the file contains an index in the string table.
   When the symbols are in core, we relocate them to point to their names,
   which remain inside the string table.  */
char *strs;

/* Header of the first gmon.out file we read.  This is used to (try to)
   make sure all the rest of the gmon.out files agree with the first one
   (and the executable.)  */
struct gm_header hdr;

/* Number of clock ticks per second.  Read from the kernel's memory,
   this number tells us how long an interval a single stab in the
   histogram represents. */
long ticks;

/* This is an array of pointers to symbols.  These are the functions that
   we'll have to print out in the flat graph.  */
struct mesym **flat_profile_functions;
int number_of_flat_profile_functions =  0; /* Size of vector */

/* This is an array of pointers to symbols.  These are the functions that
   we should mention in the call tree.  */
struct mesym **functions_in_call_tree;
int number_of_functions_in_call_tree= 0;
struct mesym **f_end;

/* Number of slots in the histogram.  */
int nhist;

/* Total number of counts in the histogram.  */
unsigned long tothist = 0;

/* Pointer to the histogram itself.  */
unsigned CHUNK *histo;	/* Stabs */

/* Now that I've re-written the ring_buffer code, we need this */
void *ring_buffer;

/* Record the specified options.  */

/* If the -a option is given, static (private) functions are not read into
   the symbol table.  This means that time spent in them, calls to them, etc,
   are instead added to whatever function was loaded next to it in the a.out
   file.  This is compatable with UN*X gprof.  (Bleh.)  The right thing to do
   is keep track of time spent in static functions, and forget about it,
   instead of adding it to another hapless function.  Rms disagrees with me.
   I think its evil to add histogram time to a function simply because it
   happend to be loaded in memory just before a function we don't want to print. */
int no_locals = 0;		/* -a flag */

/* The -b option tells gprof to not print out the obnoxious blurbs telling
   what all the fields of the output mean.  This is useful if you've seen
   the blurbs a gzillion times and you only want to look at your numbers. */
int no_blurbs = 0;		/* -b flag */

/* If the -s option is given, gprof will write out a 'sum file' gmon.sum
   which is a gmon.out file that contains the profile info from all the
   gmon.out files that gprof read in. */
int write_sum_file = 0;		/* -s option */

/* The -z option tells gprof to include functions with zero usage (never called,
   and used no time) in the output.  Usually, such functions are considered
   boring, and aren't printed. */
int print_zeros = 0;		/* -z option */

/* The -e option takes a function name, and suppress the printing of that
   function and its descendents from the call graph profile.  (If its
   descendents are called from elswhere, well. . .)  (Currently, they're
   printed, and the -e'd function is shown under the list of parents so
   you can see where the child's time is disappearing to. . .)

   The -E option takes a function name, and not only -e's it, but
   removes the time spent in the function from the total time.  (Its as if
   that function never existed (unless it calls something that is also
   called from somewhere else, in which case. . .  (Currently it works
   as described for -e above.))

   -f prints only the call tree for its argument.

   -F is like -f, but it uses only the time in the function and its
   descendents for computing total time.
 */

enum option_type { SMALL_E, BIG_E, SMALL_F, BIG_F };

struct filter {
	char *name;
	enum option_type type;
};

/* Vector with an element for each -e, -E, -f or -F option specified.  */
struct filter *filters;
/* Length of the vector.  */
int nfilters;

/* These values are stored in the flag field of a struct mesym so we can know
   what we're doing to that function */

/* This one is being saved by a -f or -F flag */
#define SAVE_ME		01
/* This one is being killed by a -e or -E flag */
#define KILL_ME		02

/* Blurbs that are copied verbatim into the output file
   to explain the data in the tables.

   If your compiler can't stand this, split this up into a vector
   of strings, and print them one after another. */

char *first_blurb = "\n\
 The above table shows how much time was spent directly in each\n\
 function.  The table was sorted by the amount of time the computer\n\
 spent in each function.\n\n\
 % time        This is the percentage of the total execution time\n\
        the program spent in this function.  These should all add\n\
        up to 100%.\n\n\
 seconds       This is the total number of seconds the computer spent\n\
        executing this function.\n\n\
 cumsec        This is the cumulative total number of seconds the\n\
        computer spent executing this functions, plus the time spent\n\
 	in all the functions above this one in this table.\n\n\
 calls         This is the total number of times the function was called.\n\
        If there isn't a number here, the function wasn't compiled with\n\
	the profiler enabled, and further information about this function\n\
	is limited.  In particular, all information about where this \n\
	function was called from has been lost.\n\n\
 function      This is the name of the function.\n\
\f";

char *second_blurb = "\n\
 This table describes the call tree of the program, and was sorted by\n\
 the total amount of time spent in each function and its children.\n\n\
 Each entry in this table consists of several lines.  The line with the\n\
 index number at the left hand margin lists the current function.\n\
 The lines above it list the functions that called this function,\n\
 and the lines below it list the functions this one called.\n\
 This line lists:\n\
     index	A unique number given to each element of the table.\n\
		Index numbers are sorted numerically.\n\
		The index number is printed next to every function name so\n\
		it is easier to look up where the function in the table.\n\n\
     % time	This is the percentage of the `total' time that was spent\n\
		in this function and its children.  Note that due to\n\
		different viewpoints, functions excluded by options, etc,\n\
		these numbers will NOT add up to 100%.\n\n\
     self	This is the total amount of time spent in this function.\n\n\
     children	This is the total amount of time propagated into this\n\
		function by its children.\n\n\
     called	This is the number of times the function was called.\n\
		If the function called itself recursively, the number\n\
		only includes non-recursive calls, and is followed by\n\
		a `+' and the number of recursive calls.\n\n\
     name	The name of the current function.  The index number is\n\
		printed after it.  If the function is a member of a\n\
		cycle, the cycle number is printed between the\n\
		function's name and the index number.\n\n\n\
 For the function's parents, the fields have the following meanings:\n\n\
     self	This is the amount of time that was propagated directly\n\
		from the function into this parent.\n\n\
     children	This is the amount of time that was propagated from\n\
		the function's children into this parent.\n\n\
     called	This is the number of times this parent called the\n\
		function `/' the total number of times the function\n\
		was called.  Recursive calls to the function are not\n\
		included in the number after the `/'.\n\n\
     name	This is the name of the parent.  The parent's index\n\
		number is printed after it.  If the parent is a\n\
		member of a cycle, the cycle number is printed between\n\
		the name and the index number.\n\n\
 If the parents of the function cannot be determined, the word\n\
 `<spontaneous>' is printed in the `name' field, and all the other\n\
 fields are blank.\n\n\
 For the function's children, the fields have the following meanings:\n\n\
     self	This is the amount of time that was propagated directly\n\
		from the child into the function.\n\n\
     children	This is the amount of time that was propagated from the\n\
		child's children to the function.\n\n\
     called	This is the number of times the function called\n\
		this child `/' the total number of times the child\n\
		was called.  Recursive calls by the child are not\n\
		listed in the number after the `/'.\n\n\
     name	This is the name of the child.  The child's index\n\
		number is printed after it.  If the child is a\n\
		member of a cycle, the cycle number is printed\n\
		between the name and the index number.\n\n\
 If there are any cycles (circles) in the call graph, there is an\n\
 entry for the cycle-as-a-whole.  This entry shows who called the\n\
 cycle (as parents) and the members of the cycle (as children.)\n\
 The `+' recursive calls entry shows the number of function calls that\n\
 were internal to the cycle, and the calls entry for each member shows,\n\
 for that member, how many times it was called from other members of\n\
 the cycle.\n\n";

#ifdef __STDC__
/* Prototypes for all the functions.  */

/* Since I don't think its in the library yet. */
int getopt(int,char **,char *);

int	main(int,char **);
void	print_flat_profile(void);
void	print_call_graph(void);
void	write_summary(void);
void	filter_graph(void);
FLOAT	convert_and_round(FLOAT);
void	add_to_lists(struct mesym *,struct mesym *,unsigned);
void	delete_from_lists(struct mesym*,struct mesym *);
void	flushfuns(void);
void	findcycles(void);
void	kill_children(struct mesym *,int);
void	save_the_children(struct mesym *,int);
void	remove_from_call_tree(struct mesym **);
struct mesym **find_funp_from_name(char *);
struct mesym **find_funp_from_pointer(struct mesym *);
void	read_syms(FILE *,int);
struct mesym *val_to_sym(unsigned long);
int	badsym(struct nlist *);
int	symcmp(void *,void *);
int	timecmp(void *,void *);
int	callcmp(void *,void *);
int	treetimecmp(void *,void *);
int	listcmp(void *,void *);
void	readgm(char *);
void	print_blurb(char *blurb);
long	get_ticks(void);
void	add_filter(char *name,int flag);
void	print_sorted_list(int,int,struct symlist *);

void	fatal(char *printf_string,...);
void	fatal_io(char *problem,char *filename);

FILE *ck_fopen(char *name,char *mode);
void ck_fseek(FILE *fp,long i,int w);
void ck_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void ck_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
void ck_fclose(FILE *stream);

void *ck_malloc(size_t size);
void *ck_calloc(size_t nmemb,size_t size);
void *ck_realloc(void *,size_t);

char *mk_sprintf(char *printf_string,...);

void *init_ring_buffer(void);
void push_ring_buffer(void *buffer,void *what_to_push);
void *pop_ring_buffer(void *buffer);
int ring_buffer_isnt_empty(void *buffer);
void flush_ring_buffer(void *buffer);

/* Since we don't have prototypes for the system funs, we add them
   ourselves. . . */
int	atoi(const char *);
long	atol(const char *);
double	atof(const char *);

void	qsort(void *,int,int,int (*)());

void	free(void *);

void	exit(int);

int	strcmp(const char *,const char *);
char	*index(const char *,int);
int	printf(const char *,...);
int	fprintf(FILE *,const char *,...);
/* char	*sprintf(const char *,const char *,...); */

int	puts(const char *);
int	fputs(const char *,FILE *);

int	fputc(int,FILE *);
int	fread(void *,size_t,size_t,FILE *);

int	nlist(const char *,struct nlist *);

int	vfprintf(FILE *,const char *,va_list);

void dbgprintf(char *s,...);
void dumpsyms(void);
void dumpfuns(void);
#else
	/* Not a real C compiler Blecch */
void *init_ring_buffer();
void *pop_ring_buffer();
void push_ring_buffer();
void *ck_malloc();
void *ck_realloc();
void *ck_calloc();

void ck_fseek();
void ck_fread();
void ck_fwrite();
void ck_fclose();
void fatal_io();
void fatal();
FILE *ck_fopen();
char *mk_sprintf();

void write_summary();
void print_flat_profile();
void print_call_graph();
void flushfuns();
void findcycles();
void filter_graph();
void print_sorted_list();
void add_to_lists();
void delete_from_lists();
void save_the_children();
void kill_children();
void remove_from_call_tree();
void read_syms();
void readgm();
void print_blurb();
void add_filter();
int timecmp();
int callcmp();
int treetimecmp();
int listcmp();
int symcmp();
struct mesym **find_funp_from_name();
struct mesym **find_funp_from_pointer();
FLOAT convert_and_round();

void dbgprintf();
void dumpsyms();
void dumpfuns();
typedef int size_t;
#endif

/* And now, the program.  */

int
#ifdef __STDC__
main(int ac,char **av)
#else
main(ac,av)
int ac;
char **av;
#endif
{
  FILE	*fp;
  int	argc;
  char	**argv;
  int	c;
  int n;

  extern char *optarg;
  extern int optind,opterr;

  argc=ac;
  argv=av;

  myname= argv[0];

  /* Omit profiling internal functions from call graph.  */
#ifdef sparc

#else
  add_filter("mcount",BIG_E);
#endif
  /* add_filter("mcleanup",BIG_E); Seems to have dissappeared? */
  add_filter("moncontrol",BIG_E);

  ring_buffer=init_ring_buffer();

  while((c=getopt(argc,argv,"abcdD:e:E:f:F:svz"))!=EOF) {
    switch(c) {
    case 'a':
      no_locals= TRUE;
      break;
    case 'b':
      no_blurbs = TRUE;
      break;
    case 'c':
      fatal("The -c option is not supported");
    case 'd':
      debug= -1;
      break;
    case 'D':
      debug=atoi(optarg);
      break;
    case 's':
      write_sum_file= TRUE;
      break;
    case 'z':
      print_zeros = TRUE;
      break;
    case 'e':
      add_filter(optarg,SMALL_E);
      break;
    case 'E':
      add_filter(optarg,BIG_E);
      break;
    case 'f':
      add_filter(optarg,SMALL_F);
      break;
    case 'F':
      add_filter(optarg,BIG_F);
      break;
    default:
      fprintf(stderr,
	      "Usage:  %s -absz -e func -E func -f func -F func executable gmon.out ...\n",
	      myname);
      break;
    }
  }
  if(optind<argc) {
    exec_file_name=argv[optind];
    optind++;
  }
  if(!exec_file_name)
    exec_file_name=OBJ_NAM;

  /* Open the a.out file, and read in selected portions */
  fp=ck_fopen(exec_file_name,"r");
  ck_fread((void *)&exec_header,sizeof(exec_header),1,fp);

  /* Make sure its really an a.out file.  If it isn't yell and scream
     and stamp our feet. */
  if(N_BADMAG(exec_header))
    fatal("`%s' is not an executable file", exec_file_name);

  /* Read in the string table.  */
  ck_fseek(fp,N_STROFF(exec_header),0);
  ck_fread((void *)&n,sizeof(n),1,fp);
  strs=(char *)ck_malloc(n);
  ck_fread((void *)(strs+sizeof(long)),sizeof(char),n-sizeof(long),fp);

  /* Read the symbols, and put the interesting ones (sorted) in SYMS.  */
  ck_fseek(fp,N_SYMOFF(exec_header),0);
  read_syms(fp,exec_header.a_syms/sizeof(struct nlist));

  ck_fclose(fp);
  /* We are done with the a.out file */

  /* Read in the gmon.out files; accumulate all histogram data in HISTO
     and put all number-of-calls figures into the call graph.  */
  if(optind>=argc) readgm(MON_NAM);
  else {
    while(optind<argc) {
      readgm(argv[optind]);
      optind++;
    }
  }

  /* If a summary output file is wanted,
     we can do it straightaway since we have merged the data.  */

  if(write_sum_file) {
    write_summary ();
    exit (0);
  }

  /* Find out how many clock ticks/second our machine puts out */
  ticks=get_ticks();

  /* Assign the ticks in the histogram to the functions they represent */
  {
    int	n;
    unsigned long pos;
    struct mesym *sym;
    int incr = (hdr.high - hdr.low) / nhist;

    if ((hdr.high - hdr.low) != 4 * nhist)
      abort ();

    for(n=0,pos=hdr.low,sym= &syms[0];n<nhist;n++,pos+=incr) {
      if(histo[n]) {

	/* We've found the right symbol when *sym<=pos && *(sym+1)>pos */
	/* This means POS lies between sym and the one after it */

	while((sym+1)->value<=pos)
	  sym++;
	if((sym+1)->value<(pos+incr)) {
		/* Wow!  On the edge.  Split the time between the symbols */
		sym->histo+=(histo[n]+1)/2;
		(sym+1)->histo+=(histo[n])/2;
		PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_HISTO,("%05lx %02d-> %s (%d) %s (%d)\n",pos,histo[n],sym->name,sym->histo,(sym+1)->name,(sym+1)->histo));
	} else {
		sym->histo+=histo[n];
		PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_HISTO,("%05lx %02d-> %s (%d)\n",pos,histo[n],sym->name,sym->histo));
	}
      }
    }
  }

  /* Avoid division by zero if there wasn't any time collected!  */
  if(tothist==0)
    tothist=1;

  print_flat_profile ();

  print_call_graph ();

  if(debug&DB_AFILE)
    dumpsyms();

  if(debug&DB_AFILE)
    dumpfuns();
}

/* Output a summary gmon file containing all our accumulated
   histogram and call-graph data.  */

void
#ifdef __STDC__
write_summary (void)
#else
write_summary ()
#endif
{
  struct mesym *p;
  struct symlist *t;
  struct gm_call call_tmp;
  FILE *fp;

  fp=ck_fopen(SUM_NAM,"w");
  hdr.nbytes=sizeof(struct gm_header)+nhist*sizeof(CHUNK);
  ck_fwrite((void *)&hdr,sizeof(hdr),1,fp);
  ck_fwrite((void *)histo,sizeof(unsigned CHUNK),nhist,fp);
  /* for(p= &syms[nsym];--p>=&syms[0];) { */
  if(nsym) {
    p= &syms[nsym-1];
    do {
      for(t=p->calls;t;t=t->next_to) {
	/* Since we've forgotten exactly where in FROM we
	   were called from, we fake it.  Since this is only
	   gonna be fed back into gprof, it doesn't matter */
	call_tmp.from=(p->value+FUDGE_FACTOR)-hdr.low;
	call_tmp.to=(t->sym_to->value+FUDGE_FACTOR)-hdr.low;
	call_tmp.ncalls=t->ncalls;
	ck_fwrite((void *)&call_tmp,sizeof(call_tmp),1,fp);
      }
    } while(p-->&syms[0]);
  }
  ck_fclose(fp);
}

/* Print the flat profile from the symbol table information.  */

void
#ifdef __STDC__
print_flat_profile (void)
#else
print_flat_profile ()
#endif
{
  int n;
  struct mesym **funp;

  /* Scan the symbol table and discover how many functions either had time
     spent in them, or had a non-zero call count */
  for(n=0;n<nsym;n++) {
    if(syms[n].histo || syms[n].ncalled || print_zeros)
      number_of_flat_profile_functions++;
  }
  /* Collect all the interesting functions */
  flat_profile_functions
    =(struct mesym **)ck_calloc(number_of_flat_profile_functions,sizeof(struct mesym *));
  for(n=0,funp=flat_profile_functions;n<nsym;n++) {
    if(syms[n].histo || syms[n].ncalled || print_zeros) {
      *funp= &syms[n];
      funp++;
    }
  }

  /* Sort them */
  qsort(flat_profile_functions,number_of_flat_profile_functions,
	sizeof(struct mesym *),timecmp);

  /* And print them out */

  if (no_blurbs)
    printf("\t\tFlat profile\n\n");
  else
    printf("\tFlat profile (explanation follows)\n\n");

  if (no_blurbs)
    printf("\t\tCall graph is on the following page.\n\n");
  else
    printf("\tCall graph is on the following page.\n\n");

  printf("Each sample counts as %g seconds.\n\n",1.0/ticks);

  puts("% time  seconds   cumsec   calls  function");

  for(n=0;n<number_of_flat_profile_functions;n++) {
    unsigned histo;
    static cumhist = 0;

    histo=flat_profile_functions[n]->histo;
    cumhist+=histo;
    printf("%6.2f ", (FLOAT)(100.0*histo)/(FLOAT)tothist);
    printf("%8.2f ", ((FLOAT)histo)/(FLOAT)ticks);
    printf("%8.2f ", ((FLOAT)cumhist)/(FLOAT)ticks);
    if(flat_profile_functions[n]->ncalled)
      printf("  %5d  ", flat_profile_functions[n]->ncalled);
    else fputs("         ",stdout);
    printf("%s\n", flat_profile_functions[n]->name);
  }

  /* RMS says we should print the blurb last, which makes sense to me */
  print_blurb(first_blurb);
}

/* Compute the call graph and print it.  */

void
#ifdef __STDC__
print_call_graph (void)
#else
print_call_graph ()
#endif
{
  int n;
  int index;
  struct mesym **funp;
  struct mesym **f;

  /* Count the functions that appear in the call tree.  */
  for(n=0;n<nsym;n++) {
    /* If a function calls something else, or is called by
       something else, its in the call tree. . . */
    if(syms[n].ncalls || syms[n].ncalled)
      number_of_functions_in_call_tree++;
  }

  /* Allocate a vector of thuese functions.  */
  functions_in_call_tree
    =(struct mesym **)ck_calloc(number_of_functions_in_call_tree,sizeof(struct mesym *));
  for(n=0,funp=functions_in_call_tree;n<nsym;n++) {
    if(syms[n].ncalls || syms[n].ncalled) {
      *funp= &syms[n];
      funp++;
    }
  }

  /* Put all the leaf nodes at the end of the call tree */
  qsort(functions_in_call_tree,number_of_functions_in_call_tree,sizeof(struct mesym *),callcmp);

  /* Root nodes are now all at the head, and can be easily found
     'cuz they have call-counts of zero (Never been called, but
     calls something else; that's spontaneous in my book.) */
  /* Ordinary nodes are in the middle, (were called, and
     called others.  Leaf nodes are at the end. */

  /* Our mission, should we choose to accept it, is to detect
     circles in the call graph. */
  /* We do this by keeping a pointer into the call tree called f_end.  This
     points to the end of the functions that we don't know if they are leaf
     nodes or not.  When we know something is a leaf node, we move it down
     past f_end */

  f= &functions_in_call_tree[number_of_functions_in_call_tree-1];
  do {
    if((*f)->ncalls!=0)
      break;
    (*f)->cycnum= -1;

    /* Note the neeto side effect here!  Is there a better way to do this? */
  } while(f-- > &functions_in_call_tree[0]);

  /* Note that functions that only call themselves (and nobody else) don't
     get marked above.  Doesn't matter; they get marked soon. */
  if(f== &functions_in_call_tree[0]) {
    (*f)->cycnum= -1;
  } else {
    int found;

    f_end=f;

    /* Eliminate recursive calls */
    do {
      struct symlist *t,*u;

      for(t= (*f)->calls;t;t=t->next_to) {
	if(t->sym_to!= (*f))
	  continue;
	(*f)->recursive+=t->ncalls;

	(*f)->ncalls-=t->ncalls;
	(*f)->ncalled-=t->ncalls;

	/* Delete from linked list */
	if(t==(*f)->calls)
	  (*f)->calls=t->next_to;
	else {
	  for(u=(*f)->calls;u->next_to!=t;u=u->next_to)
	    ;
	  u->next_to=t->next_to;
	}

	/* Find and delete from called list too */
	if(t==(*f)->called)
	  (*f)->called=t->next_from;
	else {
	  for(u=(*f)->called;u->next_from!=t;u=u->next_from)
	    ;
	  u->next_from=t->next_from;
	}
	free(t);
	break;
      }
    } while(f--!=&functions_in_call_tree[0]);

    number_of_cycles = 0;

    /* Either there weren't any cycles, or all the cycles live
       between f_end and functions_in_call_tree */

    /* Mark all functions that are not in any cycle.  */
    flushfuns();


    /* If we haven't got them all, find the cycle(s).  */
    if (f_end != &functions_in_call_tree[0])
      findcycles();
  }

  /* Add entries for the cycles to the vector of all call-graph nodes.  */

  functions_in_call_tree
    =ck_realloc(functions_in_call_tree,
		(number_of_cycles+number_of_functions_in_call_tree)*sizeof(struct mesym *));
  for(n=0;n<number_of_cycles;n++)
    functions_in_call_tree[number_of_functions_in_call_tree++]= cycles[n];

  /* Now discard the functions that the filters say should not appear.  */

  filter_graph ();

  /* So by now, the only functions left are the ones we want to print */
  qsort(functions_in_call_tree,number_of_functions_in_call_tree,sizeof(struct mesym *),treetimecmp);

  /* Assign each function its index number.  */
  for(n=0;n<number_of_functions_in_call_tree;n++)
    functions_in_call_tree[n]->numindex=n+1;

  if (no_blurbs)
    printf("\t\t\tCall graph\n\n");
  else
    printf("\t\t     Call graph (explanation follows)\n\n");

  puts("index  % time    self  children called     name");

  /* Loop over entries.  */

  for(index = 0; index <number_of_functions_in_call_tree; index++) {
    struct mesym *current = functions_in_call_tree[index];
    char	tmpstr[40];	/* Can an int have more than 40 digits?  I hope not */

    /* Print out all the things that called this */
    if(current->ncalled==0 && current->called==0)
      printf("                                             <spontaneous>\n");
    else {
      print_sorted_list(current->ncalled,current->cycnum,current->called);
    }
		
		
    sprintf(tmpstr,"[%d]",current->numindex);
    printf("%-6s %6.2f %7.2f  %7.2f ",
	   tmpstr,
	   (FLOAT)(100.0*(current->sub_histo+current->histo))/(FLOAT)tothist,
	   convert_and_round((FLOAT)current->histo),
	   convert_and_round((FLOAT)current->sub_histo));

    if(current->recursive) {
      printf("%4d+%-4d %s",
	     current->ncalled,
	     current->recursive,
	     current->name);
    } else
      printf("%4d      %s",current->ncalled,current->name);

    if(current->cycnum>0 && current->name[0]!='<')
      printf(" <cycle %d>",current->cycnum);

    printf(" [%d]\n",current->numindex);


    /* Now print out the children */

    if(current->name[0]=='<')
      print_sorted_list(-2,current->cycnum,current->calls);
    else
      print_sorted_list(-1,current->cycnum,current->calls);


    printf("----------------------------------------\n");
  }

  print_blurb(second_blurb);
}

/* Scan the call tree for virtual leaf nodes */
/* If we find any, propagate time into them from their children,
   mark them as being leaf nodes.  Then repeat the process.  When
   we drop out of here, either we've flushed the entire tree, or
   we've found a cycle. */

void
#ifdef __STDC__
flushfuns(void)
#else
flushfuns()
#endif
{
  int	found;
  struct mesym **f;

  do {
    found=0;
    f=f_end;
    do {
      struct symlist *t;

      assert(f>=functions_in_call_tree);

      for(t=(*f)->calls;t;t=t->next_to)
	if(t->sym_to->cycnum==0)
	  break;

      /* We've found an virtual leaf node.  We shold
	 propagate time into the node, and move it
	 past f_end, since we aren't interested in
	 it anymore */
      if(!t) {
	found++;

	/* If its a member of a cycle, cycnum is positive, and
	   time propagation has already been dealt with. */
	if((*f)->cycnum==0) {
	  for(t=(*f)->calls;t;t=t->next_to) {
	    struct mesym *symP;

	    if(t->sym_to->name[0]=='<')
	      continue;
	    if(t->sym_to->cycnum==-1)
	      symP= t->sym_to;
	    else 
	      symP= cycles[t->sym_to->cycnum-1];

	    t->prop_time= (FLOAT)t->ncalls*(FLOAT)symP->histo/(FLOAT)symP->ncalled;
	    t->child_time=(FLOAT)t->ncalls* symP->sub_histo  /(FLOAT)symP->ncalled;
	    (*f)->sub_histo+=t->prop_time+t->child_time;
	  }
	  (*f)->cycnum= -1;
	}

	/* move this node past f_end, since we don't care
	   about it anymore */
	if(f_end!=functions_in_call_tree) {
	  /* move this function to the end */
	  if(f!=f_end) {
	    struct mesym *tmp;

	    tmp= *f;
	    *f= *f_end;
	    *f_end=tmp;
	  }
	  --f_end;
	}
	assert(f_end>=functions_in_call_tree);
      }
    } while(f-->&functions_in_call_tree[0]);
  } while(found && f_end>&functions_in_call_tree[0]);
}

void
#ifdef __STDC__
findcycles(void)
#else
findcycles()
#endif
{
	struct mesym **f;
	struct mesym *ptr;
	struct symlist *tmp;
	struct cy {
		struct mesym *memb1;
		struct mesym *memb2;
		struct mesym **others;
		int width;
		int deepest;
		int shallowest;
	};
	struct cy *cy;
	int ncy = 0;
	int n;

	int bigdepth = 0;
	int curdepth;
	struct mesym *current_cycle_pointer;
	struct mesym *cursym;

	int tree_depth;

	for(f= &functions_in_call_tree[0];f<=f_end;f++) {
		if((*f)->ncalled==0) {
			push_ring_buffer(ring_buffer,*f);
			(*f)->flag=0;
		}
	}
	push_ring_buffer(ring_buffer,(void *)0);

	tree_depth = 1;
	for(;;) {
		ptr=pop_ring_buffer(ring_buffer);
		if(!ptr) {
			if(ring_buffer_isnt_empty(ring_buffer)) {
				push_ring_buffer(ring_buffer,(void *)0);
				tree_depth ++;
				continue;
			} else {
				break;
			}
		} else if(ptr->flag==1) {
			fprintf(stderr,"Ignoring call to spont function %s\n",ptr->name);
		} else if(ptr->flag!=0) {
			/* Save upward edge */
			/* printf("Upward edge detected in function %s\n",ptr->name); */
			if(!ncy) {
				cy=ck_malloc(sizeof (struct cy));
				ncy=1;
			} else {
				ncy++;
				cy=ck_realloc(cy,ncy*sizeof(struct cy));
			}
			cy[ncy-1].memb1=ptr;
			cy[ncy-1].memb2=0;
		} else {
			ptr->flag=tree_depth;
			for(tmp=ptr->calls;tmp;tmp=tmp->next_to)
				if(tmp->sym_to->cycnum!=-1)
					push_ring_buffer(ring_buffer,tmp->sym_to);
		}
	}
	if(!ncy)
		return;

	for(n=0;n<ncy;n++) {
		struct symlist *s;
		struct symlist *u;


		cursym=cy[n].memb1;
		if(cursym->cycnum)
			continue;

		number_of_cycles++;
		/* for(s=cursym->called;s;s=s->next_from) {
			if(s->sym_from->flag>=cursym->flag)
				break;
		}
		if(!s)
			abort(); */

		if (!cycles)
			cycles=(struct mesym **)ck_malloc(sizeof(struct mesym *));
		else
			cycles=(struct mesym **)ck_realloc((void *)cycles,
							   number_of_cycles*sizeof(struct mesym *));

		current_cycle_pointer = (struct mesym *)ck_malloc(sizeof(struct mesym));
		cycles[number_of_cycles-1] = current_cycle_pointer;
		bzero (current_cycle_pointer, sizeof *current_cycle_pointer);
		current_cycle_pointer->name=mk_sprintf("<cycle %d as a whole>",number_of_cycles);
		current_cycle_pointer->value= (unsigned long)-1;
		current_cycle_pointer->cycnum=number_of_cycles;

		cursym=cy[n].memb1;

		push_ring_buffer(ring_buffer,cursym);

		while(ring_buffer_isnt_empty(ring_buffer)) {
			cursym=pop_ring_buffer(ring_buffer);
			if(cursym->cycnum==number_of_cycles)
				continue;
			if(cursym->cycnum!=0)
				abort();
			PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_CYC,("adding %s(%d) to cycle",cursym->name,cursym->histo));
			current_cycle_pointer->histo+=cursym->histo;
			cursym->cycnum=number_of_cycles;
			add_to_lists(current_cycle_pointer,cursym,0);

			/* Now queue the subroutines of this function to be scanned
			   eventually.  */

			for(u=cursym->calls;u;u=u->next_to)
				if(u->sym_to->cycnum==0) {
					push_ring_buffer(ring_buffer,(void *)(u->sym_to));
				} else if(u->sym_to->name[0] == '<') {
					PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_CYC,("Cycle calls %s(%d)",u->sym_to->name,u->ncalls));
					add_to_lists(current_cycle_pointer,u->sym_to,u->ncalls);
				}
		}

		/* The cycle's list of children now contains all the members of the cycle.
		   Occasionally a function creeps in that doesn't belong in the cycle.
		   Find and remove them. */

		{
			struct symlist *v;
			int found;

			do {
				found = 0;
				for(u=current_cycle_pointer->calls;u;u=u->next_to) {
					for(v=u->sym_to->called;v;v=v->next_from)
						if(v->sym_from->name[0]!='<' && v->sym_from->cycnum==number_of_cycles)
							break;
					if(!v) {
						PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_CYC,("%s not really in cycle",u->sym_to->name));
						/* This 'cycle member' wasn't called by any member of the cycle.
						   thus, it isn't a cycle member. */
						u->sym_to->cycnum=0;
						delete_from_lists(current_cycle_pointer,u->sym_to);
						found++;
					}
				}
			} while(found);
		}
		/* The cycle's lists of callers and children are now correct.
		   Propagate the time through the cycle.  */


		/* Now we propagate time INTO the
		   cycle from its children.  We could do this in flushfuns,
		   but its easier to do here, since doing it here guarentees
		   it only happens once per cycle */

		for(u=current_cycle_pointer->calls;u;u=u->next_to) {
			struct mesym *symP;
			struct symlist *v;

			/* We have to remember to not propagate time that's already here. . . */
			if(u->sym_to->cycnum!=number_of_cycles) {
				current_cycle_pointer->ncalls+=u->ncalls;

				if(u->sym_to->cycnum==-1)
					symP= u->sym_to;
				else			/* Propagate time FROM another cycle here */
					symP= cycles[u->sym_to->cycnum-1];
				u->prop_time= (FLOAT)u->ncalls*(FLOAT)symP->histo/(FLOAT)symP->ncalled;
				u->child_time=(FLOAT)u->ncalls* symP->sub_histo  /(FLOAT)symP->ncalled;
				current_cycle_pointer->sub_histo+=u->prop_time+u->child_time;
				for(v=u->sym_to->called;v;v=v->next_from) {
					if(v->sym_from->cycnum==number_of_cycles) {
						v->prop_time= (FLOAT)v->ncalls*(FLOAT)symP->histo/(FLOAT)symP->ncalled;
						v->child_time=(FLOAT)v->ncalls*  symP->sub_histo /(FLOAT)symP->ncalled;
					}
				}
			} else {
				u->prop_time= u->sym_to->histo;
				u->child_time=u->sym_to->sub_histo;

				for(v=u->sym_to->calls;v;v=v->next_to) {
					if(v->sym_to->cycnum==number_of_cycles) {
						add_to_lists(current_cycle_pointer,v->sym_to,v->ncalls);
						current_cycle_pointer->recursive+=v->ncalls;
					} else {
						symP = v->sym_to;
						v->prop_time= (FLOAT)v->ncalls*(FLOAT)symP->histo/(FLOAT)symP->ncalled;
						v->child_time=(FLOAT)v->ncalls*  symP->sub_histo /(FLOAT)symP->ncalled;
					}
				}

				for(v=u->sym_to->called;v;v=v->next_from) {
					assert(v->sym_from->cycnum==0 || v->sym_from->cycnum==number_of_cycles);
					if(v->sym_from->cycnum==0) {
 PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_CYC,("Cycle called by %s(%d)",v->sym_from->name,v->ncalls));
						add_to_lists(v->sym_from,current_cycle_pointer,v->ncalls);
					}
				}
			}
		}


		/* Sum all calls into the cycle, from each caller not in the cycle,
		   to get the number of calls into the cycle.  */
		for(u=current_cycle_pointer->called;u;u=u->next_from) {
			if(u->sym_from->cycnum!=number_of_cycles)
				current_cycle_pointer->ncalled+=u->ncalls;
		}

		/* Loop over functions in this cycle.  */
		for(u=current_cycle_pointer->calls;u;u=u->next_to) {
			struct symlist *v;

			if(u->sym_to->cycnum!=number_of_cycles)
				continue;

			/* U->sym_to is a function in this cycle.  */

			/* Find all calls to this function from functions outside the cycle
			   and add remove them from the count of calls "from the cycle" to this function.  */

			for(v=u->sym_to->called;v;v=v->next_from) {
				if(v->sym_from != current_cycle_pointer
				   && v->sym_from->cycnum==number_of_cycles)
					u->sym_to->ncalled-=v->ncalls;
			}
		}

		/* Compute amounts of time to propagate out of the cycle
		   to the callers-in.  */

		for (u=current_cycle_pointer->called;u;u=u->next_from)
			if (u->sym_from->cycnum != number_of_cycles) {
				struct symlist *v;

				u->prop_time= ((FLOAT)u->ncalls*(FLOAT)current_cycle_pointer->histo     /(FLOAT)current_cycle_pointer->ncalled);
				u->child_time=((FLOAT)u->ncalls*       current_cycle_pointer->sub_histo /(FLOAT)current_cycle_pointer->ncalled);
				for(v=u->sym_from->calls;v;v=v->next_to) {
					if(v->sym_to->cycnum==number_of_cycles) {
						v->prop_time= ((FLOAT)v->ncalls*(FLOAT)current_cycle_pointer->histo     /(FLOAT)current_cycle_pointer->ncalled);
						v->child_time=((FLOAT)v->ncalls*       current_cycle_pointer->sub_histo /(FLOAT)current_cycle_pointer->ncalled);
					}
				}
			}

		flushfuns();
	}
}


/* Remove from the call tree all nodes that are rejected by
   the -e, -E, -f and -F filters that were specified.
   Their nodes are removed from functions_in_call_tree
   and their edges are deleted from the lists they are in.  */

void
#ifdef __STDC__
filter_graph (void)
#else
filter_graph ()
#endif
{
  int n;
  int the_bomb_is_falling = 0;

  for(n=0;n<nfilters;n++) {
    struct mesym **call_tree_pointer;

    call_tree_pointer=find_funp_from_name(filters[n].name);
    /* Couldn't find it?  Skip it! */
    if(!call_tree_pointer) {
      /* It may have taken time, although it
	 isn't in the call tree.  Seek and
	 destroy! */
      if(filters[n].type==BIG_E) {
	struct mesym *p;

	for(p=syms;p< &syms[nsym];p++) {
	  if(!strcmp(p->name,filters[n].name)) {
	    tothist-=p->histo;
	    break;
	  }
	}
	if(p==&syms[nsym])
	  fprintf(stderr,"Warning: couldn't find function %s\n",filters[n].name);
      } else
	fprintf(stderr,"Warning:  %s is not in the call tree.\n",filters[n].name);
      continue;
    }
    PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_OPT,("Option %d on %s",filters[n].type,(*call_tree_pointer)->name));

    switch(filters[n].type) {
    case SMALL_E:
      kill_children(*call_tree_pointer,FALSE);
      break;
    case BIG_E:
      kill_children(*call_tree_pointer,TRUE);
      break;
    case SMALL_F:
      if(!the_bomb_is_falling)
	the_bomb_is_falling = 1;
      save_the_children(*call_tree_pointer,FALSE);
      break;
    case BIG_F:
      if(!the_bomb_is_falling) {
	the_bomb_is_falling = 1;
	tothist=0;
      }
      save_the_children(*call_tree_pointer,TRUE);
      break;
    default:
      abort();
    }
  }

  /* If we had -e or -E filters, now delete everything that
     was not marked to be saved.  */

  if(the_bomb_is_falling) {
    struct mesym **call_tree_pointer;

    PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_OPT,("And now we drop the bomb."));
    call_tree_pointer=&functions_in_call_tree[number_of_functions_in_call_tree-1];
    do {
      if(((*call_tree_pointer)->flag&SAVE_ME)==0)
	remove_from_call_tree(call_tree_pointer);
    } while (call_tree_pointer-->&functions_in_call_tree[0]);
  }
}

/* Collect a list of parents/children, sort them, and print them */
/* If CALLED > 0, we print parents,
      and the value of CALLED is the total number of times this fn was called.
   If CALLED == -2 we print children.
      This is used for printing the children of an entire cycle.
   If CALLED == -1, we print children and if their cycle number is the
      same as CYCLE, we print abbreviated information.
      This is used for printing the children of an ordinary function.  */
/* LIST is the list of parents/children we want to print */

void
#ifdef __STDC__
print_sorted_list(int called,int cycle,struct symlist *list)
#else
print_sorted_list(called,cycle,list)
int called;
int cycle;
struct symlist *list;
#endif
{
  static struct symlist **sbuf;
  static nsbuf,sizsbuf;
  struct symlist **sbp,*t;
  struct mesym *symP;

  if(!sizsbuf) {
    sbuf=(struct symlist **)ck_calloc(30,sizeof(struct symlist *));
    nsbuf=0;
    sizsbuf=30;
  }

  /* Extract all the symbols in LIST as a vector,
     but ignore any which represent entire cycles.  */

  t=list;
  for(sbp= &sbuf[0];t;) {
    symP = (called>=0) ? t->sym_from : t->sym_to;
    if(symP && symP->name[0] != '<') {
      *sbp=t;
      sbp++;
      nsbuf++;
      if(nsbuf==sizsbuf) {
	sizsbuf*=2;
	sbuf=(struct symlist **)ck_realloc((void *)sbuf,sizsbuf*sizeof(struct symlist *));
	sbp= &sbuf[nsbuf];
      }
    } else {
      symP++;
			
    }

    if(called>=0) t=t->next_from;
    else t=t->next_to;
  }

  /* Sort the vector.  */
  qsort(sbuf,nsbuf,sizeof(struct symlist *),listcmp);

  /* Print the elements of the vector.  */
  for(sbp= &sbuf[0];nsbuf>0;nsbuf--,sbp++) {
    t= *sbp;
    if(called>=0) symP=t->sym_from;
    else symP=t->sym_to;

    if(cycle>0 && cycle==symP->cycnum) {
      if(called==-2) {
	printf("              %7.2f  %7.2f %4d          %s",
	       convert_and_round((FLOAT)(symP->histo)),
	       convert_and_round((FLOAT)(symP->sub_histo)),
	       t->ncalls,
	       symP->name);
      } else
	/* For things in the same cycle, we only want to print
	   the number of calls */
	printf("%30s %4d          %s","", t->ncalls, symP->name);
    } else {
      printf("              %7.2f  %7.2f %4d/%-4d     %s",
	     convert_and_round(t->prop_time),
	     convert_and_round(t->child_time),
	     t->ncalls,
	     (called>=0) ? called : symP->ncalled,
	     symP->name);
    }

    if(symP->cycnum>0 && symP->name[0]!='<')
      printf(" <cycle %d>",symP->cycnum);

    if(symP->numindex)
      printf(" [%d]\n",symP->numindex);
    else
      printf(" [not printed]\n");
  }
}

/* Compare two symbols for which should come first among
   the callers or subroutines in a single call-graph entry.  */

int
#ifdef __STDC__
listcmp(void *a,void *b)
#else
listcmp(a,b)
void *a;
void *b;
#endif
{
  struct symlist *aa,*bb;
  FLOAT	n;

  aa= *(struct symlist **)a;
  bb= *(struct symlist **)b;

  n=(bb->prop_time + bb->child_time - aa->prop_time - aa->child_time);

  if(n<0) return -1;
  if(n>0) return 1;
  return 0;
}

/* Convert a number (IN) from the histogram into seconds, rounding to the
   nearest 100th of a second. */
FLOAT
#ifdef __STDC__
convert_and_round(FLOAT in)
#else
convert_and_round(in)
FLOAT in;
#endif
{
  long int inter;

  inter=((in/(FLOAT)(ticks))+.005)*100.0;
  return ((FLOAT)(inter)/100.0);
}

/* Given ncalls from fromP to toP, add a symlist element telling about it */
void
#ifdef __STDC__
add_to_lists(struct mesym *fromP,struct mesym *toP,unsigned ncalls)
#else
add_to_lists(fromP,toP,ncalls)
struct mesym *fromP;
struct mesym *toP;
unsigned ncalls;
#endif
{
  struct symlist *tmp;

  for(tmp=fromP->calls;tmp;tmp=tmp->next_to)
    if(tmp->sym_to==toP) {
      tmp->ncalls+=ncalls;
      break;
    }
  if(!tmp) {
    tmp=(struct symlist *)ck_malloc(sizeof(struct symlist));
    tmp->sym_from=fromP;
    tmp->next_from=toP->called;
    tmp->sym_to=toP;
    tmp->next_to=fromP->calls;
    tmp->ncalls=ncalls;
    tmp->prop_time = -1;
    tmp->child_time = -1;

    fromP->calls=tmp;
    toP->called=tmp;
  }
}


/* The reverse of add_to_lists.  Forget that fromP ever called toP */
void
#ifdef __STDC__
delete_from_lists(struct mesym *fromP, struct mesym *toP)
#else
delete_from_lists(fromP, toP)
struct mesym *fromP;
struct mesym *toP;
#endif
{
  struct symlist *die,*tmp,*old;

  if(fromP->calls->sym_to==toP) {
    die=fromP->calls;
    fromP->calls=fromP->calls->next_to;
  } else {
    old=0;
    for(tmp=fromP->calls;tmp;tmp=tmp->next_to) {
      if(tmp->sym_to==toP) {
        die=tmp;
	old->next_to=tmp->next_to;
      }
      old=tmp;
    }
  }

  if(toP->called->sym_from==fromP) {
    die=toP->called;
    toP->called=toP->called->next_from;
  } else {
    for(tmp=toP->called;tmp;tmp=tmp->next_from) {
      old=0;
      if(tmp->sym_from==fromP) {
        die=tmp;
	old->next_from=tmp->next_from;
      }
      old=tmp;
    }
  }
  free(die);
}


/* Implement the -e or -E option by deleting a certain function
   and all its descendents from the call graph.
   If FLUSHFLAG is set, remove its histogram time from the total, too. */

void
#ifdef __STDC__
kill_children(struct mesym *p,int flushflag)
#else
kill_children(p,flushflag)
struct mesym *p;
int flushflag;
#endif
{
  struct symlist *t;

  push_ring_buffer(ring_buffer,p);
  while(ring_buffer_isnt_empty(ring_buffer)) {
    p=pop_ring_buffer(ring_buffer);
    if(flushflag)
      tothist-=p->histo;
    PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_OPT,("Flushing %s (%d)",p->name,flushflag ? p->histo : 0));
    p->flag|=KILL_ME;
    remove_from_call_tree(find_funp_from_pointer(p));
    for(t=p->calls;t;t=t->next_to) {
      if(t->sym_to->ncalled==t->ncalls || (p->cycnum>0 && t->sym_to->cycnum==p->cycnum)) {
	push_ring_buffer(ring_buffer,t->sym_to);
      } else if(t->sym_to->cycnum>0 && t->sym_to->cycnum!=p->cycnum) {
	if(cycles[t->sym_to->cycnum-1]->ncalled==t->ncalls) {
	  push_ring_buffer(ring_buffer,cycles[t->sym_to->cycnum-1]);
	}
      } else {
	struct symlist *ztmp;

	for(ztmp=t->sym_to->called;ztmp;ztmp=ztmp->next_from) {
	  if((ztmp->sym_from->flag&KILL_ME)==0)
	    break;
	}
	if(!ztmp)
	  push_ring_buffer(ring_buffer,t->sym_to);
	else {
	  PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_OPT,("Not flushing %s. . .  More parents",t->sym_to->name));
	  /* Do we want to deal with removing FRACTIONS of time from
	     functions who were called from different places?  If so,
	     code goes here.  (F'rinstance, if half of the calls to
	     function X were made from here, cut its stored time in half */
	}
      }
    }
  }
}

/* This is the opposite of kill_children().
   -f or -F has been specified, so all call-graph nodes EXCEPT
   the descendents of specified functions will be killed.
   Find all these descendents and mark them to be saved
   by setting the `flag' fields nonzero.

   If TIMEFLAG is set, the histogram-total has
   already been nuked, and we should add our histogram time to
   it in an attempt at reconstruction. . .  */

void
#ifdef __STDC__
save_the_children(struct mesym *p,int timeflag)
#else
save_the_children(p,timeflag)
struct mesym *p;
int timeflag;
#endif
{
  struct symlist *t;

  push_ring_buffer(ring_buffer,p);
  while(ring_buffer_isnt_empty(ring_buffer)) {
    p=pop_ring_buffer(ring_buffer);
    PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_OPT,("Saving %s (%d)",p->name,p->histo));
    p->flag|=SAVE_ME;
    if(p->cycnum>0)
      cycles[p->cycnum-1]->flag|=SAVE_ME;
    if(timeflag)
      tothist+=p->histo;
    for(t=p->calls;t;t=t->next_to) {
      if((t->sym_to->flag&SAVE_ME)==0)
	push_ring_buffer(ring_buffer,t->sym_to);
    }
  }
}

/* The bomb is falling, and PT has just been hit by severe doses of
   radiation.  Remove it from the call-tree vector */
void
#ifdef __STDC__
remove_from_call_tree(struct mesym **pt)
#else
remove_from_call_tree(pt)
struct mesym **pt;
#endif
{
  if(!pt) {
    /* Just quietly returning allows us to remove cycles from
       the call tree easily. */
    /* panic("Internal Error:  trying to remove null from call tree"); */
    return;
  }
  PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_OPT,("Removing %ss from the tree",(*pt)->name));
  *pt=functions_in_call_tree[number_of_functions_in_call_tree-1];
  number_of_functions_in_call_tree--;
}

/* We want to play with a member of the call tree, but we
   only know its name.  Try to find the funp.  This is slow,
   natch, since it uses linear search, but it doesn't get called much */

/* Should be some way to scan rest of symbols so we could delete
   mcount/mcleanup histogram ticks.  Should be way to disable warning? */
struct mesym **
#ifdef __STDC__
find_funp_from_name(char *name)
#else
find_funp_from_name(name)
char *name;
#endif
{
  struct mesym **ret;

  ret= &functions_in_call_tree[number_of_functions_in_call_tree-1];
  do {
    if(strcmp((*ret)->name,name)==0)
      return ret;
  } while (ret-->&functions_in_call_tree[0]);
  return 0;
}

/* We have the mesym, but we need to know where it lives in the call-tree
   This is almost as slow as find_funp_from_name(). */
struct mesym **
#ifdef __STDC__
find_funp_from_pointer(struct mesym *p)
#else
find_funp_from_pointer(p)
struct mesym *p;
#endif
{
  struct mesym **ret;

  ret= &functions_in_call_tree[number_of_functions_in_call_tree-1];
  do {
    if(*ret==p)
      return ret;
  } while (ret-->&functions_in_call_tree[0]);
  /* fprintf(stderr,"Warning:  Can't find %s in the call tree.\n",p->name); */
  return 0;
}

/* Read symbols from a.out file open on FP.  There are N of them.  Allocate a
   vector to store the interesting ones in, and sort it numerically.  */

void
#ifdef __STDC__
read_syms(FILE *fp,int n)
#else
read_syms(fp,n)
FILE *fp;
int n;
#endif
{
  struct nlist *tmpsyms;
  struct nlist *sym;
  int i;
#ifdef __STDC__
  char buf[n];
#else
  char *buf;
  char *alloca();

  buf=alloca(n);
#endif
  /* Read the entire symbol table.  */
  tmpsyms=(struct nlist *)ck_malloc(n*sizeof(struct nlist));
  ck_fread(tmpsyms,sizeof(struct nlist),n,fp);

  bzero (buf, n);

  /* Count the useful symbols.
     Also relocate their name-fields to be C string pointers.  */
  for(sym= &tmpsyms[0],i=0;sym<&tmpsyms[n];sym++) {
    sym->n_un.n_name= strs+sym->n_un.n_strx;
    if(!badsym(sym))
      buf[sym-tmpsyms] = 1, i++;
  }

  /* Allocate permanent space and copy useful symbols into it.  */
  nsym=i;
  syms=(struct mesym *)ck_calloc(nsym,sizeof(struct mesym));

  for(sym= &tmpsyms[0],i=0;sym<&tmpsyms[n];sym++) {
    if(!badsym(sym)) {
      syms[i].name=sym->n_un.n_name;

      /* Remove the initial _ from the symbol name */
      if(*(syms[i].name)=='_')
	syms[i].name++;
      syms[i].value=sym->n_value;
      i++;
    }
    else if (buf[sym-tmpsyms])
      abort ();
  }

  if (i != nsym)
    abort ();

  /* Put symbols in numeric order.  */
  qsort(syms,nsym,sizeof(struct mesym),symcmp);
  free((void *)tmpsyms);
}

/* Return the symbol which has the largest value less than VAL.
   Since the symbol vector is sorted by value, this is done
   with a binary search.  */

struct mesym *
#ifdef __STDC__
val_to_sym(unsigned long val)
#else
val_to_sym(val)
unsigned long val;
#endif
{
  struct mesym *m;
  int gap=nsym/4;

  m= &syms[nsym/2];
  for(;;) {
    if(m->value>val) {
      m-=gap;
      gap/=2;
    } else if((m+1)->value<val) {
      m+=gap;
      gap/=2;
    } else
      break;
    if(m<&syms[0] || m>=&syms[nsym])
      abort ();
    if(gap<1)
      gap=1;
  }
  return m;
}

/* Return TRUE if the nlist-entry SYM describes a symbol
   that gprof should pay attention to.  */

int
#ifdef __STDC__
badsym(struct nlist *sym)
#else
badsym(sym)
struct nlist *sym;
#endif
{
  if ((sym->n_type & ~N_EXT) != N_TEXT)
    return TRUE;
  if (no_locals && !(sym->n_type&N_EXT))
    return TRUE;
  /* Filenames or pascal labels should be ignored */
  if(index(sym->n_un.n_name,'.') || index(sym->n_un.n_name,'$'))
    return TRUE;
  return FALSE;
}

/* This is used to qsort() the symbol table into numerical order. */

int
#ifdef __STDC__
symcmp(void *a,void *b)
#else
symcmp(a,b)
void *a;
void *b;
#endif
{
  struct mesym *aa,*bb;

  aa=(struct mesym *)a;
  bb=(struct mesym *)b;
  return aa->value - bb->value;
}

/* This is used to sort the vector for the flat profile.
   if they used different amounts of time, the one with
   the most time goes first.  If they used the same amount,
   the one that was called most goes first.  If they were
   called the same #, they are sorted alphabetically */

int
#ifdef __STDC__
timecmp(void *a,void *b)
#else
timecmp(a,b)
void *a;
void *b;
#endif
{
  struct mesym *aa,*bb;
  int	n;

  aa= *(struct mesym **)a;
  bb= *(struct mesym **)b;
  n = bb->histo - aa->histo;
  if(n==0) n=bb->ncalled - aa->ncalled;
  if(n==0) n=strcmp(aa->name,bb->name);
  return n;
}

/* This is used to sort the functions_in_call_tree[] vector
   so the leaf nodes are at the end, and the root nodes
   are at the beginning.  This bit about useless nodes could
   probably be taken out now, since they shouldn't make it
   into the vector in the first place */

int
#ifdef __STDC__
callcmp(void *a,void *b)
#else
callcmp(a,b)
void *a;
void *b;
#endif
{
  struct mesym *aa,*bb;

  aa= *(struct mesym **)a;
  bb= *(struct mesym **)b;

  /* Send useless symbols to the end */
  if(aa->ncalls==0 && aa->ncalled==0) {
    if(bb->ncalls==0 && bb->ncalled==0)
      return EQ;
    return GT;
  }
  if(bb->ncalled==0 && bb->ncalls==0)
    return LT;

  /* 	send root nodes to the front; */
  /* Functions that were called 0 times
     are spontaneous */

  if(aa->ncalled==0) {
    if(bb->ncalled==0)
      return EQ;
    return LT;
  }
  if(bb->ncalled==0)
    return GT;

  /* Send leaf nodes to the end */
  if(aa->ncalls==0) {
    if(bb->ncalls==0)
      return EQ;
    return GT;
  }
  if(bb->ncalls==0)
    return LT;

  /* And keep the rest the same */
  return EQ;
}

/* This is used to sort functions_in_call_tree[] before printing.
   To print, we want
   A:	the ones with the most time first
   	a:	(If one was never called, put it first, 'cuz it
		was invoked by GOD, else the one called the most
 		# of times first)
		1:  the one with the lower name (strcmp()) first
 */

int
#ifdef __STDC__
treetimecmp(void *a,void *b)
#else
treetimecmp(a,b)
void *a;
void *b;
#endif
{
  struct mesym *aa,*bb;
  int	n;

  aa= *(struct mesym **)a;
  bb= *(struct mesym **)b;
  n = (bb->histo+bb->sub_histo) - (aa->histo+aa->sub_histo);
  if(n==0) {

    /* Just to be bizarre:  If one was never called,
       put it first, else put the one who was called
       the most first.  Confused yet? */
    if(aa->ncalled==0)
      return LT;
    if(bb->ncalled==0)
      return GT;
    n=bb->ncalled - aa->ncalled;
    if(n==0)
      n=strcmp(aa->name,bb->name);
  }
  return n;
}

/* Read in a gmon.out file named NAME and deal with the stuff inside it.  */

void
#ifdef __STDC__
readgm(char *name)
#else
readgm(name)
char *name;
#endif
{
  FILE *fp;
  struct gm_header tmp;
  unsigned CHUNK *p;
  int	n;
  struct gm_call calltmp;

  PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_GFILE,("gmon from `%s'",name));

  fp=ck_fopen(name,"r");

  /* Read in the gmon file header and check that histogram is
     compatible with the other gmon files already read.  */

  ck_fread((void *)&tmp,sizeof(tmp),1,fp);
  if(hdr.low) {
    if(hdr.low!=tmp.low || hdr.high!=tmp.high)
      fatal("file `%s' is incompatable with previous gmon.out file",name);
  } else
    hdr=tmp;

  PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_GFILE,("lowpc=%ld  hipc=%ld  nbytes= %ld",hdr.low,hdr.high,hdr.nbytes));

  /* Allocate the total histogram if we haven't yet done so.  */

  if(!histo) {
    nhist=(hdr.nbytes-sizeof(struct gm_header))/sizeof(CHUNK);
    histo=(unsigned CHUNK *)ck_calloc(nhist,sizeof(unsigned CHUNK));
  }

  /* Read this file's histogram and merge it into the total one.  */

  p=(unsigned CHUNK *)ck_malloc(nhist*sizeof(unsigned CHUNK));
  ck_fread((void *)p,sizeof(unsigned CHUNK),nhist,fp);
  for(n=0;n<nhist;n++) {
    tothist+=p[n];
    histo[n]+=p[n];
    if(debug&DB_GFILE) {
      static ncol;

      if(n==0) ncol=0;
      if(histo[n]) {
	fprintf(stderr,"s[%05d]=%-3d",n,histo[n]);
	if(ncol++%10==9) fputc('\n',stderr);
      }
    }
  }
  free((void *)p);
  PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_GFILE,(""));

  /* We've read in the histogram, now read in the function call data.
     Loop, reading one call-graph edge from the file
     and recording the edge in the graph.  */

  while(fread((void *)&calltmp,sizeof(calltmp),1,fp)==1) {
    struct symlist *tmp;
    struct mesym *s_fm,*s_to;

    /* Find the calling and called functions's symbol entries.  */

    s_fm=val_to_sym(calltmp.from);
    s_to=val_to_sym(calltmp.to);

    if(!s_fm || !s_to) {
      PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_GFILE,("unknown call %#lx to %#lx %d times.",calltmp.from,calltmp.to,calltmp.ncalls));
      continue;
    }

    /* Updated total numbers of calls from this caller and to this callee.  */

    s_to->ncalled+=calltmp.ncalls;
    s_fm->ncalls+=calltmp.ncalls;

    /* Add these calls to the edge between them.  */

    add_to_lists(s_fm,s_to,calltmp.ncalls);

    PRINT_OBNOXIOUS_DEBUG_MESSAGE
      (DB_GFILE,("call %s (%#lx) to %s (%#lx) %ld times",s_fm->name,
		 calltmp.from,s_to->name,calltmp.to,calltmp.ncalls));
  }

  ck_fclose(fp);
}

/* Print one of those obnoxious and vaguely informative blurbs that we know
   and love so well.  Used to be, we'd print them out of a file, but nowaday
   the blurb is encoded direcly into the program.  Makes printing them
   much faster.  Also means bozo syswiz can't accidentally delete/move/etc the
   blurb file on us.  */

void
#ifdef __STDC__
print_blurb(char *blurb)
#else
print_blurb(blurb)
char *blurb;
#endif
{
  if(! no_blurbs)
    fputs(blurb,stdout);
}


/* Find the number of clock ticks/second by reading the kernel's memory.
   This means that if /dev/kmem isn't readable, this program will have to run
   set[ug]id to someone who can read the kernel.   setgid is probably best.  */

long
#ifdef __STDC__
get_ticks(void)
#else
get_ticks()
#endif
{
  static struct nlist nl[2];
  long	ret;
  FILE	*fp;

  nl[0].n_un.n_name="_hz";
  nlist("/vmunix",&nl[0]);
  if(nl[0].n_type==0)
    fatal("Can't find `%s' in namelist of /vmunix",nl[0].n_un.n_name);
  fp=ck_fopen("/dev/kmem","r");
  ck_fseek(fp,(long)nl[0].n_value,0);
  ck_fread((void *)&ret,sizeof(ret),1,fp);
  ck_fclose(fp);
  PRINT_OBNOXIOUS_DEBUG_MESSAGE(DB_MISC,("get_ticks()=%ld",ret));
  return ret;
}


/* Record one -e, -E, -f or -F option in `filters', and check for conflicts.
   All these options are recorded there for processing later
   once the call-graph has been constructed.  */

void
#ifdef __STDC__
add_filter(char *name,int type)
#else
add_filter(name,type)
char *name;
int type;
#endif
{
  if(!filters) {
    filters=(struct filter *)ck_malloc(sizeof(struct filter));
    nfilters=1;
  } else {
    int	n;

    for(n=0;n<nfilters;n++) {
      if(filters[n].name==name || !strcmp(filters[n].name,name))
	fatal("Conflicting options for function `%s'", name);
    }
    nfilters++;
    filters=(struct filter *)ck_realloc((void *)filters,sizeof(struct filter)*nfilters);
  }
  filters[nfilters-1].name=name;
  filters[nfilters-1].type=type;
}

/* Data base associating stdio streams with the file names that are open.  */

struct file_to_name {
	FILE *stream;	/* A stdio stream */
	char *name;	/* The file name (malloc'd specially for this list) */
	struct file_to_name *next;
};

struct file_to_name *files_to_names;

/* Given a stdio stream, look it up in the data base and return
   the file name.  */

char *
#ifdef __STDC__
stream_name(FILE *stream)
#else
stream_name(stream)
FILE *stream;
#endif
{
  struct file_to_name *tail;

  for (tail = files_to_names; tail; tail = tail->next)
    if (tail->stream == stream)
      return tail->name;

  return "unknown file";
}

/* Open a file like `fopen', but report a fatal error if it fails;
   if it succeeds, record the stream and filename in the data base of such.  */

FILE *
#ifdef __STDC__
ck_fopen(char *name,char *mode)
#else
ck_fopen(name,mode)
char *name;
char *mode;
#endif
{
  FILE *stream;
  int n;
  struct file_to_name *new;

  stream=fopen(name,mode);
  if(stream==(FILE *)0)
    fatal_io ("Couldn't open", name);

  new = ck_malloc (sizeof (struct file_to_name));
  new->name = ck_malloc (strlen (name) + 1);
  strcpy (new->name, name);
  new->stream = stream;
  new->next = files_to_names;
  files_to_names = new;
  return stream;
}

/* Interfaces to various functions of stdio
   which use the stream/filename database to print an error message
   if the function gets an error.  */

void
#ifdef __STDC__
ck_fseek(FILE *stream,long i,int w)
#else
ck_fseek(stream,i,w)
FILE *stream;
long i;
int w;
#endif
{
  if(fseek(stream,i,w)==-1)
    fatal_io ("Couldn't lseek", stream_name(stream));
}

void
#ifdef __STDC__
ck_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
#else
ck_fread(ptr, size, nmemb, stream)
void *ptr;
size_t size;
size_t nmemb;
FILE *stream;
#endif
{
  if(fread(ptr,size,nmemb,stream)!=nmemb)
    fatal_io ("Couldn't read", stream_name(stream));
}

void
#ifdef __STDC__
ck_fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
#else
ck_fwrite(ptr, size, nmemb, stream)
void *ptr;
size_t size;
size_t nmemb;
FILE *stream;
#endif
{
  if(fwrite(ptr,size,nmemb,stream)!=nmemb)
    fatal_io ("Couldn't write", stream_name(stream));
}

void
#ifdef __STDC__
ck_fclose(FILE *stream)
#else
ck_fclose(stream)
FILE *stream;
#endif
{
  struct file_to_name *tail;

  fflush (stream);
  if(ferror (stream))
    fatal ("I/O error on `%s'", stream_name (stream));
  if(fclose(stream)==EOF)
    fatal_io ("Couldn't close", stream_name(stream));

  if (files_to_names && files_to_names->stream == stream)
    files_to_names = files_to_names->next;
  else
    for (tail = files_to_names; tail->next; tail = tail->next)
      if (tail->next->stream == stream)
	{
	  struct file_to_name *loser = tail->next;
	  tail->next = tail->next->next;
	  free (loser->name);
	  free (loser);
	  return;
	}
}

/* Report a fatal error doing I/O, and exit.
   PROBLEM is "Couldn't whatever" and NAME is the file name.  */

void
#ifdef __STDC__
fatal_io (char *problem, char *name)
#else
fatal_io (problem, name)
char *problem;
char *name;
#endif
{
  fprintf (stderr, "%s: %s %s:", myname, problem, name);
  perror (0);
  exit (1);
}

/* Report a fatal error and exit. Arguments like `printf'.  */

void
#ifdef __STDC__
fatal(char *s,...)
{
  va_list iggy;

  va_start(iggy,s);
  fprintf(stderr,"%s:",myname);
  vfprintf(stderr,s,iggy);
  putc('\n',stderr);
  va_end(iggy);

  exit (1);
}
#else
fatal(s,va_alist)
char *s;
va_dcl
{
	va_list iggy;

	va_start(iggy);
	fprintf(stderr,"%s:",myname);
	vfprintf(stderr,s,iggy);
	/* _doprnt(s,iggy,stderr); */
	putc('\n',stderr);
	va_end(iggy);
	exit(1);
}
#endif

/* Memory allocation functions.  */

/* This function is called just like `printf'
   except that the output is put into a new string
   allocated with `malloc'.
   Returns the address of the string.  The caller must free the string.  */

char *
#ifdef __STDC__
mk_sprintf(char *str,...)
{
  char tmpbuf[2048];
  char *ret;
  va_list iggy;

  va_start(iggy,str);
  vsprintf(tmpbuf,str,iggy);
  va_end(iggy);

  ret=ck_malloc(strlen(tmpbuf)+1);
  strcpy(ret,tmpbuf);
  return ret;
}
#else
mk_sprintf(str,va_alist)
char *str;
va_dcl
{
  char tmpbuf[2048];
  char *ret;
  va_list iggy;

  va_start(iggy);
  vsprintf(tmpbuf,str,iggy);
  va_end(iggy);

  ret=ck_malloc(strlen(tmpbuf)+1);
  strcpy(ret,tmpbuf);
  return ret;
}
#endif

/* Encapsulations of `malloc', `calloc' and `realloc'
   that cause fatal errors if there is not enough memory.  */

void *
#ifdef __STDC__
ck_malloc(size_t size)
#else
ck_malloc(size)
size_t size;
#endif
{
  void *ret;
  void *malloc();

  ret=malloc(size);
  if(ret==(void *)0)
    fatal ("Virtual memory exhausted");
  return ret;
}

void *
#ifdef __STDC__
ck_calloc(size_t nmemb,size_t size)
#else
ck_calloc(nmemb,size)
size_t nmemb;
size_t size;
#endif
{
  void *ret;
  void *calloc();

  ret=calloc(nmemb,size);
  if(ret==(void *)0)
    fatal ("Virtual memory exhausted");
  return ret;
}

void *
#ifdef __STDC__
ck_realloc(void *ptr,size_t size)
#else
ck_realloc(ptr,size)
void *ptr;
size_t size;
#endif
{
  void *ret;
  void *realloc();

  ret=realloc(ptr,size);
  if(ret==(void *)0)
    fatal ("Virtual memory exhausted");
  return ret;
}

/* Implement an expandable fifo buffer of pointers
   (we don't care what they point to)
   which is used for conducting breadth-first tree walks in the call graph.

   The fifo is implemented as a ring buffer.  */

struct ring_buf {
	void **buf;
	int size;
	void **push_to_here;
	void **pop_frm_here;
};

void *
#ifdef __STDC__
init_ring_buffer(void)
#else
init_ring_buffer()
#endif
{
	struct ring_buf *ret;

	ret=ck_malloc(sizeof(struct ring_buf));
	ret->size=40;
	ret->buf=(void **)ck_calloc(ret->size,sizeof(void **));
	ret->push_to_here=ret->buf;
	ret->pop_frm_here=ret->buf;
	return ret;
}

void
#ifdef __STDC__
push_ring_buffer(void *b,void *n)
#else
push_ring_buffer(b,n)
void *b;
void *n;
#endif
{
	struct ring_buf *buf;

	buf=(struct ring_buf *)b;
	if(buf->push_to_here+1==buf->pop_frm_here || (buf->pop_frm_here==buf->buf && buf->push_to_here==buf->buf+(buf->size-1))) {
		int f,t;

		f=buf->pop_frm_here-buf->buf;
		t=buf->push_to_here-buf->buf;
		buf->size*=2;
		buf->buf=ck_realloc((void *)buf->buf,buf->size*sizeof(void **));
		buf->pop_frm_here=buf->buf+f;
		if(t==0)
			buf->push_to_here=buf->buf+buf->size/2;
		else
			buf->push_to_here=buf->buf+t;
	}
	*(buf->push_to_here)=n;
	buf->push_to_here++;
	if(buf->push_to_here==buf->buf+buf->size)
		buf->push_to_here=buf->buf;
}

void *
#ifdef __STDC__
pop_ring_buffer(void *b)
#else
pop_ring_buffer(b)
void *b;
#endif
{
	struct ring_buf *buf;
	void *ret;

	buf=(struct ring_buf *)b;
	ret= *(buf->pop_frm_here);
	buf->pop_frm_here++;
	if(buf->pop_frm_here==buf->buf+buf->size)
		buf->pop_frm_here=buf->buf;
	return ret;
}

int
#ifdef __STDC__
ring_buffer_isnt_empty(void *b)
#else
ring_buffer_isnt_empty(b)
void *b;
#endif
{
	struct ring_buf *buf;

	buf=(struct ring_buf *)b;
	return buf->pop_frm_here!=buf->push_to_here;
}

void
#ifdef __STDC__
flush_ring_buffer(void *b)
#else
flush_ring_buffer(b)
void *b;
#endif
{
	struct ring_buf *buf;

	buf=(struct ring_buf *)b;
	free(buf->buf);
	free(buf);
}


/* Functions to print debugging messages.  */

/* Like vprintf but print an extra newline at the end.  */

void
#ifdef __STDC__
dbgprintf(char *s,...)
{
  va_list iggy;

  va_start(iggy,s);
  vfprintf(stderr,s,iggy);
  putc('\n',stderr);
  va_end(iggy);
}
#else
dbgprintf(s,va_alist)
char *s;
va_dcl
{
  va_list iggy;

  va_start(iggy);
  vfprintf(stderr,s,iggy);
  putc('\n',stderr);
  va_end(iggy);
}
#endif

/* Print out the entire symbol table */

void
#ifdef __STDC__
dumpsyms(void)
#else
dumpsyms()
#endif
{
  struct mesym *np,*endp;
  struct symlist *sy;
  int n;
  char buf[80];

  n=0;
  for(np=syms,endp= &syms[nsym];np<endp;np++) {
    sprintf(buf,"%-15s %#6lx %d %2d.%2d[%d]",
	    np->name, np->value,np->histo,np->ncalled,np->ncalls,np->cycnum);
    if(n==0) {
      printf("%-35s",buf);
      n++;
    } else {
      printf("%s\n",buf);
      n=0;
    }
  }
  putchar('\n');
  if(n==0) putchar('\n');
}

/* Print out the call tree vector */
void
#ifdef __STDC__
dumpfuns(void)
#else
dumpfuns()
#endif
{
  struct mesym *np;
  struct symlist *sy;
  int	n;

  for(n=0;n<number_of_functions_in_call_tree;n++) {
    np=functions_in_call_tree[n];
    printf("%s{%d}[%d]  ",np->name,np->cycnum,np->ncalls);
    /* for(sy=np->called;sy;sy=sy->next)
       printf("%s{%d}(%d) ",sy->sym->name,sy->sym->cycnum,sy->ncalls);
       putchar('\n'); */
  }
}

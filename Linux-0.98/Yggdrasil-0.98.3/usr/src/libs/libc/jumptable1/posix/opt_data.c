#ifndef NO_JUMP_TABLE
char *optarg = 0;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns EOF, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

int optind = 0;

/* Callers store zero here to inhibit the error message
   for unrecognized options.  */

int opterr = 1;

#endif

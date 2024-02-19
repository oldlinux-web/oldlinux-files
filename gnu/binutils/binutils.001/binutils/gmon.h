/* Format of gmon.out file.  */

/* This header appears at the beginning of the gmon.out file.
   LOW and HIGH are low and high water marks for the program counter
   during the creation of the gmon.out file.
   LOW is also the offset where the histogram table starts in the
   text (code) segment.
   NBYTES is the number of bytes in this header plus the histogram itself,
   which immediately follows the header in the file.

   Therefore, the number of histogram entries is
   (NBYTES - sizeof (struct gm_header)) / (sizeof (CHUNK)).

   Each entry applies to a range of PC values.
   The first entry applies to PC values starting at LOW.
   The last entry applies to PC values ending at HIGH.
   Therefore, the span of each entry's range is
       (HIGH - LOW) / number-of-entries
   Usually this value is 4.
*/

struct gm_header {
	unsigned long low;
	unsigned long high;
	long nbytes;
};

/* Data type of an entry in the PC histogram.  */
#define CHUNK	short

/* After the histogram cone the function call count entries.
   They fill all the rest of the file.
   Each count entry records the number of calls to one function
   from one pc value.

   FROM describes the caller pc, as an offset into the text segment.
   TO is the address of the called function.
   NCALLS is the number of calls counted from FROM to TO.

   Note that if a function A is called from several places in B,
   there are separate call count entries for each call, with different FROM.
   All of them together count the number of calls from B to A.  */

struct gm_call {
	unsigned long from;
	unsigned long to;
	unsigned long ncalls;
};

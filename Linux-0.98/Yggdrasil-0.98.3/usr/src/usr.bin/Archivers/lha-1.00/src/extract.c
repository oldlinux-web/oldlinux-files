/***********************************************************
	extract.c -- extract file from archive
***********************************************************/
#include "lharc.h"
#include "intrface.h"
extern int verify_mode;

int decode_lzhuf (infp, outfp, original_size, packed_size,name , method)
FILE *infp;
FILE *outfp;
long original_size;
long packed_size;
char *name;
int method;
{
  interface.method = method;
  interface.dicbit = 13; /* method + 8; */
  interface.infile = infp;
  interface.outfile = outfp;
  interface.original = original_size;
  interface.packed = packed_size;

  switch (method) {
  case 0:
  case 8:
    start_indicator (name, original_size
		     , verify_mode ? "Testing " : "Melting ",2048);
    copyfile(infp, (verify_mode ? NULL : outfp), original_size, 2);
    break;
  case 6:	/* -lzs- */
    interface.dicbit = 11;
    start_indicator (name, original_size
		     , verify_mode ? "Testing " : "Melting "
		     , 1<<interface.dicbit);
    decode(&interface);
    break;
  case 1: /* -lh1- */
  case 4: /* -lh4- */
  case 7: /* -lz5- */
    interface.dicbit = 12;
  default:
    start_indicator (name, original_size
		     , verify_mode ? "Testing " : "Melting "
		     , 1<<interface.dicbit);
    decode(&interface);
  }
  finish_indicator (name, verify_mode ? "Tested  " : "Melted  ");
  return crc;
}

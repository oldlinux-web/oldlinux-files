/*
 * This is for the global data which are put in the text section of
 * the shared image. We do this is for putting the non global part after
 * the global data. Please don't change the order of these files.
 *
 * For the first part of jump table/global data of the library:
 *
 *	section		content
 *----------------------------------
 * 	text		jumptable
 *			global data in text section
 *			constants pointed to by global data in text
 *				section
 *		GAPS
 *	data		global data in data section
 *			
 */

#include "inet/herrlist.S"
#include "string/errlist.S"
#include "string/siglist.S"

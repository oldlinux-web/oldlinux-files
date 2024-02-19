/* mgr.h - the following defs are taken from the MGR header file lib/dump.h
*/

#ifndef _MGR_H_
#define _MGR_H_

struct old_b_header {
   char magic[2];
   char h_wide;
   char l_wide;
   char h_high;
   char l_high;
   };

struct b_header {
   char magic[2];
   char h_wide;
   char l_wide;
   char h_high;
   char l_high;
   char depth;
   char _reserved;
   };

#endif /*_MGR_H_*/

/* This file holds all knowledge of partition ID codes.
 * Thanks to leendert@cs.vu.nl (Leendert van Doorn) for
 * collecting most of this information.
 */

#define extern
#include "syscodes.h"
#undef  extern

/* Note that my boot program menu can only use the first 8 characters
 * of these names.  The colon in the nineth position shows where the
 * first truncated char is.  (There's not much room in the bootblock!)
 * changed sysCodes[] below, adding SIZE tms */
struct intString sysCodes[SIZE] = {
{ 0x01, "DOS12  :12-bit FAT" },
{ 0x02, "XENIX   :root" },
{ 0x03, "XENIX   :usr" },
{ 0x04, "DOS16  :16-bit FAT" },
{ 0x05, "DOSex :DOS 3.3 extended volume" },
{ 0x06, "DOSbi :DOS 4.0 large volume" },
{ 0x07, "OS/2    :OS/2 (or QNX or Adv. UNIX...)" },
{ 0x08, "AIX     :file system" },
{ 0x09, "AIXbt:boot partition" },

{ 0x10, "OPUS    :?" },
{ 0x40, "VENIX   :Venix 80286" },
{ 0x51, "NOVEL  :?" },
{ 0x52, "CPM     :?" },
{ 0x63, "UNIX    :System V/386" },
{ 0x64, "NOVEL  :?" },
{ 0x75, "PC/IX   :?" },
{ 0x80, "Minix   :Minix (ver. 1.4a and earlier)" },
{ 0x81, "Minix   :Minix (ver. 1.4b and later)" },
{ 0x93, "Ameba  :Amoeba file system" },
{ 0x94, "Ameba  :Amoeba bad block table?" },
{ 0xDB,	"C.DOS   :Concurrent DOS" },

/* { 0xF2, "DOS-2nd :DOS 3.3+ second partition" }, */
/* { 0xFF, "BAD-TRK :Bad track table?" }, */

/* Make sure this is last! */
{    0, "empty" }
};

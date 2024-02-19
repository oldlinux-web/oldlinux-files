/*===========================================================================

        KEYBOARD.C for MSDOS and OS/2

        extcode()

        This routine is common to the MSDOS and OS/2 implementations.
        It is used to resolve extended character codes from the keyboard
        into EMACS printable character specifications.

        This implementation can handle the extended AT-style keyboard
        if one is fitted.

        I don't know what happens on an XT but I suspect that it should
        work as if there were no extended keys.

                                Jon Saxton
                                24 Jan 1990

===========================================================================*/  

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#if MSDOS | OS2
#if HP150 == 0

int extcode(c)

unsigned c;	/* byte following a zero extended char byte */
		/* High order normally contains 0x00 but may
		   contain 0xE0 if an AT-style keyboard is attached */
{
	/* function keys 1 through 9.  High-order will be 0x00 */
	if (c >= 0x3B && c < 0x44)
		return(SPEC | c - 0x3A + '0');

	/* function key 10 */
	if (c == 0x44)
		return(SPEC | '0');

	/* shifted function keys */
	if (c >= 0x54 && c < 0x5D)
		return(SPEC | SHFT | c - 0x53 + '0');
	if (c == 0x5D)
		return(SPEC | SHFT | '0');

	/* control function keys */
	if (c >= 0x5E && c < 0x67)
		return(SPEC | CTRL | c - 0x5D + '0');
	if (c == 0x67)
		return(SPEC | CTRL | '0');

	/* ALTed function keys */
	if (c >= 0x68 && c < 0x71)
		return(SPEC | ALTD | c - 0x67 + '0');
	if (c == 0x71)
		return(SPEC | ALTD | '0');

	/* ALTed number keys */
	/* This doesn't work for DOS or OS/2.  Using ALT in conjunction with
	   the number keys lets you enter any (decimal) character value
	   you want.  It is therefore commented out.

	   Wrongo joker... alting the top row of numbers works fine DML */
	if (c >= 0x78 && c < 0x81)
		return(ALTD | c - 0x77 + '0');
	if (c == 0x81)
		return(ALTD | '0');

	/* some others as well */
	switch (c) {

case 3:		return(0);			/* null */
case 0x0F:	return(SHFT | CTRL | 'I');	/* backtab */

case 0x10:	return(ALTD | 'Q');
case 0x11:	return(ALTD | 'W');
case 0x12:	return(ALTD | 'E');
case 0x13:	return(ALTD | 'R');
case 0x14:	return(ALTD | 'T');
case 0x15:	return(ALTD | 'Y');
case 0x16:	return(ALTD | 'U');
case 0x17:	return(ALTD | 'I');
case 0x18:	return(ALTD | 'O');
case 0x19:	return(ALTD | 'P');

case 0x1E:	return(ALTD | 'A');
case 0x1F:	return(ALTD | 'S');
case 0x20:	return(ALTD | 'D');
case 0x21:	return(ALTD | 'F');
case 0x22:	return(ALTD | 'G');
case 0x23:	return(ALTD | 'H');
case 0x24:	return(ALTD | 'J');
case 0x25:	return(ALTD | 'K');
case 0x26:	return(ALTD | 'L');

case 0x2C:	return(ALTD | 'Z');
case 0x2D:	return(ALTD | 'X');
case 0x2E:	return(ALTD | 'C');
case 0x2F:	return(ALTD | 'V');
case 0x30:	return(ALTD | 'B');
case 0x31:	return(ALTD | 'N');
case 0x32:	return(ALTD | 'M');

case 0x47:	return(SPEC | '<');	/* home */
case 0x48:	return(SPEC | 'P');	/* cursor up */
case 0x49:	return(SPEC | 'Z');	/* page up */
case 0x4B:	return(SPEC | 'B');	/* cursor left */
case 0x4C:	return(SPEC | 'L');	/* NP 5 */
case 0x4D:	return(SPEC | 'F');	/* cursor right */
case 0x4F:	return(SPEC | '>');	/* end */
case 0x50:	return(SPEC | 'N');	/* cursor down */
case 0x51:	return(SPEC | 'V');	/* page down */
case 0x52:	return(SPEC | 'C');	/* insert */
case 0x53:	return(SPEC | 'D');	/* delete */

case 0x73:	return(SPEC | CTRL | 'B');	/* control left */
case 0x74:	return(SPEC | CTRL | 'F');	/* control right */
case 0x75:	return(SPEC | CTRL | '>');	/* control end */
case 0x76:	return(SPEC | CTRL | 'V');	/* control page down */
case 0x77:	return(SPEC | CTRL | '<');	/* control home */
case 0x84:	return(SPEC | CTRL | 'Z');	/* control page up */
case 0x8D:	return(SPEC | CTRL | 'P');	/* control up */
case 0x8F:	return(SPEC | CTRL | 'L');	/* control NP5 */
case 0x91:	return(SPEC | CTRL | 'N');	/* control down */
case 0x92:	return(SPEC | CTRL | 'C');	/* control grey insert */
case 0x93:	return(SPEC | CTRL | 'D');	/* control grey delete */

case 0x82:	return(ALTD | '-');	/* alt - */
case 0x83:	return(ALTD | '=');	/* alt = */
case 0x27:	return(ALTD | ';');	/* alt ; */
case 0x28:	return(ALTD | '\'');	/* alt ' */
case 0x2B:	return(ALTD | '\\');	/* alt \ */
case 0x1A:	return(ALTD | '[');	/* alt [ */
case 0x1B:	return(ALTD | ']');	/* alt ] */

#if ATKBD | OS2

/* F11 and F12 */

case 0x85:	return(SPEC | '-');	 	/* F11 */
case 0x86:	return(SPEC | '=');		/* F12 */
case 0x87:	return(SPEC | SHFT | '-');	/* shift F11 */
case 0x88:	return(SPEC | SHFT | '=');	/* shift F12 */
case 0x89:	return(SPEC | CTRL | '-');	/* control F11 */
case 0x8A:	return(SPEC | CTRL | '=');	/* control F12 */
case 0x8B:	return(SPEC | ALTD | '-');	/* alt F11 */
case 0x8C:	return(SPEC | ALTD | '=');	/* alt F12 */

/*
   This implementation distinguishes between the cursor controls on the
   number pad and those on the grey keypad if an AT-style keyboard is
   fitted.
*/ 

case 0xE047:	return(SPEC | 'a');		/* grey home */
case 0xE048:	return(SPEC | 'b');		/* grey cursor up */
case 0xE049:	return(SPEC | 'c');		/* grey page up */
case 0xE04B:	return(SPEC | 'd');		/* grey cursor left */
case 0xE04C:	return(SPEC | 'e');		/* grey center key */
case 0xE04D:	return(SPEC | 'f');		/* grey cursor right */
case 0xE04F:	return(SPEC | 'g');		/* grey end */
case 0xE050:	return(SPEC | 'h');		/* grey cursor down */
case 0xE051:	return(SPEC | 'i');		/* grey page down */
case 0xE052:	return(SPEC | 'j');		/* grey insert */
case 0xE053:	return(SPEC | 'k');		/* grey delete */

case 0xE077:	return(SPEC | CTRL | 'a');	/* control grey home */
case 0xE08D:	return(SPEC | CTRL | 'b');	/* control grey up */
case 0xE084:	return(SPEC | CTRL | 'c');	/* control grey page up */
case 0xE073:	return(SPEC | CTRL | 'd');	/* control grey left */
case 0xE074:	return(SPEC | CTRL | 'f');	/* control grey right */
case 0xE075:	return(SPEC | CTRL | 'g');	/* control grey end */
case 0xE091:	return(SPEC | CTRL | 'h');	/* control grey down */
case 0xE076:	return(SPEC | CTRL | 'i');	/* control grey page down */
case 0xE092:	return(SPEC | CTRL | 'j');	/* control grey insert */
case 0xE093:	return(SPEC | CTRL | 'k');	/* control grey delete */

case 0xE097:	return(SPEC | ALTD | 'a');	/* alt grey home */
case 0xE098:	return(SPEC | ALTD | 'b');	/* alt grey cursor up */
case 0xE099:	return(SPEC | ALTD | 'c');	/* alt grey page up */
case 0xE09B:	return(SPEC | ALTD | 'd');	/* alt grey cursor left */
case 0xE09D:	return(SPEC | ALTD | 'f');	/* alt grey cursor right */
case 0xE09F:	return(SPEC | ALTD | 'g');	/* alt grey end */
case 0xE0A0:	return(SPEC | ALTD | 'h');	/* alt grey cursor down */
case 0xE0A1:	return(SPEC | ALTD | 'i');	/* alt grey page down */
case 0xE0A2:	return(SPEC | ALTD | 'j');	/* alt grey insert */
case 0xE0A3:	return(SPEC | ALTD | 'k');	/* alt grey delete */

case 0x97:	return(SPEC | ALTD | 'a');	/* alt grey home */
case 0x98:	return(SPEC | ALTD | 'b');	/* alt grey cursor up */
case 0x99:	return(SPEC | ALTD | 'c');	/* alt grey page up */
case 0x9B:	return(SPEC | ALTD | 'd');	/* alt grey cursor left */
case 0x9D:	return(SPEC | ALTD | 'f');	/* alt grey cursor right */
case 0x9F:	return(SPEC | ALTD | 'g');	/* alt grey end */
case 0xA0:	return(SPEC | ALTD | 'h');	/* alt grey cursor down */
case 0xA1:	return(SPEC | ALTD | 'i');	/* alt grey page down */
case 0xA2:	return(SPEC | ALTD | 'j');	/* alt grey insert */
case 0xA3:	return(SPEC | ALTD | 'k');	/* alt grey delete */

case 0xA6:	return(SPEC | ALTD | 'l');	/* alt grey enter */
case 0xA4:	return(SPEC | ALTD | '/');	/* alt grey / */
case 0x37:	return(SPEC | ALTD | '*');	/* alt grey * */
case 0x4A:	return(SPEC | ALTD | '-');	/* alt grey - */
case 0x4E:	return(SPEC | ALTD | '+');	/* alt grey + */

case 0x95:	return(SPEC | CTRL | '/');	/* ctrl grey / */
case 0x96:	return(SPEC | CTRL | '*');	/* ctrl grey * */
case 0x8E:	return(SPEC | CTRL | '-');	/* ctrl grey - */
case 0x90:	return(SPEC | CTRL | '+');	/* ctrl grey + */

#endif

	}
/* printf("[ALT %d] ", c); */

	return(ALTD | c);
}

#endif
#endif

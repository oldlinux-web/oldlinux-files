/*
 *  SMG terminal driver for VMS.
 *
 *  Consults the system's terminal tables for both DEC terminals,
 *  foreign and user defined terminals.
 *
 *  Author:  Curtis Smith
 *  Update history:
 *	14-Jul-1987, first revision.
 *	02-Jan-1988, by J. A. Lomicka: Code addition for interpreting
 *		LK201 function keys, application keypad and cursor
 *		position reports.
 *	09-Apr-1988, second revision.
 *		Major changes:
 *		1) H files removed; replaced with globalvalues.
 *		2) Terminal now left in ECHO mode. Read call
 *		disables echo.
 *		3) TYPAHD macro now performs correctly.
 *		4) $sres variable now accepts NORMAL and WIDE.
 *		5) Writes to screen now use QIO without waiting.
 *		This gives a slight increase in performance.
 *		Had to make some QIOW a different event flag number.
 *		6) Function keys, special keys and arrow keys
 *		are now bound using the following table:
 *			FNN-DOWN			FNS-E1
 *			FNB-LEFT			FNC-E2
 *			FNF-RIGHT			FND-E3
 *			FNP-UP				FN@-E4
 *			FN^1-PF1			FNZ-E5
 *			FN^2-PF2			FNV-E6
 *			FN^3-PF3
 *			FN^4-PF4
 *
 *			A-0 thru A-9: Keypad 0 thru 9
 *			A-E: Enter
 *			A-., A-, : Keypad . and ,
 *
 *			FN1 thru FN0 : Function keys 1 thru 10
 *			S-FN1 thru S-FN0 : Function keys 11 thru 20
 *			
 *		See ebind.h for key bindings.
 *	2-dec-88 Curtis Smith
 *	- These have been rebound to the new machine independant bindings
 *	27-March-1989	J.A.Lomicka	Separate VMS.C from SMG.C,
 *					This file is now SMG.C, and only
 *					contains the SMG stuff.
 *
 *
 *	17-jul-89 : mike ward
 *		Modifications for proper operation of special keys. Basically,
 *		the changes are to deal with key values as ints rather rather
 *		than characters so that when a character sequence is recognized
 *		as a special key, the approriate value can be substituted for
 *		the sequence directly (in inbuf).
 *
 *		1)	Use "int" rather than "char" to declare inbuf[],
 *			*inbuft, and *inbufh since keys use more than 8-bits
 *		2)	Change the parameter declaration "char ch" to "int ch"
 *			in smgqin since we queue key values (not characters)
 *		3) 	Change calls "smgqin(ch)" to "smgqin((unsigned char)ch)"
 *			in smggcook since we don't want sign extension
 *		4)	Remove call "smgqin(0)" in smggcook. Instead of queuing
 *			a sequence of chars to represent the key value, just
 *			queue the final value instead - much cleaner.
 *		5)	Change the declaration from "char ch" to "int ch" in
 *			smggetc since ch now holds a key value rather than a
 *			character.
 *	11-dec-89:	Kevin A. Mitchell
 *		Don't restore numeric keypad if the user had it set that way
 *		in DCL.
 */
 
/** Standard include files **/
#include <stdio.h>			/* Standard I/O package		*/
#include "estruct.h"			/* Emacs' structures		*/

/*
	Empty routine make some compilers happy when SMG is not defined,
	and is also used as a noop routine in the terminal dispatch table.
*/
smg_noop()
{
}

#if	SMG

#include "eproto.h"
#include "edef.h"			/* Emacs' definitions		*/
#include "elang.h"
#include smgdef
#include ssdef
#include descrip
#include string
#include ttdef
#include tt2def

/*
	SMG constants from $$smgtrmptrdef
*/
#define	SMG$K_ADVANCED_VIDEO		1
#define	SMG$K_ANSI_CRT			2
#define	SMG$K_AUTO_MARGIN		3
#define	SMG$K_BACKSPACE			4
#define	SMG$K_BLOCK_MODE		5
#define	SMG$K_DEC_CRT			6
#define	SMG$K_EDIT			7
#define	SMG$K_EIGHT_BIT			8
#define	SMG$K_FULLDUP			9
#define	SMG$K_IGNORE_NEWLINE		10
#define	SMG$K_INSERT_MODE_NULLS		11
#define	SMG$K_LOWERCASE			12
#define	SMG$K_NO_ERASE			13
#define	SMG$K_NO_SCROLL			14
#define	SMG$K_OVERSTRIKE		15
#define	SMG$K_PRINTER_PORT		16
#define	SMG$K_REGIS			17
#define	SMG$K_SCOPE			18
#define	SMG$K_SIXEL_GRAPHICS		19
#define	SMG$K_SOFT_CHARACTERS		20
#define	SMG$K_PHYSICAL_TABS		21
#define	SMG$K_PHYSICAL_FF		22
#define	SMG$K_UNDERLINE			23
#define	SMG$K_CURSOR_REPORT_ANSI	24
#define	SMG$K_DEC_CRT_2			25
#define	SMG$K_DEC_CRT_3			26
#define	SMG$K_SET_CURSOR_COL_ROW	27
#define	SMG$K_PRIVATE_BOO_1		211
#define	SMG$K_PRIVATE_BOO_2		212
#define	SMG$K_PRIVATE_BOO_3		213
#define	SMG$K_PRIVATE_BOO_4		214
#define	SMG$K_PRIVATE_BOO_5		215
#define	SMG$K_PRIVATE_BOO_6		216
#define	SMG$K_PRIVATE_BOO_7		217
#define	SMG$K_PRIVATE_BOO_8		218
#define	SMG$K_PRIVATE_BOO_9		219
#define	SMG$K_PRIVATE_BOO_10		220
#define	SMG$K_MAX_BOOLEAN_CODE		220
#define	SMG$K_COLUMNS			221
#define	SMG$K_CR_FILL			222
#define	SMG$K_FRAME			223
#define	SMG$K_LF_FILL			224
#define	SMG$K_NUMBER_FN_KEYS		225
#define	SMG$K_ROWS			226
#define	SMG$K_VMS_TERMINAL_NUMBER	227
#define	SMG$K_WIDE_SCREEN_COLUMNS	228
#define	SMG$K_PRIVATE_NUM_1		431
#define	SMG$K_PRIVATE_NUM_2		432
#define	SMG$K_PRIVATE_NUM_3		433
#define	SMG$K_PRIVATE_NUM_4		434
#define	SMG$K_PRIVATE_NUM_5		435
#define	SMG$K_PRIVATE_NUM_6		436
#define	SMG$K_PRIVATE_NUM_7		437
#define	SMG$K_PRIVATE_NUM_8		438
#define	SMG$K_PRIVATE_NUM_9		439
#define	SMG$K_PRIVATE_NUM_10		440
#define	SMG$K_MAX_NUMERIC_CODE		440
#define	SMG$K_BEGIN_ALTERNATE_CHAR	441
#define	SMG$K_BEGIN_BLINK		442
#define	SMG$K_BEGIN_BOLD		443
#define	SMG$K_BEGIN_DELETE_MODE		444
#define	SMG$K_BEGIN_INSERT_MODE		445
#define	SMG$K_BEGIN_LINE_DRAWING_CHAR	446
#define	SMG$K_BEGIN_REVERSE		447
#define	SMG$K_BEGIN_UNDERSCORE		448
#define	SMG$K_BOTTOM_T_CHAR		449
#define	SMG$K_CLEAR_TAB			450
#define	SMG$K_CROSS_CHAR		451
#define	SMG$K_CURSOR_DOWN		452
#define	SMG$K_CURSOR_LEFT		453
#define	SMG$K_CURSOR_RIGHT		454
#define	SMG$K_CURSOR_UP			455
#define	SMG$K_DARK_SCREEN		456
#define	SMG$K_DELETE_CHAR		457
#define	SMG$K_DELETE_LINE		458
#define	SMG$K_DEVICE_ATTRIBUTES		459
#define	SMG$K_DOUBLE_HIGH_BOTTOM	460
#define	SMG$K_DOUBLE_HIGH_TOP		461
#define	SMG$K_DOUBLE_WIDE		462
#define	SMG$K_DUPLICATE			463
#define	SMG$K_END_ALTERNATE_CHAR	464
#define	SMG$K_END_BLINK			465
#define	SMG$K_END_BOLD			466
#define	SMG$K_END_DELETE_MODE		467
#define	SMG$K_END_INSERT_MODE		468
#define	SMG$K_END_LINE_DRAWING_CHAR	469
#define	SMG$K_END_REVERSE		470
#define	SMG$K_END_UNDERSCORE		471
#define	SMG$K_ERASE_TO_END_DISPLAY	472
#define	SMG$K_ERASE_TO_END_LINE		473
#define	SMG$K_ERASE_WHOLE_DISPLAY	474
#define	SMG$K_ERASE_WHOLE_LINE		475
#define	SMG$K_HOME			476
#define	SMG$K_HORIZONTAL_BAR		477
#define	SMG$K_INIT_STRING		478
#define	SMG$K_INSERT_CHAR		479
#define	SMG$K_INSERT_LINE		480
#define	SMG$K_INSERT_PAD		481
#define	SMG$K_KEY_0			482
#define	SMG$K_KEY_1			483
#define	SMG$K_KEY_2			484
#define	SMG$K_KEY_3			485
#define	SMG$K_KEY_4			486
#define	SMG$K_KEY_5			487
#define	SMG$K_KEY_6			488
#define	SMG$K_KEY_7			489
#define	SMG$K_KEY_8			490
#define	SMG$K_KEY_9			491
#define	SMG$K_KEY_BACKSPACE		492
#define	SMG$K_KEY_COMMA			493
#define	SMG$K_KEY_DOWN_ARROW		494
#define	SMG$K_KEY_E1			495
#define	SMG$K_KEY_E2			496
#define	SMG$K_KEY_E3			497
#define	SMG$K_KEY_E4			498
#define	SMG$K_KEY_E5			499
#define	SMG$K_KEY_E6			500
#define	SMG$K_KEY_ENTER			501
#define	SMG$K_KEY_F1			502
#define	SMG$K_KEY_F2			503
#define	SMG$K_KEY_F3			504
#define	SMG$K_KEY_F4			505
#define	SMG$K_KEY_F5			506
#define	SMG$K_KEY_F6			507
#define	SMG$K_KEY_F7			508
#define	SMG$K_KEY_F8			509
#define	SMG$K_KEY_F9			510
#define	SMG$K_KEY_F10			511
#define	SMG$K_KEY_F11			512
#define	SMG$K_KEY_F12			513
#define	SMG$K_KEY_F13			514
#define	SMG$K_KEY_F14			515
#define	SMG$K_KEY_F15			516
#define	SMG$K_KEY_F16			517
#define	SMG$K_KEY_F17			518
#define	SMG$K_KEY_F18			519
#define	SMG$K_KEY_F19			520
#define	SMG$K_KEY_F20			521
#define	SMG$K_KEY_LEFT_ARROW		522
#define	SMG$K_KEY_MINUS			523
#define	SMG$K_KEY_PERIOD		524
#define	SMG$K_KEY_PF1			525
#define	SMG$K_KEY_PF2			526
#define	SMG$K_KEY_PF3			527
#define	SMG$K_KEY_PF4			528
#define	SMG$K_KEY_RIGHT_ARROW		529
#define	SMG$K_KEY_UP_ARROW		530
#define	SMG$K_LABEL_F1			531
#define	SMG$K_LABEL_F2			532
#define	SMG$K_LABEL_F3			533
#define	SMG$K_LABEL_F4			534
#define	SMG$K_LABEL_F5			535
#define	SMG$K_LABEL_F6			536
#define	SMG$K_LABEL_F7			537
#define	SMG$K_LABEL_F8			538
#define	SMG$K_LABEL_F9			539
#define	SMG$K_LABEL_F10			540
#define	SMG$K_LABEL_F11			541
#define	SMG$K_LABEL_F12			542
#define	SMG$K_LABEL_F13			543
#define	SMG$K_LABEL_F14			544
#define	SMG$K_LABEL_F15			545
#define	SMG$K_LABEL_F16			546
#define	SMG$K_LABEL_F17			547
#define	SMG$K_LABEL_F18			548
#define	SMG$K_LABEL_F19			549
#define	SMG$K_LABEL_F20			550
#define	SMG$K_LEFT_T_CHAR		551
#define	SMG$K_LIGHT_SCREEN		552
#define	SMG$K_LOWER_LEFT_CORNER		553
#define	SMG$K_LOWER_RIGHT_CORNER	554
#define	SMG$K_NAME			555
#define	SMG$K_NEWLINE_CHAR		556
#define	SMG$K_PAD_CHAR			557
#define	SMG$K_RESTORE_CURSOR		558
#define	SMG$K_RIGHT_T_CHAR		559
#define	SMG$K_SAVE_CURSOR		560
#define	SMG$K_SCROLL_FORWARD		561
#define	SMG$K_SCROLL_REVERSE		562
#define	SMG$K_SEL_ERASE_TO_END_DISPLAY	563
#define	SMG$K_SEL_ERASE_TO_END_LINE	564
#define	SMG$K_SEL_ERASE_WHOLE_DISPLAY	565
#define	SMG$K_SEL_ERASE_WHOLE_LINE	566
#define	SMG$K_SET_APPLICATION_KEYPAD	567
#define	SMG$K_SET_CHAR_NOT_SEL_ERASE	568
#define	SMG$K_SET_CHAR_SEL_ERASE	569
#define	SMG$K_SET_CURSOR_ABS		570
#define	SMG$K_SET_NUMERIC_KEYPAD	571
#define	SMG$K_SET_SCROLL_REGION		572
#define	SMG$K_SET_TAB			573
#define	SMG$K_SINGLE_HIGH		574
#define	SMG$K_TAB_CHAR			575
#define	SMG$K_TOP_T_CHAR		576
#define	SMG$K_UNDERLINE_CHAR		577
#define	SMG$K_UPPER_LEFT_CORNER		578
#define	SMG$K_UPPER_RIGHT_CORNER	579
#define	SMG$K_VERTICAL_BAR		580
#define	SMG$K_WIDTH_NARROW		581
#define	SMG$K_WIDTH_WIDE		582
#define	SMG$K_CURSOR_POSITION_REPORT	583
#define	SMG$K_REQUEST_CURSOR_POSITION	584
#define	SMG$K_CR_GRAPHIC		585
#define	SMG$K_FF_GRAPHIC		586
#define	SMG$K_LF_GRAPHIC		587
#define	SMG$K_HT_GRAPHIC		588
#define	SMG$K_VT_GRAPHIC		589
#define	SMG$K_TRUNCATION_ICON		590
#define	SMG$K_CURSOR_NEXT_LINE		591
#define	SMG$K_CURSOR_PRECEDING_LINE	592
#define	SMG$K_INDEX			593
#define	SMG$K_REVERSE_INDEX		594
#define	SMG$K_BEGIN_NORMAL_RENDITION	595
#define	SMG$K_BEGIN_AUTOWRAP_MODE	596
#define	SMG$K_END_AUTOWRAP_MODE		597
#define	SMG$K_BEGIN_AUTOREPEAT_MODE	598
#define	SMG$K_END_AUTOREPEAT_MODE	599
#define	SMG$K_SET_ORIGIN_RELATIVE	600
#define	SMG$K_SET_ORIGIN_ABSOLUTE	601
#define	SMG$K_ERASE_LINE_TO_CURSOR	602
#define	SMG$K_NEXT_LINE			603
#define	SMG$K_BEGIN_AUTOPRINT_MODE	604
#define	SMG$K_END_AUTOPRINT_MODE	605
#define	SMG$K_PRINT_SCREEN		606
#define	SMG$K_SET_CURSOR_ON		607
#define	SMG$K_SET_CURSOR_OFF		608
#define	SMG$K_SET_PRINTER_OUTPUT	609
#define	SMG$K_SET_SCREEN_OUTPUT		610
#define	SMG$K_ERASE_DISPLAY_TO_CURSOR	611
#define	SMG$K_REQUEST_PRINTER_STATUS	612
#define	SMG$K_PRINTER_READY		613
#define	SMG$K_PRINTER_NOT_READY		614
#define	SMG$K_NO_PRINTER		615
#define	SMG$K_SET_JUMP_SCROLL		616
#define	SMG$K_SET_SMOOTH_SCROLL		617
#define	SMG$K_ERROR_ICON		618
#define	SMG$K_PAGE_LENGTH		619
#define	SMG$K_PRIVATE_STR_1		651
#define	SMG$K_PRIVATE_STR_2		652
#define	SMG$K_PRIVATE_STR_3		653
#define	SMG$K_PRIVATE_STR_4		654
#define	SMG$K_PRIVATE_STR_5		655
#define	SMG$K_PRIVATE_STR_6		656
#define	SMG$K_PRIVATE_STR_7		657
#define	SMG$K_PRIVATE_STR_8		658
#define	SMG$K_PRIVATE_STR_9		659
#define	SMG$K_PRIVATE_STR_10		660
#define	SMG$K_MAX_STRING_CODE		660
#define	SMG$K_BEGIN_USER1		661
#define	SMG$K_BEGIN_USER2		662
#define	SMG$K_BEGIN_USER3		663
#define	SMG$K_BEGIN_USER4		664
#define	SMG$K_BEGIN_USER5		665
#define	SMG$K_BEGIN_USER6		666
#define	SMG$K_BEGIN_USER7		667
#define	SMG$K_BEGIN_USER8		668
#define	SMG$K_END_USER1			669
#define	SMG$K_END_USER2			670
#define	SMG$K_END_USER3			671
#define	SMG$K_END_USER4			672
#define	SMG$K_END_USER5			673
#define	SMG$K_END_USER6			674
#define	SMG$K_END_USER7			675
#define	SMG$K_END_USER8			676
#define	SMG$K_BLACK_SCREEN		677
#define	SMG$K_BLUE_SCREEN		678
#define	SMG$K_CYAN_SCREEN		679
#define	SMG$K_MAGENTA_SCREEN		680
#define	SMG$K_GREEN_SCREEN		681
#define	SMG$K_RED_SCREEN		682
#define	SMG$K_WHITE_SCREEN		683
#define	SMG$K_YELLOW_SCREEN		684
#define	SMG$K_USER1_SCREEN		685
#define	SMG$K_USER2_SCREEN		686
#define	SMG$K_BEGIN_STATUS_LINE		687
#define	SMG$K_END_STATUS_LINE		688
#define	SMG$K_BEGIN_LOCATOR		689
#define	SMG$K_END_LOCATOR		690
#define	SMG$K_KEY_FIRST_DOWN		691
#define	SMG$K_KEY_SECOND_DOWN		692
#define	SMG$K_KEY_THIRD_DOWN		693
#define	SMG$K_KEY_FOURTH_DOWN		694
#define	SMG$K_KEY_FIRST_UP		695
#define	SMG$K_KEY_SECOND_UP		696
#define	SMG$K_KEY_THIRD_UP		697
#define	SMG$K_KEY_FOURTH_UP		698
#define	SMG$K_MAX_STRING2_CODE		880

/*
	Parts of VMS.C that we'll want to access here
*/
extern struct dsc$descriptor_s *descptr();
extern struct dsc$descriptor_s *descrp();
#define DESCPTR( s)	descrp( s, sizeof(s)-1)
/*
	These two structures, along with ttdef.h, are good for manipulating
	terminal characteristics.
*/
typedef struct
    {/* Terminal characteristics buffer */
    unsigned char class, type;
    unsigned short width;
    unsigned tt1 : 24;
    unsigned char page;
    unsigned long tt2;
    } TTCHAR;
extern NOSHARE TTCHAR orgchar;			/* Original characteristics */
/*
	test macro is used to signal errors from system services
*/
#define test( s) {int st; st = (s); if( (st&1)==0) LIB$SIGNAL( st);}
#define FAILURE( s) (!(s&1))
#define SUCCESS( s) (s&1)

/** Parameters **/
#define NKEYENT		128		/* Number of keymap entries	*/

/** Type definitions **/
struct keyent {				/* Key mapping entry		*/
	struct keyent * samlvl;		/* Character on same level	*/
	struct keyent * nxtlvl;		/* Character on next level	*/
	char ch;			/* Character			*/
	int code;			/* Resulting keycode		*/
};

/** Values to manage the screen **/
static int termtype;			/* Handle to pass to SMG	*/
#if	KEYPAD
static char * applic_keypad;		/* Put keypad in application mode.*/
static char * numeric_keypad;		/* Put keypad in numeric mode.	*/
#endif
static char * begin_reverse;		/* Begin reverse video		*/
static char * end_reverse;		/* End reverse video		*/
static char * begin_mouse;		/* Begin using mouse		*/
static char * end_mouse;		/* End using mouse		*/
static char * erase_to_end_line;	/* Erase to end of line		*/
static char * erase_whole_display;	/* Erase whole display		*/
static char * width_narrow;		/* Set narrow size screen	*/
static char * width_wide;		/* Set wide size screen		*/
static int narrow_char;			/* Number of characters narrow	*/
static int wide_char;			/* Number of characters wide	*/
static int inbuf[64];			/* Input buffer			*/
static int * inbufh = inbuf;		/* Head of input buffer		*/
static int * inbuft = inbuf;		/* Tail of input buffer		*/
static char keyseq[256];		/* Prefix escape sequence table	*/
static struct keyent keymap[NKEYENT];	/* Key map			*/
static struct keyent * nxtkey = keymap;	/* Next free key entry		*/


/* Forward references.          */
extern int PASCAL NEAR smgmove();
extern int PASCAL NEAR smgeeol();
extern int PASCAL NEAR smgeeop();
extern int PASCAL NEAR smgbeep();
extern int PASCAL NEAR smgopen();
extern int PASCAL NEAR smgrev();
extern int PASCAL NEAR smgcres();
extern int PASCAL NEAR smgparm();
extern int PASCAL NEAR smggetc();
extern int PASCAL NEAR smgclose();

/** Terminal dispatch table **/
NOSHARE TERM term = {
	72-1,				/* Max number of rows allowable */
	0,				/* Current number of rows used	*/
	256,				/* Max number of columns	*/
	0,				/* Current number of columns	*/
	0, 0,				/* x/y origin of screen		*/
	64,				/* Min margin for extended lines*/
	8,				/* Size of scroll region	*/
	100,				/* # times thru update to pause */
	smgopen,			/* Open terminal at the start	*/
	smgclose,			/* Close terminal at end	*/
	smg_noop,			/* Open keyboard		*/
	smg_noop,			/* Close keyboard		*/
	smggetc,			/* Get character from keyboard	*/
	ttputc,				/* Put character to display	*/
	ttflush,			/* Flush output buffers		*/
	smgmove,			/* Move cursor, origin 0	*/
	smgeeol,			/* Erase to end of line		*/
	smgeeop,			/* Erase to end of page		*/
	smgbeep,			/* Beep				*/
	smgrev,				/* Set reverse video state	*/
	smgcres				/* Change screen resolution	*/
#if COLOR
	,
	smg_noop,			/* Set forground color		*/
	smg_noop			/* Set background color		*/
#endif /* COLOR */
};

/***
 *  smgmove  -  Move the cursor (0 origin)
 *
 *  smgmove calls to the SMG run-time library to produce a character
 *  sequence to position the cursor.  If the sequence cannot be made,
 *  a string "OOPS" is produced instead, much like the termcap library
 *  under UNIX.  In the case of "OOPS", the user will soon know that
 *  his terminal entry is incorrect.
 *
 *  Nothing returned.
 ***/
smgmove(int row, int column)
{
	char buffer[32];
	int rlen, status;
	
	static int code = SMG$K_SET_CURSOR_ABS;
	static int len = sizeof(buffer);
	static int arg[3] = { 2 };

	/* SMG assumes the row/column positions	are 1 based. */
	arg[1] = row + 1;
	arg[2] = column + 1;

	/* Call to SMG for the sequence */
	status = SMG$GET_TERM_DATA(&termtype, &code, &len, &rlen, buffer, arg);
	if (SUCCESS(status)) {
		buffer[rlen] = '\0';
		smgputs(buffer);
	} else
		smgputs("OOPS");
}

/***
 *  smgcres  -  Change screen resolution
 *
 *  smgcres changes the screen resolution of the current window.
 *  Allowable sizes are NORMAL and WIDE.
 *
 *  Nothing returned
 ***/
smgcres(char *value)
{
	int width;

	/* Skip if not supported */
	if (width_wide == NULL || width_narrow == NULL)
		return;

	/* Check value */
	if (strcmp(value, "WIDE") == 0) {
		width = wide_char;
		smgputs(width_wide);
	} else if (strcmp(value, "NORMAL") == 0) {
		width = narrow_char;
		smgputs(width_narrow);
	}

	/* Change width */
	orgchar.width = width;
	newwidth(TRUE, width);

	/* Set resolution variable */
	strcpy(sres, value);
}

/***
 *  smgrev  -  Set the reverse video status
 *
 *  smgrev either sets or resets the reverse video state, based on the
 *  boolean argument.  This function is only called if the revexist
 *  boolean variable is set to TRUE.  Otherwise there is no reverse
 *  video available.
 *
 *  Nothing returned.
 ***/
smgrev(int status)
{
	smgputs(status ? begin_reverse : end_reverse);
}

/***
 *  smgeeol  -  Erase to end of line
 *
 *  When this function is called, the lines worth of text after the
 *  cursor is erased.  This function is only called if the eolexist
 *  boolean variable is set to TRUE.  Otherwise the display manager
 *  will produce enough spaces to erase the line.
 *
 *  Nothing returned.
 ***/
smgeeol()
{
	smgputs(erase_to_end_line);
}

/***
 *  smgeeop  -  Erase to end of page (clear screen)
 *
 *  smgeeop really should be called smgclear because it really should
 *  be an erase screen function.  When called, this routine will send
 *  the erase entire screen sequence to the output.
 *
 *  Nothing returned.
 ***/
smgeeop()
{
	smgputs(erase_whole_display);
}

/***
 *  smgbeep  -  Ring the bell
 *
 *  smgbeep send a bell character to the output.  It might be possible
 *  in the future to include the NOISY definition and attempt to flash
 *  the screen, perhaps using LIGHT_SCREEN and DARK_SCREEN.
 *
 *  Nothing returned.
 ***/
smgbeep()
{
	ttputc('\007');
}

/***
 *  smggetstr  -  Get an SMG string capability by name
 *
 *  smggetstr attempts to obtain the escape sequence for a particular
 *  job from the SMG library.  Most sequences do not require a parameter
 *  with the sequence, others do.  In order to obtain the definition
 *  without knowing ahead of time whether ornot the definition has a
 *  parameter, we call SMG once with a parameter and if that fails, we
 *  try again without one.  If both attempts fail, we will return the
 *  NULL string.
 *
 *  Storage for the sequence comes from a local pool.
 *
 *  Returns:	Escape sequence
 *		NULL	No escape sequence available
 ***/ 
char * smggetstr(int code)	/* Request code			*/
{
	char * result;
	int rlen, status;
	
	static char seq[1024];
	static char * buffer = seq;
	static int len = sizeof(seq);
	static int arg[2] = { 1, 1 };

	/* Get sequence with one parameter */
	status = SMG$GET_TERM_DATA(&termtype, &code, &len, &rlen, buffer, arg);
	if (FAILURE(status)) {
		/* Try again with zero parameters */
		status = SMG$GET_TERM_DATA(&termtype, &code, &len, &rlen, buffer);
		if (FAILURE(status))
			return NULL;
	}

	/* Check for empty result */
	if (rlen == 0)
		return NULL;
	
	/* Save current position so we can return it to caller */
	result = buffer;
	buffer[rlen++] = '\0';
	buffer += rlen;

	/* Return capability to user */
	return result;
}

/***
 *  smggetnum  -  Get numerical constant from SMG
 *
 *  smggetnum attempts to get a numerical constant from the SMG package.
 *  If the constant cannot be found, -1 is returned.
 *
 *  code - SMG code
 ***/
int smggetnum(int code)
{
	int status, result;

	/* Call SMG for code translation */
	status = SMG$GET_NUMERIC_DATA(&termtype, &code, &result);
	return FAILURE(status) ? -1 : result;
}

/***
 *  smgaddkey  -  Add key to key map
 *
 *  smgaddkey adds a new escape sequence to the sequence table.
 *  I am not going to try to explain this table to you in detail.
 *  However, in short, it creates a tree which can easily be transversed
 *  to see if input is in a sequence which can be translated to a
 *  function key (arrows and find/select/do etc. are treated like
 *  function keys).  If the sequence is ambiguous or duplicated,
 *  it is silently ignored.
 *
 *  Nothing returned
 *
 *  code - SMG key code
 *  fn   - Resulting keycode
 ***/
smgaddkey(int code, int fn)
{
	char * seq;
	int first;
	struct keyent * cur, * nxtcur;
	
	/* Skip on NULL sequence */
	seq = smggetstr(code);
	if (seq == NULL)
		return;
	
	/* If no keys defined, go directly to insert mode */
	first = 1;
	if (nxtkey != keymap) {
		
		/* Start at top of key map */
		cur = keymap;
		
		/* Loop until matches exhast */
		while (*seq) {
			
			/* Do we match current character */
			if (*seq == cur->ch) {
				
				/* Advance to next level */
				seq++;
				cur = cur->nxtlvl;
				first = 0;
			} else {
				
				/* Try next character on same level */
				nxtcur = cur->samlvl;
				
				/* Stop if no more */
				if (nxtcur)
					cur = nxtcur;
				else
					break;
			}
		}
	}
	
	/* Check for room in keymap */
	if (strlen(seq) > NKEYENT - (nxtkey - keymap))
		return;
		
	/* If first character if sequence is inserted, add to prefix table */
	if (first)
		keyseq[(unsigned char) *seq] = 1;
		
	/* If characters are left over, insert them into list */
	for (first = 1; *seq; first = 0) {
		
		/* Make new entry */
		nxtkey->ch = *seq++;
		nxtkey->code = fn;
		
		/* If root, nothing to do */
		if (nxtkey != keymap) {
			
			/* Set first to samlvl, others to nxtlvl */
			if (first)
				cur->samlvl = nxtkey;
			else
				cur->nxtlvl = nxtkey;
		}

		/* Advance to next key */
		cur = nxtkey++;
	}
}

/***
 *  smgcap  -  Get capabilities from VMS's SMG library
 *
 *  smgcap retrives all the necessary capabilities from the SMG
 *  library to operate microEmacs.  If an insufficent number of
 *  capabilities are found for the particular terminal, an error
 *  status is returned.
 *
 *  Returns:	0 if okay, <>0 if error
 ***/
int smgcap()
{
	char * set_cursor_abs;
	int status;
	
	/* Start SMG package */
	status = SMG$INIT_TERM_TABLE_BY_TYPE( &orgchar.type, &termtype);
	if (FAILURE(status)) {
		printf(TEXT189);
/*                     "Cannot find entry for terminal type.\n" */
		printf(TEXT190);
/*                     "Check terminal type with \"SHOW TERMINAL\" or\n" */
		printf(TEXT191);
/*                     "try setting with \"SET TERMINAL/INQUIRE\"\n" */
		return 1;
	}
		
	/* Get reverse video */
	begin_reverse = smggetstr(SMG$K_BEGIN_REVERSE);
	end_reverse = smggetstr(SMG$K_END_REVERSE);
#if	KEYPAD
	applic_keypad = smggetstr(SMG$K_SET_APPLICATION_KEYPAD);
	numeric_keypad = smggetstr(SMG$K_SET_NUMERIC_KEYPAD);
#endif
	begin_mouse = smggetstr(SMG$K_BEGIN_LOCATOR);
	end_mouse = smggetstr(SMG$K_END_LOCATOR);
	revexist = begin_reverse != NULL && end_reverse != NULL;
	
	/* Get erase to end of line */
	erase_to_end_line = smggetstr(SMG$K_ERASE_TO_END_LINE);
	eolexist = erase_to_end_line != NULL;
	
	/* Get more neat stuff */
	erase_whole_display = smggetstr(SMG$K_ERASE_WHOLE_DISPLAY);
	width_wide = smggetstr(SMG$K_WIDTH_WIDE);
	width_narrow = smggetstr(SMG$K_WIDTH_NARROW);
	narrow_char = smggetnum(SMG$K_COLUMNS);
	wide_char = smggetnum(SMG$K_WIDE_SCREEN_COLUMNS);
	set_cursor_abs = smggetstr(SMG$K_SET_CURSOR_ABS);

	/* Disable resoultion if unreasonable */
	if (narrow_char < 10 || wide_char < 10) {
		width_wide = width_narrow = NULL;
		strcpy(sres, "NORMAL");
	} else
		/* Kludge resolution */
		strcpy(sres, orgchar.width == wide_char ? "WIDE" : "NORMAL");

	/* Check for minimal operations */
	if (set_cursor_abs == NULL || erase_whole_display == NULL) {
		printf(TEXT192);
/*                     "The terminal type does not have enough power to run\n" */
		printf(TEXT193);
/*                     "MicroEMACS.  Try a different terminal or check\n" */
		printf(TEXT194);
/*                     "type with \"SHOW TERMINAL\".\n" */
		return 1;
	}
	
	/* Add function keys to keymapping table */
	smgaddkey(SMG$K_KEY_DOWN_ARROW,		SPEC | 'N');
	smgaddkey(SMG$K_KEY_LEFT_ARROW,		SPEC | 'B');
	smgaddkey(SMG$K_KEY_RIGHT_ARROW,	SPEC | 'F');
	smgaddkey(SMG$K_KEY_UP_ARROW,		SPEC | 'P');

	smgaddkey(SMG$K_KEY_PF1,	CTRL | SPEC | '1');
	smgaddkey(SMG$K_KEY_PF2,	CTRL | SPEC | '2');
	smgaddkey(SMG$K_KEY_PF3,	CTRL | SPEC | '3');
	smgaddkey(SMG$K_KEY_PF4,	CTRL | SPEC | '4');

	smgaddkey(SMG$K_KEY_0,		ALTD | '0');
	smgaddkey(SMG$K_KEY_1,		ALTD | '1');
	smgaddkey(SMG$K_KEY_2,		ALTD | '2');
	smgaddkey(SMG$K_KEY_3,		ALTD | '3');
	smgaddkey(SMG$K_KEY_4,		ALTD | '4');
	smgaddkey(SMG$K_KEY_5,		ALTD | '5');
	smgaddkey(SMG$K_KEY_6,		ALTD | '6');
	smgaddkey(SMG$K_KEY_7,		ALTD | '7');
	smgaddkey(SMG$K_KEY_8,		ALTD | '8');
	smgaddkey(SMG$K_KEY_9,		ALTD | '9');
	smgaddkey(SMG$K_KEY_PERIOD,	ALTD | '.');
	smgaddkey(SMG$K_KEY_ENTER, 	ALTD | 'E');
	smgaddkey(SMG$K_KEY_COMMA, 	ALTD | ',');
	smgaddkey(SMG$K_KEY_MINUS, 	ALTD | '-');

	smgaddkey(SMG$K_KEY_F1,		SPEC | '1');
	smgaddkey(SMG$K_KEY_F2,		SPEC | '2');
	smgaddkey(SMG$K_KEY_F3,		SPEC | '3');
	smgaddkey(SMG$K_KEY_F4,		SPEC | '4');
	smgaddkey(SMG$K_KEY_F5,		SPEC | '5');
	smgaddkey(SMG$K_KEY_F6,		SPEC | '6');
	smgaddkey(SMG$K_KEY_F7,		SPEC | '7');
	smgaddkey(SMG$K_KEY_F8,		SPEC | '8');
	smgaddkey(SMG$K_KEY_F9,		SPEC | '9');
	smgaddkey(SMG$K_KEY_F10,	SPEC | '0');
	smgaddkey(SMG$K_KEY_F11,	SHFT | SPEC | '1');
	smgaddkey(SMG$K_KEY_F12,	SHFT | SPEC | '2');
	smgaddkey(SMG$K_KEY_F13,	SHFT | SPEC | '3');
	smgaddkey(SMG$K_KEY_F14,	SHFT | SPEC | '4');
	smgaddkey(SMG$K_KEY_F15,	SHFT | SPEC | '5');
	smgaddkey(SMG$K_KEY_F16,	SHFT | SPEC | '6');
	smgaddkey(SMG$K_KEY_F17,	SHFT | SPEC | '7');
	smgaddkey(SMG$K_KEY_F18,	SHFT | SPEC | '8');
	smgaddkey(SMG$K_KEY_F19,	SHFT | SPEC | '9');
	smgaddkey(SMG$K_KEY_F20,	SHFT | SPEC | '0');

	smgaddkey(SMG$K_KEY_E1,		SPEC | 'S');
	smgaddkey(SMG$K_KEY_E2,		SPEC | 'C');
	smgaddkey(SMG$K_KEY_E3,		SPEC | 'D');
	smgaddkey(SMG$K_KEY_E4,		SPEC | '@');
	smgaddkey(SMG$K_KEY_E5,		SPEC | 'Z');
	smgaddkey(SMG$K_KEY_E6,		SPEC | 'V');

	smgaddkey(SMG$K_KEY_FIRST_DOWN,		MOUS | 'a');
	smgaddkey(SMG$K_KEY_FIRST_UP,		MOUS | 'b');
	smgaddkey(SMG$K_KEY_SECOND_DOWN,	MOUS | 'c');
	smgaddkey(SMG$K_KEY_SECOND_UP,		MOUS | 'd');
	smgaddkey(SMG$K_KEY_THIRD_DOWN,		MOUS | 'e');
	smgaddkey(SMG$K_KEY_THIRD_UP,		MOUS | 'f');
	smgaddkey(SMG$K_KEY_FOURTH_DOWN,	MOUS | 'g');
	smgaddkey(SMG$K_KEY_FOURTH_UP,		MOUS | 'h');
	/* Everything okay */
	return 0;
}

/***
 *  smgopen  -  Get terminal type and open terminal
 *
 *  Nothing returned
 ***/
smgopen()
{
	static int first_time = 1;
	
	/* Open channel to terminal (also sets sizes in TERM structure) */
	ttopen();

	/* Get SMG */
	if (first_time)
	{
		first_time = 0;
		if (smgcap())
			meexit( 1);
	}

#if	KEYPAD
	smgputs(applic_keypad);
#endif
#ifdef NEVER
	smgputs( begin_mouse);
#endif
}

smgclose()
{
#ifdef NEVER
	smgputs( end_mouse);
#endif
#if	KEYPAD
	if ((orgchar.tt2 & TT2$M_APP_KEYPAD)==0)
	    smgputs(numeric_keypad);
#endif
	ttclose();
}

/***
 *  smgputs  -  Send a string to ttputc
 *
 *  smgputs is a short-cut routine to handle sending a string of characters
 *  to the character output routine.  A check is made for a NULL string,
 *  while is considered valid.  A NULL string will produce no output.
 *
 *  Nothing returned.
 ***/
smgputs(string)
char * string;				/* String to write		*/
{
	if (string)
		while (*string)
			ttputc(*string++);
}

/***
 *  smgqin  -  Queue character for input
 *
 *  smgqin queues the character into the input buffer for later
 *  reading.  This routine will mostly be used by mouse support
 *  and other escape sequence processing.
 *
 *  Nothing returned.
 *
 *  ch  - (Extended) character to add
 ***/
smgqin(int ch)
{
	/* Check for overflow */
	if (inbuft == &inbuf[sizeof(inbuf)]) {
		
		/* Annoy user */
		smgbeep();
		return;
	}
	
	/* Add character */
	*inbuft++ = ch;
}

/***
 *  smggcook  -  Get characters from input device
 *
 *  smggcook "cooks" input from the input device and places them into
 *  the input queue.
 *
 *  Nothing returned.
 ***/
smggcook()
{
	char ch;
	struct keyent * cur;
	
	/* Get first character untimed */
	ch = ttgetc();
	smgqin((unsigned char)ch);
	
	/* Skip if the key isn't a special leading escape sequence */
	if (keyseq[(unsigned char)ch] == 0)
		return;

	/* Start translating */
	cur = keymap;
	while (cur) {
		if (cur->ch == ch) {
			/* Is this the end */
			if (cur->nxtlvl == NULL) {
				/* Replace all character with new sequence */
				inbuft = inbuf;
				smgqin(cur->code);
				return;
			} else {
				/* Advance to next level */
				cur = cur->nxtlvl;
			
				/* Get next character, timed */
				ch = ttgetc_shortwait();
				if (ch < 0)
					return;

				/* Queue character */
				smgqin((unsigned char)ch);
			}
		} else
			/* Try next character on same level */
			cur = cur->samlvl;
	}
}

/***
 *  smggetc  -  Get a character
 *
 *  smggetc obtains input from the character input queue.  If the queue
 *  is empty, a call to smggcook() is called to fill the input queue.
 *
 *  Returns:	character
 ***/
int smggetc()
{
	int ch;

	/* Loop until character found */
	while (1) {
	
		/* Get input from buffer, if available */
		if (inbufh != inbuft) {
			ch = *inbufh++;
			if (inbufh == inbuft)
				inbufh = inbuft = inbuf;
			break;
		} else
	
			/* Fill input buffer */
			smggcook();
	}
	
	/* Return next character */
	return (int) ch;
}

/***
 *  spal  -  Set palette type
 *
 *  spal sets the palette colors for the 8 colors available.  Currently,
 *  there is nothing here, but some DEC terminals, (VT240 and VT340) have
 *  a color palette which is available under the graphics modes.
 *  Further, a foreign terminal could also change color registers.
 *
 *  Nothing returned
 ***/
spal()
{
	/* Nothing */
}

#if FLABEL
/***
 *  fnclabel  -  Label function keys
 *
 *  Currently, smg does not have function key labeling.
 *
 *  Returns:	status.
 *
 *  flag - TRUE if default
 *  n    - Numerical argument
 ***/
int fnclabel(int flag, int n)
{
	/* On machines with no function keys...don't bother */
	return TRUE;
}
#endif /* FLABEL */

#endif	/* End of SMG terminal type */

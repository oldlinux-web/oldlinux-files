/*  ========================================================================  *
 *				Mined.h					      *	
 *  ========================================================================  */

#include <minix/config.h>
#ifndef YMAX
#ifdef UNIX
#include <stdio.h>
#undef putchar
#undef getchar
#undef NULL
#undef EOF
extern int _putchar();
extern char *CE, *VS, *SO, *SE, *CL, *AL, *CM;
#define YMAX		49
#else
#define YMAX		24		/* Maximum y coordinate starting at 0 */
/* Escape sequences. */
extern char *enter_string;	/* String printed on entering mined */
extern char *rev_video;		/* String for starting reverse video */
extern char *normal_video;	/* String for leaving reverse video */
extern char *rev_scroll;	/* String for reverse scrolling */
extern char *pos_string;	/* Absolute cursor positioning */
#define X_PLUS	' '		/* To be added to x for cursor sequence */
#define Y_PLUS	' '		/* To be added to y for cursor sequence */
#endif UNIX

#define XMAX		79		/* Maximum x coordinate starting at 0 */
#define SCREENMAX	(YMAX - 1)	/* Number of lines displayed */
#define XBREAK		(XMAX - 2)	/* Line shift at this coordinate */
#define SHIFT_SIZE	25		/* Number of chars to shift */
#define SHIFT_MARK	'!'		/* Char indicating that line continues*/
#define MAX_CHARS	1024		/* Maximum chars on one line */
		    /* LINE_START must be rounded up to the lowest SHIFT_SIZE */
#define LINE_START	(((-MAX_CHARS - 1) / SHIFT_SIZE) * SHIFT_SIZE \
  				   - SHIFT_SIZE)
#define LINE_END	(MAX_CHARS + 1)	/* Highest x-coordinate for line */

#define LINE_LEN	(XMAX + 1)	/* Number of characters on line */
#define SCREEN_SIZE	(XMAX * YMAX)	/* Size of I/O buffering */
#define BLOCK_SIZE	1024

/* Return values of functions */
#define ERRORS		-1
#define NO_LINE		(ERRORS - 1)	/* Must be < 0 */
#define FINE	 	(ERRORS + 1)
#define NO_INPUT	(ERRORS + 2)

#define STD_OUT	 	1		/* Filedescriptor for terminal */
#define FILE_LENGTH	14		/* Length of filename in minix */
#if (CHIP == INTEL)
#define MEMORY_SIZE	(50 * 1024)	/* Size of data space to malloc */
#endif

#define REPORT	2			/* Report change of lines on # lines */

typedef int FLAG;

/* General flags */
#define	FALSE		0
#define	TRUE		1
#define	NOT_VALID	2
#define	VALID		3
#define	OFF		4
#define	ON		5

/* Expression flags */
#define	FORWARD		6
#define	REVERSE		7

/* Yank flags */
#define	SMALLER		8
#define	BIGGER		9
#define	SAME		10
#define	EMPTY		11
#define	NO_DELETE	12
#define	DELETE		13
#define	READ		14
#define	WRITE		15

/*
 * The Line structure.  Each line entry contains a pointer to the next line,
 * a pointer to the previous line, a pointer to the text and an unsigned char
 * telling at which offset of the line printing should start (usually 0).
 */
struct Line {
  struct Line *next;
  struct Line *prev;
  char *text;
  unsigned char shift_count;
};

typedef struct Line LINE;

/* Dummy line indicator */
#define DUMMY		0x80
#define DUMMY_MASK	0x7F

/* Expression definitions */
#define NO_MATCH	0
#define MATCH		1
#define REG_ERROR	2

#define BEGIN_LINE	(2 * REG_ERROR)
#define END_LINE	(2 * BEGIN_LINE)

/*
 * The regex structure. Status can be any of 0, BEGIN_LINE or REG_ERROR. In
 * the last case, the result.err_mess field is assigned. Start_ptr and end_ptr
 * point to the match found. For more details see the documentation file.
 */
struct regex {
  union {
  	char *err_mess;
  	int *expression;
  } result;
  char status;
  char *start_ptr;
  char *end_ptr;
};

typedef struct regex REGEX;

/* NULL definitions */
#define NIL_PTR		((char *) 0)
#define NIL_LINE	((LINE *) 0)
#define NIL_REG		((REGEX *) 0)
#define NIL_INT		((int *) 0)

/*
 * Forward declarations
 */
extern int nlines;		/* Number of lines in file */
extern LINE *header;		/* Head of line list */
extern LINE *tail;		/* Last line in line list */
extern LINE *top_line;		/* First line of screen */
extern LINE *bot_line;		/* Last line of screen */
extern LINE *cur_line;		/* Current line in use */
extern char *cur_text;		/* Pointer to char on current line in use */
extern int last_y;		/* Last y of screen. Usually SCREENMAX */
extern int ymax;
extern int screenmax;
extern char screen[SCREEN_SIZE];/* Output buffer for "writes" and "reads" */

extern int x, y;			/* x, y coordinates on screen */
extern FLAG modified;			/* Set when file is modified */
extern FLAG stat_visible;		/* Set if status_line is visible */
extern FLAG writable;			/* Set if file cannot be written */
extern FLAG quit;			/* Set when quit character is typed */
extern FLAG rpipe;		/* Set if file should be read from stdin */
extern int input_fd;			/* Fd for command input */
extern FLAG loading;			/* Set if we're loading a file */
extern int out_count;			/* Index in output buffer */
extern char file_name[LINE_LEN];	/* Name of file in use */
extern char text_buffer[MAX_CHARS];	/* Buffer for modifying text */
extern char blank_line[LINE_LEN];	/* Line filled with spaces */

extern char yank_file[];		/* Temp file for buffer */
extern FLAG yank_status;		/* Status of yank_file */
extern long chars_saved;		/* Nr of chars saved in buffer */

/*
 * Empty output buffer
 */
#define clear_buffer()			(out_count = 0)

/*
 * Print character on terminal
 */
#define putchar(c)			(void) write_char(STD_OUT, (c))

/*
 * Ring bell on terminal
 */
#define ring_bell()			putchar('\07')

/*
 * Print string on terminal
 */
#define string_print(str)		(void) writeline(STD_OUT, (str))

/*
 * Flush output buffer
 */
#define flush()				(void) flush_buffer(STD_OUT)

/*
 * Convert cnt to nearest tab position
 */
#define tab(cnt)			(((cnt) + 8) & ~07)
#define is_tab(c)			((c) == '\t')

/*
 * Word defenitions
 */
#define white_space(c)	((c) == ' ' || (c) == '\t')
#define alpha(c)	((c) != ' ' && (c) != '\t' && (c) != '\n')

/*
 * Print line on terminal at offset 0 and clear tail of line
 */
#define line_print(line)		put_line(line, 0, TRUE)

/*
 * Move to coordinates and set textp. (Don't use address)
 */
#define move_to(nx, ny)			move((nx), NIL_PTR, (ny))

/*
 * Move to coordinates on screen as indicated by textp.
 */
#define move_address(address)		move(0, (address), y)

/*
 * Functions handling status_line. ON means in reverse video.
 */
#define status_line(str1, str2)	(void) bottom_line(ON, (str1), \
						    (str2), NIL_PTR, FALSE)
#define error(str1, str2)	(void) bottom_line(ON, (str1), \
						    (str2), NIL_PTR, FALSE)
#define get_string(str1,str2, fl) bottom_line(ON, (str1), NIL_PTR, (str2), fl)
#define clear_status()		(void) bottom_line(OFF, NIL_PTR, NIL_PTR, \
						    NIL_PTR, FALSE)

/*
 * Print info about current file and buffer.
 */
#define fstatus(mess, cnt)	file_status((mess), (cnt), file_name, \
					     nlines, writable, modified)

/*
 * Get real shift value.
 */
#define get_shift(cnt)		((cnt) & DUMMY_MASK)

/* Forward declarations */
extern LINE *proceed(), *install_line();
extern LINE *match(), *line_insert();
extern char *alloc(), *num_out(), *basename();

#endif YMAX

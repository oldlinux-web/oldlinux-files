/*	me.c	MicroEmacs command interface to kept microemacs
		For VAX/VMS operating system
		for MicroEMACS 3.10
		Copyright 1989 by Jeffrey A. Lomicka and Daniel M. Lawrence

	This is a separate program that is used to start and re-attach to
	a microemacs that's kept in a subprocess.  To use the kept
	facility, point the microemacs symbol at this program instead of
	MESHR.EXE, and store MESHR.EXE in the SYS$SHARE search list.

	To use microemacs in the normal way, just point the microemacs
	command directly at tghe MESHR.EXE image.
*/
#include stdio
#include ssdef
#include descrip
#include jpidef
#include iodef
#include accdef
/*
	These are the LIB$SPAWN mode flags.  There's no .h for
	them in VAX C V2.4.
*/
#define CLI$M_NOCONTROL 32
#define CLI$M_NOCLISYM 2
#define CLI$M_NOLOGNAM 4
#define CLI$M_NOKEYPAD 8
#define CLI$M_NOTIFY 16
#define CLI$M_NOWAIT 1
/*
	test macro is used to signal errors from system services
*/
#define test( s) {int st; st = (s); if( (st&1)==0) LIB$SIGNAL( st);}

/*
	This routine returns a pointer to a descriptor of the supplied
	string. The descriptors are static allocated, and up to
	"NUM_DESCRIPTORS" may be used at once.  After that, the old ones
	are re-used. Be careful!

	The primary use of this routine is to allow passing of C strings into
	VMS system facilities and RTL functions.

	There are three forms:

		descrp( s, l)	String descriptor for buffer s, length l
		descptr( s)	String descriptor for asciz buffer s
		DESCPTR( s)	String descriptor for buffer s, using sizeof()
*/
#define NUM_DESCRIPTORS 10
struct  dsc$descriptor_s *descrp( s, l)
char *s;	/* String to make descriptor for */
int l;		/* length of string */
    {
    static next_d = 0;
    static struct dsc$descriptor_s dsclist[ NUM_DESCRIPTORS];

    if( next_d >= NUM_DESCRIPTORS) next_d = 0;
    dsclist[ next_d].dsc$w_length = l;
    dsclist[ next_d].dsc$b_dtype =  DSC$K_DTYPE_T;
    dsclist[ next_d].dsc$b_class =  DSC$K_CLASS_S;
    dsclist[ next_d].dsc$a_pointer = s;
    return( &dsclist[ next_d++]);
    }

struct  dsc$descriptor_s *descptr( s)	/* Make pointer to descriptor */
char *s;		/* Asciz string to make descriptor for */
    {
    return( descrp( s, strlen( s)));
    }

#define DESCPTR( s)	descrp( s, sizeof(s)-1)

static long g_pid, g_status, g_efn;
struct acc$record msg;		/* Message buffer */
static int good_reason = 0;	/* True if good reason to exit */

me_exited()
   {
   test( LIB$PUT_OUTPUT( DESCPTR( "Microemacs has exited.")));
   good_reason = 1;
   test( SYS$WAKE( 0, 0));
   }

start_microemacs( argc, argv)
int argc;
char *argv[];
    {
    char scrap[ 512], imagename[ 256], *i;
    int a;

    test( LIB$PUT_OUTPUT( DESCPTR( "Starting a new Microemacs process.")));
/*
	Locate the microemacs sharable image
*/
    a = 0;
    test( LIB$FIND_FILE(
	DESCPTR( "MESHR"),
	DESCPTR( imagename),
	&a,
	DESCPTR( "SYS$SHARE:.EXE"),
	0, 0, 0));
    test( LIB$FIND_FILE_END( &a));
/*
	Make it asciz by terminating it at the ";"
*/
    for( i=imagename; *i != ';'; i++);
    *i = 0;
/*
	Build a command line that invokes it
*/
    strcpy( scrap, "MCR ");
    strcat( scrap, imagename);
    for( a=1; a<argc; a++)
	{ /* Build command string */
	strcat( scrap, " ");
	strcat( scrap, argv[ a]);
	}
    test( LIB$GET_EF( &g_efn));
    test( LIB$SPAWN( descptr( scrap), DESCPTR( "NLA0:"), 0, &CLI$M_NOWAIT,
	0, &g_pid, &g_status, &g_efn, me_exited, &g_pid, 0, 0));
    }

main( argc, argv)
int argc;
char *argv[];
    {
    char *env;
    char scrap[ 512];
    char defdir[ 512];
    int a;
    unsigned long pid;
    short len;
/*
	Identify where we come from
*/
    test( LIB$GETJPI( &JPI$_PID, 0, 0, &pid, 0, 0));
    sprintf( scrap, "%d", pid);
    test( LIB$SET_LOGICAL(
	DESCPTR( "MICROEMACS$PARENT"),
	descptr( scrap),
	DESCPTR( "LNM$JOB")));
/*
	Identify where we are going
*/
    env = getenv( "MICROEMACS$PROCESS");
    if( env == NULL) start_microemacs( argc, argv);
    else
	{ /* Microemacs exists, send command line to it and wake it up */
	pid = atoi( env);
/*
	Get rid of the old definition.  We know our wakeup is legitimate
	when either a new definition is created, or when our AST routine
	is executed.
*/
	test( LIB$DELETE_LOGICAL(
	    DESCPTR( "MICROEMACS$PROCESS"),
	    DESCPTR( "LNM$JOB")));
/*
	Fetch the current default directory
*/
	len = 0;
	test( SYS$SETDDIR( 0, &len, DESCPTR( defdir)));
	defdir[ len] = 0;
/*
	Encode an argv string for emacs to decode.  Using "\200" instead
	of spaces just in case quotes were involved.
*/
	strcpy( scrap, getenv( "SYS$DISK"));
	strcat( scrap, "\200");
	strcat( scrap, defdir);
	for( a=0; a<argc; a++)
	    { /* Build command string */
	    strcat( scrap, "\200");
	    strcat( scrap, argv[ a]);
	    }
	test( LIB$SET_LOGICAL(
	    DESCPTR( "MICROEMACS$COMMAND"),
	    descptr( scrap),
	    DESCPTR( "LNM$JOB")));
/*
	Attempt to kiss our sleeping beauty, and thus wake it up
*/
	test( LIB$PUT_OUTPUT( DESCPTR( "Using existing Microemacs process.")));
	a = SYS$WAKE( &pid, 0);
	if( a == SS$_NONEXPR)
	    { /* Failed to wake it up, try starting a new one instead */
	    start_microemacs( argc, argv);
	    }
	else
	    { /* Successful at waking up microemacs, set up for termination */
	    test( a);
	    test( LIB$GETJPI( &JPI$_TMBU, &pid, 0, &a, 0, 0));
	    sprintf( scrap, "MBA%d:", a);
	    test( SYS$ASSIGN( descptr( scrap), &a, 0, 0));
	    test( SYS$QIO( 0, a, IO$_READVBLK, 0, me_exited, 0,
		msg, sizeof( msg), 0, 0, 0, 0));
	    }
	}
/*
	If Emacs was started, hibernate until it comes back
*/
    while( !good_reason)
	{ /* Hibernate until winter is over */
	test( SYS$HIBER());
	env = getenv( "MICROEMACS$PROCESS");
	if( env != NULL) break;
	}
    }

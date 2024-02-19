/*
 * NOTE: Portions of this code was directly derived from the Amiga C-Kermit
 * and is copyrighted by Jack J. Rouse.  Please read the following Copyright
 * notice.
 *								Mikel Matthews
/*
 Author: Jack Rouse
 Contributed to Columbia University for inclusion in C-Kermit.
 Copyright (C) 1986, Jack J. Rouse, 106 Rubin Ct. Apt. A-4, Cary NC 27511
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided 
 this copyright notice is retained. 

 The file status routines assume all file protection modes are real, instead
 of just delete protection on files and write protection on disks.
*/
#include <stdio.h>		/* standard I/O stuff */
#include "exec/types.h"
#include "exec/exec.h"
#include "devices/serial.h"
#include "devices/timer.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "intuition/intuition.h"
#include "intuition/intuitionbase.h"
#include "amiga.h"
#include "fcntl.h"
#include "global.h"
#include "iface.h"
#include "smtp.h"
#include "dir.h"
extern	errno;
struct Process *MyProc;
struct CommandLineInterface *MyCLI;
struct FileHandle *Fptr;	/* used for input */

/* static definitions */

static struct MsgPort *serport;		/* message port for serial comm */
static struct IOExtSer *Read_Request;	/* serial input request */
static struct IOExtSer *Write_Request;	/* serial output request */
static serialopen;

/* Exec routines */

APTR AllocMem();
struct IORequest *CheckIO();
VOID CloseDevice();
LONG DoIO();
struct Task *FindTask();
struct Message *GetMsg();
LONG OpenDevice();
struct Library *OpenLibrary();
VOID ReplyMsg();
VOID SendIO();
LONG Wait();
LONG WaitIO();
struct Message *WaitPort();

/* Exec support */
struct MsgPort *CreatePort();
VOID DeletePort();

/* AmigaDOS routines */
BPTR Input();
LONG Read();
LONG WaitForChar();

struct asy asy[ASY_MAX];

extern x, y, maxcol, MINX, MINY, MAXX;
struct MsgPort *mySerPort;
struct MsgPort *myTimerPort;

extern char *mailqdir;

struct Store_input {
	char *head;
	char store[MAX_STORE];
	char *tail;
	int	amt;
} Store_input[ASY_MAX];

/* Interface list; initialized at compile time to contain
 * just the local interface
 */
struct interface *ifaces;

char *ttbuf;

char Amiga_drive[255];
char	con_buff[255];
/*
 *  sysinit -- Amiga specific initialization
 */
ioinit() 
{
	register struct IOExtSer *iob;
	int	i;
	int speed;
	char *ptr;
	FILE *ffptr;
	int	fd;
	extern char version[];


	MyProc = (struct Process *)FindTask((char *)NULL);

	MyCLI = (struct CommandLineInterface *)BADDR(MyProc->pr_CLI);

	/* allocate serial ports and IO blocks */
	(void) Execute("cd >RAM:tmpfilename", NULL, NULL);

	if ( ( ffptr = fopen("RAM:tmpfilename", "r") ) == NULL)
	{
		return;
	}

	(void) fgets( Amiga_drive, sizeof(Amiga_drive), ffptr);
	(void) fclose(ffptr);
	(void) unlink("RAM:tmpfilename");
	ptr = Amiga_drive;
	while ( *ptr )
	{
		if (*ptr == '\n')
		{
			*ptr = '\0';
		}
		else if (*ptr == ':')
		{
			*++ptr = '\0';
		}
		else
			ptr++;
	}

}


iostop(str, val)
char *str;
int val;
{
	/* close everything */
	/*
	printf("%s, error = %d\n\r", str, val);
	*/

	if (serialopen)
	{
		CloseDevice(Read_Request);
	}
	if ( mySerPort )
	{
		DeletePort(mySerPort);
	}

}
char Receive_store[MAX_STORE];
char Output_store[MAX_STORE];

asy_init(dev, arg1,arg2,bufsize)
int dev, bufsize;
char *arg1,*arg2;
{
	register struct asy *ap;
	register struct interface *if_asy;
	extern struct interface *ifaces;
	register struct IOExtSer *iob;
	char *b, *c;
	int i;
	struct Store_input *store;
	iob = Read_Request;
	
	/*
	printf("ASY_INIT: dev = %d\n\r", dev);
	*/
	
	ap = &asy[dev];

	if ( ap == NULL)
	{
		printf("asy_init: returing -1\n");
		return(-1);
	}
	ap->tty = malloc(strlen(arg2)+1);
	strcpy(ap->tty, arg2);

	if (serialopen)
	{
		printf("asy_init: serial line already open\n");
		return(0);	/* ignore if already open */
	}

	Read_Request = (struct IOExtSer *)
		AllocMem((long) sizeof(*Read_Request), MEMF_PUBLIC | MEMF_CLEAR);


	Read_Request->IOSer.io_Message.mn_ReplyPort = CreatePort(0L, 0L);

	if (OpenDevice(SERIALNAME, NULL, Read_Request, NULL))
	{
		iostop("Cant open Read device", 5L);
	}

	setmem(&Read_Request->io_TermArray, sizeof(struct IOTArray), 0xc0);
	Read_Request->io_SerFlags = SERF_XDISABLED | SERF_EOFMODE;
	Read_Request->io_Baud = 9600L;
	Read_Request->io_ReadLen = 8L;
	Read_Request->io_WriteLen = 8L;
	Read_Request->io_CtlChar = 0x00;
	Read_Request->io_RBufLen = sizeof(Receive_store);
	Read_Request->io_BrkTime = 0L;
	Read_Request->IOSer.io_Command = SDCMD_SETPARAMS;

	DoIO(Read_Request);

	Read_Request->IOSer.io_Command = CMD_READ;
	Read_Request->IOSer.io_Length = 1L;
	Read_Request->IOSer.io_Data = (APTR)Receive_store;

	Write_Request = (struct IOExtSer *)
		AllocMem((long) sizeof(*Write_Request), MEMF_PUBLIC | MEMF_CLEAR);

	b = (BYTE *) Read_Request;
	c = (BYTE *) Write_Request;

	for ( i = 0; i < sizeof (struct IOExtSer); i++)
	{
		*c++ = *b++;
	}

	Write_Request->io_SerFlags = SERF_XDISABLED;
	Write_Request->IOSer.io_Message.mn_ReplyPort = CreatePort(0L, 0L);
	Write_Request->IOSer.io_Command = CMD_WRITE;
	Write_Request->IOSer.io_Length = 1L;
	Write_Request->IOSer.io_Data = (APTR) Output_store;

	store = &Store_input[dev];
	store -> head = store -> tail = store -> store;
	store -> amt = 0;

	serialopen = TRUE;

	return (0L);

}

asy_stop(interface) 	/* ttclos() */
struct interface *interface;
{
	register struct asy *ap;

	if (!serialopen)
	{
		return(0);
	}
	CloseDevice(Read_Request);
	serialopen = FALSE;
	return(0);
}


asy_ioctl(interface, argc, argv)
struct interface *interface;
int argc;
char *argv[];
{
	if ( argc < 1 ) 
	{
		printf("%d\n", asy[interface->dev].speed);
		return 0;
	}

	return ( asy_speed( interface->dev, atoi(argv[0])));
}

asy_speed(dev, baud)
int dev, baud;
{
	register struct IOExtSer *iob = Read_Request;
	int	speed;
	int e;

	/*
	printf("asy_speed, baud = %d\n\r", baud);
	*/
	
	if(baud == 0 || dev >= nasy)
	{
		printf("Baud = %d, dev = %d, returning BAD!!\n", baud, dev);
		return(-1);
	}
	asy[dev].speed = baud;
	/*
	printf("serialopen = %d\n", serialopen);
	*/
	if (!serialopen ) 
		return(-1);

	/* fill in parameters */

	iob->io_CtlChar = 0x1113000L;

	if (baud >= 0L && baud <= 19200L) 
	{
		iob->io_Baud = baud;
	}

	setmem(&iob->io_TermArray, sizeof(struct IOTArray), 0x12);

	iob->io_ReadLen = iob->io_WriteLen = 8L;
	iob->io_StopBits = 1L;
	iob->io_SerFlags = SERF_XDISABLED ;

	if ( baud > 19200L)
	{
		iob->io_SerFlags |= SERF_RAD_BOOGIE;
	}
	else
	{
		iob->io_SerFlags &= ~SERF_RAD_BOOGIE;
	}

	iob->io_SerFlags &= ~(SERF_PARTY_ON | SERF_EOFMODE);

	/* set the parameters */
	
	iob->IOSer.io_Command = SDCMD_SETPARAMS;

	DoIO(iob);
	return(0);
}


asy_output(dev, buf, n)
int dev;
char *buf;
int n;
{
	int err;
	register struct IOExtSer *write = Write_Request;
/*	
	printf("asy_output\n");
	printf("asy_output: dev = %d, count = %d\n", dev, n);
	printf("asy_output: serialopen = %d\n", serialopen);
*/	

	if (!serialopen) 
	{
		printf("asy_output: returning -1\n");
		return(-1);
	}
	write->IOSer.io_Command = CMD_WRITE;
	write->IOSer.io_Data    = (APTR)buf;
	write->IOSer.io_Length  = (long)n;
	if ( ( err = DoIO(write)) != 0)
	{
		printf("asy_output: err = %d\n", err);
	}
	return(n);
}

asy_recv(dev,buf,cnt)
int dev;
char *buf;
unsigned cnt;
{
	register struct IOExtSer *read = Read_Request;
	int tot = 0;
	struct Store_input *store;
	char store_it[MAX_STORE];
	int got = 0;
	int tt, e;
	struct asy *ap;

#ifdef DEBUG
printf("asy_recv: dev = %d\n", dev);	
#endif

	if ( dev > nasy )
	{
		printf("Error on asy_recv. dev = %d, max = %d\n", dev, nasy);
		return(-1);
	}

	ap = &asy[dev];

	store = &Store_input[dev];		/* point to current */

/* printf("asy_recv: want %d, buffer has %d bytes\n", cnt, store->amt);	*/

	if ( cnt > 0 && store->amt > 0)
	{
		tt = min(cnt , store->amt);
		tt = min(tt,	MAX_STORE);
		got = 0;
		
/*
 printf ( "(if cnt = %d && store->amt = %d) tt = %d\n", cnt, store->amt, tt);
*/

		while ( tt-- > 0)
		{
			buf[got++] = *store->tail++;
			if ( store->tail >= &store->store[MAX_STORE])
			{
				store->tail = store->store;	/* wrap around */
			}
			if ( got >= cnt )
				break;
		}
		store->amt -= got;
		return( got );
	}
	else if ( cnt == 0 )
	{
		return(0);
	}

	/* set up the query */
	read->IOSer.io_Command = SDCMD_QUERY;

	/* perform read quickly if possible */
	read->IOSer.io_Flags = IOF_QUICK;
	e = DoIO(read);
	if ( e != 0)
	{
		printf("asy_read: read failed. Error = %d\n", e);
		return(-1);
	}
	if (  (tt = read->IOSer.io_Actual) > 0) 
	{
		/* set up the read */
		read->IOSer.io_Command = CMD_READ;
		read->IOSer.io_Data    = (APTR)ap->recv_buf;
		read->IOSer.io_Length  = tt;

		e =  DoIO(read);

		if ( e != 0)
		{
			printf("asy_read: read failed. Error = %d\n", e);
			return(-1);
		}
		tt = read->IOSer.io_Actual;
		got = 0;
		while (tt-- > 0)
		{
			*store->head++ = ap->recv_buf[got++];
			if ( store->head == &store->store[MAX_STORE] )
			{
				/* printf("xx got wrap\n"); */
				store->head = store->store;
			}
			store->amt++;
		}
		if ( cnt > 0 && store->amt > 0)
		{
			tt = min(cnt , store->amt);
			tt = min(tt,	MAX_STORE);
			got = 0;
		
/*	printf ( "(if cnt = %d && store->amt = %d) tt = %d\n", cnt, store->amt, tt); */

			while ( tt-- > 0)
			{
				buf[got++] = *store->tail++;
				if ( store->tail >= &store->store[MAX_STORE])
				{
					store->tail = store->store;	/* wrap around */
				}
				if ( got >= cnt )
					break;
			}
			store->amt -= got;
			return( got );
		}
		return ( 0 );
	}
	return (got);

}

long
clksec()
{
	long tloc;

	time(&tloc);
	return(tloc);

}

/* Create a directory listing in a temp file and return the resulting file
 * descriptor. If full == 1, give a full listing; else return just a list
 * of names.
 */
FILE *
dir(path,full)
char *path;
int full;
{
    FILE *fp;
    char cmd[50];

    if(full)
    	sprintf(cmd,"list >RAM:dir.tmp %s",path);
    else
    	sprintf(cmd,"dir >RAM:dir.tmp %s",path);
    system(cmd);
    fp = fopen("RAM:dir.tmp","r");
	unlink("RAM:dir.tmp");
    return fp;
}

disable()
{
	/* Disable(); */
}

restore()
{
	/* Enable(); */
}

stxrdy()
{
	return(1);
}

Fail(str)
char *str;
{
	char mybuff[255];
	printf("Failure: %s\n", str);
}
eihalt()
{
	FILE *fp;
	return(0);
}
memstat()
{}
int KeepGoing = TRUE;
char
kbread()
{
	char c;
	if ( WaitForChar(Output(), 2) == 0)
	{
		return(-1);
	}
	Read( Input(), &c, 1);
	return(c);
}

/* wildcard filename lookup */
static	DIR	 *dirp;
static	struct FileInfoBlock *dp;
static 	IsOpen = 0;
filedir(name, times, ret_str)
char *name;
int times;
char *ret_str;
{
	char *ptr;
	char *index();
	char *rindex();
	char *name_ptr;
	/*
	 * Make sure that the NULL is there incase we don't find anything
	 */
	ret_str[0] = NULL;
	if ( ( name_ptr = rindex ( name, '.')) != NULL)
	{
		name_ptr++;
	}
	else
	{
		/*
		 * no name to find, so go back
		 */
		return;
	}


	if ( times == 0 && IsOpen == 0 )
	{
		if ( (dirp = opendir(mailqdir)) == NULL ) {
			return;
		}
		IsOpen = 1;
	}

	for ( dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if ( ( ptr = index( dp ->fib_FileName, '.')) == NULL) {
			continue;
		}
		else {
			ptr++;
			if ( strncmp(ptr, name_ptr, strlen(name_ptr)) != 0) {
				continue;
			}
			else {
				sprintf(ret_str,"%s", dp->fib_FileName);
				break;
			}
		}
	}
	if ( dp == NULL )
	{
		IsOpen = 0;
		closedir(dirp);
	}
}


/* checks the time then ticks and updates ISS */
static long clkval = 0;
void
check_time()
{
	int32 iss();

	if(clkval != clksec()){
		clkval = clksec();
		tick();
		(void)iss();
	}
}

memcmp(str1, str2, len)
char *str1, *str2;
int len;
{
	return(strncmp(str1, str2, len));
}

memset(str, value, len)
char *str;
int value, len;
{
	while ( len-- !=  0)
	{
		*str++ = value;
	}
}

memcpy(str1, str2, len)
char *str1, *str2;
int len;
{
	while ( len-- != 0 )
	{
		*str1++ = *str2++;
	}
}

/* Change working directory */
docd(argc,argv)
int argc;
char *argv[];
{
	char dirname[128],*getwd();

	if(argc > 1){
		if(chdir(argv[1]) == -1){
			printf("Can't change directory\n");
			return 1;
		}
	}
	if(getwd(dirname,0) != NULLCHAR){
		printf("%s\n",dirname);
	}
	return 0;
}

/* List directory to console */
dodir(argc,argv)
int argc;
char *argv[];
{
	char *path;
	char buff[255];

	if ( argc > 2)
		sprintf(buff, "dir %s\n", argv[1]);
	else
		sprintf(buff, "dir");

	Execute(buff, NULL, NULL);
	return 0;
}

doshell()
{
	if ( ( Fptr = Open("CON:0/0/640/40/CLI", MODE_NEWFILE) ) == 0L)
	{
		printf("doshell: could not open window.  Error = %d\n", IoErr());
		return;
	}
	if ( Execute("", Fptr, NULL) != -1)
	{
		printf("Could not execute the newcli command.\n");
	}
	(void)Close(Fptr);

	return;
}

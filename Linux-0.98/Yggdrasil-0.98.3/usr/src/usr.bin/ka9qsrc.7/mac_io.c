/* OS- and machine-dependent stuff for Mac  */

/*mac_io.c
 *	Routines for Macintosh IO and file stuff
 */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "internet.h"
#include "iface.h"
#include "mac.h"
#include "cmdparse.h"

#include "DeviceMgr.h"
#include "WindowMgr.h"
#include "EventMgr.h"
#include "SerialDvr.h"
#include "HFS.h"
#include <time.h>
extern long timezone;
extern errno;

struct Store_input Store_input[ASY_MAX];

struct asy asy[ASY_MAX];

static ioParam	MacSer;

struct interface *ifaces;
static struct RemoveIt {
	struct RemoveIt *next;
	char *name_ptr;
} *Head;


/* Called at startup time to set up console I/O, memory heap */
ioinit()
{
	timezone = 60*60+1;
	tzname[0] = "CST";
	tzname[1] = "CST";
	Click_On(0);
	Head = malloc(sizeof (struct RemoveIt));
	Head->next = NULL;
	Head->name_ptr = NULL;
	return(0);
}
/* Called just before exiting to restore console state */
iostop()
{
	int i;
	struct RemoveIt *rptr;
	
	while(ifaces != NULLIF){
		if(ifaces->stop != NULLFP)
			(*ifaces->stop)(ifaces);
		ifaces = ifaces->next;
	}
	/*
	 * I want to close down all the files and then remove the (possibly still existing) files
	 * because the MAC will not allow the file to be removed when it still has an open file
	 * descriptor
	 */
	 
	for( i = 3; i < _NFILE; i++)
	{
		close(i);
	}
	(void)unlink("dir.temp");
	rptr = Head;
	while( rptr->next != NULL)
	{
		if ( rptr->name_ptr != NULL)
			unlink(rptr->name_ptr);
		rptr = rptr->next;
	}	
}

/* Initialize asynch port "dev" */

int slipisopen = 0;
int iref;
int oref;

char Recv_buf[MAX_STORE];
char Send_buf[MAX_STORE];

int
asy_init(dev,arg1,arg2,bufsize)
int16 dev;
char *arg1,*arg2;
unsigned bufsize;
{
	register struct asy *ap;
	register struct interface *if_asy;
	extern struct interface *ifaces;
	char ser_name[255];
	struct Store_input *store;
	
	OsErr e;
	ap = &asy[dev];

	ap->tty = malloc(strlen(arg2)+1);
	strcpy(ap->tty, arg2);

#ifdef DEBUG
	printf("asy_init: as->tty = %s, dev = %d\n", ap->tty, dev); 
#endif
	if ( ap == NULL)
	{
		return(-1);
	}
	
	switch ( ap->tty[0] )
	{
		case 'a':
		case 'A':
			if ( ap->devopen == 0 )
			{
				e = RAMSDOpen(sPortA);
				if ( e != noErr )
				{
					printf("RAMSDOpen failed, e = %d\n",e);
					SysBeep(4);
					ExitToShell();
				}
				e = OpenDriver ("\p.AIn", &iref);
				if ( e != noErr) {
					printf("OpenDriver for AIn failed, e = %d\n",e);
					SysBeep (4);
					ExitToShell ();
				}
				e = OpenDriver ("\p.AOut", &oref);
				if ( e != noErr) {
					printf("OpenDriver for AOut failed, e = %d\n",e);
					SysBeep (4);
	    			CloseDriver (iref);
					ExitToShell ();
				}
				ap->portIn = AinRefNum;
				ap->portOut = AoutRefNum;
				ap->devopen = 1;
			}
			else
			{
				printf("Device %c is already open.\n", ap->tty[0]);
				return(-1);
			}
			break;
		
		case 'b':
		case 'B':
			if ( ap->devopen == 0)
			{
				e = RAMSDOpen(sPortB);
				if ( e != noErr )
				{
					printf("RAMSDOpen failed, e = %d\n", e);
					SysBeep(4);
					ExitToShell();
				}
				e = OpenDriver ("\p.BIn", &iref);
				if ( e != noErr) {
					printf("OpenDriver for BIn failed, e = %d\n", e);
					SysBeep (4);
					ExitToShell ();
				}
				e = OpenDriver ("\p.BOut", &oref);
				if ( e != noErr) {
					printf("Opendriver for Bout failed e = %d\n",e);
					SysBeep (4);
	 		   		CloseDriver (iref);
					ExitToShell ();
				}
				ap->portIn = BinRefNum;
				ap->portOut = BoutRefNum;
				ap->devopen = 1;
			}
			else
			{
				printf("Device %c is already open.\n", ap->tty[0] );
				return(-1);
			}
			break;
		
		default:
			printf("Unknown device %c, could not configure port,\n", ap->tty);
			return(-1);
	}
		
		
	store = &Store_input[dev];
	store->head =  store->store;
	store->tail = store->store;
	store->amt = 0;
	e = SerSetBuf( ap->portIn, Recv_buf, MAX_STORE);
	if ( e != noErr) {
		printf("asy_init: SerSetBuf error on %d\n", ap->portIn);
		SysBeep (4);
	    CloseDriver (iref);
		ExitToShell ();
	}
	SerSetBuf( ap->portOut, Send_buf, MAX_STORE);
	if ( e != noErr) {
		printf("asy_init: SerSetBuf error on %d\n", ap->portOut);
		SysBeep (4);
	    CloseDriver (iref);
		ExitToShell ();
	}
	slipisopen = 1;
	return (0);
}

int
asy_stop(interface)
struct interface *interface;
{
	register struct asy *ap;
	int ref;
	
	ap = &asy[interface->dev];

	if (slipisopen)
	{
		MacSer.ioActCount = 0;
		MacSer.ioRefNum = ap->portIn;
		MacSer.ioCompletion = 0;
		MacSer.ioBuffer = ap->recv_buf;
		MacSer.ioReqCount = 0;
		MacSer.ioPosMode = 1;
		(void) PBKillIO(&MacSer, FALSE);
		
		MacSer.ioActCount = 0;
		MacSer.ioRefNum = ap->portOut;
		MacSer.ioCompletion = 0;
		MacSer.ioBuffer = ap->recv_buf;
		MacSer.ioReqCount = 0;
		MacSer.ioPosMode = 1;
		(void) PBKillIO(&MacSer, FALSE);

		FSClose(ap->portIn);
		FSClose(ap->portOut);
		if ( ap->portIn = AinRefNum)
		{
			OpenDriver("\p.Ain", &ref);
			OpenDriver("\p.Aout", &ref);
		}
		else
		{
			OpenDriver("\p.Bin", &ref);
			OpenDriver("\p.Bout", &ref);
		}
		
		slipisopen = 0;
	}
}

/* Asynchronous line I/O control */
asy_ioctl(interface,argc,argv)
struct interface *interface;
int argc;
char *argv[];
{
	if(argc < 1){
		printf("%d\r\n",asy[interface->dev].speed);
		return 0;
	}
	return asy_speed(interface->dev,atoi(argv[0]));
}


/* Set asynch line speed */
int
asy_speed(dev,speed)
int dev;
int speed;
{
	OsErr e;
	int serialconfig;
	SerShk	HandShake;
	struct asy *ap;

	
	ap = &asy[dev];

	if(speed == 0 || dev >= nasy)
		return(-1);
	
#ifdef DEBUG
	printf("asy_speed: Setting speed for device %d to %d\n",dev, speed);
#endif
	asy[dev].speed = speed;

	switch(speed)
	{
		case 300:
			serialconfig = baud300;
			break;
		case 600:
			serialconfig = baud600;
			break;
		case 1200:
			serialconfig = baud1200;
			break;
		case 1800:
			serialconfig = baud1800;
			break;
		case 2400:
			serialconfig = baud2400;
			break;
		case 3600:
			serialconfig = baud3600;
			break;
		case 4800:
			serialconfig = baud4800;
			break;
		case 7200:
			serialconfig = baud7200;
			break;
		case 9600:
			serialconfig = baud9600;
			break;
		case 19200:
			serialconfig = baud19200;
			break;
		case (long)57600:
			serialconfig = baud57600;
			break;
		default:
			printf("asy_speed: Unknown speed (%ld)\n", speed);
			break;
	}
/*	
	printf("serialconfig = %d\n", serialconfig);
*/
	serialconfig |= (stop10|noParity|data8);


	e = SerReset( ap->portIn, serialconfig);
	if ( e != noErr) {
		printf("asy_speed: could not set config for %d\n", ap->portIn);
		SysBeep (4);
	    CloseDriver (iref);
		ExitToShell ();
	}
	
	bzero(&HandShake, sizeof (SerShk));
	e = SerHShake(ap->portIn, &HandShake);
	if ( e != noErr) {
		printf("asy_speed: could not set handshake for %d\n", ap->portIn);
		SysBeep (4);
	    CloseDriver (iref);
		ExitToShell ();
	}
	
	e = SerReset( ap->portOut, serialconfig);
	if ( e != noErr) {
		printf("asy_speed: could not set config for %d\n", ap->portOut);
		SysBeep (4);
		ExitToShell ();
	}
	bzero(&HandShake, sizeof (SerShk));
	e = SerHShake(ap->portOut, &HandShake);
	if ( e != noErr) {
		printf("asy_speed: could not set handshake for %d\n", ap->portOut);
		SysBeep (4);
	    CloseDriver (iref);
		ExitToShell ();
	}
	/*
	printf("asy_speed: completed.\n");
	*/
	return(0);

}
/* Send a buffer to serial transmitter */
asy_output(dev,buf,cnt)
unsigned dev;
char *buf;
unsigned short cnt;
{
	register struct asy *ap;
	long amount = (long)cnt;
	ap = &asy[dev];
	/*
	printf("asy_output called. dev = %x, cnt = %d\n", dev, cnt);
	*/
	if(dev >= nasy)
		return(-1);
	FSWrite(ap->portOut,&amount, buf);
	return(0);

}

/*
 * Receive characters from asynch line
 * Returns count of characters read
 */

unsigned
asy_recv(dev,buf,cnt)
int dev;
char *buf;
unsigned cnt;
{
	long amount[8];
	int tot = 0;
	struct Store_input *store;
	char store_it[MAX_STORE];
	int got = 0;
	int tt;
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
	/* printf("got = %d\n", got); */
		return( got );
	}
	else if ( cnt == 0 )
	{
		return(0);
	}
	
	Status( ap->portIn, 2, &amount[0]);
	if (  amount[0] > 0L) 
	{
		MacSer.ioRefNum = ap->portIn;
		MacSer.ioCompletion = 0;
		MacSer.ioBuffer = ap->recv_buf;
		MacSer.ioReqCount = (amount[0] < MAX_STORE) ? amount[0] : MAX_STORE;
		MacSer.ioPosMode = 1;
		tot = PBRead(&MacSer, FALSE);
		if ( MacSer.ioResult != noErr)
		{
			printf("asy_read: read failed.\n");
		}
		tt = MacSer.ioActCount;
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

/*
 * dir: Create a directory listing in a temp file and return the resulting file
 * descriptor. If full == 1, give a full listing; else if full ==3, output to console,
 * else return just a list
 * of names.
 *
 */
FILE *
dir(path,full)
char *path;
int full;
{
	WDPBRec Mydisk;
	FILE *fp;
	CInfoPBRec Everything;
	OSErr e;
	char working_vol[255];
	char working_dir[255];
	char holding_file[255];
	char *PtoCstr(), *ctime();
	char *ptr;
	char buff[255];


	bzero(&Mydisk, sizeof(WDPBRec));
	bzero(&Everything, sizeof(Everything));

	errno = 0;
	if ( full < 3)
	{
		if ( ( fp = fopen("dir.temp", "w")) == NULL)
		{
			printf("Open failed, errno = %d\n",errno);
			return((FILE *)NULL);
		}
	}
	else
	{
		fp = stdout;
	}
	
	Mydisk.ioWDProcID = 0L;
	Mydisk.ioWDDirID = 0L;
	Mydisk.ioVRefNum = 0;
	Mydisk.ioWDVRefNum = 0;
	Mydisk.ioNamePtr = (StringPtr)working_vol;
	if ( path[0] == '\0' )
	{
		path = ":";
	}
	MoveIt(Mydisk.ioNamePtr, path);
	
	e = PBOpenWD( &Mydisk, FALSE);
	
	if ( e != noErr )
	{
		printf("ERROR in PWOpenWD\n");
		if ( full < 3)
		{
			fclose(fp);
		}
		return((FILE *)NULL);
	}

	Everything.hFileInfo.ioNamePtr = (StringPtr)holding_file;
	Everything.hFileInfo.ioVRefNum = Mydisk.ioVRefNum;
	sprintf(Everything.hFileInfo.ioNamePtr, "\p");
	Everything.hFileInfo.ioFDirIndex = 1;
	Everything.hFileInfo.ioDirID = Mydisk.ioWDDirID;
	Everything.hFileInfo.ioCompletion = 0;

	while( (e = PBGetCatInfo( &Everything, FALSE)) != fnfErr)
	{
		if ( e == noErr )
		{
			if( full == 0)
			{
				fprintf(fp, "%s\n",
					 PtoCstr(Everything.hFileInfo.ioNamePtr));
			}
			else
			{
				
				ptr = ctime((long *)&Everything.hFileInfo.ioFlCrDat);
				ptr +=3;
				ptr[strlen(ptr)-9] = '\0';
				fprintf(fp,"%c  %7ld  %s  %s\n",
					(Everything.hFileInfo.ioFlAttrib & (1<<4))?'d':'-',
					Everything.hFileInfo.ioFlLgLen +
					 Everything.hFileInfo.ioFlRLgLen, ptr,
					PtoCstr(Everything.hFileInfo.ioNamePtr));
			}
			
			Everything.hFileInfo.ioFlLgLen = 0;
			Everything.hFileInfo.ioFDirIndex++;

		}
		else
		{
			PBCloseWD(&Mydisk, FALSE);
			if ( full < 3 )
			{
				fclose(fp);
				fp = fopen("dir.temp", "r");
				unlink("dir.temp");
				return(fp);
			}
		}
		Everything.hFileInfo.ioDirID = Mydisk.ioWDDirID;
	}
	PBCloseWD(&Mydisk, FALSE);
	if ( full < 3)
	{
		fclose(fp);
		fp = fopen("dir.temp", "r");
		unlink("dir.temp");
	}
	return(fp);
}

/*
 * MoveIt: this is like a bcopy, but uses pascal string type
 * of data.
 */

MoveIt(to,from)
char *to, *from;
{
	int size;

	bzero(to, 3);
	to[0] = size = strlen(from);
	if ( size <= 0)
	{
		printf("MoveIt: size <= 0 (%d\n", size);
		return;
	}
	to++;
	while( size-- )
		*to++ = *from++;	
}

/*
 * mktemp: make temporary file and return a pointer to it.
 */

char *mktemp(ff)
char *ff;
{
	long tt;
	char *ptr;
	if ( ( ptr = index(ff, 'X') ) == NULL)
	{
		ptr = ff;
	}
	tt = TickCount();
	tt &= 0xffffffff;
	sprintf(ptr, "%ld", tt);
	return(ff);
}

/*
 * index: find the first char in string that matches arg.  LSC does not
 * have this routine.
 */

char *index(str, find)
char *str;
char find;
{
	char *strchr();

	return(strchr(str, find));
}


static WDPBRec Mydisk;
static CInfoPBRec Everything;
static IsOpen = 0;	/* tells me that the file is open */
static char *name_ptr;
static char working_dir[255];
static char holding_file[255];
static 	char working_vol[255];

/*
 * filedir: find a file that matches the dest arg
 */
 
char *filedir(path,value,dest)
char *path;
int   value;
char *dest;
{

	FILE *fp;
	OSErr e;
	char *PtoCstr(), *ctime();
	char *ptr;
	char buff[255];
	char *rindex();
	char keep;
	
	dest[0] = '\0';
	if ( IsOpen == 0 && value == 0)
	{
		Mydisk.ioWDProcID = 0L;
		Mydisk.ioWDDirID = 0L;
		Mydisk.ioVRefNum = 0;
		Mydisk.ioWDVRefNum = 0;
		Mydisk.ioNamePtr = (StringPtr)working_vol;
		if ( path[0] == '\0' )
		{
			path = ":";
		}
		if ( (ptr = rindex(path, ':')) != NULL)
		{
			keep = *ptr;
			*ptr = '\0';
			MoveIt(Mydisk.ioNamePtr, path);
			*ptr = keep;
		}
		else
		{
			MoveIt(Mydisk.ioNamePtr, path);
		}
		if ( ( name_ptr = rindex( path, '.')) != NULL)
		{
			name_ptr++;
		}
		else
		{
			name_ptr = "";
		}
	}
	
	e = PBOpenWD( &Mydisk, FALSE);
	
	if ( e != noErr )
	{
		printf("ERROR in PWOpenWD\n");
		IsOpen = 0;
		return( (char *)-1);
	}
		
		/* mark as open */
		
	IsOpen = 1;

	/*
	 * set up for getting the info from the directory
	 */
	 
	Everything.hFileInfo.ioNamePtr = (StringPtr)holding_file;
	Everything.hFileInfo.ioVRefNum = Mydisk.ioVRefNum;
	sprintf(Everything.hFileInfo.ioNamePtr, "\p");
	
	if ( value == 0 )
	{
		Everything.hFileInfo.ioFDirIndex = 1;
	}
	
	Everything.hFileInfo.ioDirID = Mydisk.ioWDDirID;
	Everything.hFileInfo.ioCompletion = 0;

	
	while( (e = PBGetCatInfo( &Everything, FALSE)) != fnfErr)
	{
		if ( e == noErr )
		{
			
			PtoCstr(Everything.hFileInfo.ioNamePtr);
			if ( ( ptr = index( Everything.hFileInfo.ioNamePtr, '.')) != NULL) 
			{
				ptr++;
				if ( strncmp(ptr, name_ptr, strlen(name_ptr)) == 0) 
				{
					sprintf(dest,"%s", Everything.hFileInfo.ioNamePtr);
					Everything.hFileInfo.ioFDirIndex++;
					PBCloseWD(&Mydisk, FALSE);
					return;
				}
			}
			Everything.hFileInfo.ioFlLgLen = 0;
			Everything.hFileInfo.ioFDirIndex++;
			Everything.hFileInfo.ioDirID = Mydisk.ioWDDirID;

		}
		else
		{
			IsOpen = 0;
			PBCloseWD(&Mydisk, FALSE);
			return(0);
		}
		Everything.hFileInfo.ioDirID = Mydisk.ioWDDirID;
	}
	IsOpen = 0;
	PBCloseWD(&Mydisk, FALSE);
}

/*
 * rindex: find the last char in string which matches arg.  LSC does not
 * have this routine.
 */

char *rindex(str, c)
char *str, c;
{
	char *strrchr();
	return(strrchr(str, c));
}

int asy_attach();
extern struct cmds attab[];

/*
 * eihalt: normally this would wait for an interrupt, but it doesn't here.
 * this will check for the button.  If the button has been depressed, the process
 * exits
 */
 
eihalt()
{
	if (Button())
	{
		iostop();
		exit(0);
	}
	return(0);
}

/*
 * kbread: see if a char is available on keyboard, if so go get it.
 */
kbread()
{
	int	mask;
	int	ok;
	char	c = -1;
	if( kbhit() ){
		c = getch();
	}
	return(c);
}

/*
 * clksec: return the amount of time in secs.
 */
 
int32
clksec()
{
	return(time(NULL));
}

/*
 * tmpfile: create a temporary file.  Remember it so we can delete it later because
 * the mac does not allow the file to be deleted when it is open.
 */
 
FILE *
tmpfile()
{
	FILE *tmp;
	char *mktemp();
	char *ptr = "SMTPXXXXXXXX";
	char *name;
	struct RemoveIt *rptr;
	
	name = mktemp(ptr);
	if ( ( tmp = fopen(name, "w+") ) == NULL)
	{
		printf("tmpfile: counld not create temp file. (%s)\n", name);
		return(NULL);
	}
	rptr = Head;
	while(rptr->next != NULL )
	{
		rptr = rptr->next;
	}
	
	if ( (rptr->next = (struct RemoveIt *)malloc(sizeof (struct RemoveIt)) ) == NULL)
	{
		printf("Could not allocate memory for structure RemoveIt\n");
		return(tmp);
	}
	rptr = rptr->next;
	rptr->next = NULL;
	if ( (rptr->name_ptr = malloc(strlen(name)+1) ) == NULL)
	{
		printf("Could not allocate memory for %s\n", name);
		return(tmp);
	}
	
	sprintf(rptr->name_ptr, "%s", name);
	return ( tmp );
}

/* restore: stub */
restore()
{}

/* stxrdy: stub */
stxrdy()
{return(1);}

/* disable: stub */
disable()
{}

/* memstat: stub */
memstat()
{
	return(0);
}

/* checks the time then ticks and updates ISS */
static int32 clkval = 0;
void
check_time()
{
	int32 iss();
	int32 clksec();

	if(clkval != clksec()){
		clkval = clksec();
		tick();
		(void)iss();
	}
}

/*
 * Access: check the file for access permission.  Some of this has to be faked on
 * a Mac, since it does not have access bits.
 */

access(str, perm)
 char *str;
 int perm;
{
	FILE *fptr;
	CInfoPBRec	paramBlock;
	StringPtr CtoPstr();
	OSErr e;
	
	paramBlock.hFileInfo.ioCompletion = 0;
	paramBlock.hFileInfo.ioVRefNum = 0;
	paramBlock.hFileInfo.ioFDirIndex = 0;
	paramBlock.hFileInfo.ioDirID = 0;
	paramBlock.hFileInfo.ioNamePtr = CtoPstr(str);

	/*
	 * Get info on file named in ioNamePtr
	 */
	 
	e = PBGetCatInfo( &paramBlock, FALSE);
	PtoCstr(str);
	
	/*
	 * if there is an error then find out if the file is present.  If so
	 * see if user wants to create the file
	 */
	 
	if ( e != noErr )
	{
		if ( ( e == fnfErr) && (perm == 2))
		{
			return(0);
		}
		else
		{
			return(1);
		}
	}
	/*
	 * check to see if the file is locked or open.  refuse it if it is.
	 */
	 
	if ( (BitTst( &paramBlock.hFileInfo.ioFlAttrib, 0) 
			|| BitTst( &paramBlock.hFileInfo.ioFlAttrib, 7) )
			&& perm == 4 )
	{
		return(1);
	}
	else
		return(0);
	
}

/*
 * Bzero: zero out a buffer
 */

bzero(str, cnt)
char *str;
int	cnt;
{
	while(cnt-- != 0)
		*str++ = '\0';
}

/*
 * memcmp: compare memory
 */
 
memcmp(str1, str2, len)
char *str1, *str2;
int len;
{
	return( strncmp(str1, str2, len));
}

/* 
 * memset: set a value in memory
 */
 
memset(str, value, len)
char *str;
int value, len;
{
	while ( len-- != 0)
	{
		*str++ = value;
	}
}

/*
 * memcpy: copy from one place to another
 */

memcpy(str1, str2, len)
char *str1, *str2;
int len;
{
	while ( len-- != 0)
		*str1++ = *str2++;
}

/* List directory to console */
dodir(argc,argv)
int argc;
char *argv[];
{
	if ( argc > 2)
		dir( argv[1], 3);
	else
		dir(":",3);
	return 0;
}

/*
 * docd: perform a change directory
 */

docd(argc, argv)
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

chdir( path )
char * path;
{
	WDPBRec	Mydisk;
	int err;
	register char * cp;
	char *index();
	OSErr e;

#ifdef DEBUG
	printf("Entering chdir\n");
#endif

	Mydisk.ioWDProcID = 0L;
	Mydisk.ioWDDirID = 0L;
 	Mydisk.ioVRefNum = 0;
	printf("chdir: path = %s\n", path);
	Mydisk.ioWDVRefNum = 0;
	CtoPstr(path);
	Mydisk.ioNamePtr = path;
#ifdef DEBUG
	printf("chdir: calling PBOpenWD.\n");
	printf("Mydisk.ioVRefNum = %d\n", Mydisk.ioVRefNum);
#endif	
	e = PBOpenWD( &Mydisk, FALSE);
	
	if ( e != noErr )
	{
		PtoCstr(path);
		return(-1);
	}
	
	if ( ( e = PBHSetVol( &Mydisk, FALSE) ) != noErr)
	{
		PtoCstr(path);
		return(-1);
	}

	PtoCstr(path);
	return 0;

}

char *getwd()
{
	return(NULL);
}

/*
 * doshell:  execute a shell for the user
 */
 
doshell()
{
	printf("SHELL is not implemented.\n");
}

char *timez = "CST";

char *getenv(str)
char *str;
{
	return(timez);
}

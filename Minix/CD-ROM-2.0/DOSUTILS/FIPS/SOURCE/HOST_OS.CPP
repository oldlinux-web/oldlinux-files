// host_os.cpp	host operating system classes
//		dave mccaldon (d.mccalden@staffordshire.ac.uk)

#include "host_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

char *msdos_info (void)
{
	return ("MS-DOS version %d.%d");
};

char *dosemu_info (void)
{
	return ("Linux dosemu version %d.%d");
};

char *mswindows_info (void)
{
	return ("MS-Windows version %d.%d");
};

char *desqview_info (void)
{
	return ("Desq-View version %d.%d");
};

host_os::host_os (void)
{
	status = NOT_OK;
	if (mswindows_detect () == true) format = mswindows_info;
//	else if (dosemu_detect () == true) format = dosemu_info;
	else if (desqview_detect () == true) format = desqview_info;
	else
	{
		status = OK;
		msdos_version ();
		format = msdos_info;
	}
}


char *host_os::information( char *p )
{
	if( p == NULL )
		p = (char *) malloc( strlen( format() ) + 12 );
	sprintf( p, format(), ver_major, ver_minor );

	return p;
}

void host_os::msdos_version()
{
	ver_major = _osmajor;			// borlandc constants
	ver_minor = _osminor;
}

boolean host_os::mswindows_detect()
{
	union   REGS r;

	r.x.ax = 0x1600;
	int86( 0x2F, &r, &r );
	if( r.h.al & 0x7F )
	{
		ver_major = r.h.al;
		ver_minor = r.h.ah;
		return (true);
	}

	return (false);
}

boolean host_os::dosemu_detect()
{
	union   REGS r;

	// this is slightly more difficult than just calling the dosemu
	// interrupt (0xE5), we need to check if the interrupt has a
	// handler, as DOS and BIOS don't establish a default handler

	if( getvect( 0xE5 ) == NULL )
		return (false);
	r.x.ax = 0;
	int86( 0xE5, &r, &r );
	if( r.x.ax == 0xAA55 )			// check signature
	{
		ver_major = r.h.bh;
		ver_minor = r.h.bl;
		return (true);
	}

	return (false);
}

boolean host_os::desqview_detect()
{
	union   REGS r;

	r.x.ax = 0x2B01;                // AL=01 => get desqview version
	r.x.cx = 0x4445;		// CX = 'DE'
	r.x.dx = 0x5351;		// DX = 'SQ'
	int86( 0x21, &r, &r );
	if( r.h.al != 0xFF )
	{
		ver_major = r.h.bh;
		ver_minor = r.h.bl;
		return (true);
	}

	return (false);
}

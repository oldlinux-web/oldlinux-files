/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module disk_io.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/disk_io.cpp 1.4 1995/01/19 00:00:51 schaefer Exp schaefer $

	Copyright (C) 1993 Arno Schaefer

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


	Report problems and direct all questions to:

	schaefer@rbg.informatik.th-darmstadt.de
*/

#include "disk_io.h"
#include <dos.h>
#include <bios.h>

#define DISK_INT 0x13

#define RESET_DISK 0
#define READ_SECTOR 2
#define WRITE_SECTOR 3
#define VERIFY_SECTOR 4
#define GET_DRIVE_PARAMS 8
#define GET_DISK_TYPE 0x15


/* ----------------------------------------------------------------------- */
/* Bios call to get the number of drives attached                          */
/* ----------------------------------------------------------------------- */

int get_no_of_drives (void)
{
	union REGS regs;

	regs.h.ah = GET_DRIVE_PARAMS;
	regs.h.dl = 0x80;
	int86 (DISK_INT, &regs, &regs);

	if (regs.h.ah != 0) return (1); // will be checked again
	return (regs.h.dl);
}


/* ----------------------------------------------------------------------- */
/* Calculates physical sector number (Head, Cylinder, Sector).             */
/* Log_sector is absolute logical sector number (0 = master boot record).  */
/* ----------------------------------------------------------------------- */

physical_sector_no::physical_sector_no
(
	dword logical_sector,
	const drive_geometry &geometry
)
{
	cylinder =
		logical_sector
		/ (geometry.heads * geometry.sectors);

	head =
	(
		logical_sector
		- (cylinder * geometry.heads * geometry.sectors)
	)
	/ geometry.sectors;

	sector =
		logical_sector
		- (cylinder * geometry.heads * geometry.sectors)
		- (head * geometry.sectors)
		+ 1;
}


/* ----------------------------------------------------------------------- */
/* Bios call get_drive_geometry, returns error status in var errorcode     */
/* ----------------------------------------------------------------------- */

void physical_drive::get_geometry (void)
{
	union REGS regs;

	regs.h.ah = GET_DRIVE_PARAMS;
	regs.h.dl = number;
	int86 (DISK_INT, &regs, &regs);

	if ((errorcode = regs.h.ah) != 0) return;

	geometry.heads = (dword) regs.h.dh + 1;
	geometry.sectors = (dword) regs.h.cl & 0x3f;
	geometry.cylinders =
	(
		(dword) regs.h.ch
		| (((dword) regs.h.cl << 2) & 0x300)
	) + 1;
}


/* ----------------------------------------------------------------------- */
/* Bios call reset_drive, returns error status in var errorcode            */
/* ----------------------------------------------------------------------- */

void physical_drive::reset (void)
{
	union REGS regs;

	regs.h.ah = RESET_DISK;
	regs.h.dl = number;
	int86 (DISK_INT, &regs, &regs);

	errorcode = regs.h.ah;
}


/* ----------------------------------------------------------------------- */
/* Initialization physical_drive, requires drive number.                   */
/* Calls get_drive_geometry, errorcode contains return status              */
/* ----------------------------------------------------------------------- */

physical_drive::physical_drive (int number)
{
	physical_drive::number = number;
	get_geometry ();
};


/* ----------------------------------------------------------------------- */
/* Initialization physical_drive with physical_drive object                */
/* ----------------------------------------------------------------------- */

physical_drive::physical_drive (physical_drive &pd)
{
	number = pd.number;
	errorcode = pd.errorcode;
	geometry = pd.geometry;
}


/* ----------------------------------------------------------------------- */
/* Assignment operator for physical drive                                  */
/* ----------------------------------------------------------------------- */

void physical_drive::operator= (physical_drive &pd)
{
	number = pd.number;
	errorcode = pd.errorcode;
	geometry = pd.geometry;
}


/* ----------------------------------------------------------------------- */
/* Read sector                                                             */
/* ----------------------------------------------------------------------- */

int physical_drive::read_sector (struct sector *sector, dword sector_number)
{
	physical_sector_no p (sector_number, geometry);

	boolean done = false;

	for (int i=0; i<3; i++)
	{
		if (biosdisk
		(
			READ_SECTOR,
			number,
			p.head,
			p.cylinder,
			p.sector,
			1,
			sector->data
		) == 0)
		{
			done=true;
			break;
		}

		reset ();
	}

	if (!done) return (-1);
	return 0;
}


/* ----------------------------------------------------------------------- */
/* Write sector with verify                                                */
/* ----------------------------------------------------------------------- */

int physical_drive::write_sector (struct sector *sector, dword sector_number)
{
	physical_sector_no p (sector_number,geometry);

	boolean done = false;

	for (int i=0; i<3; i++)
	{
		if (biosdisk
		(
			WRITE_SECTOR,
			number,
			p.head,
			p.cylinder,
			p.sector,
			1,
			sector->data
		) == 0)
		{
			done=true;
			break;
		}

		reset ();
	}

	if (!done) return (-1);

	if (biosdisk
	(
		VERIFY_SECTOR,
		number,
		p.head,
		p.cylinder,
		p.sector,
		1,
		sector->data

	) != 0) return (-1);

	return 0;
}


/* ----------------------------------------------------------------------- */
/* Bios call get_disk_type - returns 0 if drive not present.               */
/* Valid drive numbers: 0 - 255, result: 1 - floppy without disk change    */
/* detection, 2 - floppy with disk change detection, 3 - harddisk          */
/* ----------------------------------------------------------------------- */

int get_disk_type (int drive_number)
{
	union REGS regs;

	regs.h.ah = GET_DISK_TYPE;
	regs.h.dl = drive_number;
	int86 (DISK_INT, &regs, &regs);

	if (regs.x.cflag) return 0;
	return (regs.h.ah);                     // disk type
}

/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module hdstruct.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/hdstruct.cpp 1.4 1995/01/19 00:20:01 schaefer Exp schaefer $

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

#include "types.h"
#include "hdstruct.h"

root_sector::root_sector (root_sector &rs)
{
	drive = rs.drive;
	for (int i=0; i<512; i++) data[i] = rs.data[i];
}

void root_sector::operator= (root_sector &rs)
{
	drive = rs.drive;
	for (int i=0; i<512; i++) data[i] = rs.data[i];
}

void harddrive::operator= (harddrive &hd)
{
	physical_drive::operator= (hd);
	*root_sector = *(hd.root_sector);
	partition_table () = hd.partition_table ();
}

void harddrive::get_partition_table (void)
{
	partition_table().get (root_sector);

	for (int i = 0; i < 4; i++)
	{
		class partition_info* p
			= &(partition_table().partition_info[i]);

		if (p->system == 0) continue;

		while
		(
			p->start_sector_abs
			> (
				(p->start_cylinder + 1000UL)
					* geometry.heads
					* geometry.sectors
				+ p->start_head
					* geometry.sectors
				+ p->start_sector
				- 1
			)
		)
		{
			p->start_cylinder += 1024;	// more than 1024 cylinders
		}

		while
		(
			(p->start_sector_abs + p->no_of_sectors_abs - 1)
			> (
				(p->end_cylinder + 1000UL)
					* geometry.heads
					* geometry.sectors
				+ p->end_head
					* geometry.sectors
				+ p->end_sector
				- 1
			)
		)
		{
			p->end_cylinder += 1024;	// more than 1024 cylinders
		}
	}
}


/* ----------------------------------------------------------------------- */
/* Extract Partition Table from root sector                                */
/* ----------------------------------------------------------------------- */

void partition_table::get (root_sector *root_sector)
{
	for (int i=0;i<4;i++)
	{
		class partition_info *p = &partition_info[i];
		byte *pi = &(root_sector->data[0x1be+16*i]);

		p->bootable = *pi;
		p->start_head = *(pi+1);
		p->start_cylinder = *(pi+3) | ((*(pi+2) << 2) & 0x300);
		p->start_sector = *(pi+2) & 0x3f;
		p->system = *(pi+4);
		p->end_head = *(pi+5);
		p->end_cylinder = *(pi+7) | ((*(pi+6) << 2) & 0x300);
		p->end_sector = *(pi+6) & 0x3f;
		p->start_sector_abs = (dword) *(pi+8) | ((dword) *(pi+9) << 8) | ((dword) *(pi+10) << 16) | ((dword) *(pi+11) << 24);
		p->no_of_sectors_abs = (dword) *(pi+12) | ((dword) *(pi+13) << 8) | ((dword) *(pi+14) << 16) | ((dword) *(pi+15) << 24);
	}
}


/* ----------------------------------------------------------------------- */
/* Write Partition Table back into root sector                             */
/* ----------------------------------------------------------------------- */

void partition_table::put (root_sector *root_sector)
{
	for (int i=0; i<4; i++)
	{
		class partition_info p = partition_info[i];
		byte *pi = &(root_sector->data[0x1be+16*i]);

		*pi = p.bootable;
		*(pi+1) = p.start_head;
		*(pi+2) = ((p.start_cylinder >> 2) & 0xc0) | (p.start_sector & 0x3f);
		*(pi+3) = p.start_cylinder & 0xff;
		*(pi+4) = p.system;
		*(pi+5) = p.end_head;
		*(pi+6) = ((p.end_cylinder >> 2) & 0xc0) | (p.end_sector & 0x3f);
		*(pi+7) = p.end_cylinder & 0xff;
		*(pi+8) = p.start_sector_abs & 0xff;
		*(pi+9) = (p.start_sector_abs >> 8) & 0xff;
		*(pi+10) = (p.start_sector_abs >> 16) & 0xff;
		*(pi+11) = (p.start_sector_abs >> 24) & 0xff;
		*(pi+12) = p.no_of_sectors_abs & 0xff;
		*(pi+13) = (p.no_of_sectors_abs >> 8) & 0xff;
		*(pi+14) = (p.no_of_sectors_abs >> 16) & 0xff;
		*(pi+15) = (p.no_of_sectors_abs >> 24) & 0xff;
	}
}

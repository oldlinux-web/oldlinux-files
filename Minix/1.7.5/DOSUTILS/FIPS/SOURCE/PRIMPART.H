/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module hdstruct.h

	RCS - Header:
	$Id$

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

/* ----------------------------------------------------------------------- */
/* Partition Class, derived from logical_drive and raw_partition           */
/* Represents one primary DOS partition. Read_sector() and write_sector()  */
/* are instances of the virtual functions in the logical_drive class       */
/* ----------------------------------------------------------------------- */

#ifndef PRIMPART_H
#define PRIMPART_H

#include "types.h"
#include "disk_io.h"
#include "logdr_st.h"
#include "hdstruct.h"

class partition:public logical_drive
{
public:
	int number;
	physical_drive *drive;
	partition_info *partition_info;

	int read_sector (dword number, sector *sector)
	{
		return (drive->read_sector
		(
			sector,
			partition_info->start_sector_abs
			+ number
		));
	}
	int write_sector (dword number, sector *sector)
	{
		return (drive->write_sector
		(
			sector,
			partition_info->start_sector_abs
			+ number
		));
	}

	partition (class harddrive *drive, int number)
	{
		partition::number = number;
		partition::drive = drive;
		partition_info =
			&(drive->partition_table().partition_info[number]);

		boot_sector = new class boot_sector (this);
	}
	~partition (void) { delete boot_sector; }
};

#endif

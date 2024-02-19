/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module calculat.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/calculat.cpp 1.4 1995/01/19 00:00:49 schaefer Exp schaefer $

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

#include "hdstruct.h"
#include "fipsspec.h"

/* ----------------------------------------------------------------------- */
/* Some calculations                                                       */
/* ----------------------------------------------------------------------- */

void fips_partition_table::calculate_new_root
(
	dword new_start_cylinder,
	partition *partition,
	const drive_geometry &geometry
)
{
	for (int i = 0; i < 3; i++) if (!partition_info[i].system)
	// move DOS partitions to the beginning of the partition table
	{
		for (int j = i + 1; j < 4; j++) if
		(
			(partition_info[j].system == 1) ||
			(partition_info[j].system == 4) ||
			(partition_info[j].system == 6)
		)
		{
			struct partition_info tmppart = partition_info[i];
			partition_info[i] = partition_info[j];
			partition_info[j] = tmppart;

			if (partition->number == j) partition->number = i;
			break;
		}
	}

	int partition_no = partition->number;
	partition->partition_info = &partition_info[partition_no];

	for (i=0;i<4;i++) if (!partition_info[i].system) break;
	// search for first empty slot

	struct partition_info *newpart = &partition_info[i];
	struct partition_info *oldpart = &partition_info[partition_no];

	newpart->bootable = 0;

	newpart->start_sector_abs =
		new_start_cylinder *
		geometry.heads *
		geometry.sectors;

	newpart->no_of_sectors_abs =
		oldpart->start_sector_abs +
		oldpart->no_of_sectors_abs -
		newpart->start_sector_abs;

	if
	(
		(newpart->no_of_sectors_abs > 0xffff) ||
		(newpart->start_sector_abs > 0xffff)
	)
	{
		newpart->system = 6;
	}
	else if
	(
		newpart->no_of_sectors_abs >= 20740
	)
	{
		newpart->system = 4;
	}
	else
	{
		newpart->system = 1;
	}

	oldpart->no_of_sectors_abs =
		newpart->start_sector_abs -
		oldpart->start_sector_abs;

	if
	(
		(oldpart->no_of_sectors_abs > 0xffff) ||
		(oldpart->start_sector_abs > 0xffff)
	)
	{
		oldpart->system = 6;
	}
	else
	{
		oldpart->system = 4;
	}

	correct_physical (geometry);
}


void fips_bpb::calculate_new_boot (const partition_info &partition_info)
{
	if ((partition_info.no_of_sectors_abs > 0xffff) || (partition_info.start_sector_abs > 0xffff))
	{
		no_of_sectors = 0;
		no_of_sectors_long = partition_info.no_of_sectors_abs;
	}
	else
	{
		no_of_sectors_long = 0;
		no_of_sectors = partition_info.no_of_sectors_abs;
	}
}

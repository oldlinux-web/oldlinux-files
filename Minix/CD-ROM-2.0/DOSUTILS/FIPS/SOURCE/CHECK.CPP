/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module check.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/check.cpp 1.4 1995/01/19 00:20:41 schaefer Exp schaefer $

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

#include <stdlib.h>

#include "hdstruct.h"
#include "global.h"
#include "fipsspec.h"
#include "input.h"


/* ----------------------------------------------------------------------- */
/* Consistency check of root sector / partition table                      */
/* ----------------------------------------------------------------------- */

void fips_partition_table::correct_physical (const drive_geometry &geometry)
{
	for (int i = 0; i < 4; i++)
	{
		if (partition_info[i].system)
		{
			physical_sector_no start
			(
				partition_info[i].start_sector_abs,
				geometry
			);

			partition_info[i].start_cylinder = start.cylinder;
			partition_info[i].start_head = start.head;
			partition_info[i].start_sector = start.sector;
			// recalculate 'physical' start sector

			physical_sector_no end
			(
				partition_info[i].start_sector_abs
				+ partition_info[i].no_of_sectors_abs
				- 1,
				geometry
			);

			partition_info[i].end_cylinder = end.cylinder;
			partition_info[i].end_head = end.head;
			partition_info[i].end_sector = end.sector;
			// recalculate 'physical' end sector
		}
	}
}


void fips_harddrive::check (boolean final_check)
{
	int i,j,k;
	boolean bootable = false;
	boolean do_correct = false;

	byte *root_sector = harddrive::root_sector->data;
	partition_info *parts = partition_table().partition_info;

	int order[4] = {-1,-1,-1,-1};

	printx ("\nChecking root sector ... ");

	if ((root_sector[510] != 0x55) || (root_sector[511] != 0xaa))
		error ("Invalid root sector signature: %02X %02X", root_sector[510], root_sector[511]);

	for (i = 0; i < 4; i++)
	{
		if (parts[i].bootable == 0x80)
		{
			if (bootable)
			{
				warning (false, "More than one active partition");

				printx ("Continue (y/n)? ");
				if (ask_yes_no () == 'n') exit (-1);
			}
			else bootable = true;
		}
		else if (parts[i].bootable != 0)
		{
			warning (false, "Invalid active flag: partition %u: %02Xh",i+1,parts[i].bootable);
			// must be 0 or 80h

			printx ("Do you want to set the flag to zero (y/n)? ");
			if (ask_yes_no () == 'y') parts[i].bootable = 0;
		}

		if (parts[i].system)
		{
			if ((parts[i].start_sector == 0) || (parts[i].start_sector > geometry.sectors))
			{
				if (final_check)
					error ("Calculation error: Invalid start sector partition %u: %u", i + 1, parts[i].start_sector);

				infomsg ("Partition table inconsistency");
				do_correct = true;
			}

			if ((parts[i].end_sector == 0) || (parts[i].end_sector > geometry.sectors))
			{
				if (final_check)
					error ("Calculation error: Invalid end sector partition %u: %u", i + 1, parts[i].end_sector);

				if (!do_correct)
				{
					infomsg ("Partition table inconsistency");
					do_correct = true;
				}
			}

			if
			(
				(parts[i].start_head > (geometry.heads - 1)) ||

				(parts[i].end_head > (geometry.heads - 1)) ||

				(parts[i].start_sector_abs !=
				(parts[i].start_cylinder * geometry.heads * geometry.sectors +
				parts[i].start_head * geometry.sectors + parts[i].start_sector - 1)) ||
				// physical start sector does not match logical start sector

				((parts[i].start_sector_abs + parts[i].no_of_sectors_abs - 1) !=
				(parts[i].end_cylinder * geometry.heads * geometry.sectors +
				parts[i].end_head * geometry.sectors + parts[i].end_sector - 1))
				// physical end sector does not match logical end sector
			)
			{
				if (final_check)
					error ("Calculation error: Inconsistent table entry for partition %u", i + 1);

				if (!do_correct)
				{
					infomsg ("Partition table inconsistency");
					do_correct = true;
				}
			}

			for (j = 0; j < 4; j++)       // insert partition in ordered table
			{
				if (order[j] == -1)
				{
					order[j] = i;
					break;
				}
				else if (parts[i].start_sector_abs < parts[order[j]].start_sector_abs)
				{
					for (k=3;k>j;k--) order[k] = order[k-1];
					order[j] = i;
					break;
				}
			}
		}
		else            // system = 0
		{
			for (j = 0; j < 16; j++)
			{
				if (root_sector[0x1be + 16 * i + j] != 0)
				{
					warning (false, "Invalid partition entry: partition %u", i+1);
					printx ("Do you want to delete this entry (y/n)? ");
					if (ask_yes_no () == 'y')
					{
						parts[i].bootable = 0;
						parts[i].start_head = 0;
						parts[i].start_cylinder = 0;
						parts[i].start_sector = 0;
						parts[i].end_head = 0;
						parts[i].end_cylinder = 0;
						parts[i].end_sector = 0;
						parts[i].start_sector_abs = 0;
						parts[i].no_of_sectors_abs = 0;
					}
					break;
				}
			}
		}
	}

	if (do_correct)
	{
		pr_partition_table.correct_physical (geometry);
		printx ("\nPartition table adapted to the current drive geometry:\n\n");
		pr_partition_table.print();
	}

	if (!bootable && number == 0x80) warning (true, "No active partition");

	for (i = 0; i < 4; i++)
	{
		if ((k = order[i]) != -1)         // valid partition
		{
			if ((parts[k].end_sector != geometry.sectors) || (parts[k].end_head != (geometry.heads - 1)))
				warning (true, "Partition does not end on cylinder boundary: partition %u", k + 1);

			if (i != 0) if ((parts[k].start_sector != 1) || (parts[k].start_head != 0))
				warning (true, "Partition does not begin on cylinder boundary: partition %u", k + 1);

			if (i < 3) if ((j = order[i + 1]) != -1)       // following valid partition
			{
				if ((parts[k].start_sector_abs + parts[k].no_of_sectors_abs) > parts[j].start_sector_abs)
					error ("Overlapping partitions: %u and %u", k + 1, j + 1);

				if ((parts[k].start_sector_abs + parts[k].no_of_sectors_abs) < parts[j].start_sector_abs)
					warning (true, "Free space between partitions: %u and %u", k + 1, j + 1);
			}
		}
	}

	printx ("OK\n");
}


void fips_partition::check (void)
{
	printx ("Checking boot sector ... ");

	byte *boot_sector = partition::boot_sector->data;

	if (boot_sector[0] == 0xeb)
	{
		if (boot_sector[2] != 0x90)
			error ("Invalid jump instruction in boot sector: %02X %02X %02X", boot_sector[0], boot_sector[1], boot_sector[2]);
	}
	else if (boot_sector[0] != 0xe9)
		error ("Invalid jump instruction in boot sector: %02X %02X %02X", boot_sector[0], boot_sector[1], boot_sector[2]);

	if ((boot_sector[510] != 0x55) || (boot_sector[511] != 0xaa))
		error ("Invalid boot sector: %02X %02X", boot_sector[510], boot_sector[511]);

	if (bpb().bytes_per_sector != 512)
		error ("Can't handle number of bytes per sector: %u",bpb().bytes_per_sector);

	switch (bpb().sectors_per_cluster)
	{
		case 1:case 2:case 4:case 8:case 16:case 32:case 64:case 128: break;
		default:
			error ("Number of sectors per cluster must be a power of 2: actually it is %u",bpb().sectors_per_cluster);
	}

	if (bpb().reserved_sectors != 1)
	{
		warning (false, "Number of reserved sectors should be 1: actually it is %u",bpb().reserved_sectors);
		if (ask_correction () == 'y') bpb().reserved_sectors = 1;
	}

	if (bpb().no_of_fats != 2)
		error ("Partition must have 2 FATs: actually it has %u",bpb().no_of_fats);

	if (bpb().no_of_rootdir_entries % 16)
	{
		warning (false, "Number of root directory entries must be multiple of 16: actually it is %u",bpb().no_of_rootdir_entries);

		printx ("Do you want to set the number to the next multiple of 16 (y/n)? ");
		if (ask_yes_no () == 'y')
			bpb().no_of_rootdir_entries += (16 - bpb().no_of_rootdir_entries % 16);
	}

	if (bpb().no_of_rootdir_entries == 0)
		error ("Number of root directory entries must not be zero");

	if (bpb().media_descriptor != 0xf8)
	{
		warning (false, "Wrong media descriptor byte in boot sector: %02X",bpb().media_descriptor);
		if (ask_correction () == 'y') bpb().media_descriptor = 0xf8;
	}

	if (bpb().sectors_per_fat > 256)
	{
		warning (false, "FAT too large: %u sectors",bpb().sectors_per_fat);

		printx ("Continue (y/n)? ");
		if (ask_yes_no () == 'n') exit (-1);
	}

	if (bpb().sectors_per_fat < (info().no_of_clusters + 1) / 256 + 1)
	{
		warning (false, "FAT too small: %u sectors (should be %u)",bpb().sectors_per_fat, (unsigned int) ((info().no_of_clusters + 1) / 256 + 1));

		printx ("Continue (y/n)? ");
		if (ask_yes_no () == 'n') exit (-1);
	}

	if (bpb().sectors_per_track != drive->geometry.sectors)
	{
		warning (false, "Sectors per track incorrect: %u instead of %u",bpb().sectors_per_track,(int) drive->geometry.sectors);
		if (ask_correction () == 'y') bpb().sectors_per_track = drive->geometry.sectors;
	}

	if (bpb().drive_heads != drive->geometry.heads)
	{
		warning (false, "Number of drive heads incorrect: %u instead of %u",bpb().drive_heads,(int) drive->geometry.heads);
		if (ask_correction () == 'y') bpb().drive_heads = drive->geometry.heads;
	}

	if (bpb().hidden_sectors != partition_info->start_sector_abs)
		error ("Number of hidden sectors incorrect: %lu instead of %lu",bpb().hidden_sectors,partition_info->start_sector_abs);

	if (info().no_of_clusters <= 4084)
		error ("12-bit FAT not supported: number of clusters is %u",(int) info().no_of_clusters);

	if (bpb().no_of_sectors)
	{
		if (partition_info->no_of_sectors_abs > 0xffff)
			error ("Number of sectors (short) must be zero");

		if (bpb().no_of_sectors != partition_info->no_of_sectors_abs)
			error ("Number of sectors (short) does not match partition info:\n%u instead of %lu",bpb().no_of_sectors,partition_info->no_of_sectors_abs);

		if (partition_info->system != 4)
		{
			warning (true, "Wrong system indicator byte: %u instead of 4",partition_info->system);
			if (ask_correction () == 'y') partition_info->system = 4;
		}
	}
	else
	{
		if (bpb().no_of_sectors_long != partition_info->no_of_sectors_abs)
			error ("Number of sectors (long) does not match partition info:\n%lu instead of %lu",bpb().no_of_sectors_long,partition_info->no_of_sectors_abs);

		if (bpb().signature != 0x29)
		{
			warning (false, "Wrong signature: %02Xh",bpb().signature);
			if (ask_correction () == 'y') bpb().signature = 0x29;
		}

		if (partition_info->system != 6)
		{
			warning (true, "Wrong system indicator byte: %u instead of 6",partition_info->system);
			if (ask_correction () == 'y') partition_info->system = 6;
		}
	}

	printx ("OK\n");
}

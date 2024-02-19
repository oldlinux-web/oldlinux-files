/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module fat.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/fat.cpp 1.4 1995/01/19 00:00:51 schaefer Exp schaefer $

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

#include "logdr_st.h"
#include "global.h"
#include "fat.h"
#include "input.h"


fat::fat (class logical_drive *logical_drive,int number)
{
	fat::logical_drive = logical_drive;
	fat::number = number;
	buffer = new sector;
	start_sector = (number == 1) ? logical_drive->info().start_fat1 : logical_drive->info().start_fat2;
	sector_in_buffer = -1;
}


dword fat16::next_cluster (dword cluster_number)
{
	dword sector = cluster_number / 256;
	int offset = (cluster_number % 256) * 2;

	if (sector != sector_in_buffer)
	{
		read_sector (sector);
	}

	return ((dword) buffer->data[offset] | ((dword) buffer->data[offset + 1] << 8));
}


void fat::read_sector (dword sector)
{
	if (logical_drive->read_sector (sector + start_sector,buffer))
		if (number == 1)
			error ("Error reading FAT 1");
		else
			error ("Error reading FAT 2");

	sector_in_buffer = sector;
}


void fat16::check_against (class fat16 *fat2)
{
	printx ("Checking FAT ... ");

	for (int i=0;i<logical_drive->bpb().sectors_per_fat;i++)
	{
		read_sector (i);
		fat2->read_sector (i);

		for (int j=0;j<512;j++) if (buffer->data[j] != fat2->buffer->data[j])
			error ("FAT copies differ: FAT 1 -> %02Xh, FAT 2 -> %02Xh in sector %u, byte %u",buffer->data[j],fat2->buffer->data[j],i,j);

		if (i == 0)
		{
			if (buffer->data[0] != 0xf8)
			{
				warning (false, "Wrong media descriptor byte in FAT: %02Xh",buffer->data[0]);

				printx ("Continue (y/n)? ");
				if (ask_yes_no () == 'n') exit (-1);
			}

			if ((buffer->data[1] != 0xff) || (buffer->data[2] != 0xff) || (buffer->data[3] != 0xff))
				warning (true, "Wrong FAT entries 1 & 2: %02X %02X %02X %02X",buffer->data[0],buffer->data[1],buffer->data[2],buffer->data[3]);
		}
	}
	printx ("OK\n");
}


void fat16::check_empty (dword new_start_sector)
{
	dword first_cluster = (new_start_sector - logical_drive->info().start_data) / logical_drive->bpb().sectors_per_cluster + 2;

	dword last_cluster = logical_drive->info().no_of_clusters + 1;

	if (last_cluster > ((dword) 256 * logical_drive->bpb().sectors_per_fat - 1)) last_cluster = (dword) 256 * logical_drive->bpb().sectors_per_fat - 1;

	printx ("First Cluster: %lu\nLast Cluster: %lu\n\n",first_cluster,last_cluster);
	printx ("Testing if empty ... ");

	for (dword i=first_cluster;i <= last_cluster;i++)
	{
		dword fat_entry = next_cluster (i);

		if (fat_entry != 0) if (fat_entry != 0xfff7)
		{
			if (fat_entry == 0xffff)
			{
				error ("New partition not empty: cluster %lu ( FAT entry: <EOF> )",i);
			}
			else
			{
				error ("New partition not empty: cluster %lu ( FAT entry: %lu )",i,fat_entry);
			}
		}
	}
	
	printx ("OK\n");
}


dword fat16::min_free_cluster (void)
{
	dword first_cluster = 2;

	dword last_cluster = logical_drive->info().no_of_clusters + 1;

	if (last_cluster > ((dword) 256 * logical_drive->bpb().sectors_per_fat - 1)) last_cluster = (dword) 256 * logical_drive->bpb().sectors_per_fat - 1;

	printx ("Searching for free space ... ");

	dword i;

	for (i=last_cluster;i >= first_cluster;i--)
	{
		dword fat_entry = next_cluster (i);

		if (fat_entry != 0) if (fat_entry != 0xfff7)
		{
			i++;
			break;
		}
	}
	printx ("OK\n\n");
	return (i);
}

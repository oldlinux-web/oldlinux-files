/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module input.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/input.cpp 1.4 1995/01/19 00:00:54 schaefer Exp schaefer $

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

#include <ctype.h>
#include <stdlib.h>
#include "types.h"
#include "disk_io.h"
#include "global.h"
#include "input.h"

/* ----------------------------------------------------------------------- */
/* User Input                                                              */
/* ----------------------------------------------------------------------- */

static void wait_for_key (void)
{
	printx ("\nPress any Key\n");
	getx();
}


int ask_for_drive_number (void)

/*
 * Find Disk Drives - if more than one, ask for drive number. If no drive
 * was found, issue warning, try drive 0x80 anyway
 */

{
	int drives_found = 0;
	int drive_table[] = {0,0,0,0,0,0,0,0,0};

	int no_of_drives = get_no_of_drives ();

	for (int i=0x80; i < 0x80 + no_of_drives; i++)
	{
		if (get_disk_type (i) == 3)
		{
			drive_table[drives_found++] = i;
			if (drives_found == 9)
				break;
		}
	}

	if (drives_found == 0)
	{
		warning (false, "No compatible hard disk found");
		ask_if_continue ();

		return (0x80);
	}

	if (drives_found == 1)
		return (drive_table[0]);

	printx ("Which Drive (");

	for (i=0; i<drives_found; i++)
		printx ("%u=0x%02X/", i+1, drive_table[i]);
	printx ("\b)? ");

	while (true)
	{
		i = getx ();

		if (i >= '1' && i <= '9')
			if (drive_table[i - '1'] != 0) break;
	}

	printx ("%c\n",i);
	return (drive_table[i - '1']);
}


int ask_for_partition_number (partition_info parts[])
{
	int number_of_partitions = (parts[0].system != 0) + (parts[1].system != 0) +
		 (parts[2].system != 0) + (parts[3].system != 0);

	if (number_of_partitions == 0)
		error ("No valid partition found");

	if (number_of_partitions == 4)
		error ("No free partition");

	if (number_of_partitions == 1)
	{
		wait_for_key();
		for (int i = 0; i < 4; i++) if (parts[i].system) return i;
	}

	printx ("\nWhich Partition do you want to split (");

	for (int i = 0; i < 4; i++) if (parts[i].system) printx ("%u/", i + 1);
	printx ("\b)? ");

	while (true)
	{
		i = getx ();
		if (isdigit (i)) if (('0' < i) && (i <= '4')) if (parts[i - '1'].system) break;
	}
	printx ("%c\n", i);
	return (i - '1');
}


dword ask_for_new_start_cylinder (int start_cylinder, int min_cylinder, int max_cylinder, int sectors_per_cylinder)
{
	int akt_cylinder = min_cylinder;

	printx ("\nEnter start cylinder for new partition (%u - %u):\n\n",min_cylinder,max_cylinder);
	printx ("Use the cursor keys to choose the cylinder, <enter> to continue\n\n");
	printx ("Old partition      Cylinder       New Partition\n");

	while (true)
	{
		double oldsize = (akt_cylinder - start_cylinder) * (double) sectors_per_cylinder / 2048;
		double newsize = (max_cylinder - akt_cylinder + 1) * (double) sectors_per_cylinder / 2048;

		printf (" %6.1f MB          %4u           %6.1f MB\r", oldsize, akt_cylinder, newsize);

		int input = getx ();
		if (input == '\r')
		{
			printx (" %6.1f MB          %4u           %6.1f MB\n\n", oldsize, akt_cylinder, newsize);
			return (akt_cylinder);
		}
		else if (input != 0) continue;

		input = getx ();
		switch (input)
		{
			case 75:
				if (akt_cylinder > min_cylinder) akt_cylinder--;
				break;
			case 77:
				if (akt_cylinder < max_cylinder) akt_cylinder++;
				break;
			case 72:
				if (akt_cylinder - 10 >= min_cylinder) akt_cylinder -= 10;
				break;
			case 80:
				if (akt_cylinder + 10 <= max_cylinder) akt_cylinder += 10;
				break;
		}
	}
}

char ask_yes_no (void)
{
	int character;
	do character = getx(); while ((character != 'y') && (character != 'n'));
	printx ("%c\n",character);
	return (character);
}

char ask_correction (void)
{
	printx ("Do you want to correct this (y/n) ");
	return (ask_yes_no ());
}


void ask_for_write_permission (void)
{
	printx ("\nReady to write new partition scheme to disk\n");
	printx ("Do you want to proceed (y/n)? ");

	if (ask_yes_no () == 'n') exit (0);
}

boolean ask_if_continue (void)
{
	printx ("\nDo you want to continue or reedit the partition table (c/r)? ");

	int character;
	do character = getx(); while ((character != 'c') && (character != 'r'));
	printx ("%c\n",character);
	if (character == 'r') return (false);
	return (true);
}

boolean ask_if_save (void)
{
	int character;

	printx ("Do you want to make a backup copy of your root and boot sector before\nproceeding (y/n)? ");
	if (ask_yes_no () == 'n') return (false);

	printx ("Do you have a bootable floppy disk in drive A: as described in the\ndocumentation (y/n)? ");
	if (ask_yes_no () == 'n')
	{
		printx ("Please read the file FIPS.DOC!\n");
		exit (0);
	}

	return (true);
}

void ask_if_proceed (void)
{
	printx ("Do you want to proceed (y/n)? ");

	if (ask_yes_no () == 'n') exit (0);
}

/*
	FIPS - the First nondestructive Interactive Partition Splitting program

	Module main.cpp

	RCS - Header:
	$Header: c:/daten/fips/source/main/RCS/main.cpp 1.4 1995/01/19 00:00:55 schaefer Exp schaefer $

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
#include "input.h"
#include "fat.h"
#include "fipsspec.h"
#include "host_os.h"


#define FIRST_CHECK false
#define FINAL_CHECK true


extern unsigned _stklen = 20000U;


int main (int argc, char *argv[])
{
	// *********************************************************
	// Initialize Program
	// *********************************************************

	evaluate_argument_vector (argc, argv);

	atexit (exit_function);

	if (global.debug_mode)
		global.open_debugfile (argc,argv);

	notice ();

	host_os os;
	char infostring[256];

	if (os.ok () != OK)
	{
		printx ("\nWARNING: FIPS has detected that it is running under %s\n"
			"FIPS should not be used under a multitasking OS. If possible, boot from a DOS\n"
			"disk and then run FIPS. Read FIPS.DOC for more information.\n\n",
			os.information (infostring));

		ask_if_proceed ();
	}


	// *********************************************************
	// Select Drive
	// *********************************************************

	int drive_number;

	if (global.drive_number_cmdline != 0)
		drive_number = global.drive_number_cmdline;
	else
		drive_number = ask_for_drive_number ();

	fips_harddrive harddrive (drive_number);	// reads geometry

	if (harddrive.errorcode)
		error
		(
			"Error reading drive geometry: Errorcode %u",
			harddrive.errorcode
		);

	harddrive.reset ();

	if (harddrive.errorcode)
	{
		warning
		(
			false,
			"Drive initialization failure: Errorcode %u",
			harddrive.errorcode
		);

		ask_if_proceed ();
	}


	// *********************************************************
	// Select partition
	// *********************************************************

	if (harddrive.read_root_sector () != 0)
		error ("Error reading root sector");

	if (global.debug_mode)
	{
		fprintf
		(
			global.debugfile,
			"\nRoot sector drive %02Xh:\n\n",
			drive_number
		);

		hexwrite (harddrive.root_sector->data, 512, global.debugfile);
	}

	while (true)
	{
		fips_harddrive hd = harddrive;

		hd.get_partition_table();

		printx ("\nPartition table:\n\n");
		hd.print_partition_table ();

		hd.check (FIRST_CHECK);

		int partition_number =
			ask_for_partition_number
			(
				hd.partition_table().partition_info
			);

		int system = hd.partition_table()
			.partition_info[partition_number].system;

		switch (system)
		{
			case 5:
				error ("Can't split extended partitions");
				break;
			case 1: case 4: case 6:
			{
				fips_partition* partition =
					new fips_partition (&hd, partition_number);

				if (partition->split (hd) == true)
					return (0);

				delete partition;
			}
				break;
			default:
				error ("Unknown file system: %02Xh", system);
				break;
		}
	}
}
